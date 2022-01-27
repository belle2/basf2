##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


"""
Script to train the expert for detecting background tracks found by VXDTF2. The training data
is expected to be in a root file named train.root. It creates a localdb containing a FastBDT
weigthfile. This weightfile can either be used as temporary payload locally or must be uploaded
into a GT on the conditions data base.

Usage:

python3 bgr_teacher.py
"""


import var_set
import basf2_mva

go = basf2_mva.GeneralOptions()
go.m_datafiles = basf2_mva.vector('train.root')
go.m_treename = 'tree'
go.m_weight_variable = ""
go.m_identifier = 'vxdtf2_vtx_bgr_mva'
go.m_variables = basf2_mva.vector(*var_set.var_set)
go.m_target_variable = 'isSignal'

sp = basf2_mva.FastBDTOptions()

basf2_mva.teacher(go, sp)
