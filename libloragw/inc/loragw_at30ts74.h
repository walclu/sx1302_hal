/*

Description:
    Basic driver for the AT30TS74 temperature sensor

License: Revised BSD License, see LICENSE.TXT file included in the project
*/

#ifndef _LORAGW_AT30TS74_H
#define _LORAGW_AT30TS74_H

/* -------------------------------------------------------------------------- */
/* --- DEPENDENCIES --------------------------------------------------------- */

#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */

#include "config.h"     /* library configuration options (dynamically generated) */

/* -------------------------------------------------------------------------- */
/* --- INTERNAL SHARED TYPES ------------------------------------------------ */

/* -------------------------------------------------------------------------- */
/* --- INTERNAL SHARED FUNCTIONS -------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PUBLIC CONSTANTS ----------------------------------------------------- */

/*
  0x4a: AT30TS74-U1FMCB-T
  */
static const uint8_t I2C_PORT_TEMP_SENSOR_AT30TS74[] = {0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f};

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */

/**
@brief Configure the temperature sensor (AT30TS74)
@param i2c_fd file descriptor to access the sensor through I2C
@param i2c_addr the I2C device address of the sensor
@return LGW_I2C_ERROR if fails, LGW_I2C_SUCCESS otherwise
*/
int at30ts74_configure(int i2c_fd, uint8_t i2c_addr);

/**
@brief Get the temperature from the sensor
@param i2c_fd file descriptor to access the sensor through I2C
@param i2c_addr the I2C device address of the sensor
@param temperature pointer to store the temerature read from sensor
@return LGW_I2C_ERROR if fails, LGW_I2C_SUCCESS otherwise
*/
int at30ts74_get_temperature(int i2c_fd, uint8_t i2c_addr, float * temperature);

#endif

/* --- EOF ------------------------------------------------------------------ */
