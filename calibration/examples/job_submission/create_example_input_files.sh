#!/bin/bash
EXAMPLE_DIR=test_data
mkdir -p ${EXAMPLE_DIR}
# This creates cosmic track events usually used for VXD alignment (they are nice and quick to
# generate) for a few runs.
basf2 ../generate_test_data.py $EXAMPLE_DIR 0 1 1 500
basf2 ../generate_test_data.py $EXAMPLE_DIR 0 2 2 500
basf2 ../generate_test_data.py $EXAMPLE_DIR 0 3 3 500
basf2 ../generate_test_data.py $EXAMPLE_DIR 0 4 4 500
basf2 ../generate_test_data.py $EXAMPLE_DIR 0 5 6 500
basf2 ../generate_test_data.py $EXAMPLE_DIR 0 7 8 500
basf2 ../generate_test_data.py $EXAMPLE_DIR 0 10 10 500
basf2 ../generate_test_data.py $EXAMPLE_DIR 0 11 11 500
