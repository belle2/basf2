#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2


if __name__ == "__main__":
    path = basf2.Path()
    path.add_module("ReceiveEvent", Host="erctl", Port=4002)

    # path.add_module("PrintCollections", printForEvent=0)
    path.add_module("EventInfoPrinter")

    basf2.process(path)
