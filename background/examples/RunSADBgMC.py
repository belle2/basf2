#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!
#
# Deprecated! Please use generateSADBg.py
#
# -!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

# -------------------------------------------------------------------------
# BG simulation using SAD input files
#
# make a link to the SAD files before running it (at KEKCC):
#    ln -s /home/belle/nakayama/fs2/BGdata/12th_fullsim/ input
# -------------------------------------------------------------------------

from basf2 import *
import os
import sys
import math
import string
import datetime
from background import add_output

d = datetime.datetime.today()
print(d.strftime('job start: %Y-%m-%d %H:%M:%S\n'))

# read parameters

argvs = sys.argv
argc = len(argvs)

if argc == 3:
    name = argvs[1]
    num = argvs[2]
    sampleType = 'usual'
elif argc == 4:
    name = argvs[1]
    num = argvs[2]
    sampleType = argvs[3]
else:
    print('./RunSADBgMC.py [(RBB,Touschek,Coulomb,BHWide)_(HER,LER)(,_far)] [num] [(study,usual,ECL,PXD)]')
    sys.exit()

# set accring (0:LER, 1:HER)
if name.find('LER') != -1:
    accring = 0
elif name.find('HER') != -1:
    accring = 1
else:
    print('name should include either of HER or LER')
    sys.exit()

if name.find('far') != -1:
    range = 2800
else:
    range = 400

inputfilename = 'input/' + name + '.root'
fname = name + '_' + sampleType + '_' + num
outputfilename = 'output/output_' + fname + '.root'
bgType = name

readouttime = 0
nevent = 1000000
realTime = 1.0e3  # ns

seed = str(1234567 + int(num))
nummod = str(int(num) % 100)

if name == 'RBB_LER':
    readmode = 0
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
    seed = seed + '1'
elif name == 'RBB_HER':
    readmode = 0
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
    seed = seed + '2'
elif name == 'Touschek_LER':
    readmode = 0  # 1us
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
    seed = seed + '3'
elif name == 'Touschek_HER':
    readmode = 0  # 1us
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
    seed = seed + '4'
elif name == 'Coulomb_LER':
    readmode = 0  # 1us
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
    seed = seed + '5'
elif name == 'Coulomb_HER':
    readmode = 0
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
    seed = seed + '6'
elif name == 'RBB_LER_far':
    readmode = 0
    nevent = 11459  # 0.1us
    seed = seed + '7'
elif name == 'RBB_HER_far':
    readmode = 0
    nevent = 3498  # 0.1us
    seed = seed + '8'
elif name == 'Touschek_LER_far':
    readmode = 1
    readouttime = 100  # 0.1us
    seed = seed + '9'
elif name == 'Touschek_HER_far':
    readmode = 1
    readouttime = 100  # 0.1us
    seed = seed + '10'
elif name == 'BHWide_HER':
    readmode = 0
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
    seed = seed + '11'
elif name == 'BHWide_LER':
    readmode = 0
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
    seed = seed + '12'
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

# --- put modules into path ---------------------------------------------------

set_log_level(LogLevel.WARNING)
set_random_seed(int(seed))

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [nevent], 'runList': [1], 'expList': [1]})
main.add_module(eventinfosetter)

gearbox = register_module('Gearbox')
if sampleType == 'study':
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
        # ("/DetectorComponent[@name='EKLM']//Mode", '1', ''),
        ])
else:
    gearbox.param('override', [('/Global/length', '40.0', 'm')])  # needed for FarBeamLine
main.add_module(gearbox)

sadinput = register_module('SADInput')
sadinput.param('Filename', inputfilename)
sadinput.param('ReadMode', readmode)
sadinput.param('AccRing', accring)
sadinput.param('ReadoutTime', readouttime)  # needed only for ReadMode = 1
sadinput.param('Range', range)
main.add_module(sadinput)

geometry = register_module('Geometry')
geometry.param('additionalComponents', ['FarBeamLine'])
main.add_module(geometry)

fullsim = register_module('FullSim')
fullsim.param('PhysicsList', 'FTFP_BERT_HP')
fullsim.param('UICommands', ['/process/inactivate nKiller'])
fullsim.param('StoreAllSecondaries', True)
fullsim.param('SecondariesEnergyCut', 0.000001)  # [MeV] need for CDC EB neutron flux
main.add_module(fullsim)

progress = register_module('Progress')
main.add_module(progress)

add_output(main, bgType, realTime, sampleType, outputfilename)

process(main)

print('Event Statistics:')
print(statistics)

d = datetime.datetime.today()
print(d.strftime('job finish: %Y-%m-%d %H:%M:%S\n'))
