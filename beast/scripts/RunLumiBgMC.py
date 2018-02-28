import basf2
from basf2 import *
from beamparameters import add_beamparameters
import os
import sys
import math
import string
import datetime
from background import add_output

d = datetime.datetime.today()
print(d.strftime('job start: %Y-%m-%d %H:%M:%S\n'))

if len(sys.argv) != 7:
    print("Usage: requires 5 arguments")
    print("Argument 1: (bbbrem | bhwide | bhwide_largeangle | aafh | koralw)")
    print("Argument 2: seed number")
    print("Argument 3: (study | usual | ECL | PXD)")
    print("Argument 4: phase ( 2 | 3)")
    print("Argument 5: ROOT output directory path")
    print("Argument 6: digitization (true | false)")
    sys.exit(1)

# read parameters
argvs = sys.argv
argc = len(argvs)
generator = sys.argv[1].lower()
num = argvs[2]
sampleType = sys.argv[3]
phase = int(argvs[4])
output_dir = argvs[5]
digitization = sys.argv[6]

if phase == 2:
    sampleType = 'usual'

outputfilename = output_dir + '/output_phase_' + argvs[4] + '_' + generator + '_' + num + '.root'

# set random seed
seed = str(1234567 + int(num))
set_random_seed(seed)

kill = basf2.create_path()
main = basf2.create_path()
# main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=1000000)
# main.add_module("EventInfoPrinter")

# basf2.set_log_level(basf2.LogLevel.DEBUG)
# basf2.set_debug_level(250)
# basf2.logging.package("framework").log_level = basf2.LogLevel.WARNING


def add_cut(name, minParticles, maxParticles, minTheta, maxTheta=None):
    """Add a generator level cut and kill the event if the cut is not passed.  In
    this case the cut is on the min/max charged particles which have a
    center-of-mass theta angle between minTheta and maxTheta. If maxTheta is not
    given assume it to be 180-minTheta for a symmetric window"""

    # if only one angle make it symmetric
    if maxTheta is None:
        maxTheta = 180 - minTheta
    selection = main.add_module("GeneratorPreselection", applyInCMS=True, nChargedMin=minParticles, nChargedMax=maxParticles,
                                MinChargedTheta=minTheta, MaxChargedTheta=maxTheta, MinChargedP=0., MinChargedPt=0.)
    selection.if_value("!=11", kill)
    selection.set_name("generator cut: " + name)


# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
beamparameters.param("smearVertex", True)
beamparameters.param("generateCMS", False)

if generator == "bbbrem":
    bgType = 'RBB'
    evtnum = 8430
    realTime = 10  # 10ns
    main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=evtnum)
    main.add_module("BBBremInput", MinPhotonEnergyFraction=0.000001, Unweighted=True, MaxWeight=1.57001e+07)
    # at least one track below 0.5 degree means maximum one particle in 0.5-179.5
    add_cut("at least one track below 0.5 degree", 0, 1, 0.5)
elif generator == "bhwide":
    bgType = 'BHWide'
    evtnum = 984
    realTime = 10.0e3  # 10us
    main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=evtnum)
    main.add_module("BHWideInput", ScatteringAngleRangeElectron=[0.5, 179.5], ScatteringAngleRangePositron=[0.5, 179.5])
    add_cut("both tracks at least 0.5 degree", 2, 2, 0.5)
    # but if one is above 1 and the other above 10 degree so we in 1-170 and
    # 10-179
    add_cut("max one track in 1-170", 0, 1, 1, 170)
    add_cut("max one track in 10-179", 0, 1, 10, 179)
elif generator == "bhwide_largeangle":
    bgType = 'BHWideLargeAngle'
    evtnum = 98400
    realTime = 1.0e6  # 1ms
    main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=evtnum)
    main.add_module("BHWideInput", ScatteringAngleRangeElectron=[0.5, 179.5], ScatteringAngleRangePositron=[0.5, 179.5])
    add_cut("both tracks at least 1 degree", 2, 2, 1)
    add_cut("at least one 10 degree", 1, 2, 10)
elif generator == "koralw":
    bgType = 'twoPhoton'
    evtnum = 1720
    realTime = 1.0e3  # in ns = 1 us
    main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=evtnum)
    koralw = register_module('KoralWInput')
    koralw.param('RandomSeed', int(seed))
    koralw.param('DataPath', './data/')
    koralw.param('UserDataFile', 'KoralW_ee_mod.data')
    # koralw.logging.log_level = LogLevel.INFO
    main.add_module(koralw)
elif generator == "aafh":
    bgType = 'twoPhoton'
    evtnum = 5640
    realTime = 1.0e3  # in ns = 1 us
    main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=evtnum)
    aafh = register_module('AafhInput')
    aafh.param({
        # decay mode to generate.
        # 1: e+e- -> mu+mu-L+L- where L is a user defined particle (default: tau)
        # 2: e+e- -> mu+mu-mu+mu-
        # 3: e+e- -> e+e-mu+mu-
        # 4: e+e- -> e+e-L+L- where L is a user defined particle (default: tau)
        # 5: e+e- -> e+e-e+e-
        'mode': 5,
        # to set the particle for modes 1 and 4 use set parameter "particle"
        # rejection scheme to generate unweighted events
        # 1: use rejection once for the final event weight
        # 2: use rejection per sub generator and then for the final event
        'rejection': 2,
        # max subgenerator event weight, only used if rejection is set to 2
        # (default). If this value is to low the generation will produce errors. If
        # it is to high generation runs slower.
        # 'maxSubgeneratorWeight': 1.0,
        # max final event weight which is always used. If this value is to low the
        # generation will produce errors. If it is to high generation runs slower.
        # ==> should be around 2-4
        # 'maxFinalWeight': 1.5,
        # adjust subgenerator weights so that each sub generator has same
        # probability to be called and the maximum weight is equal as well. These
        # values are printed at the end of generation when output level is set to
        # INFO. These weights strongly depend on the mode
        # 'subgeneratorWeights': [1.000e+00, 2.216e+01, 3.301e+03, 6.606e+03, 1.000e+00, 1.675e+00, 5.948e+00, 6.513e+00],
        # set to awfully precise
        'suppressionLimits': [1e100] * 4,
        # minimum invariant mass of the secondary pair
        # 'minMass': 0.50,

        # 0.50GeV 2nd ###
        # 'minMass': 0.50,
        # 'maxFinalWeight': 3.59,
        # 'maxSubgeneratorWeight': 0.716,
        # 'subgeneratorWeights': [1.000e+00, 6.088e+01, 2.348e+04, 2.551e+04, 1.000e+00, 1.956e+00, 2.937e+00, 7.169e-01],

        # 0.01GeV 3rd ###
        # 'minMass': 0.01,
        # 'maxFinalWeight': 2.5,
        # 'maxSubgeneratorWeight': 1.0,
        # 'subgeneratorWeights': [1.000e+00, 2.216e+01, 3.301e+03, 6.606e+03, 1.000e+00, 1.675e+00, 5.948e+00, 6.513e+00],

        # 0.002GeV 1st ###
        # 'minMass': 0.002,
        # 'maxFinalWeight': 2.5,
        # 'maxSubgeneratorWeight': 1.0,
        # 'subgeneratorWeights': [1.000e+00, 2.216e+01, 3.301e+03, 6.606e+03, 1.000e+00, 1.675e+00, 5.948e+00, 6.513e+00],

        # 0.001GeV 1st ###
        'minMass': 0.001,
        'maxFinalWeight': 2.5,
        'maxSubgeneratorWeight': 1.0,
        'subgeneratorWeights': [1.000e+00, 2.216e+01, 3.301e+03, 6.606e+03, 1.000e+00, 1.675e+00, 5.948e+00, 6.513e+00],

    })
    # aafh.logging.log_level = LogLevel.INFO
    main.add_module(aafh)
else:
    print("unknown generation setting: {}".format(generator))

print('generator ', generator)
print('bgtype', bgType)
print('reaTime', realTime)

gearbox = register_module('Gearbox')
if sampleType == 'study' and phase == 3:
    gearbox.param('override', [
        ('/Global/length', '40.0', 'm'),
        ("/DetectorComponent[@name='PXD']//ActiveChips", 'true', ''),
        ("/DetectorComponent[@name='PXD']//SeeNeutrons", 'true', ''),
        ("/DetectorComponent[@name='SVD']//ActiveChips", 'true', ''),
        ("/DetectorComponent[@name='SVD']//SeeNeutrons", 'true', ''),
        ("/DetectorComponent[@name='TOP']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='ARICH']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='ECL']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='BKLM']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='BeamPipe']//LimitStepLength", '1', ''),
        ("/DetectorComponent[@name='Cryostat']//LimitStepLength", '1', ''),
        ("/DetectorComponent[@name='FarBeamLine']//LimitStepLength", '1', ''),
    ])
else:
    gearbox.param('override', [('/Global/length', '40.0', 'm')])  # needed for FarBeamLine
if phase == 2:
    gearbox.param('fileName', '/geometry/Beast2_phase2.xml')
main.add_module(gearbox)

geometry = register_module('Geometry')
geometry.param({
    "excludedComponents": ["MagneticField"],
    "additionalComponents": ["MagneticField3dQuadBeamline"],
})
# geometry.param('additionalComponents', ['FarBeamLine'])
main.add_module(geometry)

# print generated particles
# mcparticleprinter = register_module('PrintMCParticles')
# mcparticleprinter.logging.log_level = LogLevel.INFO
# main.add_module(mcparticleprinter)

fullsim = register_module('FullSim')
fullsim.param('PhysicsList', 'FTFP_BERT_HP')
fullsim.param('UICommands', ['/process/inactivate nKiller'])
fullsim.param('StoreAllSecondaries', True)
fullsim.param('SecondariesEnergyCut', 0.0)  # [MeV]
main.add_module(fullsim)

main.add_module("Progress")

if phase == 2 and digitization == 'true':
    rootoutput = register_module('RootOutput')
    rootoutput.param('outputFileName', outputfilename)
    rootoutput.param('updateFileCatalog', False)
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
else:
    add_output(main, bgType, realTime, sampleType, phase, outputfilename)

# main.add_module("RootOutput", outputFileName="%s.root" % generator)
basf2.process(main)

print('Event Statistics:')
print(basf2.statistics)

d = datetime.datetime.today()
print(d.strftime('job finish: %Y-%m-%d %H:%M:%S\n'))
