#!/bin/sh

# This script is intended to be used on SX1302 CoreCell platform, it performs
# the following actions:
#       - export/unexport GPIO23 used to reset the SX1302 chip
#
# Usage examples:
#       ./reset_lgw.sh stop
#       ./reset_lgw.sh start

SX1302_RESET_PIN=23     # SX1302 reset

WAIT_GPIO() {
    sleep 0.1
}

init() {
    # setup GPIOs
    raspi-gpio set $SX1302_RESET_PIN dl op
}

reset() {
    echo "CoreCell reset through GPIO$SX1302_RESET_PIN..."

    raspi-gpio set $SX1302_RESET_PIN dl op; WAIT_GPIO
    raspi-gpio set $SX1302_RESET_PIN dh op; WAIT_GPIO
    raspi-gpio set $SX1302_RESET_PIN dl op; WAIT_GPIO

}

case "$1" in
    start)
    init
    reset
    ;;
    stop)
    reset
    ;;
    *)
    echo "Usage: $0 {start|stop}"
    exit 1
    ;;
esac

exit 0