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

payload = Belle2.PXDDHHFirmwareVersionPar()

# payload.setDHHFirmwareVersion(1)  # 1 is 2019ab
# reserve 2-9 for some intermediate versions with different behaviour
payload.setDHHFirmwareVersion(10)  # 10 is default in 2019c, 2020ab

iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
Belle2.Database.Instance().storeData('PXDDHHFirmwareVersion', payload, iov)
