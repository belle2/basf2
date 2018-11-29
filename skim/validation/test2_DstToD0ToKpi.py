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
# Create and fill final state ParticleLists
# --------------------------------------------------
# loadStdCharged(path=myAna_Main)
# good: PID(pi)>0.5 + goodTrk (thetaInCDCAcceptance and chiProb > 0.001 + d0 < 0.5 and abs(z0) < 2 )
stdK("good", path=myAna_Main)
stdPi('good', path=myAna_Main)
# loose: PID(pi)>0.1 + goodTrk
stdPi('all', path=myAna_Main)
cutAndCopyLists('pi+:slow', 'pi+:all', 'p < 0.5', path=myAna_Main)

# reconstruct D0:kpi and perform a vertex fit
reconstructDecay('D0:kpi -> K-:good pi+:good',
                 cut='useCMSFrame(p) > 2.2 and 1.81 < M < 1.92', path=myAna_Main)
vertexKFit(list_name='D0:kpi', conf_level=0.001, path=myAna_Main)

# reconstruct the D*+ from the D0:kpi and pi+:all
reconstructDecay('D*+:sig -> D0:kpi pi+:slow', '0 < Q < 0.018', path=myAna_Main)
vertexKFit(list_name='D*+:sig', conf_level=0.001, constraint='ipprofile', path=myAna_Main)
# vertexKFit(list_name='D*+:sig', conf_level=0.001, path=myAna_Main)

applyCuts('D*+:sig', '0.0 < Q < 0.015', path=myAna_Main)
matchMCTruth('D*+:sig', path=myAna_Main)

# only save truth-matched signals
applyCuts('D*+:sig', 'isSignal==1', path=myAna_Main)

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------
variables.addAlias('ps_Dst', 'useCMSFrame(p)')
variables.addAlias('M_D0', 'daughter(0,InvM)')
variables.addAlias('ps_D0', 'daughter(0,useCMSFrame(p))')
variables.addAlias('ps_spi', 'daughter(1,useCMSFrame(p))')
variables.addAlias('chiProb_D', 'daughter(0, chiProb)')

DstTree = ['Q', 'M_D0', 'ps_Dst', 'ps_D0', 'ps_spi', 'chiProb_D', 'chiProb']

variablesToNtuple(filename='WG6_DstToD0ToKpi.ntup.root',
                  decayString='D*+:sig', treename='hrec',
                  variables=DstTree, path=myAna_Main)

# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------
myAna_Main.add_module('Progress')

process(myAna_Main)

print(statistics)
