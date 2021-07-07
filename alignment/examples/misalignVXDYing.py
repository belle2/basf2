#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2

payload = Belle2.VXDAlignment()

ying = Belle2.VxdID(1, 0, 0, 1).getID()
yang = Belle2.VxdID(1, 0, 0, 2).getID()
pat = Belle2.VxdID(3, 0, 0, 1).getID()
mat = Belle2.VxdID(3, 0, 0, 2).getID()

x = 1
y = 2
z = 3

payload.setGlobalParam(0.01, ying, y)

iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
Belle2.Database.Instance().storeData('VXDAlignment', payload, iov)
