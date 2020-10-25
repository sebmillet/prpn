#!/usr/bin/bash

# Compile files in winpkg with NSIS (NSIS is fed with prpn.nsi), so as to
# produce installer executable for Windows.

# Copyright 2020 Sébastien Millet

set -euo pipefail

WP=winpkg
NSI_FILE=prpn.nsi

NSIS_EXE="/c/Program Files (x86)/NSIS/makensis.exe"

if [ ! -f "${WP}/${NSI_FILE}" ]; then
	echo "File '${WP}/${NSI_FILE}' not found. Aborted."
	exit 1
fi

cd "${WP}"

"${NSIS_EXE}" "${NSI_FILE}"

