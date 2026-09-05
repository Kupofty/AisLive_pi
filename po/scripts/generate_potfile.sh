#!/bin/bash

# ==============================================================
# Translation Template Generation Script
# ==============================================================

# --------------------------------------------------------------
# Author: Kupofty
#
# This script prepares the translation template (.pot) used by
# gettext-based internationalization.
#
# It performs two main tasks:
# 1. Build the list of source files containing translatable
#    strings (POTFILES.in).
# 2. Extract these strings and generate/update the POT file.
#
# Expected project structure:
#
# project/
# ├─ src/        → C/C++ source & → H/H++ header files
# └─ po/         → translation files (.po / .pot)
# ==============================================================


# ---------- ANSI COLORS ----------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'


# ---------- LOG HELPERS ----------
info() { echo -e "${CYAN}[INFO]${NC}  $1"; }
step() { echo -e "${YELLOW}[STEP]${NC}  $1"; }
ok()   { echo -e "${GREEN}[OK]${NC}    $1"; }
fail() { echo -e "${RED}[FAIL]${NC}  $1"; exit 1; }


# --------------------------------------------------------------
# STEP 0 - Move to project root directory
#
# The script must be executed from po/scripts folder.
# We move to the repository root to ensure relative paths work.
# --------------------------------------------------------------
step "0/3 → Moving to project root directory"
cd ../.. || fail "Cannot move to project root"
ok "Project root: $(pwd)"


# --------------------------------------------------------------
# STEP 1: Generate the list of source files to scan
#
# POTFILES.in is used by gettext tools to know which files
# contain translatable strings.
#
# This command recursively searches the src/ directory for:
# - .cpp  (C++ source files)
# - .h    (header files)
# - .hpp  (C++ header files)
#
# The resulting file list is written to:
# po/POTFILES.in
# --------------------------------------------------------------
step "1/3 → Generating POTFILES.in"

find src \
    \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
    | sort > po/POTFILES.in

[[ -s po/POTFILES.in ]] || fail "POTFILES.in generation failed or empty"
ok "POTFILES.in generated successfully"


# --------------------------------------------------------------
# STEP 2: Generate or update the translation template (.pot)
#
# xgettext scans all files listed in POTFILES.in and extracts
# strings marked for translation using the _() macro.
#
# Important options:
#
# --add-comments=TRANSLATORS
# Include developer comments starting with "TRANSLATORS:"
# in the generated POT file to help translators.
#
# --force-po
# Always generate the output file even if no strings are found.
#
# --from-code=UTF-8
# Specifies the encoding used in the source files.
#
# --keyword=_
# Indicates that translatable strings are wrapped using _()
#
# --width=80
# Sets the maximum line width in the generated file.
#
# --files-from
# Provides the list of files to scan.
#
# Output:
# po/template_pi.pot
#
# This file is the master translation template used to update
# language-specific .po files.
# --------------------------------------------------------------
step "2/3 → Generating template_pi.pot"

xgettext \
    --add-comments=TRANSLATORS \
    --force-po \
    --from-code=UTF-8 \
    --output=po/template_pi.pot \
    --keyword=_ \
    --width=80 \
    --files-from=po/POTFILES.in

ok "POT file generated successfully"


# --------------------------------------------------------------
# STEP 3: Update template_pi.pot header
#
# Replace default xgettext header with custom project header
# while preserving extracted translation entries.
# --------------------------------------------------------------
step "3/3 → Updating POT header"

{
cat <<'EOF'
# Translation of Template plugin
# Copyright (C) 2026 Kupofty
# This file is distributed under the same license as the template_pi package.
EOF

tail -n +7 po/template_pi.pot
} > po/template_pi.pot.tmp && mv po/template_pi.pot.tmp po/template_pi.pot

ok "POT header updated"


# --------------------------------------------------------------
# DONE
# --------------------------------------------------------------
echo -e "${GREEN}"
echo "=============================================================="
echo " Translation template generation completed"
echo " Output: po/template_pi.pot"
echo "=============================================================="
echo -e "${NC}"