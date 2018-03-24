# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

set -e

makedir() {
    if [ ! -d $1 ]; then
	mkdir $1
    fi
}

makedir build
makedir build/make
makedir build/make_obj
makedir build/include
makedir build/obj
makedir build/dist

if [ -f build/make/target.mk ]; then
    . build/make/target.mk
    makedir build/obj/${MACHINE}-${OS}
    makedir build/obj/${MACHINE}
    makedir build/obj/${OS}
fi
