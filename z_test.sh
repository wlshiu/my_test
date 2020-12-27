#!/bin/bash

set -e

help()
{
    echo -e "usage: $0 <e/d> <input> <output>"
    echo -e "    e:     compress"
    echo -e "    d:     decompress"
    exit 1;
}

if [ $# != 3 ]; then
    help
fi

./out/lzma $*
