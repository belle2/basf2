#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Database Retriever.
Simple example for testing the SVD calibrations access.
"""

from basf2 import *
import ROOT

import os
import sys
import glob
import subprocess
from fnmatch import fnmatch
"""
If you want to use test beam data run 111, without magnetic field,
or run 400, with magnetic field, you should retrieve with this tag
Select the run by setting the EventInfoSetter param, runList
"""
# use_central_database("beamtest_vxd_april2017_rev1")
use_local_database("localDB/database.txt")

"""
For phase 3 related studies, use instead this tag
"""
# use_central_database("development")
# use_central_database("GT_gen_prod_004.05_prerelease-01-00-00b")

"""
using localDB for testing
"""
# use_local_database("localDB_run111/database_run111.txt", "localDB_run111")
# use_local_database("localDB_run400_toImport/database_run400_toImport.txt", "localDB_run400_toImport")
main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': [3], 'runList': [400]})
# eventinfosetter.param({'evtNumList': [1], 'expList' : [3], 'runList': [111]})
# eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Event info setter - execute single event
svddatabaseTest = register_module('SVDDatabaseTest')

main.add_module(svddatabaseTest)
print("svd database accessed")

# process single event
process(main)


print("done: FINISH!!!")
