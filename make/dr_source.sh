# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

GIT=$(which git 2> /dev/null || true)
if [ "${GIT}x" = "x" -o ! -f "${GIT}" ]; then
    SOURCE_REVISION=unknown
    SOURCE_STATUS=dirty
    SOURCE_DATE=$(date -u "+%Y-%m-%dT%H:%M:%S+00:00")
else
    SOURCE_REVISION=$(git rev-parse HEAD)
    SOURCE_STATUS=$(git diff-index --quiet HEAD -- && echo clean || echo dirty)
    SOURCE_DATE=$(git --no-pager show -s --format=%cI)
fi
