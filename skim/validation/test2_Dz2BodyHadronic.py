#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>./WG6_DstToD0ToKpi.udst.root</input>
  <output>./WG6_DstToD0ToKpi.ntup.root</output>
  <contact>lilongke@ihep.ac.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma

from variables import variables as vm

# create path
myAna_Main = b2.create_path()

# load input ROOT file
ma.inputMdst('default', '../WG6_DstToD0ToKpi.udst.root', path=myAna_Main)

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------
vm.addAlias('ps_Dst', 'useCMSFrame(p)')
vm.addAlias('M_D0', 'daughter(0,InvM)')
vm.addAlias('ps_D0', 'daughter(0,useCMSFrame(p))')
vm.addAlias('ps_spi', 'daughter(1,useCMSFrame(p))')

DstTree = ['Q', 'M_D0', 'ps_Dst', 'ps_D0', 'ps_spi']

ma.copyLists('D*+:sig', ['D*+:HpJm0', 'D*+:HpJm1', 'D*+:HpJm2'], writeOut=True, path=myAna_Main)

ma.variablesToNtuple(filename='WG6_DstToD0ToKpi.ntup.root',
                     decayString='D*+:sig', treename='hrec',
                     variables=DstTree, path=myAna_Main)

# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------
myAna_Main.add_module('Progress')

b2.process(myAna_Main)

print(b2.statistics)
