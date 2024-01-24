#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>../1110021010.dst.root</input>
  <output>1110021010_Validation.root</output>
  <contact>Frank Meier; frank.meier@duke.edu</contact>
</header>
"""
# Runs a simple analysis on signal [B0 -> rho0 gamma] events. Saves histograms
# in a format for the validation server. There are better example analysis
# scripts in analysis/examples and analysis/examples/tutorials.

import basf2
from modularAnalysis import inputMdst, reconstructDecay, matchMCTruth
from stdPhotons import stdPhotons
from stdCharged import stdPi
from validation_tools.metadata import create_validation_histograms

DECAY_MODE_ID = 1110021010
INPUT_FILENAME = f"../{DECAY_MODE_ID}.dst.root"
OUTPUT_FILENAME = f"{DECAY_MODE_ID}_Validation.root"

main = basf2.Path()
inputMdst(INPUT_FILENAME, path=main)

stdPi('loose', path=main)
stdPhotons('loose', path=main)

reconstructDecay('rho0:pipi -> pi+:loose pi-:loose', '0.35 < M < 1.2', path=main)
matchMCTruth('rho0:pipi', path=main)

reconstructDecay('B0 -> rho0:pipi gamma:loose', '4.8 < M < 5.8', path=main)
matchMCTruth('B0', path=main)

create_validation_histograms(
    main, OUTPUT_FILENAME, "B0",
    variables_1d=[
        (
            "Mbc", 180, 5.2, 5.29,
            "Beam-constrained mass of #it{B^{0} #rightarrow #rho^{0}#gamma} candidates",
            "Frank Meier <frank.meier@duke.edu>",
            r"The beam-constrained mass distribution of $B^0\to\rho^0\gamma$ decays",
            "Distribution should be peaking at the nominal $B^0$ mass. Tail towards low mass.",
            "M_{bc} [GeV/c^{2}]", "Candidates"
        ),
        (
            "useRestFrame(daughter(1, E))", 50, 1.5, 4.0,
            "The photon energy distribution for #it{B^{0} #rightarrow #rho^{0}#gamma} decays",
            "Frank Meier <frank.meier@duke.edu>; Torben Ferber <torben.ferber@desy.de>",
            r"Photon energy distribution of $B^0\to\rho^0\gamma$ decays",
            "Sharp distribution at 2.5 GeV. Look for differences in "
            "the absolute scale. This could be an indication of a loss of photon efficiency.",
            "E_{#gamma} [GeV]", "Candidates"
        ),
    ]
)

main.add_module('Progress')
basf2.process(main)
print(basf2.statistics)
