# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

set -e

makedir() {
    if [ ! -d $1 ]; then
	mkdir $1 2> /dev/null || true
    fi
}

makedir build
makedir build/dist
makedir build/include
makedir build/make
makedir build/make_obj
makedir build/obj
makedir build/src

if [ -f build/make/target.mk ]; then
    . build/make/target.mk
    makedir build/obj/${MACHINE}-${OS}
    makedir build/obj/${MACHINE}
    makedir build/obj/${OS}
fi
