#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>./WG6_DstToD0ToKpi.udst.root</input>
  <output>./WG6_DstToD0ToKpi.ntup.root</output>
  <contact>lilongke@ihep.ac.cn</contact>
</header>
"""

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *

from stdCharged import *
from variables import variables

# create path
myAna_Main = create_path()

# load input ROOT file
inputMdst('default', './WG6_DstToD0ToKpi.udst.root', path=myAna_Main)

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------
variables.addAlias('ps_Dst', 'useCMSFrame(p)')
variables.addAlias('M_D0', 'daughter(0,InvM)')
variables.addAlias('ps_D0', 'daughter(0,useCMSFrame(p))')
variables.addAlias('ps_spi', 'daughter(1,useCMSFrame(p))')

DstTree = ['Q', 'M_D0', 'ps_Dst', 'ps_D0', 'ps_spi']

variablesToNtuple(filename='WG6_DstToD0ToKpi.ntup.root',
                  decayString='D*+:sig', treename='hrec',
                  variables=DstTree, path=myAna_Main)

# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------
myAna_Main.add_module('Progress')

process(myAna_Main)

print(statistics)
