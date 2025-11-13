#!/bin/bash

dd if=/dev/zero of=fat12.img bs=512 count=20

mkfs.vfat -F12 fat12.img
file fat12.img

# mkfs.vfat -F16 fat16.img
# file fat16.img

mkdir ./tmp
sudo mount -o loop fat12.img ./tmp

cp _script ./tmp

sudo umount -v ./tmp
