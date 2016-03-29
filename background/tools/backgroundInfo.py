#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

# -----------------------------------------------------------
# Prints BackgroundInfo stored in the root file
# Use -i switch to specify the input file
# -----------------------------------------------------------


class printBGInfo(Module):

    '''
    Print BackgroundInfo stored in the root file
    '''

    def event(self):
        ''' event function '''

        bgInfo = Belle2.PyStoreObj('BackgroundInfo', 1)
        if bgInfo.isValid():
            print("             ==================")
            print("             * Backgroud info *")
            print("             ==================")
            print()
            bgInfo.obj().print()
        else:
            print("No background info available")

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        evtMetaData.obj().setEndOfData()


set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# input
roinput = register_module('RootInput')
main.add_module(roinput)

# print info
main.add_module(printBGInfo())

process(main)
