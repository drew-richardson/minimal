# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

flag_rule() {
    # DR This is crummy
    UPPER=$(echo $2 | tr '[:lower:]' '[:upper:]' | sed 's/[&,/=-]//g')
    SPLIT=$(echo $2 | sed 's/&/ /g')
    FILES="${FILES} build/make/$1_${UPPER}.mk"
    printf "%s\n" \
	   "build/make/$1_${UPPER}.mk: build/make/dr_config.mk \$(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c \$(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS\$(AEXT)" \
	   "	\$(E_GEN) \\" \
	   "if ! (cp \$(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS.c build/make_obj/DR_ALWAYS_SUCCEEDS.${UPPER}.c && \\" \
	   "        cd build/make_obj && \\" \
	   "        \$(CC) \$(CSTD) ${SPLIT} \$(CPPFLAGS) \$(CFLAGS) \$(LDFLAGS) DR_ALWAYS_SUCCEEDS.${UPPER}.c \$(OUTPUT_L)$1_${UPPER}\$(EEXT) && \\" \
	   "        \$(CCAS) ${SPLIT} \$(CPPFLAGS) \$(OUTPUT_C)$1_${UPPER}\$(OEXT) ../../\$(PROJROOT)config/checks/DR_ALWAYS_SUCCEEDS\$(AEXT) || \\" \
	   "        echo error) 2>&1 | egrep -i 'error|warn' > /dev/null; then \\" \
	   "    echo $1_${UPPER}=${SPLIT}; \\" \
	   "else \\" \
	   "    echo $1_${UPPER}=; \\" \
	   "fi > \$@" \
	   ""
}

cppflag_rule() {
    flag_rule CPPFLAG $1
}

cflag_rule() {
    flag_rule CFLAG $1
}

printf "%s\n" \
       "# SPDX-License-Identifier: GPL-2.0-only" \
       "# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>" \
       ""

CPPRULES="-Wundef -MD&-MP"
FILES=
for i in ${CPPRULES}; do
    cppflag_rule $i
done
printf "%s\n" \
       "build/make/cppflags.mk:${FILES}" \
       "	\$(E_GEN)cat${FILES} > \$@" \
       ""

FILES=

cflag_rule "-Wfails"
cflag_rule "-Wall"
cflag_rule "-Wextra"

cflag_rule "-Wc++-compat"
cflag_rule "-Wcast-qual"
cflag_rule "-Wdouble-promotion"
cflag_rule "-Wduplicated-branches"
cflag_rule "-Wduplicated-cond"
cflag_rule "-Wformat=2"
cflag_rule "-Wimplicit-function-declaration"
cflag_rule "-Wjump-misses-init"
cflag_rule "-Wlogical-op"
cflag_rule "-Wmissing-prototypes"
cflag_rule "-Wnull-dereference"
cflag_rule "-Wold-style-definition"
cflag_rule "-Wpadded"
cflag_rule "-Wpointer-arith"
cflag_rule "-Wrestrict"
cflag_rule "-Wshadow"
cflag_rule "-Wstrict-prototypes"
cflag_rule "-Wvla"
cflag_rule "-Wwrite-strings"

cflag_rule "-Wa,--noexecstack"

cflag_rule "-fno-asynchronous-unwind-tables"
cflag_rule "-fno-exceptions"
cflag_rule "-fno-unwind-tables"
cflag_rule "-fomit-frame-pointer"
cflag_rule "-fpie"
cflag_rule "-fvisibility=hidden"

cflag_rule "-pipe"

cflag_rule "/nologo"

printf "%s\n" \
       "build/make/cflags.mk:${FILES}" \
       "	\$(E_GEN)cat${FILES} > \$@" \
       ""
