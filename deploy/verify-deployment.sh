#!/bin/bash

# LAST BRIDGE - Deployment Verification Script
# Copyright (c) 2025 Fragillidae Software - Chuck Finch
# Licensed under MIT License

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}LAST BRIDGE - Deployment Verification${NC}"
echo -e "${BLUE}=====================================${NC}"
echo ""

# Navigate to project root
cd "$(dirname "$0")/.."

ERRORS=0

# Function to check file exists and is executable
check_executable() {
    local file="$1"
    local description="$2"
    
    if [[ -f "$file" ]]; then
        if [[ -x "$file" ]]; then
            echo -e "${GREEN}✓${NC} $description: $file"
        else
            echo -e "${YELLOW}⚠${NC} $description exists but not executable: $file"
            ((ERRORS++))
        fi
    else
        echo -e "${RED}✗${NC} $description missing: $file"
        ((ERRORS++))
    fi
}

# Function to check file exists
check_file() {
    local file="$1"
    local description="$2"
    
    if [[ -f "$file" ]]; then
        echo -e "${GREEN}✓${NC} $description: $file"
    else
        echo -e "${RED}✗${NC} $description missing: $file"
        ((ERRORS++))
    fi
}

# Function to check directory exists
check_directory() {
    local dir="$1"
    local description="$2"
    
    if [[ -d "$dir" ]]; then
        echo -e "${GREEN}✓${NC} $description: $dir"
    else
        echo -e "${RED}✗${NC} $description missing: $dir"
        ((ERRORS++))
    fi
}

echo -e "${BLUE}Checking project structure...${NC}"

# Check main project files
check_file "README.md" "Main README"
check_file "LICENSE" "License file"
check_file "Makefile" "Main Makefile"
check_file ".gitignore" "Git ignore file"
check_file "CONTRIBUTING.md" "Contributing guidelines"

# Check LAST application
echo ""
echo -e "${BLUE}Checking LAST application...${NC}"
check_directory "LAST" "LAST directory"
check_directory "LAST/src" "LAST source directory"
check_file "LAST/Makefile" "LAST Makefile"
check_file "LAST/src/main.c" "LAST main source"
check_file "LAST/src/common.h" "LAST common header"

# Check BRIDGE application
echo ""
echo -e "${BLUE}Checking BRIDGE application...${NC}"
check_directory "BRIDGE" "BRIDGE directory"
check_directory "BRIDGE/src" "BRIDGE source directory"
check_file "BRIDGE/Makefile" "BRIDGE Makefile"
check_file "BRIDGE/src/main.c" "BRIDGE main source"
check_file "BRIDGE/src/common.h" "BRIDGE common header"

# Check deployment files
echo ""
echo -e "${BLUE}Checking deployment files...${NC}"
check_directory "deploy" "Deployment directory"
check_executable "deploy/install.sh" "Installation script"
check_executable "deploy/uninstall.sh" "Uninstallation script"
check_executable "deploy/create-tarball.sh" "Tarball creation script"
check_file "deploy/README.md" "Deployment README"

# Check GitHub files
echo ""
echo -e "${BLUE}Checking GitHub repository files...${NC}"
check_directory ".github" "GitHub directory"
check_directory ".github/workflows" "GitHub workflows directory"
check_directory ".github/ISSUE_TEMPLATE" "GitHub issue templates directory"
check_file ".github/workflows/ci.yml" "CI/CD workflow"
check_file ".github/ISSUE_TEMPLATE/bug_report.md" "Bug report template"
check_file ".github/ISSUE_TEMPLATE/feature_request.md" "Feature request template"

# Test build system
echo ""
echo -e "${BLUE}Testing build system...${NC}"

# Check dependencies
echo "Checking build dependencies..."
if command -v gcc >/dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} GCC compiler found"
else
    echo -e "${RED}✗${NC} GCC compiler missing"
    ((ERRORS++))
fi

if command -v make >/dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} Make build system found"
else
    echo -e "${RED}✗${NC} Make build system missing"
    ((ERRORS++))
fi

if command -v pkg-config >/dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} pkg-config found"
else
    echo -e "${RED}✗${NC} pkg-config missing"
    ((ERRORS++))
fi

if pkg-config --exists gtk+-3.0 2>/dev/null; then
    echo -e "${GREEN}✓${NC} GTK3 development libraries found"
else
    echo -e "${YELLOW}⚠${NC} GTK3 development libraries missing (needed for build)"
fi

if command -v socat >/dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} socat utility found"
else
    echo -e "${YELLOW}⚠${NC} socat utility missing (needed for BRIDGE)"
fi

# Test clean build
echo ""
echo "Testing clean build..."
if make clean >/dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} Clean target works"
else
    echo -e "${RED}✗${NC} Clean target failed"
    ((ERRORS++))
fi

# Test dependency check
echo "Testing dependency check..."
if make check-deps >/dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} Dependency check works"
else
    echo -e "${YELLOW}⚠${NC} Dependency check failed (may be missing dependencies)"
fi

# Test build (if dependencies are available)
if pkg-config --exists gtk+-3.0 2>/dev/null; then
    echo "Testing build..."
    if make >/dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} Build successful"
        
        # Check if binaries were created
        if [[ -f "LAST/last" ]]; then
            echo -e "${GREEN}✓${NC} LAST binary created"
        else
            echo -e "${RED}✗${NC} LAST binary not created"
            ((ERRORS++))
        fi
        
        if [[ -f "BRIDGE/bridge" ]]; then
            echo -e "${GREEN}✓${NC} BRIDGE binary created"
        else
            echo -e "${RED}✗${NC} BRIDGE binary not created"
            ((ERRORS++))
        fi
    else
        echo -e "${RED}✗${NC} Build failed"
        ((ERRORS++))
    fi
else
    echo -e "${YELLOW}⚠${NC} Skipping build test (GTK3 dev libraries not available)"
fi

# Test deployment scripts
echo ""
echo -e "${BLUE}Testing deployment scripts...${NC}"

# Test tarball creation
if [[ -x "deploy/create-tarball.sh" ]]; then
    echo "Testing tarball creation..."
    if ./deploy/create-tarball.sh >/dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} Tarball creation works"
        if [[ -f "dist/last-bridge-1.0.0.tar.gz" ]]; then
            echo -e "${GREEN}✓${NC} Tarball file created"
        fi
    else
        echo -e "${YELLOW}⚠${NC} Tarball creation failed (may need dependencies)"
    fi
fi

# Summary
echo ""
echo -e "${BLUE}Verification Summary${NC}"
echo "==================="

if [[ $ERRORS -eq 0 ]]; then
    echo -e "${GREEN}✓ All checks passed! Project is ready for GitHub deployment.${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Initialize git repository: git init"
    echo "2. Add files: git add ."
    echo "3. Commit: git commit -m 'Initial commit'"
    echo "4. Create GitHub repository"
    echo "5. Add remote: git remote add origin <repository-url>"
    echo "6. Push: git push -u origin main"
else
    echo -e "${YELLOW}⚠ Found $ERRORS issues that should be addressed.${NC}"
    echo ""
    echo "Please fix the issues above before deploying to GitHub."
fi

exit $ERRORS
