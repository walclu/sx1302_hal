/*

Description:
    Basic driver for the AT30TS74 temperature sensor

License: Revised BSD License, see LICENSE.TXT file included in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDENCIES --------------------------------------------------------- */

#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */
#include <stdio.h>      /* printf fprintf */

#include "loragw_i2c.h"
#include "loragw_at30ts74.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#if DEBUG_I2C == 1
    #define DEBUG_MSG(str)              fprintf(stdout, str)
    #define DEBUG_PRINTF(fmt, args...)  fprintf(stdout,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)               if(a==NULL){fprintf(stderr,"%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_REG_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)               if(a==NULL){return LGW_REG_ERROR;}
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define AT30TS74_REG_TEMP        0x00
#define AT30TS74_REG_CONF        0x01

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int at30ts74_configure(int i2c_fd, uint8_t i2c_addr) {
    int err;
    uint8_t b = AT30TS74_REG_TEMP;
    uint8_t tmp[2];

    /* Check Input Params */
    if (i2c_fd <= 0) {
        printf("ERROR: invalid I2C file descriptor\n");
        return LGW_I2C_ERROR;
    }

    DEBUG_PRINTF("INFO: configuring AT30TS74 temperature sensor on 0x%02X...\n", i2c_addr);

    /* read from non-zero T_high register to check if chip is present */
    err = i2c_linuxdev_read_buffer( i2c_fd, i2c_addr, tmp, 2);
    if (err != 0) {
        DEBUG_PRINTF("ERROR: failed to read I2C device 0x%02X (err=%i)\n", i2c_addr, err);
        return LGW_I2C_ERROR;
    }

    /* Set conversion resolution to 12 bits */
    err = i2c_linuxdev_write( i2c_fd, i2c_addr, AT30TS74_REG_CONF, 0x60);
    if ( err != 0 )
    {
        printf( "ERROR: failed to write to I2C device 0x%02X (err=%i)\n", i2c_addr, err );
        return LGW_I2C_ERROR;
    }

    err = i2c_linuxdev_write_buffer( i2c_fd, i2c_addr, &b, 1);
    if ( err != 0 )
    {
        printf( "ERROR: failed to write to I2C device 0x%02X (err=%i)\n", i2c_addr, err );
        return LGW_I2C_ERROR;
    }

    return LGW_I2C_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int at30ts74_get_temperature(int i2c_fd, uint8_t i2c_addr, float * temperature) {
    int err;
    uint8_t t[2];
    uint16_t t16;

    /* Check Input Params */
    if (i2c_fd <= 0) {
        printf("ERROR: invalid I2C file descriptor\n");
        return LGW_I2C_ERROR;
    }

    /* Read Temperature */
    err = i2c_linuxdev_read_buffer( i2c_fd, i2c_addr, t, 2);
    if (err != 0) {
        printf("ERROR: failed to read I2C device 0x%02X (err=%i)\n", i2c_addr, err);
        return LGW_I2C_ERROR;
    }

    t16 = (t[0] << 4) | (t[1] >> 4);
    if (t16 & 0x0800) {
        *temperature = -(float)(((~t16) + 1) & 0x0fff) / 16.0;
    } else {
        *temperature = (float)t16 / 16.0;
    }
    DEBUG_PRINTF("Temperature: %f C (h:0x%02X l:0x%02X)\n", *temperature, t[0], t[1]);

    return LGW_I2C_SUCCESS;
}

/* --- EOF ------------------------------------------------------------------ */
