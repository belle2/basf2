#!/bin/bash

# A standalone script to downgrade NumPy if needed.
# Usage: run this AFTER a Python script failed due to NumPy 1.x/2.x mismatch.

echo "=== Checking installed NumPy version ==="

PYTHON_CMD=${PYTHON_CMD:-python3}  # Use $PYTHON_CMD if set, else default to python3

NUMPY_VERSION=$($PYTHON_CMD -c "import numpy; print(numpy.__version__)" 2>/dev/null || echo "not_installed")

if [[ "$NUMPY_VERSION" == "not_installed" ]]; then
    echo "NumPy is not installed in this environment. Nothing to do."
    exit 0
fi

MAJOR_VERSION=$(echo "$NUMPY_VERSION" | cut -d. -f1)

if [[ "$MAJOR_VERSION" -lt 2 ]]; then
    echo "NumPy version $NUMPY_VERSION is already < 2. No downgrade needed."
    exit 0
fi

echo "NumPy version $NUMPY_VERSION is >= 2, which may cause crashes with compiled modules built against NumPy 1.x."
echo "Downgrading NumPy to the latest 1.x version..."

# Safe downgrade
pip install "numpy<2" --upgrade --force-reinstall --no-cache-dir

echo "Downgrade complete. Current NumPy version:"
$PYTHON_CMD -c "import numpy; print(numpy.__version__)"
echo ""
echo "You can now rerun your Python script safely."
