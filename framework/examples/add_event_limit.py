#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Create the payload with an object of EventLimit class for use in EventLimiter
module. The payload interval of validity is set equal to the specified experiment
and run; payload file is created in localdb directory.
"""

from ROOT import Belle2
import argparse

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('exp',   type=int, help='Experiment number')
parser.add_argument('run',   type=int, help='Run number')
parser.add_argument('limit', type=int, help='Event limit')
args = parser.parse_args()

DBOBJECT_NAME = 'EventLimit'

# Payload creation code
payload = Belle2.EventLimit(args.limit)

db = Belle2.Database.Instance()
iov = Belle2.IntervalOfValidity(args.exp, args.run, args.exp, args.run)
db.storeData(DBOBJECT_NAME, payload, iov)
