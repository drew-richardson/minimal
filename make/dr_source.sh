# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

GIT=$(which git 2> /dev/null || true)
if [ "${GIT}x" != "x" -a -f "${GIT}" ] && git rev-parse HEAD > /dev/null 2>&1; then
    SOURCE_REVISION=$(git rev-parse HEAD)
    if [ "$(git status --porcelain)x" = "x" ]; then
	SOURCE_STATUS=clean
    else
	SOURCE_STATUS=dirty
    fi
    SOURCE_DATE=$(git --no-pager show -s --format=%cI 2> /dev/null)
else
    SOURCE_REVISION=unknown
    SOURCE_STATUS=dirty
    SOURCE_DATE=$(date -u +%Y-%m-%dT%H:%M:%S+00:00)
fi
printf "%s\n" \
       "#include \"dr.h\"" \
       "const char *dr_source_revision(void) { return \"${SOURCE_REVISION}\"; }" \
       "const char *dr_source_status(void) { return \"${SOURCE_STATUS}\"; }" \
       "const char *dr_source_date(void) { return \"${SOURCE_DATE}\"; }"
