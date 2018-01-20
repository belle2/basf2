#!/usr/bin/env python3
# -*- coding: utf-8 -*-


##############################################################################
#
# Imports PXD calibration payloads
# Some PXD calibration constants are currently stored in numpy files
# or binary files with pickled python objects.
#
# This example reads these constants, fills dbobjects and imports them
# into the conditions db.
#
##############################################################################

from basf2 import *
from ROOT import Belle2

offset = Belle2.PXDClusterOffsetPar()
offset.setU(-1.0)

print("offset ", offset.getU())

iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
Belle2.Database.Instance().storeData('PXDClusterOffsetPar', offset, iov)
