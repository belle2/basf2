#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import signal
from pybasf2 import *

# -----------------------------------------------
#             Set basf2 information
# -----------------------------------------------
basf2label = 'BASF2 (Belle Analysis Framework 2)'
basf2version = os.environ.get('BELLE2_RELEASE', 'unknown')
basf2copyright = 'Copyright(C) 2010-2013  Belle II Collaboration'
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
    'Christian Pulvermacher'
    ]

# -----------------------------------------------
#               Prepare basf2
# -----------------------------------------------

# Reset the signal handler to allow the framework execution
# to be stopped with Ctrl-c
signal.signal(signal.SIGINT, signal.SIG_DFL)

# Check for environment variables set by the belle 2 release script
envarReleaseDir = os.environ.get('BELLE2_RELEASE_DIR', None)
envarLocalDir = os.environ.get('BELLE2_LOCAL_DIR', None)
if not envarReleaseDir and not envarLocalDir:
    print """The basf2 environment is not set up.
             Please execute the 'setuprel' script first."""
    Exit(1)

envarSubDir = os.environ.get('BELLE2_SUBDIR', None)
if envarSubDir is None:
    print """The environment variable BELLE2_SUBDIR is not set.
             Please execute the 'setuprel' script first."""
    Exit(1)

envarExtDir = os.environ.get('BELLE2_EXTERNALS_DIR', None)
if envarExtDir is None:
    print """The environment variable BELLE2_EXTERNALS_DIR is not set.
             Please execute the 'setuprel' script first."""
    Exit(1)

# -----------------------------------------------
#       Create default framework object
# -----------------------------------------------

# Create default framework
fw = Framework()

# Add the module search path pointing to the modules shipped with the framework
basf2moddir = []
if envarLocalDir:
    basf2moddir.append(os.path.join(envarLocalDir, 'modules', envarSubDir))
if envarReleaseDir:
    basf2moddir.append(os.path.join(envarReleaseDir, 'modules', envarSubDir))

for moddir in basf2moddir:
    fw.add_module_search_path(moddir)

# Sets the path in which the externals of the framework are located
fw.set_externals_path(envarExtDir)

# -----------------------------------------------
#             Print output
# -----------------------------------------------
print '>>> basf2 Python environment set'
print '>>> Framework object created: fw'
