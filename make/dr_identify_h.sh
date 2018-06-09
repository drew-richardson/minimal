# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

. build/make/target.mk

echo '#if !defined(DR_IDENTIFY_H)'
echo '#define DR_IDENTIFY_H'

bit64() {
    echo '#define DR_64_BIT'
}

bit32() {
    echo '#define DR_32_BIT'
}

printf '#define DR_MACHINE_'
case ${MACHINE} in
    x86_64)
	echo 'X86_64'
	bit64
	;;
    i686)
	echo 'I686'
	bit32
	;;
    arm64)
	echo 'ARM64'
	bit64
	;;
    arm)
	echo 'ARM'
	bit32
	;;
    *)
	echo 'UNKNOWN'
	;;
esac

printf '#define DR_OS_'
case ${OS} in
    linux)
	echo 'LINUX'
	;;
    freebsd)
	echo 'FREEBSD'
	;;
    openbsd)
	echo 'OPENBSD'
	;;
    netbsd)
	echo 'NETBSD'
	;;
    dragonfly)
	echo 'DRAGONFLY'
	;;
    minix)
	echo 'MINIX'
	;;
    darwin)
	echo 'DARWIN'
	;;
    windows)
	echo 'WINDOWS'
	;;
    cygwin)
	echo 'CYGWIN'
	;;
    solaris)
	echo 'SOLARIS'
	;;
    *)
	echo 'UNKNOWN'
	;;
esac

echo '#endif // DR_IDENTIFY_H'
