/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Minimum test program for reading temperatures from at30ts74 via loragw_i2c

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

/* Fix an issue between POSIX and C99 */
#if __STDC_VERSION__ >= 199901L
    #define _XOPEN_SOURCE 600
#else
    #define _XOPEN_SOURCE 500
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>     /* sigaction */
#include <unistd.h>     /* getopt, access */
#include <time.h>

#include "loragw_i2c.h"
#include "loragw_aux.h"
#include "loragw_hal.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define I2C_DEFAULT              "/dev/i2c-0"
#define I2C_PORT_AT30TS74        0x4a

#define AT30TS74_REG_TEMP        0x00
#define AT30TS74_REG_CONF        0x01

/* -------------------------------------------------------------------------- */
/* --- GLOBAL VARIABLES ----------------------------------------------------- */

/* Signal handling variables */
static int exit_sig = 0; /* 1 -> application terminates cleanly (shut down hardware, close open files, etc) */
static int quit_sig = 0; /* 1 -> application terminates without shutting down the hardware */

static int i2c_dev = -1;

/* -------------------------------------------------------------------------- */
/* --- SUBFUNCTIONS DECLARATION --------------------------------------------- */

static void sig_handler(int sigio);
static void usage(void);

/* -------------------------------------------------------------------------- */
/* --- MAIN FUNCTION -------------------------------------------------------- */

float get_temp(uint16_t t0, uint16_t t1)
{
    uint16_t t16 = (t0 << 4) | (t1 >> 4);
    if (t16 & 0x0800) {
        return -(float)(((~t16) + 1) & 0x0fff) / 16.0;
    } else {
        return (float)t16 / 16.0;
    }
}
//        return -((~((t0 << 4) | (t1 >> 4)) + 1) & 0x0fff) / 16.0;

int main(int argc, char ** argv)
{
    printf("125      %f\n", get_temp(0x7d, 0x00));
    printf("100      %f\n", get_temp(0x64, 0x00));
    printf(" 75      %f\n", get_temp(0x4b, 0x00));
    printf(" 50.5    %f\n", get_temp(0x32, 0x00));
    printf(" 25.25   %f\n", get_temp(0x19, 0x40));
    printf(" 10.125  %f\n", get_temp(0x0a, 0x20));
    printf("  0.0625 %f\n", get_temp(0x00, 0x10));
    printf("  0      %f\n", get_temp(0x00, 0x00));
    printf(" -0.0625 %f\n", get_temp(0xff, 0xf0));
    printf("-10.125  %f\n", get_temp(0xf5, 0xe0));
    printf("-25.25   %f\n", get_temp(0xe7, 0xc0));
    printf("-50.5    %f\n", get_temp(0xce, 0x80));
    printf("-55      %f\n", get_temp(0xc9, 0x00));
    return 0;

    int i, err;
    static struct sigaction sigact; /* SIGQUIT&SIGINT&SIGTERM signal handling */
    uint8_t val;
    uint8_t high_byte, low_byte;
    int8_t h;
    float temperature;
    const char* i2c_path = I2C_DEFAULT;

    /* Parse command line options */
    while ((i = getopt(argc, argv, "hd:")) != -1) {
        switch (i) {
            case 'h':
                usage();
                return EXIT_SUCCESS;
                break;

            case 'd':
                if (optarg != NULL) {
                    i2c_path = optarg;
                }
                break;

            default:
                printf("ERROR: argument parsing options, use -h option for help\n");
                usage();
                return EXIT_FAILURE;
            }
    }

    /* Configure signal handling */
    sigemptyset( &sigact.sa_mask );
    sigact.sa_flags = 0;
    sigact.sa_handler = sig_handler;
    sigaction( SIGQUIT, &sigact, NULL );
    sigaction( SIGINT, &sigact, NULL );
    sigaction( SIGTERM, &sigact, NULL );

    printf( "+++ Start of I2C test program +++\n" );

    /* Open I2C port expander */
    err = i2c_linuxdev_open( i2c_path, I2C_PORT_AT30TS74, &i2c_dev );
    if ( (err != 0) || (i2c_dev <= 0) )
    {
        printf( "ERROR: failed to open I2C device %s (err=%i)\n", i2c_path, err );
        return EXIT_FAILURE;
    }

    /* Set conversion resolution to 12 bits */
    err = i2c_linuxdev_write( i2c_dev, I2C_PORT_AT30TS74, AT30TS74_REG_CONF, 0x60);
    if ( err != 0 )
    {
        printf( "ERROR: failed to write to I2C device 0x%02X (err=%i)\n", I2C_PORT_AT30TS74, err );
        return EXIT_FAILURE;
    }

    uint8_t b = AT30TS74_REG_TEMP;
    err = i2c_linuxdev_write_buffer( i2c_dev, I2C_PORT_AT30TS74, &b, 1);
    if ( err != 0 )
    {
        printf( "ERROR: failed to write to I2C device 0x%02X (err=%i)\n", I2C_PORT_AT30TS74, err );
        return EXIT_FAILURE;
    }

    while ((quit_sig != 1) && (exit_sig != 1)) {
        uint8_t t[2];

        /* Read Temperature */
        err = i2c_linuxdev_read_buffer( i2c_dev, I2C_PORT_AT30TS74, t, 2);
        if ( err != 0 )
        {
            printf( "ERROR: failed to read I2C device 0x%02X (err=%i)\n", I2C_PORT_AT30TS74, err );
            return EXIT_FAILURE;
        }

        if (t[0] & 0x80) {
            temperature = -(((~t[0] + 1) & 0xff) + ((~t[1] + 1) & 0xff) / 256.0);
        } else {
            temperature = t[0] + t[1] / 256.0;
        }

        printf( "Temperature: %f C\n", temperature );
        wait_ms( 100 );
    }

    /* Terminate */
    printf( "+++ End of I2C test program +++\n" );

    err = i2c_linuxdev_close( i2c_dev );
    if ( err != 0 )
    {
        printf( "ERROR: failed to close I2C device (err=%i)\n", err );
        return EXIT_FAILURE;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* --- SUBFUNCTIONS DEFINITION ---------------------------------------------- */

static void sig_handler(int sigio) {
    if (sigio == SIGQUIT) {
        quit_sig = 1;
    } else if((sigio == SIGINT) || (sigio == SIGTERM)) {
        exit_sig = 1;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void usage(void) {
    printf("~~~ Library version string~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf(" %s\n", lgw_version_info());
    printf("~~~ Available options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf(" -h            print this help\n");
    printf(" -d <path>     use Linux I2C device driver\n");
    printf("               => default path: " I2C_DEFAULT "\n");
}

/* --- EOF ------------------------------------------------------------------ */
