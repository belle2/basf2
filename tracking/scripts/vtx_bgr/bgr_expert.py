##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


"""
Script to apply the bgr expert on independent test data. The weightfile is expected to be in a localdb produced
by bgr_teacher.py. The test data is expected to be in a root file named test.root produced by collect_data.py.

Usage:

python3 bgr_expert.py
"""

import basf2_mva
from basf2 import conditions


# NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
conditions.testing_payloads = [
    'localdb/database.txt'
]

basf2_mva.expert(basf2_mva.vector('vxdtf2_vtx_bgr_mva'),
                 basf2_mva.vector('test.root'),
                 'tree', 'expert.root')
