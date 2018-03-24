# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

include build/make/dr_config.mk
include $(PROJROOT)make/quiet.mk
include build/make/target.mk
include build/make/cppflags.mk
include build/make/cflags.mk
include build/make/deps.mk
include build/make/overrides.mk
include build/make/ACCEPT_LDLIBS.mk
include build/make/ACCEPTEX_LDLIBS.mk

# https://news.ycombinator.com/item?id=13993681 ?
CPPFLAGS_linux = -D_GNU_SOURCE
# Windows 7
CPPFLAGS_windows = -DWINVER=0x0601 -D_WIN32_WINNT=0x0601
CPPFLAGS_ALL = $(CPPFLAG_WUNDEF) $(CPPFLAGS_$(OS)) $(CPPFLAG_MD) -Ibuild/include -I$(PROJROOT)src
CFLAGS_ALL = $(CFLAG_WFAILS) $(CFLAG_WALL) $(CFLAG_WEXTRA) $(CFLAG_WMISSINGPROTOTYPES) $(CFLAG_WOLDSTYLEDEFINITION) $(CFLAG_WPOINTERARITH) $(CFLAG_WSHADOW) $(CFLAG_WSTRICTPROTOTYPES) $(CFLAG_WWRITESTRINGS) $(CFLAG_WIMPLICITFUNCTIONDECLARATION) $(CFLAG_WDUPLICATEDCOND) $(CFLAG_WDUPLICATEDBRANCHES) $(CFLAG_WLOGICALOP) $(CFLAG_WRESTRICT) $(CFLAG_WNULLDEREFERENCE) $(CFLAG_WJUMPMISSESINIT) $(CFLAG_WDOUBLEPROMOTION) $(CFLAG_WANOEXECSTACK) $(CFLAG_FNOASYNCHRONOUSUNWINDTABLES) $(CFLAG_FNOEXCEPTIONS) $(CFLAG_FNOUNWINDTABLES) $(CFLAG_FOMITFRAMEPOINTER) $(CFLAG_FVISIBILITYHIDDEN) $(CFLAG_FPIE) $(CFLAG_NOLOGO) $(CFLAG_PIPE)
# -Wno-override-init -Wno-missing-field-initializers
# These are helpful but result in some warnings I'm unsure how to fix
#CFLAGS_ALL += $(CFLAG_WC++COMPAT) $(CFLAG_WCASTQUAL) $(CFLAG_WFORMAT2)
FLAGS_C = $(CSTD) $(CPPFLAGS_ALL) $(CFLAGS_ALL) $(CPPFLAGS) $(CFLAGS)
FLAGS_L = $(CFLAG_NOLOGO) $(CFLAG_FPIE) $(LDFLAGS)
#FLAGS_CCLD = $(CSTD) $(CPPFLAGS_ALL) $(CFLAGS_ALL) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS)

# DR ...
# Optimizations

build/obj/getopt$(OEXT): build/make/dr_config.mk $(PROJROOT)src/getopt.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/getopt.c $(OUTPUT_C)$@

build/obj/dr_9p_decode$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_9p_decode.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_9p_decode.c $(OUTPUT_C)$@

build/obj/dr_9p_encode$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_9p_encode.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_9p_encode.c $(OUTPUT_C)$@

build/obj/dr_clock$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_clock.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_clock.c $(OUTPUT_C)$@

build/obj/dr_console$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_console.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_console.c $(OUTPUT_C)$@

build/obj/dr_event$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_event.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_event.c $(OUTPUT_C)$@

build/obj/dr_io$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_io.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_io.c $(OUTPUT_C)$@

build/obj/dr_log$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_log.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_log.c $(OUTPUT_C)$@

build/obj/dr_pipe$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_pipe.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_pipe.c $(OUTPUT_C)$@

build/obj/dr_socket$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_socket.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_socket.c $(OUTPUT_C)$@

build/obj/dr_sem$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_sem.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_sem.c $(OUTPUT_C)$@

build/obj/dr_str$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_str.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_str.c $(OUTPUT_C)$@

build/obj/dr_task$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_task.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_task.c $(OUTPUT_C)$@

build/obj/$(dr_task_destroy_on_do$(AEXT))dr_task_destroy_on_do$(OEXT): build/make/dr_config.mk $(PROJROOT)src/$(dr_task_destroy_on_do$(AEXT))dr_task_destroy_on_do$(AEXT)
	$(E_CCAS)$(CCAS) $(FLAGS_C) $(OUTPUT_C)$@ $(PROJROOT)src/$(dr_task_destroy_on_do$(AEXT))dr_task_destroy_on_do$(AEXT)

build/obj/$(dr_task_switch$(AEXT))dr_task_switch$(OEXT): build/make/dr_config.mk $(PROJROOT)src/$(dr_task_switch$(AEXT))dr_task_switch$(AEXT)
	$(E_CCAS)$(CCAS) $(FLAGS_C) $(OUTPUT_C)$@ $(PROJROOT)src/$(dr_task_switch$(AEXT))dr_task_switch$(AEXT)

build/obj/dr_vfs$(OEXT): build/make/dr_config.mk $(PROJROOT)src/dr_vfs.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/dr_vfs.c $(OUTPUT_C)$@

build/obj/9p_code$(OEXT): build/make/dr_config.mk $(PROJROOT)test/9p_code.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)test/9p_code.c $(OUTPUT_C)$@

build/obj/9p_fuzz$(OEXT): build/make/dr_config.mk $(PROJROOT)test/9p_fuzz.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)test/9p_fuzz.c $(OUTPUT_C)$@

build/obj/9p_client$(OEXT): build/make/dr_config.mk $(PROJROOT)src/9p_client.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/9p_client.c $(OUTPUT_C)$@

build/obj/9p_server$(OEXT): build/make/dr_config.mk $(PROJROOT)src/9p_server.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)src/9p_server.c $(OUTPUT_C)$@

build/obj/client$(OEXT): build/make/dr_config.mk $(PROJROOT)test/client.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)test/client.c $(OUTPUT_C)$@

build/obj/perms$(OEXT): build/make/dr_config.mk $(PROJROOT)test/perms.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)test/perms.c $(OUTPUT_C)$@

build/obj/queue$(OEXT): build/make/dr_config.mk $(PROJROOT)test/queue.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)test/queue.c $(OUTPUT_C)$@

build/obj/server$(OEXT): build/make/dr_config.mk $(PROJROOT)test/server.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)test/server.c $(OUTPUT_C)$@

build/obj/task$(OEXT): build/make/dr_config.mk $(PROJROOT)test/task.c
	$(E_CC)$(CC) $(FLAGS_C) $(PROJROOT)test/task.c $(OUTPUT_C)$@

build/dist/9p_code$(EEXT): build/make/dr_config.mk build/obj/dr_9p_decode$(OEXT) build/obj/dr_9p_encode$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_log$(OEXT) build/obj/9p_code$(OEXT)
	$(E_CCLD)$(CC) $(FLAGS_L) build/obj/dr_9p_decode$(OEXT) build/obj/dr_9p_encode$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_log$(OEXT) build/obj/9p_code$(OEXT) $(ACCEPT_LDLIBS) $(LDLIBS) $(OUTPUT_L)$@

build/dist/9p_fuzz$(EEXT): build/make/dr_config.mk build/obj/dr_9p_decode$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_console$(OEXT) build/obj/dr_io$(OEXT) build/obj/dr_log$(OEXT) build/obj/9p_fuzz$(OEXT)
	$(E_CCLD)$(CC) $(FLAGS_L) build/obj/dr_9p_decode$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_console$(OEXT) build/obj/dr_io$(OEXT) build/obj/dr_log$(OEXT) build/obj/9p_fuzz$(OEXT) $(ACCEPT_LDLIBS) $(LDLIBS) $(OUTPUT_L)$@

build/dist/9p_client$(EEXT): build/make/dr_config.mk build/obj/getopt$(OEXT) build/obj/dr_9p_encode$(OEXT) build/obj/dr_9p_decode$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_io$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_pipe$(OEXT) build/obj/dr_socket$(OEXT) build/obj/9p_client$(OEXT)
	$(E_CCLD)$(CC) $(FLAGS_L) build/obj/getopt$(OEXT) build/obj/dr_9p_encode$(OEXT) build/obj/dr_9p_decode$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_io$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_pipe$(OEXT) build/obj/dr_socket$(OEXT) build/obj/9p_client$(OEXT) $(ACCEPT_LDLIBS) $(LDLIBS) $(OUTPUT_L)$@

build/dist/9p_server$(EEXT): build/make/dr_config.mk build/obj/getopt$(OEXT) build/obj/dr_9p_decode$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_9p_encode$(OEXT) build/obj/dr_event$(OEXT) build/obj/dr_io$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_pipe$(OEXT) build/obj/dr_socket$(OEXT) build/obj/dr_str$(OEXT) build/obj/dr_task$(OEXT) build/obj/$(dr_task_destroy_on_do$(AEXT))dr_task_destroy_on_do$(OEXT) build/obj/$(dr_task_switch$(AEXT))dr_task_switch$(OEXT) build/obj/dr_vfs$(OEXT) build/obj/9p_server$(OEXT)
	$(E_CCLD)$(CC) $(FLAGS_L) build/obj/getopt$(OEXT) build/obj/dr_9p_decode$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_9p_encode$(OEXT) build/obj/dr_event$(OEXT) build/obj/dr_io$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_pipe$(OEXT) build/obj/dr_socket$(OEXT) build/obj/dr_str$(OEXT) build/obj/dr_task$(OEXT) build/obj/$(dr_task_destroy_on_do$(AEXT))dr_task_destroy_on_do$(OEXT) build/obj/$(dr_task_switch$(AEXT))dr_task_switch$(OEXT) build/obj/dr_vfs$(OEXT) build/obj/9p_server$(OEXT) $(ACCEPT_LDLIBS) $(ACCEPTEX_LDLIBS) $(LDLIBS) $(OUTPUT_L)$@

build/dist/client$(EEXT): build/make/dr_config.mk build/obj/getopt$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_console$(OEXT) build/obj/dr_io$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_socket$(OEXT) build/obj/client$(OEXT)
	$(E_CCLD)$(CC) $(FLAGS_L) build/obj/getopt$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_console$(OEXT) build/obj/dr_io$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_socket$(OEXT) build/obj/client$(OEXT) $(ACCEPT_LDLIBS) $(LDLIBS) $(OUTPUT_L)$@

build/dist/perms$(EEXT): build/make/dr_config.mk build/obj/dr_clock$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_str$(OEXT) build/obj/dr_vfs$(OEXT) build/obj/perms$(OEXT)
	$(E_CCLD)$(CC) $(FLAGS_L) build/obj/dr_clock$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_str$(OEXT) build/obj/dr_vfs$(OEXT) build/obj/perms$(OEXT) $(ACCEPT_LDLIBS) $(LDLIBS) $(OUTPUT_L)$@

build/dist/queue$(EEXT): build/make/dr_config.mk build/obj/dr_clock$(OEXT) build/obj/dr_log$(OEXT) build/obj/queue$(OEXT)
	$(E_CCLD)$(CC) $(FLAGS_L) build/obj/dr_clock$(OEXT) build/obj/dr_log$(OEXT) build/obj/queue$(OEXT) $(ACCEPT_LDLIBS) $(LDLIBS) $(OUTPUT_L)$@

build/dist/server$(EEXT): build/make/dr_config.mk build/obj/getopt$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_event$(OEXT) build/obj/dr_io$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_sem$(OEXT) build/obj/dr_socket$(OEXT) build/obj/dr_task$(OEXT) build/obj/$(dr_task_destroy_on_do$(AEXT))dr_task_destroy_on_do$(OEXT) build/obj/$(dr_task_switch$(AEXT))dr_task_switch$(OEXT) build/obj/server$(OEXT)
	$(E_CCLD)$(CC) $(FLAGS_L) build/obj/getopt$(OEXT) build/obj/dr_clock$(OEXT) build/obj/dr_event$(OEXT) build/obj/dr_io$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_sem$(OEXT) build/obj/dr_socket$(OEXT) build/obj/dr_task$(OEXT) build/obj/$(dr_task_destroy_on_do$(AEXT))dr_task_destroy_on_do$(OEXT) build/obj/$(dr_task_switch$(AEXT))dr_task_switch$(OEXT) build/obj/server$(OEXT) $(ACCEPT_LDLIBS) $(ACCEPTEX_LDLIBS) $(LDLIBS) $(OUTPUT_L)$@

build/dist/task$(EEXT): build/make/dr_config.mk build/obj/dr_clock$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_task$(OEXT) build/obj/$(dr_task_destroy_on_do$(AEXT))dr_task_destroy_on_do$(OEXT) build/obj/$(dr_task_switch$(AEXT))dr_task_switch$(OEXT) build/obj/task$(OEXT)
	$(E_CCLD)$(CC) $(FLAGS_L) build/obj/dr_clock$(OEXT) build/obj/dr_log$(OEXT) build/obj/dr_task$(OEXT) build/obj/$(dr_task_destroy_on_do$(AEXT))dr_task_destroy_on_do$(OEXT) build/obj/$(dr_task_switch$(AEXT))dr_task_switch$(OEXT) build/obj/task$(OEXT) $(ACCEPT_LDLIBS) $(LDLIBS) $(OUTPUT_L)$@
