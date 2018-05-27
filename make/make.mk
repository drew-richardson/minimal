# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

include build/make/dr_config.mk
include $(PROJROOT)make/quiet.mk

VERSION_MAJOR = 0
VERSION_MINOR = 1
VERSION_PATCH = 0
VERSION_EXTRA = -a0

all: deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk build/dist/9p_client$(EEXT) build/dist/9p_code$(EEXT) build/dist/9p_fuzz$(EEXT) build/dist/9p_server$(EEXT) build/dist/client$(EEXT) build/dist/perms$(EEXT) build/dist/printf$(EEXT) build/dist/queue$(EEXT) build/dist/server$(EEXT) build/dist/task$(EEXT)

check: check_9p_code check_perms check_printf check_queue check_task check_server_client

check_9p_code: all
	$(Q)build/dist/9p_code$(EEXT)

check_perms: all
	$(Q)build/dist/perms$(EEXT)

check_printf: all
	$(Q)build/dist/printf$(EEXT)

check_queue: all
	$(Q)build/dist/queue$(EEXT)

check_task: all
	$(Q)if [ $$(build/dist/task$(EEXT))"x" = "aone2two3three4four5five6six7sev10bone2two3three4four5five6six7sev10cSleepingfoodone2two3three4four5five6six7sev10eone2two3three4four5five6six7sev10fone2two3three4four5five6six7sev10gExitingfoohCleanupfooiBackx" ]; then echo "$$(date -u +%s)           check_task(make/make.mk)                : OK"; true; else echo "$$(date -u +%s)           check_task(make/make.mk)                : FAIL"; false; fi

check_server_client: all
	$(Q)if [ $$(build/dist/server$(EEXT) -p 6000 > /dev/null 2>&1 & \
	SERVER_PID=$$!; \
	i=0; \
	while [ $$i -lt 3 ]; do \
	    sh -c "( printf 'Hello'; sleep 1; printf 'world' ) | build/dist/client$(EEXT) -p 6000" & \
	    i=$$((i+1)); \
	done; \
	sleep 2; \
	kill $${SERVER_PID})"x" = "HelloHelloHelloworldworldworldx" ]; then echo "$$(date -u +%s)           check_server_client(make/make.mk)       : OK"; true; else echo "$$(date -u +%s)           check_server_client(make/make.mk)       : FAIL"; false; fi

build/dist/9p_client$(EEXT): deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk $@

build/dist/9p_code$(EEXT): deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk $@

build/dist/9p_fuzz$(EEXT): deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk $@

build/dist/9p_server$(EEXT): deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk $@

build/dist/client$(EEXT): deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk $@

build/dist/perms$(EEXT): deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk $@

build/dist/printf$(EEXT): deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk $@

build/dist/queue$(EEXT): deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk $@

build/dist/server$(EEXT): deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk $@

build/dist/task$(EEXT): deps
	$(Q)$(MAKE) -f $(PROJROOT)make/build.mk $@

force:

include build/make/flags.mk
include build/make/dr_config_h.mk

build/make/target.mk: build/make/dr_config.mk $(PROJROOT)config/identify.c
	$(E_GEN) \
rm -f build/make_obj/target.mk.out && \
( \
    cd build/make_obj && \
    $(CC) $(CSTD) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) ../../$(PROJROOT)config/identify.c $(OUTPUT_L)identify$(EEXT) > target.mk.out 2>&1 && \
    (egrep -a '^(MACHINE|OS)=' identify$(EEXT) || egrep '^(MACHINE|OS)=' identify$(EEXT)) 2> /dev/null \
) > $@ || \
( \
    rm -f $@; \
    cat build/make_obj/target.mk.out; \
    false; \
)

build/make/overrides.mk: build/make/compiler.mk build/make/target.mk
	$(E_GEN). $(PROJROOT)make/overrides.sh > $@ 2> /dev/null || ( rm -f $@; false )

build/make/ACCEPT_LDLIBS.mk: build/make/dr_config.mk $(PROJROOT)config/libs/accept.c
	$(E_GEN) \
if ! (cd build/make_obj && \
        $(CC) $(CSTD) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) ../../$(PROJROOT)config/libs/accept.c $(OUTPUT_L)ACCEPT_LDLIBS$(EEXT) || \
        echo error) 2>&1 | egrep -i 'error|warn' > /dev/null; then \
    echo ACCEPT_LDLIBS=; \
else \
    if ! (cd build/make_obj && \
            $(CC) $(CSTD) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) ../../$(PROJROOT)config/libs/accept.c $(LDLIB_PREFIX)ws2_32$(LEXT) $(OUTPUT_L)ACCEPT_LDLIBS$(EEXT) || \
            echo error) 2>&1 | egrep -i 'error|warn' > /dev/null; then \
        echo ACCEPT_LDLIBS=$(LDLIB_PREFIX)ws2_32$(LEXT); \
    else \
        if ! (cd build/make_obj && \
                $(CC) $(CSTD) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) ../../$(PROJROOT)config/libs/accept.c $(LDLIB_PREFIX)socket$(LEXT) $(LDLIB_PREFIX)nsl$(LEXT) $(OUTPUT_L)ACCEPT_LDLIBS$(EEXT) || \
                echo error) 2>&1 | egrep -i 'error|warn' > /dev/null; then \
            echo ACCEPT_LDLIBS=$(LDLIB_PREFIX)socket$(LEXT) $(LDLIB_PREFIX)nsl$(LEXT); \
        else \
            false; \
        fi \
    fi \
fi > $@ || \
( \
    rm -f $@; \
    false; \
)

build/make/ACCEPTEX_LDLIBS.mk: build/make/dr_config.mk $(PROJROOT)config/libs/acceptex.c
	$(E_GEN) \
if ! (cd build/make_obj && \
        $(CC) $(CSTD) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) ../../$(PROJROOT)config/libs/acceptex.c $(OUTPUT_L)ACCEPTEX_LDLIBS$(EEXT) || \
        echo error) 2>&1 | egrep -i 'error|warn' > /dev/null; then \
    echo ACCEPTEX_LDLIBS=; \
else \
    if ! (cd build/make_obj && \
            $(CC) $(CSTD) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) ../../$(PROJROOT)config/libs/acceptex.c $(LDLIB_PREFIX)mswsock$(LEXT) $(OUTPUT_L)ACCEPTEX_LDLIBS$(EEXT) || \
            echo error) 2>&1 | egrep -i 'error|warn' > /dev/null; then \
        echo ACCEPTEX_LDLIBS=$(LDLIB_PREFIX)mswsock$(LEXT); \
    else \
        false; \
    fi \
fi > $@ || \
( \
    rm -f $@; \
    false; \
)

build/make/deps.mk: force
	$(E_GEN)find build/obj -type f -name '*.d' 2> /dev/null | xargs cat > $@

build/src/dr_source.c: force
	$(E_GEN) \
. make/dr_source.sh; \
printf "%s\n" \
"#include \"dr.h\"" \
"const char *dr_source_revision(void) { return \"$${SOURCE_REVISION}\"; }" \
"const char *dr_source_status(void) { return \"$${SOURCE_STATUS}\"; }" \
"const char *dr_source_date(void) { return \"$${SOURCE_DATE}\"; }" > $@.new; \
if ! diff $@.new $@ > /dev/null 2>&1; then \
    mv $@.new $@; \
fi

build/include/dr_version.h:
	$(E_GEN)printf "%s\n" \
"#if !defined(DR_VERSION_H)" \
"#define DR_VERSION_H" \
"#define DR_VERSION_MAJOR $(VERSION_MAJOR)" \
"#define DR_VERSION_MINOR $(VERSION_MINOR)" \
"#define DR_VERSION_PATCH $(VERSION_PATCH)" \
"#define DR_VERSION_MAJOR_STR \"$(VERSION_MAJOR)\"" \
"#define DR_VERSION_MINOR_STR \"$(VERSION_MINOR)\"" \
"#define DR_VERSION_PATCH_STR \"$(VERSION_PATCH)\"" \
"#define DR_VERSION_EXTRA \"$(VERSION_EXTRA)\"" \
"#endif // DR_VERSION_H" > $@

build/include/dr_types.h: build/include/dr_config.h build/include/dr_version.h $(PROJROOT)config/dr_util.h $(PROJROOT)src/dr_compiler.h $(PROJROOT)src/dr_types_common.h $(PROJROOT)src/dr_types_impl.h $(PROJROOT)src/list.h $(PROJROOT)config/dr_types.c
	$(E_GEN) \
rm -f build/make_obj/dr_types.h.out && \
( \
    cd build/make_obj && \
    printf "%s\n" \
"#if !defined(DR_TYPES_H)" \
"#define DR_TYPES_H" \
"" && \
    $(CC) $(CSTD) -I../../$(PROJROOT)src -I../../build/include $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) ../../$(PROJROOT)config/dr_types.c $(OUTPUT_L)dr_types$(EEXT) >> dr_types.h.out 2>&1 && \
    (egrep -a '^(struct|typedef) ' dr_types$(EEXT) || egrep '^(struct|typedef) ' dr_types$(EEXT)) 2> /dev/null && \
    printf "%s\n" \
"" \
"#include \"dr_types_common.h\"" \
"" \
"#endif // DR_TYPES_H\n" \
) > $@ || \
( \
    rm -f $@; \
    cat build/make_obj/dr_types.h.out; \
    false; \
)

#	@echo MAKECMDGOALS = $(MAKECMDGOALS)
#	@echo .TARGETS = $(.TARGETS)
deps: build/make/target.mk build/make/overrides.mk build/make/cppflags.mk build/make/cflags.mk build/make/ACCEPT_LDLIBS.mk build/make/ACCEPTEX_LDLIBS.mk build/make/deps.mk build/include/dr_config.h build/include/dr_version.h build/include/dr_types.h build/src/dr_source.c
	$(Q)$(SHELL) $(PROJROOT)make/mkdirs.sh

clean:
	rm -rf build/make_obj build/obj build/dist

info:
	$(Q)cat build/make/dr_config.mk build/make/target.mk build/make/cppflags.mk build/make/cflags.mk build/make/overrides.mk build/include/dr_config.h

install: all
	$(E_INSTALL)DESTDIR=$${DESTDIR:-$(PREFIX)} && \
	mkdir -p $${DESTDIR}/bin && \
	cp build/dist/9p_client$(EEXT) $${DESTDIR}/bin && \
	chmod 755 $${DESTDIR}/bin/9p_client$(EEXT) && \
	cp build/dist/9p_server$(EEXT) $${DESTDIR}/bin && \
	chmod 755 $${DESTDIR}/bin/9p_server$(EEXT)

dist: minimal.tar.gz

minimal.tar.gz: force
	$(E_GEN) \
. make/dr_source.sh; \
if [ "${VERSION_PATCH}" = "0" ]; then \
    VERSION=${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_EXTRA}; \
else \
    VERSION=${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_EXTRA}; \
fi; \
LC_ALL=C.UTF-8 tar cf - --mode=u+rw,go=rX --mtime="$${SOURCE_DATE}" --group=0 --numeric-owner --owner=0 --sort=name --transform='s,^,minimal/,' config configure COPYING COPYRIGHT.musl make README.md src test | gzip -n9 > minimal-$${VERSION}.tar.gz

distclean:
	rm -rf build Makefile minimal*.tar.gz
