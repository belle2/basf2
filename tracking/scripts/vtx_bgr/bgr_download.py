##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Script to download a weightfile and writes a file weightfile.root into current dir. Here the weightfile
is expected to to be a localdb produced by the script bgr_teacher.py.

Usage:

python3 bgr_download.py
"""


# Usage: python3 bgr_download.py
import basf2_mva
from basf2 import conditions


# NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
conditions.testing_payloads = [
    'localdb/database.txt'
]

# Download the weightfile from the database and store it on disk in a root file
basf2_mva.download('vxdtf2_vtx_bgr_mva', 'weightfile.root')