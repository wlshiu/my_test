#!/bin/bash

set -e


help()
{
    echo -e "usage: $0 [t/r] [115200] [file path]"
    echo -e "    t: transmition"
    echo -e "    r: receive"
    exit 1
}

if [ $# != 3 ]; then
    help
fi

# dev_path=/dev/ttyS0  # respberry pi
dev_path=/dev/ttyUSB0  # ubuntu
# dev_path=/dev/ttyAMA0

sudo chmod 666 ${dev_path}

case $1 in
    "t")
        rm -f serial_port_tx
        gcc serial_port_tx.c -o serial_port_tx

        ./serial_port_tx ${dev_path} $2 $3
        ;;
    "r")
        rm -f serial_port_rx
        gcc serial_port_rx.c -o serial_port_rx

        ./serial_port_rx ${dev_path} $2 $3
        ;;
    *)
        help
        ;;
esac
