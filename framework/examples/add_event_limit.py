#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2

"""
Create the payload with the object of EventLimit class.
This payload is used in EventLimiter module
"""

FIRST_EXP = 0
FIRST_RUN = 0
LAST_EXP = -1
LAST_RUN = -1
DBOBJECT_NAME = 'EventLimit'

EVENT_LIMIT = 100


def main():
    """Payload creation code
    """
    payload = Belle2.EventLimit(EVENT_LIMIT)

    db = Belle2.Database.Instance()
    iov = Belle2.IntervalOfValidity(FIRST_EXP, FIRST_RUN, LAST_EXP, LAST_RUN)
    db.storeData(DBOBJECT_NAME, payload, iov)


if __name__ == '__main__':
    main()
