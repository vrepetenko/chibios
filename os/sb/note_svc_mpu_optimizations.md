# Note: SVC / MPU Context Switch Optimizations (ALT ports)

Analysis of the shared SVC path used by both RT context switching and the
SB syscall mechanism in the ALT ports (`os/common/ports/ARMv7-M-ALT`,
applies conceptually to `ARMv8-M-ML-ALT` too). Findings ordered by
expected payoff. Date: 2026-06-11.

## Background

`SVC_Handler` in `ARMv7-M-ALT/compilers/GCC/chcoreasm.S` serves three roles:

1. Synchronous context switch: `svc #0` from privileged mode, issued by
   `__port_switch()` with ntp/otp in R0/R1.
2. Syscall return: `svc 1` from privileged mode at the end of
   `__sb_dispatch_syscall`.
3. Syscall/fastcall entry from unprivileged (sandbox) mode.

Discrimination is done first on CONTROL.nPRIV, then by **reading the SVC
immediate from code memory** (`ldr` of the stacked PC, then
`ldrb [pc, #-2]`).

## 1. Split the SVC roles: dedicated context-switch IRQ (preferred design)

Design decision (2026-06-11): stop sharing SVC between switch and
syscalls. The syscall entry cannot move — SVC is the only
architecturally-defined trap reachable from unprivileged thread mode —
and the switch must remain exception-based (hardware extctx stacking,
FPU lazy stacking and CONTROL handling are deliberate ALT features a
function-call switch would regress). So the context switch is the
movable side.

**No other system exception works** (survey, so the reasoning is not
lost):

- *PendSV*: lowest priority by design, masked by BASEPRI inside the lock
  zone, so a pend does not fire until unlock — that is the deferred-switch
  scheduler model (FreeRTOS-style), a scheduler rework, not a port change.
- *DebugMonitor* (DEMCR.MON_PEND): disabled as soon as a halting debugger
  attaches (C_DEBUGEN) — the OS would stop switching under a probe.
- *NMI* (ICSR.NMIPENDSET): shared with real sources on real silicon
  (e.g. STM32 clock-security), and priority -2 blocks the fast non-kernel
  band that ChibiOS promises never to mask.
- *SysTick / fault soft-pends* (PENDSTSET, SHCSR pending bits): shared
  with their real users, which reintroduces the discrimination being
  removed.

A **software-pended, otherwise-unused NVIC IRQ** is therefore the only
clean mechanism, not a hack of last resort.

**Trigger sequence** — no DSB/ISB, no literal pool:

```
    movw/movt  rX, #:lower16/upper16:0xE000EF00   ; NVIC STIR, 2 cycles
    movs       rY, #PORT_SWITCH_IRQ
    str        rY, [rX]
1:  b.n        1b              ; narrow encoding mandatory: the handler
                               ; skips exactly 2 bytes on exit
```

The handler adds +2 to the stacked PC before saving the context.
Correctness invariant: the IRQ can only pend after the `str`, and the
instruction following the `str` *is* the loop, so the stacked PC is the
loop address in every case — write-buffer drift, late recognition, or a
fast ISR preempting the spin (it tail-chains into the switch IRQ, same
frame). The +2 is unconditionally correct because `__port_switch` is the
single trigger site in the system. The spin replaces both barriers with
something strictly weaker and cheaper: it waits only for this one write
to reach the NVIC (a DSB drains the *entire* write buffer) and overlaps
the recognition latency. Failure-mode bonus: a misconfigured IRQ
(priority, vector) is a visible hang at a known address instead of
execution silently continuing past barriers into scheduler state that
assumes the switch happened.

**Cycle accounting:**

- Switch path: trigger ~4-6 cycles with zero variance, plus a ~4-cycle
  stacked-PC fixup in the handler (SRAM), versus today's `svc` +
  privileged discrimination (mrs/tst/beq + stacked-PC load + **flash
  `ldrb`** + cbz, ~10-20+ with wait-state variance). Parity with the
  shared-SVC fallback below on average, strictly better worst case: no
  code-memory data read, no DSB variance anywhere on the path.
- **Every syscall round trip wins regardless**: the `svc 1` return
  currently pays the full discrimination (stacked-PC load + flash `ldrb`)
  only to prove it is not a context switch. With the switch gone,
  privileged SVC has exactly one meaning and the return becomes
  straight-line restore code: ~8-12 cycles off every syscall, kernel-side
  only, no guest ABI change. Note: fastcalls do *not* execute `svc 1`
  (handled and returned entirely in handler mode), so they gain nothing
  from the split alone — their entry-side gain comes from point 2.
- Composition with point 2: the split fixes the privileged side (switch
  trigger + syscall return), point 2 fixes the unprivileged side
  (syscall + fastcall entry). Together the SVC/switch machinery contains
  **no data reads from code memory at all**; every residual access is
  SRAM or NVIC, with flat worst-case timing.

**Security:** the switch handler becomes unreachable from the sandbox by
construction — NVIC/STIR live in the SCS, privileged-only regardless of
MPU, provided CCR.USERSETMPEND stays 0 (worth a debug assertion).
SVC_Handler shrinks to guest-facing code only, with the existing
CONTROL.nPRIV test fully discriminating its two remaining clients
(unprivileged entry, privileged return) — no immediate fetch anywhere.

**Portability / configuration:**

- Opt-in port option (`PORT_USE_SWITCH_IRQ` + IRQ number + a vector alias
  such as `.set VectorXX, __port_switch_handler` — vectors are weak
  symbols, no device-header changes). Default off, falling back to the
  shared-SVC scheme below; nothing regresses for platforms that do not
  opt in.
- The "this IRQ line is unused" guarantee is a per-board contract the OS
  cannot verify. Acceptable: SB-class applications run on curated larger
  devices (M4/M7/M33, 100+ NVIC lines) where reserved/unconnected lines
  exist.
- The chosen IRQ's priority must sit above the BASEPRI kernel mask (the
  same rule SVC obeys today); it is only ever pended inside the lock
  zone, so no new nesting cases.
- SMP: STIR pends on the executing core's local NVIC — the wanted
  behavior, but must be stated.
- Debugger note for the port docs: single-stepping with interrupts masked
  (C_MASKINTS) sits in the spin until interrupts are released — the usual
  pend-and-wait annoyance.

### Fallback: shared SVC with a register convention

For platforms without a spare IRQ line. The `ldrb` of the immediate is a
data read from code memory (flash wait states, on M7 typically missing
the ART/cache) and is paid on **every** `port_switch`, i.e. the hottest
RT path, only to distinguish it from the much rarer syscall return.

Both privileged callers are kernel-controlled, so a register convention
can replace the immediate: ntp (R0) is never NULL for a real switch, so
the dispatcher's return trap can issue `movs r0, #0; svc 0` and the
handler discriminates with a single `cbz r0` after the `ldmia r2, {r0, r1}`
it already executes. Removes the stacked-PC load and the flash byte read
from both privileged paths; also removes the overhead that
`PORT_USE_SYSCALL` currently imposes on non-SB context switches. The
split above subsumes this entirely (with one privileged client left,
there is nothing to discriminate), so this applies to the fallback
configuration only.

## 2. Pass the syscall number in a register instead of the SVC immediate

The unprivileged entry path performs the same stacked-PC + `ldrb` dance,
but reading from the *sandbox's* code memory, which may be external/QSPI
flash (tens of cycles for a random data read). Changing the user-side
stubs in `user/sbuser.h` to "n in R12, always `svc #0`" lets the handler
read the number from the hardware-stacked frame in SRAM
(`ldr r0, [r1, #16]`). Side benefit: no privileged load at an address
derived from guest state. Complementary to point 1: the split fixes the
privileged side, this fixes the unprivileged side (and is the only one
of the two that benefits fastcalls).

Caveat: this is a guest-visible ABI break, sandbox binaries must be
rebuilt. Needs a config option or an SB ABI version bump.

Required hardening (recorded in the security note, point 3): the number
becomes a 32-bit guest-controlled value instead of a one-byte immediate,
so the handler must clamp it (`uxtb`, ~1 cycle) before any table
indexing — without the clamp, the dispatch's OOB-impossible-by-
construction property is lost and it becomes a guest-indexed `blx`.

## 3. MPU regions as a shared table pointer (preferred design)

Design decision (2026-06-11), superseding the earlier "drop store-back +
burst load" tweaks by making them correct by construction.

`port_context` no longer embeds `port_mpureg_t regions[N]`; it holds a
single pointer:

- **Default threads** point at one shared `const` default table in flash:
  every entry pre-baked as `rbar = VALID | region#`, `rasr = 0` (region
  disabled), so loading it programs the switched regions off and the
  static MPU configuration (`PORT_MPU_RBARx_INIT` + PRIVDEFENA background)
  applies. `__PORT_SETUP_CONTEXT_MPU` collapses to one pointer assignment.
  No NULL special case exists.
- **Sandbox threads** point at a table owned by `sb_class_t`, shared by
  all threads of that SB (today each thread carries its own identical
  copy, filled at spawn in `sbhost.c`).

Switch behavior:

- **Switch-out: nothing.** `PORT_STORE_MPU_CONTEXT` is removed entirely.
  The tables are the master copy; the MPU is never read back. This drops
  ~20+ cycles of MMIO from *every* switch, including pure-RT workloads
  that pay it today for nothing.
- **Switch-in: pointer equality only, never contents.**
  Compare `otp->ctx.regions` with `ntp->ctx.regions`: equal -> skip the
  MPU entirely; different -> one `ldmia` of the table + one `stm` burst to
  the ARMv7-M RBAR/RASR alias registers (0xE000ED9C-0xE000EDB8) — the
  pre-baked `VALID | region#` makes RNR writes unnecessary. ~16 cycles
  for 4 regions versus ~24 MMIO accesses today, and the dominant
  transitions become free: kernel <-> kernel (same default table) and
  SB-sibling <-> SB-sibling (same SB table). The MPU is reprogrammed only
  on actual protection-domain crossings. No DSB/ISB needed, exception
  return serializes.

Why the default table instead of a NULL-means-skip convention: an
SB -> kernel switch compares different and loads the defaults, so **no
privileged thread ever runs with a dead sandbox's regions still
programmed**. The "switched regions may only grant unprivileged access,
never restrict privileged access" rule drops from correctness requirement
to defense-in-depth (still worth asserting in `get_mpu_settings()`). It
also makes table-pointer reuse after SB destruction harmless: the last
switch away from the dying SB already wiped the MPU to defaults.

Rules the design imposes:

- **Equality-only means mutation is invisible to the switch code.** If a
  writer (the shared-memory API, below) changes an entry of the currently
  loaded table, no reload ever happens on its own. Consistency is entirely
  the writer's job: mutate the table *and* write the live RBAR/RASR iff
  the current thread points at that table, in one critical section.
- **Lifetime**: a table must outlive every thread pointing at it. Trivial
  for the default table (linkage) and for SB tables (`sb_class_t` owns its
  threads). Must be stated in the port API contract since `port_context`
  no longer self-contains protection state.
- Per-thread divergence is not lost: a thread needing unique regions
  points at a private one-off table.

Side benefits: 32 bytes saved per thread (4 regions x 8 bytes) on
`thread_t`; thread "protection classes" fall out of table identity (a
product can define additional shared tables, e.g. for unprivileged
non-SB threads); the four-way `__PORT_SETUP_CONTEXT_MPU` `#elif` ladder
disappears.

Cost: one extra `ldr` (pointer indirection) on switch-in, ~2 cycles,
repaid by everything above. The only transition paying more than the
NULL design would is SB -> kernel (one default-table load), which is
exactly where deterministic state is wanted.

## 4. VRQ pending check on syscall return (minor)

`__sb_vrq_check_pending()` (`host/sbvrq.c`) loads `vrq.isr`, `vrq.wtmask`,
`vrq.enmask` and masks them on every syscall return. The mutation sites
are few and slow-path; a single precomputed "deliverable" word maintained
at post/enable/mask time reduces the hot path to one load + branch.

## Deliberately left alone

- Fastcall path: already minimal (handler mode, one table lookup).
- Two-exception syscall design (entry SVC -> privileged dispatcher ->
  exit SVC): ~4 hardware mode transitions of floor cost, but it is what
  makes syscalls preemptable/blockable. A thread-mode return (manual
  unstack + privilege drop) could shave ~15-20 cycles but interacts badly
  with FPCA/lazy stacking and nPRIV ordering. Poor risk/reward.
- `__sb_dispatch_syscall` table dispatch: already minimal.

## Shared-memory API interaction (planned)

Security note: a shared-memory region is the feature that activates the
TOCTOU / double-fetch risk described in
[note_sb_isolation_security.md](note_sb_isolation_security.md) point 1.
The API contract must require copy-in-once for any buffer backed by shared
memory (validate and use a private privileged copy, never re-dereference
guest memory after the check).

Under the region-table design of point 3 the consistency problem shrinks:
there are no per-thread copies to walk. Adding/removing a shared-memory
region is:

- update the entry in the SB's table (the single master copy), and
- iff the currently running thread's table pointer equals that table,
  also write the live RBAR/RASR — the switch code compares pointers only
  and will never pick up the change by itself.

Both steps in one critical section, one helper. Note the **revocation**
direction is the security-sensitive one: unmapping a shared region from a
currently-running SB must write the live registers in the same critical
section, otherwise the sandbox retains access until the next
domain-crossing switch.

Constraints:

- `SB_CFG_NUM_REGIONS` and `PORT_SWITCHED_REGIONS_NUMBER` are both capped
  at 4, matching the 4 RBAR/RASR alias slots. A shared region taken out of
  the existing 1..4 budget composes cleanly; a dedicated 5th region needs
  the caps raised and the burst load becomes stm-of-4 plus one RNR-based
  write.
- The same RBAR/RASR encoding helper (`get_mpu_settings()`) must be used
  by the new API, especially once VALID|region# is pre-OR'ed into stored
  RBAR values.
- PMSAv7 alignment: power-of-2 size, size-aligned base; constrain the API
  or use subregion-disable bits. Design against v7-M as the lowest common
  denominator (v8-M-ML gets base/limit semantics for free).
- SMP: a target running on another core cannot have its MPU written
  remotely; would need a forced reschedule/IPI on that core or an API
  restriction to self/sleeping targets.
