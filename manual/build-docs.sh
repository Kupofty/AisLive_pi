#!/bin/bash

set -e  # stop script on first error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # reset

echo -e "${BLUE}[1/2] Building Antora documentation...${NC}"
antora site.yml

echo -e "${BLUE}[2/2] Ensuring GitHub Pages compatibility (add .nojekyll)...${NC}"
touch ../docs/.nojekyll

echo -e "${GREEN}Build completed successfully${NC}"
echo "----------------------------------------"