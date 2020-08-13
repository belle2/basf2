#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
You can run ``basf2 variables.py`` to list all available variables.
"""
from terminal_utils import Pager
import argparse
from variables import getCommandLineOptions
from variables import printVars

if __name__ == "__main__":
    args = getCommandLineOptions()

    warning = (
        "The following variables have been renamed since release-04:\n"
        "old: mcFlavorOfOtherB0 --> new: mcFlavorOfOtherB\n"
        "old: mdstSource --> new: uniqueParticleIdentifier\n"
        "old: MCDeltaT --> new: mcDeltaT\n"
        "old: mc{X, Y, Z} --> new: mcDecayVertex{X, Y, Z}\n"
        "old: mcRho --> new: mcDecayVertexRho\n"
        "old: mcProdVertex{X, Y, Z} --> new: mcProductionVertex{X, Y, Z}\n"
        "old: mcProdVertexD{X, Y, Z} --> new: mcProductionVertexFromIP{X, Y ,Z}\n"
        "old: mcDistance --> new: mcDecayVertexFromIPDistance\n"
        "old: WE_XXX, ROE_XXX, Rec_XXX --> new: weXXX, roeXXX, recXXX\n\n"
        "The variables 'decayAngle', 'daughterAngle', and 'pointingAngle' return angles in rad and no longer the corresponding "
        "cosine!\n"

        "The following variables have been renamed since release-05:\n"
        "old: TagVmcLBoost --> new: mcTagVLBoost\n"
        "old: TagVmcOBoost --> new: mcTagVOBoost\n"
        "old: mcDeltaL --> new: mcDeltaBoost\n"
    )
    if args.pager:
        with Pager(r'Available variables in Variable\:\:Manager'):
            printVars(warning)
    else:
        printVars(warning)
