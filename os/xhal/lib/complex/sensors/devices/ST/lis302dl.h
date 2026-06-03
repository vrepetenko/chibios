/*
    ChibiOS - Copyright (C) 2006-2026 Giovanni Di Sirio.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file        lis302dl.h
 * @brief       Generated LIS302DL Driver header.
 * @note        This is a generated file, do not edit directly.
 *
 * @addtogroup  LIS302DL
 * @brief       LIS302DL accelerometer chip driver.
 * @{
 */

#ifndef LIS302DL_H
#define LIS302DL_H

#include "hal.h"
#include "hal_accelerometer.h"

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name    Version identification
 * @{
 */
/**
 * @brief       LIS302DL driver version string.
 */
#define LIS302DL_VERSION                    "2.0.0"

/**
 * @brief       LIS302DL driver version major number.
 */
#define LIS302DL_MAJOR                      2

/**
 * @brief       LIS302DL driver version minor number.
 */
#define LIS302DL_MINOR                      0

/**
 * @brief       LIS302DL driver version patch number.
 */
#define LIS302DL_PATCH                      0
/** @} */

/**
 * @name    LIS302DL accelerometer characteristics
 * @{
 */
#define LIS302DL_ACC_NUMBER_OF_AXES         3U
#define LIS302DL_ACC_2G                     2.0f
#define LIS302DL_ACC_8G                     8.0f
#define LIS302DL_ACC_SENS_2G                18.0f
#define LIS302DL_ACC_SENS_8G                72.0f
#define LIS302DL_ACC_BIAS                   0.0f
/** @} */

/**
 * @name    LIS302DL SPI command bits
 * @{
 */
#define LIS302DL_MS                         (1U << 6)
#define LIS302DL_RW                         (1U << 7)
/** @} */

/**
 * @name    LIS302DL register addresses
 * @{
 */
#define LIS302DL_AD_WHO_AM_I                0x0FU
#define LIS302DL_AD_CTRL_REG1               0x20U
#define LIS302DL_AD_CTRL_REG2               0x21U
#define LIS302DL_AD_CTRL_REG3               0x22U
#define LIS302DL_AD_STATUS_REG              0x27U
#define LIS302DL_AD_OUT_X                   0x29U
#define LIS302DL_AD_OUT_Y                   0x2BU
#define LIS302DL_AD_OUT_Z                   0x2DU
/** @} */

/**
 * @name    LIS302DL CTRL_REG1 register bits
 * @{
 */
#define LIS302DL_CTRL_REG1_XEN              (1U << 0)
#define LIS302DL_CTRL_REG1_YEN              (1U << 1)
#define LIS302DL_CTRL_REG1_ZEN              (1U << 2)
#define LIS302DL_CTRL_REG1_FS_MASK          0x20U
#define LIS302DL_CTRL_REG1_PD               (1U << 6)
/** @} */

/**
 * @name    LIS302DL CTRL_REG2 register bits
 * @{
 */
#define LIS302DL_CTRL_REG2_HPCF1            (1U << 0)
#define LIS302DL_CTRL_REG2_HPCF2            (1U << 1)
#define LIS302DL_CTRL_REG2_HPFFWU1          (1U << 2)
#define LIS302DL_CTRL_REG2_HPFFWU2          (1U << 3)
#define LIS302DL_CTRL_REG2_FDS              (1U << 4)
/** @} */

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief       Enables SPI bus locking around LIS302DL transactions.
 */
#if !defined(LIS302DL_SHARED_SPI) || defined(__DOXYGEN__)
#define LIS302DL_SHARED_SPI                 FALSE
#endif

/**
 * @brief       Enables advanced LIS302DL configuration fields.
 */
#if !defined(LIS302DL_USE_ADVANCED) || defined(__DOXYGEN__)
#define LIS302DL_USE_ADVANCED               FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/* Checks on LIS302DL_SHARED_SPI configuration.*/
#if (LIS302DL_SHARED_SPI != FALSE) && (LIS302DL_SHARED_SPI != TRUE)
#error "invalid LIS302DL_SHARED_SPI value"
#endif

/* Checks on LIS302DL_USE_ADVANCED configuration.*/
#if (LIS302DL_USE_ADVANCED != FALSE) && (LIS302DL_USE_ADVANCED != TRUE)
#error "invalid LIS302DL_USE_ADVANCED value"
#endif

#if !defined(__CHIBIOS_XHAL_CONF__)
#error "LIS302DL chip driver requires XHAL"
#endif

#if HAL_USE_SPI != TRUE
#error "LIS302DL requires HAL_USE_SPI"
#endif

#if SPI_USE_SYNCHRONIZATION != TRUE
#error "LIS302DL requires SPI_USE_SYNCHRONIZATION"
#endif

#if (LIS302DL_SHARED_SPI == TRUE) && (HAL_USE_MUTUAL_EXCLUSION != TRUE)
#error "LIS302DL_SHARED_SPI requires HAL_USE_MUTUAL_EXCLUSION"
#endif

/**
 * @name    LIS302DL accelerometer method aliases
 * @{
 */
#define lis302dlAccelerometerGetAxesNumber(devp)halAccelerometerGetAxesNumber(&((devp)->accelerometer))
#define lis302dlAccelerometerReadRaw(devp, axes)halAccelerometerReadRaw(&((devp)->accelerometer), axes)
#define lis302dlAccelerometerReadCooked(devp, axes)halAccelerometerReadCooked(&((devp)->accelerometer), axes)
#define lis302dlAccelerometerSetBias(devp, biases)halAccelerometerSetBias(&((devp)->accelerometer), biases)
#define lis302dlAccelerometerResetBias(devp)halAccelerometerResetBias(&((devp)->accelerometer))
#define lis302dlAccelerometerSetSensitivity(devp, sensitivities)halAccelerometerSetSensitivity(&((devp)->accelerometer), sensitivities)
#define lis302dlAccelerometerResetSensitivity(devp)halAccelerometerResetSensitivity(&((devp)->accelerometer))
#define lis302dlAccelerometerSetFullScale(devp, fs)lis302dlSetFullScale(devp, fs)
/** @} */

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

typedef struct lis302dl_config lis302dl_config_t;

typedef enum {
  LIS302DL_ACC_FS_2G = 0x00U,
  LIS302DL_ACC_FS_8G = 0x20U
} lis302dl_acc_fs_t;

typedef enum {
  LIS302DL_ACC_ODR_100HZ = 0x00U,
  LIS302DL_ACC_ODR_400HZ = 0x80U
} lis302dl_acc_odr_t;

typedef enum {
  LIS302DL_ACC_HP_DISABLED = 0x00U,
  LIS302DL_ACC_HP_0        = 0x10U,
  LIS302DL_ACC_HP_1        = 0x11U,
  LIS302DL_ACC_HP_2        = 0x12U,
  LIS302DL_ACC_HP_3        = 0x13U
} lis302dl_acc_hp_t;

struct lis302dl_config {
  hal_spi_driver_c          *spip;
  const hal_spi_config_t    *spicfg;
  const float               *accsensitivity;
  const float               *accbias;
  lis302dl_acc_fs_t         accfullscale;
  lis302dl_acc_odr_t        accodr;
#if LIS302DL_USE_ADVANCED == TRUE
  lis302dl_acc_hp_t         acchighpass;
#endif
};

/**
 * @class       lis302dl_driver_c
 * @extends     hal_base_driver_c
 * @implements  hal_accelerometer_i
 *
 * @brief       LIS302DL accelerometer chip driver class.
 *
 * @name        Class @p lis302dl_driver_c structures
 * @{
 */

/**
 * @brief       Type of a LIS302DL driver class.
 */
typedef struct lis302dl_driver lis302dl_driver_c;

/**
 * @brief       Class @p lis302dl_driver_c virtual methods table.
 */
struct lis302dl_driver_vmt {
  /* From base_object_c.*/
  void (*dispose)(void *ip);
  /* From hal_base_driver_c.*/
  msg_t (*start)(void *ip, const void *config);
  void (*stop)(void *ip);
  const void * (*setcfg)(void *ip, const void *config);
  const void * (*selcfg)(void *ip, unsigned cfgnum);
  /* From lis302dl_driver_c.*/
};

/**
 * @brief       Structure representing a LIS302DL driver class.
 */
struct lis302dl_driver {
  /**
   * @brief       Virtual Methods Table.
   */
  const struct lis302dl_driver_vmt *vmt;
  /**
   * @brief       Driver state.
   */
  driver_state_t            state;
  /**
   * @brief       Associated configuration structure.
   */
  const void                *config;
  /**
   * @brief       Driver argument.
   */
  void                      *arg;
#if (HAL_USE_MUTUAL_EXCLUSION == TRUE) || defined (__DOXYGEN__)
  /**
   * @brief       Driver mutex.
   */
  mutex_t                   mutex;
#endif /* HAL_USE_MUTUAL_EXCLUSION == TRUE */
#if (HAL_USE_REGISTRY == TRUE) || defined (__DOXYGEN__)
  /**
   * @brief       Driver identifier.
   */
  unsigned int              id;
  /**
   * @brief       Driver name.
   */
  const char                *name;
  /**
   * @brief       Registry link structure.
   */
  hal_regent_t              regent;
#endif /* HAL_USE_REGISTRY == TRUE */
  /**
   * @brief       Implemented interface @p hal_accelerometer_i.
   */
  hal_accelerometer_i       accelerometer;
  /**
   * @brief       Accelerometer subsystem axes number.
   */
  size_t                    accaxes;
  /**
   * @brief       Current accelerometer sensitivity.
   */
  float                     accsensitivity[LIS302DL_ACC_NUMBER_OF_AXES];
  /**
   * @brief       Current accelerometer bias.
   */
  float                     accbias[LIS302DL_ACC_NUMBER_OF_AXES];
  /**
   * @brief       Current accelerometer full scale value.
   */
  float                     accfullscale;
};
/** @} */

/**
 * @brief       LIS302DL driver compatibility type.
 */
typedef lis302dl_driver_c LIS302DLDriver;

/**
 * @brief       LIS302DL configuration compatibility type.
 */
typedef lis302dl_config_t LIS302DLConfig;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  /* Methods of lis302dl_driver_c.*/
  void *__lis302dl_objinit_impl(void *ip, const void *vmt);
  void __lis302dl_dispose_impl(void *ip);
  msg_t __lis302dl_start_impl(void *ip, const void *config);
  void __lis302dl_stop_impl(void *ip);
  const void *__lis302dl_setcfg_impl(void *ip, const void *config);
  const void *__lis302dl_selcfg_impl(void *ip, unsigned cfgnum);
  msg_t lis302dlSetFullScale(void *ip, lis302dl_acc_fs_t fs);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/**
 * @name        Default constructor of lis302dl_driver_c
 * @{
 */
/**
 * @brief       Default initialization function of @p lis302dl_driver_c.
 *
 * @param[out]    self          Pointer to a @p lis302dl_driver_c instance to
 *                              be initialized.
 * @return                      Pointer to the initialized object.
 *
 * @objinit
 */
CC_FORCE_INLINE
static inline lis302dl_driver_c *lis302dlObjectInit(lis302dl_driver_c *self) {
  extern const struct lis302dl_driver_vmt __lis302dl_driver_vmt;

  return __lis302dl_objinit_impl(self, &__lis302dl_driver_vmt);
}
/** @} */

#endif /* LIS302DL_H */

/** @} */
