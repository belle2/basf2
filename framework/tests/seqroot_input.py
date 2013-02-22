#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Test SeqRootInputModule. The input file is created by seqroot_output.py

from basf2 import *

import os
import shutil

from ROOT import Belle2


class TestModule(Module):

    """Test to read relations."""

    def __init__(self):
        """constructor."""

        super(TestModule, self).__init__()
        self.setName('TestModule')

    def event(self):
        """reimplementation of Module::event().

        access all relations from/to MCParticles,
        any invalid indices should be caught.
        """

        mcparticles = Belle2.PyStoreArray('MCParticles')
        # this will generate an index internally, checking consistency
        from_relations = mcparticles[0].getRelationsFrom("ALL")
        to_relations = mcparticles[0].getRelationsTo("ALL")


# copy input file into current dir to avoid having the full path in .out file
try:
    shutil.copy(os.getenv('BELLE2_LOCAL_DIR')
                + '/framework/tests/seqroot_input.sroot', '.')
except:
    pass  # we're probably in tests/ directory, no copy necessary

# ============================================================================
# Now lets create the necessary modules to perform a simulation

# Create Event information
# Show progress of processing
progress = register_module('Progress')
# Set input filename
input = register_module('SeqRootInput')
input.param('inputFileName', 'seqroot_input.sroot')
input.logging.log_level = LogLevel.WARNING  # ignore read rate

# ============================================================================
# Do the simulation

main = create_path()
# init path
main.add_module(input)
main.add_module(register_module('PrintCollections'))
main.add_module(progress)
main.add_module(TestModule())

process(main)
