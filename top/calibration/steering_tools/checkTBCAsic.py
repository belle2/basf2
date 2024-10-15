#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Search for the shared TBC constants within an ASIC.
#
# usage: basf2 checkTBCAsic.py expNo runNo [globalTag or localDB]
# ---------------------------------------------------------------------------------------

import basf2 as b2
from ROOT import Belle2
import sys

argvs = sys.argv
if len(argvs) < 3:
    print("usage: basf2", argvs[0], "expNo runNo [globalTag or localDB]")
    sys.exit()
expNo = int(argvs[1])
runNo = int(argvs[2])

# Database
tag = '(main)'
if len(argvs) == 4:
    tag = argvs[3]
    if '.txt' in tag:
        b2.conditions.append_testing_payloads(tag)
    else:
        b2.conditions.append_globaltag(tag)


class CheckTBCAsic(b2.Module):
    ''' Search for the shared TBC constants within an ASIC '''

    def initialize(self):
        ''' initialize: search is done here '''

        print()
        print('Experiment =', expNo, 'Run =', runNo, 'global tag =', tag)
        print()

        mapper = Belle2.TOP.TOPGeometryPar.Instance().getFrontEndMapper()
        tbc = Belle2.PyDBObj('TOPCalTimebase')
        for slot in range(1, 17):
            for bs in range(4):
                femap = mapper.getMap(slot, bs)
                scrod = femap.getScrodID()
                print('slot' + str(slot), 'BS' + str(bs), 'scrodID =', scrod)
                for asic in range(16):
                    sampleTimes = []
                    for ch in range(8):
                        chan = asic * 8 + ch
                        if tbc.isAvailable(scrod, chan):
                            sampleTimes.append(tbc.getSampleTimes(scrod, chan))
                    while len(sampleTimes) > 1:
                        not_eq = []
                        for i in range(1, len(sampleTimes)):
                            if sampleTimes[i].getTimeAxis() == sampleTimes[0].getTimeAxis():
                                ch = sampleTimes[i].getChannel() - asic * 8
                                ch0 = sampleTimes[0].getChannel() - asic * 8
                                print('--> asic =', asic, ': ch' + str(ch), 'sample times are same as ch' + str(ch0))
                            else:
                                not_eq.append(sampleTimes[i])
                        sampleTimes = not_eq


# create path
main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [runNo], 'expList': [expNo]})
main.add_module(eventinfosetter)

# Geometry parameters
main.add_module('TOPGeometryParInitializer')

# Search for the shared TBC constants within an ASIC
main.add_module(CheckTBCAsic())

# process single event
b2.process(main)
