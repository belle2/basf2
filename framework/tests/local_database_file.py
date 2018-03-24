#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
test parsing of local database files
"""

from basf2 import logging, LogLevel, LogInfo, reset_database, use_local_database
from conditions_db.cli_upload import parse_database_file
from b2test_utils import clean_working_directory


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

with clean_working_directory():
    for i, entry in enumerate(entries.splitlines(True)):
        print("testing", repr(entry))
        filename = "database-%02d.txt" % i
        logging.log_level = LogLevel.ERROR
        with open(filename, "w") as f:
            f.write(entry)
        reset_database()
        logging.log_level = LogLevel.DEBUG
        logging.debug_level = 200
        use_local_database(filename)
        entries = parse_database_file(filename, check_existing=False)
        if entries:
            print(entries)
