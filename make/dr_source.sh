# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

GIT=$(which git 2> /dev/null || true)
SOURCE_REVISION_DEFAULT=unknown
SOURCE_DATE_DATE_CMD="date -u +%Y-%m-%dT%H:%M:%S+00:00"
if [ "${GIT}x" = "x" -o ! -f "${GIT}" ]; then
    SOURCE_REVISION=${SOURCE_REVISION_DEFAULT}
    SOURCE_STATUS=dirty
    SOURCE_DATE=$(${SOURCE_DATE_DATE_CMD})
else
    SOURCE_REVISION=$(git rev-parse HEAD 2> /dev/null || echo ${SOURCE_REVISION_DEFAULT})
    SOURCE_STATUS=$(git diff-index --quiet HEAD -- 2> /dev/null && echo clean || echo dirty)
    SOURCE_DATE=$(git --no-pager show -s --format=%cI 2> /dev/null || ${SOURCE_DATE_DATE_CMD})
fi
