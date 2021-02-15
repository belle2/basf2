#!/usr/bin/env python3

"""
test parsing of local database files
"""

from basf2 import logging, LogLevel, LogInfo
from conditions_db.testing_payloads import parse_testing_payloads_file
from b2test_utils import clean_working_directory, run_in_subprocess, configure_logging_for_tests
import ROOT

ROOT.gInterpreter.Declare("#include <framework/database/TestingPayloadStorage.h>")


entries = """
dbstore/empty 1 -1,-1,-1,-1
dbstore/good 1 0,0,-1,-1
dbstore/good_tabs\t1\t0,0,-1,-1
dbstore/good_many_spaces              1\t\t\t\t\t\t\t0,1,2,3
dbstore/good_with_comment 1 0,1,2,3 # comment
dbstore/bad_revision revision 0,0,0,0
dbstore/no_revision 0,1,2,3
no_package 1 0,0,-1,-1
dbstore/no_iov 1
dbstore/iov_only_1 1 0
dbstore/iov_only_2 1 0,0
dbstore/iov_only_3 1 0,0,0
dbstore/bad_with_spaces_02 1    0 ,1,2,3
dbstore/bad_with_spaces_11 1    0, 1,2,3
dbstore/bad_with_spaces_12 1    0,1 ,2,3
dbstore/bad_with_spaces_21 1    0,1, 2,3
dbstore/bad_with_spaces_22 1    0,1,2 ,3
dbstore/bad_with_spaces_31 1    0,1,2, 3
dbstore/bad_text_exp1 1 exp1,0,0,0
dbstore/bad_text_run1 1 0,run1,0,0
dbstore/bad_text_exp2 1 0,0,exp2,0
dbstore/bad_text_run2 1 0,0,0,run2
#only comment
    # and with whitespace
\t#including tabs
"""

# modify logging to remove the useless module: lines
for level in LogLevel.values.values():
    logging.set_info(level, LogInfo.LEVEL | LogInfo.MESSAGE)

logging.enable_summary(False)
logging.log_level = LogLevel.DEBUG
logging.debug_level = 39
evt = ROOT.Belle2.EventMetaData(0, 0, 0)
payload = ROOT.Belle2.Conditions.PayloadMetadata("test")

with clean_working_directory():
    configure_logging_for_tests()
    for i, entry in enumerate(entries.splitlines(True)):
        print("testing", repr(entry))
        filename = "database-%02d.txt" % i
        with open(filename, "w") as f:
            f.write(entry)

        # parse in python
        entries = parse_testing_payloads_file(filename, check_existing=False)
        if entries:
            print(entries)
        # and parse in C++
        storage = ROOT.Belle2.Conditions.TestingPayloadStorage(filename)
        run_in_subprocess(evt, payload, target=storage.get)
