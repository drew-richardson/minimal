# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

include $(PROJROOT)make/quiet.mk
include build/make/env.mk

all: build/make/dr_config.mk build/make/flags.mk build/make/dr_config_h.mk

build/make/dr_config.mk: build/make/env.mk build/make/compiler.mk build/make/eext.mk build/make/oext.mk build/make/cstd.mk
	$(E_GEN)cat build/make/env.mk build/make/compiler.mk build/make/eext.mk build/make/oext.mk build/make/cstd.mk > $@

build/make/compiler.mk: build/make/env.mk $(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c
	$(E_GEN) \
rm -f build/make_obj/compiler.mk.out && \
if rm -f build/make_obj/cl.test.obj && \
        $(CC) $(CPPFLAGS) $(CFLAGS) $(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c /c /Fo: build/make_obj/cl.test.obj >> build/make_obj/compiler.mk.out 2>&1 && \
        [ -f build/make_obj/cl.test.obj ]; then \
    printf "%s\n" \
        "OUTPUT_C=/c /Fo" \
        "OUTPUT_L=/Fe" \
        "LDLIB_PREFIX=" \
        "AEXT=.asm" \
        "LEXT=.lib"; \
else \
    if rm -f build/make_obj/cc.test.o && \
            $(CC) $(CPPFLAGS) $(CFLAGS) $(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c -c -o build/make_obj/cc.test.o >> build/make_obj/compiler.mk.out 2>&1 && \
            [ -f build/make_obj/cc.test.o ]; then \
        printf "%s\n" \
           "OUTPUT_C=-c -o " \
           "OUTPUT_L=-o " \
           "LDLIB_PREFIX=-l" \
           "AEXT=.S" \
           "LEXT="; \
    else \
        false; \
    fi \
fi > $@ || \
( \
    rm -f $@; \
    cat build/make_obj/compiler.mk.out; \
    false; \
)

build/make/eext.mk: build/make/env.mk $(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c
	$(E_GEN) \
rm -f build/make_obj/eext.mk.out && \
if cd build/make_obj && \
        rm -f a.out a.exe DR_ALWAYS_SUCCEEDS.eext.exe && \
        cp ../../$(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c DR_ALWAYS_SUCCEEDS.eext.c && \
        $(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) DR_ALWAYS_SUCCEEDS.eext.c >> eext.mk.out 2>&1 && \
        [ -f a.out ]; then \
    echo EEXT=; \
else \
    if [ -f a.exe -o -f DR_ALWAYS_SUCCEEDS.eext.exe ]; then \
        echo EEXT=.exe; \
    else \
        false; \
    fi \
fi > $@ || \
( \
    rm -f $@; \
    cat eext.mk.out; \
    false; \
)

build/make/oext.mk: build/make/env.mk $(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c
	$(E_GEN) \
rm -f build/make_obj/oext.mk.out && \
if \
        cd build/make_obj && \
        rm -f DR_ALWAYS_SUCCEEDS.oext.o DR_ALWAYS_SUCCEEDS.oext.obj && \
        cp ../../$(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c DR_ALWAYS_SUCCEEDS.oext.c && \
        $(CC) $(CPPFLAGS) $(CFLAGS) DR_ALWAYS_SUCCEEDS.oext.c -c >> oext.mk.out 2>&1 && \
        [ -f DR_ALWAYS_SUCCEEDS.oext.o ]; then \
    echo OEXT=.o; \
else \
    if [ -f DR_ALWAYS_SUCCEEDS.oext.obj ]; then \
        echo OEXT=.obj; \
    else \
        false; \
    fi \
fi > $@ || \
( \
    rm -f $@; \
    cat oext.mk.out; \
    false; \
)

build/make/cstd.mk: build/make/env.mk $(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c
	$(E_GEN) \
if cd build/make_obj && \
        cp ../../$(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c DR_ALWAYS_SUCCEEDS.cstd.c && \
        ! ($(CC) -std=gnu11 $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) DR_ALWAYS_SUCCEEDS.cstd.c -o stdgnu11 2>&1 || \
        echo error) | egrep -i 'error|warn' > /dev/null; then \
    echo CSTD=-std=gnu11; \
else \
    if ! ($(CC) -std=gnu99 $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) DR_ALWAYS_SUCCEEDS.cstd.c -o stdgnu99 2>&1 || \
            echo error) | egrep -i 'error|warn' > /dev/null; then \
        echo CSTD=-std=gnu99; \
    else \
        if ! ($(CC) -xc11 $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) DR_ALWAYS_SUCCEEDS.cstd.c -o xc11 2>&1 || \
                echo error) | egrep -i 'error|warn' > /dev/null; then \
            echo CSTD=-xc11; \
        else \
            if ! ($(CC) -xc99 $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) DR_ALWAYS_SUCCEEDS.cstd.c -o xc99 2>&1 || \
                    echo error) | egrep -i 'error|warn' > /dev/null; then \
                echo CSTD=-xc99; \
            else \
                echo CSTD=; \
            fi \
        fi \
    fi \
fi > $@ || \
( \
    rm -f $@; \
    false; \
)

build/make/flags.mk:
	$(E_GEN). $(PROJROOT)make/flags.sh > $@ || ( rm -f $@; false )

build/make/dr_config_h.mk:
	$(E_GEN). $(PROJROOT)make/dr_config_h.sh > $@ || ( rm -f $@; false )
