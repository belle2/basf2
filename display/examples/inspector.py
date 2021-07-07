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

# this loads gui libraries


class InspectorModule(b2.Module):

    """Inspect DataStore contents"""

    #: InfoWidget pointer
    info = None

    def initialize(self):
        """reimplementation of Module::initialize()."""
        from ROOT import gClient
        from ROOT import gSystem
        gSystem.Load('libdisplay')
        from ROOT import Belle2

        root = gClient.GetRoot()
        self.info = Belle2.InfoWidget(root)
        self.info.MapSubwindows()
        self.info.Resize()
        self.info.MapWindow()

    def event(self):
        """reimplementation of Module::event()."""

        self.info.newEvent()

        print("Press Ctrl+D to exit.")
        import interactive
        interactive.embed()


main = b2.create_path()
main.add_module('RootInput')
main.add_module(InspectorModule())

b2.process(main)
