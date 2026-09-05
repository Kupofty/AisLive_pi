#!/bin/bash

# ==============================================================
# Cleanup Script for gettext artifacts
# ==============================================================

# ---------- ANSI COLORS ----------
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m'

# ---------- LOG HELPERS ----------
info() { echo -e "${CYAN}[INFO]${NC}  $1"; }
step() { echo -e "${YELLOW}[STEP]${NC}  $1"; }
ok()   { echo -e "${GREEN}[OK]${NC}    $1"; }
fail() { echo -e "${RED}[FAIL]${NC}  $1"; exit 1; }

# --------------------------------------------------------------
# STEP 0 - Move to project root
# --------------------------------------------------------------
step "0/1 → Moving to project root"
cd ../.. || fail "Cannot move to project root"

PROJECT_ROOT="$(pwd)"
ok "Project root: ${PROJECT_ROOT}"


# --------------------------------------------------------------
# STEP 1 - Remove gettext artifacts (*.pot, *.in)
#
# This removes:
# - all translation template files (*.pot)
# - all gettext input files (*.in)
# inside the po/ directory
# --------------------------------------------------------------
step "1/1 → Cleaning gettext artifacts (*.pot, *.in)"

DELETED=0

while IFS= read -r file; do
    rm -f "$file"
    ok "Deleted: $file"
    ((DELETED++))
done < <(find po -type f \( -name "*.pot" -o -name "*.in" \))

if [[ $DELETED -eq 0 ]]; then
    info "No gettext artifacts found"
else
    ok "Total removed: $DELETED file(s)"
fi


echo -e "${GREEN}"
echo "=============================================================="
echo " Cleanup completed"
echo " Target: po/*.pot and po/*.in"
echo "=============================================================="
echo -e "${NC}"