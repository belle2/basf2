#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *

# this loads gui libraries
from ROOT import TGClient


class InspectorModule(Module):

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


main = create_path()
main.add_module('RootInput')
main.add_module(InspectorModule())

process(main)
