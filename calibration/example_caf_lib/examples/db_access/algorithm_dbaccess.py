#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

import sys
if len(sys.argv) != 3:
    print("Usage: python3 algorithm_dbaccess.py <iteration num> <data directory>")
    sys.exit(1)

# If the iteration is 0 then a localdb is created but no DBObjects are accessed inside
# the algorithm
# If the iteration is >0 then the previous DBObjects are read and then new ones are created
iteration = int(sys.argv[1])
data_dir = sys.argv[2]

set_log_level(LogLevel.DEBUG)
# View the framework debugging
set_debug_level(100)
# For just the Algorithm debugging
# set_debug_level(29)

algo = Belle2.TestDBAccessAlgorithm()

import os
# Can use a Python list of input files/wildcards. It will resolve the existing files
inputFileNames = [os.path.join(data_dir, "CollectorOutput.root")]
algo.setInputFileNames(inputFileNames)

# We iterate over some runs and execute separately.
# This means that we repeatedly access the DB interface after committing
for i in range(1, 5):
    # We're doing this here to test what happens when resetting in a single Python process
    reset_database()
    use_database_chain()
    # The local db that we will both write to and read from
    use_local_database("localdb/database.txt",
                       directory="localdb",
                       readonly=False)
    print("Result of calibration =", algo.execute([(0, i)], iteration))
    algo.commit()
