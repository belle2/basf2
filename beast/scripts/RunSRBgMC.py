#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import string
argvs = sys.argv
argc = len(argvs)

print("The input HEPEvt files needed to run SR MC sim are located in @kekcc:~igal/src/beast/SR")

if len(sys.argv) != 4:
    print("Usage: requires 2 arguments")
    print("Argument 1: (SynchRad_HER | SynchRad_LER | test)")
    print("Argument 2: file number")
    print("Argument 3: ROOT output directory path")
    sys.exit(1)

name = argvs[1]
num = argvs[2]
output_dir = argvs[3]
# Set realTime you want to use
# realTime = 1.0e4         # 10us for each  file <-- time in ns
# realTime = 20         # 20ns for each  file <-- time in ns - TEST

outputfilename = output_dir + "/output_" + name + "_" + num + ".root"

# tagname = SynchRad_HER for HER and SynchRad_LER for LER
tagname = name

##############################################################################
# This steering file shows how to comibine a particle gun simulated data
# with background using the background mixer.
#
# Example steering file - 2012 Belle II Collaboration
##############################################################################

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.DEBUG)

# PHASE 2
# FileIn = "Ph2_dt_4_8HER21445M.HEPEvt"      # data for HER Phase2  dt_4-8 6.6um -> 1.07 of bunch current 0.8A
#
# FileIn = "Ph2_dt_4_8LER35124M.HEPEvt"     #data for LERPhase2  dt_4-8 6.6um -> 1.4 of bunch current 1.0A

eventinfosetter = register_module('EventInfoSetter')
hepevtreader = register_module('HepevtInput')
# Ph2_dt_4_8HER21445M.HEPEvt
# Ph2_dt_4_8HER21445MK2M.HEPEvt
# Ph2_dt_4_8LER35124M.HEPEvt
# PHASE 2 -> HER 6148 repetitions = 1ROF = 20us | LER 3560 repet = 1ROF = 20us
if name == "SynchRad_HER":
    realTime = 1.0e4  # 10us per file
    FileIn = "Ph2_dt_4_8HER21445M.HEPEvt"      # data for HER Phase2  dt_4-8 6.6um -> 1.07 of bunch current 0.8A
    hepevtreader.param('inputFileList', [FileIn] * 2340)  # 2340 - 10us realTime Ph2_dt_4_8HER21445M.HEPEvt
elif name == "SynchRad_LER":
    realTime = 1.0e4  # 10us per file
    FileIn = "Ph2_dt_4_8LER35124M.HEPEvt"  # data for LERPhase2  dt_4-8 6.6um -> 1.4 of bunch current 1.0A
    hepevtreader.param('inputFileList', [FileIn] * 2340)  # 2340 - 10us realTime Ph2_dt_4_8HER21445M.HEPEvt
elif name == "test":
    realTime = 20  # 20ns per file
    FileIn = "Ph2_dt_4_8HER21445MK2M.HEPEvt"   # data HER KeV -> MeV,is used for test to be sure that code works or for cross-check.
    hepevtreader.param('inputFileList', [FileIn] * 5)    # - for quick TEST 5 ->~ 20nsec
    name = "ynchRad_HER"
# hepevtreader.param('inputFileList', [FileIn]*1780) # 1780 - 10us realTime for Ph2_dt_4_8LER35124M.HEPEvt
# hepevtreader.param('inputFileList', [FileIn] * 2340)  # 2340 - 10us realTime Ph2_dt_4_8HER21445M.HEPEvt

# Register
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
simulation = register_module('FullSim')
tagSetter = register_module('BeamBkgTagSetter')
progress = register_module('Progress')

# Setting the option for all non-hepevt reader modules:
# number of events in the list must be >= number of entries in input file times number of repetitions
eventinfosetter.param('evtNumList', [1000000000])
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

# PHASE 2
gearbox.param('fileName', '/geometry/Beast2_phase2.xml')
# select component you need, but always keep 'MagneticField', 'BeamPipe', 'Cryostat','HeavyMetalShield' !
# geometry.param('components', ['MagneticField', 'BeamPipe', 'Cryostat', 'PXD', 'HeavyMetalShield',
#                              'SVD', 'BEAMABORT', 'PLUME', 'FANGS', 'CLAWS', 'CDC',
#                              'ARICH', 'TOP', 'COIL', 'ECL', 'BKLM', 'EKLM', 'HE3', ''])
#
geometry.set_log_level(LogLevel.INFO)

# simulation.param('PhysicsList', "QGSP_BERT_HP")
simulation.param('PhysicsList', "QGSP_BERT_EMV")  # faster than QGSP_BERT_HP
# simulation.param('PhysicsList', "FTFP_BERT_EMV")
simulation.param('UICommands', ["/process/inactivate nKiller"])
simulation.param("StoreAllSecondaries", True)
simulation.param("SecondariesEnergyCut", 0.000001)  # in MeV   we need this for CDC EB neutron flux simulation

main = create_path()

main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(hepevtreader)
# print_params(hepevtreader)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)

# set background tag in SimHits, leave main path if all SimHits are empty
tagSetter.param('backgroundType', tagname)
tagSetter.param('realTime', realTime)
main.add_module(tagSetter)
emptyPath = create_path()
tagSetter.if_false(emptyPath)
print_params(tagSetter)

# output: SimHits only
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', outputfilename)
rootoutput.param('updateFileCatalog', False)

# PHASE 2
# Select branches you need in output.
rootoutput.param('branchNames', ["SVDSimHits", "SVDTrueHits", "SVDTrueHitsToSVDSimHits",
                                 "PXDSimHits", "MCParticleToPXDSimHits",
                                 "CLAWSSimHits", "ClawsHits",
                                 "FANGSSimHits", "FANGSHits",
                                 "PlumeSimHits",
                                 "BeamabortSimHits", "BeamabortHits",
                                 "PindiodeSimHits", "PindiodeHits",
                                 "QcsmonitorSimHits", "QcsmonitorHits",
                                 "He3tubeSimHits", "He3tubeHits",
                                 "MicrotpcSimHits", "MicrotpcHits",
                                 "SADMetaHits"])
MIP_to_PE = [12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12]
he3digi = register_module('He3Digitizer')
he3digi.param('conversionFactor', 0.303132019)
he3digi.param('useMCParticles', False)
main.add_module(he3digi)
diadigi = register_module('BeamDigitizer')
diadigi.param('WorkFunction', 13.25)
diadigi.param('FanoFactor', 0.382)
main.add_module(diadigi)
pindigi = register_module('PinDigitizer')
pindigi.param('WorkFunction', 3.64)
pindigi.param('FanoFactor', 0.13)
main.add_module(pindigi)
clawsdigi = register_module('ClawsDigitizer')
clawsdigi.param('ScintCell', 16)
clawsdigi.param('C_keV_to_MIP', 457.114)
clawsdigi.param('C_MIP_to_PE', MIP_to_PE)
clawsdigi.param('PEthres', 1.0)
main.add_module(clawsdigi)
qcssdigi = register_module('QcsmonitorDigitizer')
qcssdigi.param('ScintCell', 40)
qcssdigi.param('C_keV_to_MIP', 1629.827)
qcssdigi.param('C_MIP_to_PE', 15.0)
qcssdigi.param('MIPthres', 0.5)
main.add_module(qcssdigi)
fangsdigi = register_module('FANGSDigitizer')
main.add_module(fangsdigi)
tpcdigi = register_module('TpcDigitizer')
main.add_module(tpcdigi)

main.add_module(rootoutput)

# Process events
process(main)

# Print call statistics
print(statistics)
