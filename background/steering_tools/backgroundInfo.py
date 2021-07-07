#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2

# -----------------------------------------------------------
# Prints BackgroundInfo stored in the root file
# Use -i switch to specify the input file
# -----------------------------------------------------------


class printBGInfo(b2.Module):

    '''
    Print BackgroundInfo stored in the root file
    '''

    def event(self):
        ''' event function '''

        bgInfo = Belle2.PyStoreObj('BackgroundInfo', 1)    # new version
        bgInfos = Belle2.PyStoreArray('BackgroundInfos', 1)  # old version
        if bgInfo.isValid():
            bgInfo.print()
        elif bgInfos.isValid():
            if bgInfos.getEntries() == 0:
                print("Background info is empty")
            i = 0
            for bgInfo in bgInfos:
                print("===========================")
                print("* Backgroud info: entry", str(i))
                print("===========================")
                bgInfo.print()
                i += 1
        else:
            print("No background info available")

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        evtMetaData.obj().setEndOfData()


b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('RootInput')
main.add_module(roinput)

# print info
main.add_module(printBGInfo())

b2.process(main)
