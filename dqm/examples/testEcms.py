#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from basf2 import create_path, register_module
from analysisDQM import get_hadB_path


rec_path_1 = create_path()

rec_path_1.add_module('RootInput')

get_hadB_path(rec_path_1)

histo = register_module("HistoManager")
rec_path_1.add_module(histo)
histo.param('histoFileName', 'outHist.root')

basf2.process(rec_path_1)
