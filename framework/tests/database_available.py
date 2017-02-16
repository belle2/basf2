#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Check if the central database is accessible. It's meant as an indication that
other tests might fail due to missing database connection."""

from basf2 import *
from conditions_db import ConditionsDB

print("TEST SKIPPED: Frequent failures", file=sys.stderr)
sys.exit(1)

# make output a bit less verbose in case of errors
logging.set_info(LogLevel.ERROR, LogInfo.LEVEL | LogInfo.MESSAGE)
logging.set_info(LogLevel.FATAL, LogInfo.LEVEL | LogInfo.MESSAGE)

B2INFO("Checking availability of central database. If this test fails other tests might fail as well")
db = ConditionsDB()
if db.get_globalTags() is None:
    sys.exit(1)
