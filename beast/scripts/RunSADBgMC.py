#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import sys
import datetime
from background import add_output

d = datetime.datetime.today()
print(d.strftime('job start: %Y-%m-%d %H:%M:%S\n'))

print("Make a link to the input SAD files before running this steering script:")
print("ln -s ~nakayama/[FIXME -> ask for the latest campaing] input")
print("OR")
print("Produce first the input SAD files before running this steering script:")
print("Make a link to the input SAD files before running this steering script:")
print("ln -s ~login_name/[FIXME -> ask your-self where did I put it] input")

if len(sys.argv) != 9:
    print("Usage: requires 8 arguments")
    print("Argument 1: SAD input directory path")
    print("Argument 2: (Touschek | Coulomb | Brens)_(HER | LER)(,_far)")
    print("Argument 3: file number")
    print("Argument 4: (study | usual | ECL | PXD)")
    print("Argument 5: time-eqv in ns")
    print("Argument 6: phase 1 | 2 |3")
    print("Argument 7: ROOT output directory path")
    print("Argument 8: digitization (true | false)")
    sys.exit(1)

# read parameters
argvs = sys.argv
argc = len(argvs)

input_dir = argvs[1]
name = argvs[2]
num = argvs[3]
sampleType = argvs[4]
sampleTime = int(argvs[5])
phase = int(argvs[6])
output_dir = argvs[7]
digitization = argvs[8]

print('Input dir: ', input_dir)
print('Name :', name)
print('File number: ', num)
print('Sample type: ', sampleType)
print('Sample time: ', sampleTime, 'ns')
print('Phase: ', phase)
print('Output dir', output_dir)

# set accring (0:LER, 1:HER)
if name.find('LER') != -1:
    accring = 0
elif name.find('HER') != -1:
    accring = 1
else:
    print('Name should include either of HER or LER')
    sys.exit()

if name.find('far') != -1:
    range = 2800
else:
    range = 400

# inputfilename = input_dir + '/' + name + '.root'
fname = 'phase_' + str(argvs[6]) + '_' + name + '_' + sampleType + '_' + num
outputfilename = output_dir + '/output_' + fname + '.root'
bgType = name

readouttime = 0
nevent = 1000000
# realTime = sampleTime  # ns

seed = str(1234567 + int(num))
nummod = str(int(num) % 5000)

if phase == 2 or phase == 3:
    realTime = 10.0e3  # ns
    readmode = 0
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
    if name == 'Touschek_LER':
        seed = seed + '3'
    elif name == 'Touschek_HER':
        seed = seed + '4'
    elif name == 'Coulomb_LER':
        seed = seed + '5'
    elif name == 'Coulomb_HER':
        seed = seed + '6'
    elif name == 'Touschek_LER_far':
        realTime = 0.1e3  # ns
        readmode = 1
        readouttime = 100  # 0.1us
        seed = seed + '9'
    elif name == 'Touschek_HER_far':
        realTime = 0.1e3  # ns
        readmode = 1
        readouttime = 100  # 0.1us
        seed = seed + '10'
    elif name == 'Brems_HER':
        seed = seed + '13'
    elif name == 'Brems_LER':
        readmode = 0
        seed = seed + '14'
    else:
        print('Unknown name! (' + name + ')')
        sys.exit()
elif phase == 1:
    readmode = 1
    readouttime = sampleTime
    inputfilename = 'input/' + name + '_' + nummod + '.root'
else:
    print('Unknown name! (' + name + ')')
    sys.exit()

if sampleType == 'study':
    seed = seed + '1'
elif sampleType == 'usual':
    seed = seed + '2'
elif sampleType == 'ECL':
    seed = seed + '3'
elif sampleType == 'PXD':
    seed = seed + '4'
else:
    print('Unknown sample type! (' + sampleType + ')')
    sys.exit()

print('accring: ', accring, '(0:LER, 1:HER)')
print('input:   ', inputfilename)
print('output:  ', outputfilename)
print('range:   ', range)
print('nevent:  ', nevent)
print('readmode: ', readmode)
print('readouttime:', readouttime)
print('bgType: ', bgType)
print('sampleType: ', sampleType)
print('realTime: ', realTime, 'ns')
print('seed: ', seed)

b2.set_log_level(b2.LogLevel.WARNING)
b2.set_random_seed(int(seed))

main = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [nevent], 'runList': [1], 'expList': [1]})
main.add_module(eventinfosetter)

gearbox = b2.register_module('Gearbox')
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
elif phase == 1:
    gearbox.param('fileName', '/geometry/Beast2_phase1.xml')
main.add_module(gearbox)

geometry = b2.register_module('Geometry')
if phase == 2 or phase == 3:
    geometry.param({
        "excludedComponents": ["MagneticField"],
        "additionalComponents": ["MagneticField3dQuadBeamline"],
    })
main.add_module(geometry)

sadinput = b2.register_module('SADInput')
sadinput.param('Filename', inputfilename)
sadinput.param('ReadMode', readmode)
sadinput.param('AccRing', accring)
sadinput.param('ReadoutTime', readouttime)  # needed only for ReadMode = 1
sadinput.param('Range', range)
main.add_module(sadinput)

fullsim = b2.register_module('FullSim')
fullsim.param('PhysicsList', 'FTFP_BERT_HP')
fullsim.param('UICommandsAtIdle', ['/process/inactivate nKiller'])
fullsim.param('StoreAllSecondaries', True)
fullsim.param('SecondariesEnergyCut', 0.0)  # [MeV] need for CDC EB neutron flux
main.add_module(fullsim)

progress = b2.register_module('Progress')
main.add_module(progress)
if phase == 1 and digitization == 'true':
    rootoutput = b2.register_module('RootOutput')
    rootoutput.param('outputFileName', outputfilename)
    rootoutput.param('updateFileCatalog', False)
    rootoutput.param('branchNames', ["ClawSimHits", "ClawsHits",
                                     "BeamabortSimHits", "BeamabortHits",
                                     "PindiodeSimHits", "PindiodeHits",
                                     "BgoSimHits", "BgobortHits",
                                     "CsiSimHits", "CsiHit_v2s",
                                     "QcsmonitorSimHits", "QcsmonitorHits",
                                     "He3tubeSimHits", "He3tubeHits",
                                     "MicrotpcSimHits", "MicrotpcHits",
                                     "SADMetaHits"])
    bgodigi = b2.register_module('BgoDigitizer')
    main.add_module(bgodigi)
    # dosidigi = register_module('DosiDigitizer')
    # main.add_module(dosidigi)
    csidigi = b2.register_module('CsiDigitizer_v2')
    main.add_module(csidigi)
    he3digi = b2.register_module('He3Digitizer')
    he3digi.param('conversionFactor', 0.303132019)
    he3digi.param('useMCParticles', False)
    main.add_module(he3digi)
    diadigi = b2.register_module('BeamDigitizer')
    diadigi.param('WorkFunction', 13.25)
    diadigi.param('FanoFactor', 0.382)
    main.add_module(diadigi)
    pindigi = b2.register_module('PinDigitizer')
    pindigi.param('WorkFunction', 3.64)
    pindigi.param('FanoFactor', 0.13)
    main.add_module(pindigi)
    tpcdigi = b2.register_module('TpcDigitizer')
    main.add_module(tpcdigi)
    MIP_to_PE1 = [12.97, 12.46, 14.86, 15.71, 13.63, 14.56, 14.53, 15.31]
    MIP_to_PE2 = [15.21, 12.46, 14.86, 15.71, 16.02, 15.83, 14.53, 15.31]
    clawsdigi = b2.register_module('ClawDigitizer')
    clawsdigi.param('ScintCell', 8)
    clawsdigi.param('C_keV_to_MIP', 457.114)
    clawsdigi.param('C_MIP_to_PE', MIP_to_PE2)
    clawsdigi.param('PEthres', 1.0)
    main.add_module(clawsdigi)
    qcssdigi = b2.register_module('QcsmonitorDigitizer')
    qcssdigi.param('ScintCell', 2)
    qcssdigi.param('C_keV_to_MIP', 1629.827)
    qcssdigi.param('C_MIP_to_PE', 15.0)
    qcssdigi.param('MIPthres', 0.5)
    main.add_module(qcssdigi)
    main.add_module(rootoutput)
elif phase == 2 and digitization == 'true':
    rootoutput = b2.register_module('RootOutput')
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
    he3digi = b2.register_module('He3Digitizer')
    he3digi.param('conversionFactor', 0.303132019)
    he3digi.param('useMCParticles', False)
    main.add_module(he3digi)
    diadigi = b2.register_module('BeamDigitizer')
    diadigi.param('WorkFunction', 13.25)
    diadigi.param('FanoFactor', 0.382)
    main.add_module(diadigi)
    pindigi = b2.register_module('PinDigitizer')
    pindigi.param('WorkFunction', 3.64)
    pindigi.param('FanoFactor', 0.13)
    main.add_module(pindigi)
    clawsdigi = b2.register_module('ClawsDigitizer')
    clawsdigi.param('ScintCell', 16)
    clawsdigi.param('C_keV_to_MIP', 457.114)
    clawsdigi.param('C_MIP_to_PE', MIP_to_PE)
    clawsdigi.param('PEthres', 1.0)
    main.add_module(clawsdigi)
    qcssdigi = b2.register_module('QcsmonitorDigitizer')
    qcssdigi.param('ScintCell', 40)
    qcssdigi.param('C_keV_to_MIP', 1629.827)
    qcssdigi.param('C_MIP_to_PE', 15.0)
    qcssdigi.param('MIPthres', 0.5)
    main.add_module(qcssdigi)
    fangsdigi = b2.register_module('FANGSDigitizer')
    main.add_module(fangsdigi)
    tpcdigi = b2.register_module('TpcDigitizer')
    main.add_module(tpcdigi)
    main.add_module(rootoutput)
else:
    add_output(main, bgType, realTime, sampleType, phase, outputfilename)

b2.process(main)

print('Event Statistics:')
print(b2.statistics)

d = datetime.datetime.today()
print(d.strftime('job finish: %Y-%m-%d %H:%M:%S\n'))
