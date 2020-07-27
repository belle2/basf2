#!/usr/bin/env python3

import basf2
import modularAnalysis as ma
from variables import variables


outputfilename = 'B2CAL901-cDSTECLTRG'

main_path = basf2.create_path()

# TODO: A cdst input file is needed!

ma.inputMdstList(
    'default',
    '',
    path=main_path)
main_path.add_module('Progress')

# get ECL trigger information
main_path.add_module('ECLTRGInformation', logLevel=basf2.LogLevel.INFO, debugLevel=29)


def getL1Variables(n_trigs=80):
    listL1PSNM = []
    listL1FTDL = []
    listL1Prescales = []

    for ix in range(n_trigs):
        variables.addAlias('psnm_%i' % ix, 'L1PSNMBit(%i)' % ix)
        listL1PSNM.append('psnm_%i' % ix)

        variables.addAlias('ftdl_%i' % ix, 'L1FTDLBit(%i)' % ix)
        listL1FTDL.append('ftdl_%i' % ix)

        variables.addAlias('prescale_%i' % ix, 'L1PSNMBitPrescale(%i)' % ix)
        listL1Prescales.append('prescale_%i' % ix)

    return listL1PSNM, listL1FTDL, listL1Prescales


def getECLTRGVariables():
    listeclEnergyTC = []
    listeclEnergyTCECLCalDigit = []
    for ix in range(1, 577):
        variables.addAlias('tc_%i' % ix, 'eclEnergyTC(%i)' % ix)
        listeclEnergyTC.append('tc_%i' % ix)

        variables.addAlias('tceclcal_%i' % ix, 'eclEnergyTCECLCalDigit(%i)' % ix)
        listeclEnergyTCECLCalDigit.append('tceclcal_%i' % ix)

    return listeclEnergyTC, listeclEnergyTCECLCalDigit


# L1 trigger
listL1PSNM, listL1FTDL, listL1Prescales = getL1Variables()

# ECL TC
eclEnergyTC, eclEnergyTCECLCalDigit = getECLTRGVariables()

variables.addAlias('eclEnergySumTC_2_15', 'eclEnergySumTC(2, 15)')
variables.addAlias('eclEnergySumTCECLCalDigit_2_15_0', 'eclEnergySumTCECLCalDigit(2, 15, 0)')
variables.addAlias('eclEnergySumTCECLCalDigit_2_15_1', 'eclEnergySumTCECLCalDigit(2, 15, 1)')

# dont vertex the leptons
ma.variablesToNtuple('',
                     variables=['eclEnergySumTC_2_15', 'eclEnergySumTCECLCalDigit_2_15_0', 'eclEnergySumTCECLCalDigit_2_15_1'] +
                     listL1PSNM +
                     eclEnergyTC +
                     eclEnergyTCECLCalDigit,
                     treename='event',
                     filename=outputfilename + '_event.root',
                     path=main_path)


# process the events
basf2.process(path=main_path)

# print out the summary
print(basf2.statistics)
