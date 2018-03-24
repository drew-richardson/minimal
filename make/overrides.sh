# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

set -e

set +e
# Some vars may have spaces in them
. build/make/env.mk
. build/make/compiler.mk
set -e
. build/make/target.mk

find_override() {
    if [ -f ${PROJROOT}src/${MACHINE}-${OS}/$1 ]; then
	echo $1=${MACHINE}-${OS}/
	return
    fi
    if [ -f ${PROJROOT}src/${MACHINE}/$1 ]; then
	echo $1=${MACHINE}/
	return
    fi
    if [ -f ${PROJROOT}src/${OS}/$1 ]; then
	echo $1=${OS}/
	return
    fi
    echo $1=
}

find_override dr_task_destroy_on_do${AEXT}
find_override dr_task_switch${AEXT}
