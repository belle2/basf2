#!/usr/bin/env python
# -*- coding: utf-8 -*-

# example script to print FileMetaData for a given file
# Usage: basf2 framework/examples/filemetadata.py -i inputfile.root
# (Hint: for inspecting file metadata it is recommended to use the showmetadata tool)

from basf2 import *
from ROOT import Belle2


class PrintFileMetaData(Module):

    """Print FileMetaData object."""

    def initialize(self):
        """reimplementation of Module::initialize()."""

        metadata = Belle2.PyStoreObj('FileMetaData', 1)
        if not metadata:
            B2ERROR('No meta data found')
        else:
            obj = metadata.obj()
            obj.Dump()


# create main path
main = create_path()

rootinput = register_module('RootInput')
main.add_module(rootinput)
main.add_module(PrintFileMetaData())

process(main, 1)  # stop after 1 event
