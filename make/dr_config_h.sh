# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

config_rule() {
    FILES="${FILES} build/include/$1.h"
    printf "%s\n" \
	   "build/include/$1.h: build/make/dr_config.mk \$(PROJROOT)config/checks/$1.c" \
	   "	\$(E_GEN) \\" \
	   "if ! (cd build/make_obj && \\" \
	   "        \$(CC) \$(CSTD) \$(CPPFLAGS) \$(CFLAGS) \$(LDFLAGS) ../../\$(PROJROOT)config/checks/$1.c \$(OUTPUT_L)$1\$(EEXT) || \\" \
	   "        echo error) 2>&1 | egrep -i 'error|warn' > /dev/null; then \\" \
	   "    echo '#define $1 1'; \\" \
	   "else \\" \
	   "    echo '/* #undef $1 */'; \\" \
	   "fi > \$@" \
	   ""
}

FILES=

printf "%s\n" \
       "# SPDX-License-Identifier: GPL-2.0-only" \
       "# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>" \
       ""

for i in $(cd ${PROJROOT}config/checks && ls *.c); do
    config_rule ${i%.c}
done

printf "%s\n" \
       "build/include/dr_config.h:${FILES}" \
       "	\$(E_GEN) \\" \
       "( \\" \
       "    echo '#if !defined(DR_CONFIG_H)'; \\" \
       "    echo '#define DR_CONFIG_H'; \\" \
       "    cat ${FILES}; \\" \
       "    echo '#endif // DR_CONFIG_H'\\" \
       ") > \$@" \
       ""
