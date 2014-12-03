#!/usr/bin/env python
# -*- coding: utf-8 -*-

# -------------------------------------------------------------------------
# BG simulation using SAD input files
#
# make a link to the SAD files before running it:
#    ln -s /home/belle/nakayama/fs2/BGdata/10th_fullsim/ input
# and make two changes in Belle2.xml:
#    1. uncomment inclusion of FarBeamLine.xml
#    2. change length of a Global volume from 8 to 40 m
#
# To generate samples for BG studies use patch provided by Nakayama-san
# -------------------------------------------------------------------------

from basf2 import *
import os
import sys
import math
import string
import datetime
from background import add_output

d = datetime.datetime.today()
print d.strftime('job start: %Y-%m-%d %H:%M:%S\n')

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
    print './RunSADBgMC.py [(RBB,Touschek,Coulomb)_(HER,LER)(,_far)] [num] [(study,usual,ECL,PXD)]'
    sys.exit()

# set accring (0:LER, 1:HER)
if name.find('LER') != -1:
    accring = 0
elif name.find('HER') != -1:
    accring = 1
else:
    print 'name should include either of HER or LER'
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

nummod = str(int(num) % 100)

if name == 'RBB_LER':
    readmode = 0
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
elif name == 'RBB_HER':
    readmode = 0
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
elif name == 'Touschek_LER':
    readmode = 0  # 1us
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
elif name == 'Touschek_HER':
    readmode = 0  # 1us
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
elif name == 'Coulomb_LER':
    readmode = 0  # 1us
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
elif name == 'Coulomb_HER':
    readmode = 0
    inputfilename = 'input/EvtbyEvt/' + name + '_EvtbyEvt_' + nummod + '.root'
elif name == 'RBB_LER_far':
    readmode = 0
    nevent = 11459  # 0.1us
elif name == 'RBB_HER_far':
    readmode = 0
    nevent = 3498  # 0.1us
elif name == 'Touschek_LER_far':
    readmode = 1
    readouttime = 100  # 0.1us
elif name == 'Touschek_HER_far':
    readmode = 1
    readouttime = 100  # 0.1us
else:
    print 'Unknown name! (' + name + ')'
    sys.exit()

print 'accring: ', accring, '(0:LER, 1:HER)'
print 'input:   ', inputfilename
print 'output:  ', outputfilename
print 'range:   ', range
print 'nevent:  ', nevent
print 'readmode: ', readmode
print 'readouttime:', readouttime
print 'bgType: ', bgType
print 'sampleType: ', sampleType
print 'realTime: ', realTime, 'ns'

# --- put modules into path ---------------------------------------------------

set_log_level(LogLevel.WARNING)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [nevent], 'runList': [1], 'expList': [1]})
main.add_module(eventinfosetter)

gearbox = register_module('Gearbox')
main.add_module(gearbox)

sadinput = register_module('SADInput')
sadinput.param('Filename', inputfilename)
sadinput.param('ReadMode', readmode)
sadinput.param('AccRing', accring)
sadinput.param('ReadoutTime', readouttime)  # needed only for ReadMode = 1
sadinput.param('Range', range)
main.add_module(sadinput)

geometry = register_module('Geometry')
main.add_module(geometry)

fullsim = register_module('FullSim')
fullsim.param('PhysicsList', 'QGSP_BERT_HP')
fullsim.param('UICommands', ['/process/inactivate nKiller'])
fullsim.param('StoreAllSecondaries', True)
fullsim.param('SecondariesEnergyCut', 0.000001)  # [MeV] need for CDC EB neutron flux
main.add_module(fullsim)

progress = register_module('Progress')
main.add_module(progress)

add_output(main, bgType, realTime, sampleType, outputfilename)

process(main)

print 'Event Statistics:'
print statistics

d = datetime.datetime.today()
print d.strftime('job finish: %Y-%m-%d %H:%M:%S\n')
