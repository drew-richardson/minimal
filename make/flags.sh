# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

flag_rule() {
    # DR This is crummy
    CLEAN=$(echo $2 | tr '[:lower:]' '[:upper:]' | tr -d ',-/=')
    FILES="${FILES} build/make/$1_${CLEAN}.mk"
    printf "%s\n" \
	   "build/make/$1_${CLEAN}.mk: build/make/dr_config.mk \$(PROJROOT)config/checks/ALWAYS_SUCCEEDS.c \$(PROJROOT)config/checks/ALWAYS_SUCCEEDS\$(AEXT)" \
	   "	\$(E_GEN) \\" \
	   "if ! (cp \$(PROJROOT)config/checks/ALWAYS_SUCCEEDS.c build/make_obj/ALWAYS_SUCCEEDS.${CLEAN}.c && \\" \
	   "        cd build/make_obj && \\" \
	   "        \$(CC) \$(CSTD) $2 \$(CPPFLAGS) \$(CFLAGS) \$(LDFLAGS) ALWAYS_SUCCEEDS.${CLEAN}.c \$(OUTPUT_L)$1_${CLEAN}\$(EEXT) && \\" \
	   "        \$(CCAS) $2 \$(CPPFLAGS) \$(OUTPUT_C)$1_${CLEAN}\$(OEXT) ../../\$(PROJROOT)config/checks/ALWAYS_SUCCEEDS\$(AEXT) || \\" \
	   "        echo error) 2>&1 | egrep -i 'error|warn' > /dev/null; then \\" \
	   "    echo $1_${CLEAN}=$2; \\" \
	   "else \\" \
	   "    echo $1_${CLEAN}=; \\" \
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

CPPRULES="-Wundef -MD"
FILES=
for i in ${CPPRULES}; do
    cppflag_rule $i
done
printf "%s\n" \
       "build/make/cppflags.mk:${FILES}" \
       "	\$(E_GEN)cat${FILES} > \$@" \
       ""

CRULES="-Wfails -Wall -Wextra -Wc++-compat -Wcast-qual -Wmissing-prototypes -Wold-style-definition -Wpointer-arith -Wshadow -Wstrict-prototypes -Wwrite-strings -Wimplicit-function-declaration -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wjump-misses-init -Wdouble-promotion -Wformat=2 -Wa,--noexecstack -fno-asynchronous-unwind-tables -fno-exceptions -fno-unwind-tables -fomit-frame-pointer -fvisibility=hidden -fpie -pipe /nologo"
FILES=
for i in ${CRULES}; do
    cflag_rule $i
done
printf "%s\n" \
       "build/make/cflags.mk:${FILES}" \
       "	\$(E_GEN)cat${FILES} > \$@" \
       ""
