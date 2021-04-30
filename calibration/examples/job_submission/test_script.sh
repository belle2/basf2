#!/bin/bash

OUTPUT_FILE="example.csv"
echo "This is a test script. It should create '${OUTPUT_FILE}'"
cat >> $OUTPUT_FILE << EOF
"argument 1","$1"
"argument 2","$2"
EOF
