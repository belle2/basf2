#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from pybasf2 import *

# -----------------------------------------------
#             Set basf2 information
# -----------------------------------------------
basf2label = 'BASF2 (Belle Analysis Framework 2)'
basf2version = os.environ.get('BELLE2_RELEASE', 'unknown')
basf2copyright = 'Copyright(C) 2010 - Belle II Collaboration'
basf2authors = [
    'Andreas Moll',
    'Martin Heck',
    'Thomas Kuhr',
    'Ryosuke Itoh',
    'Martin Ritter',
    'Susanne Koblitz',
    'Nobu Katayama',
    'Kolja Prothmann',
    'Zbynek Drasal',
    ]

# -----------------------------------------------
#               Prepare basf2
# -----------------------------------------------

# Check for environment variables set by the belle 2 release script
envarReleaseDir = os.environ.get('BELLE2_RELEASE_DIR', None)
envarLocalDir = os.environ.get('BELLE2_LOCAL_DIR', None)
if envarLocalDir is None:
    print """The environment variable BELLE2_LOCAL_DIR is not set. Please execute the 'setuprel' script first."""
    Exit(1)

envarSubDir = os.environ.get('BELLE2_SUBDIR', None)
if envarSubDir is None:
    print """The environment variable BELLE2_SUBDIR is not set. Please execute the 'setuprel' script first."""
    Exit(1)

# Get the architecture of the computer
unamelist = os.uname()
archstring = unamelist[0] + '_' + unamelist[4]

# Set basf2 directories
basf2dir = envarLocalDir  # basf2 directory
basf2moddir = os.path.join(basf2dir, 'modules', envarSubDir)  # basf2 module directory
basf2datadir = os.path.join(basf2dir, 'data')  # basf2 data directory

# -----------------------------------------------
#       Create default framework object
# -----------------------------------------------

# Create default framework
fw = Framework()

# Add the module search path pointing to the modules shipped with the framework
fw.add_module_search_path(basf2moddir)

# -----------------------------------------------
#         Load evtgen particle tables
# -----------------------------------------------
evtgen_loaded = fw.read_evtgen_table(os.path.join(envarLocalDir,
                                     'externals/evtgen/DecFiles/scripts/evt.pdl'
                                     ))
if evtgen_loaded is not True:
    evtgen_loaded = fw.read_evtgen_table(os.path.join(envarReleaseDir,
            'externals/evtgen/DecFiles/scripts/evt.pdl'))

if evtgen_loaded is not True:
    print """ERROR: Could not load the evtgen table file !"""

# -----------------------------------------------
#             Print output
# -----------------------------------------------
print '>>> basf2 Python environment set'
print '>>> Framework object created: fw'

