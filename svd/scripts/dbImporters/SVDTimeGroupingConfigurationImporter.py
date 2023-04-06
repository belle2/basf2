#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2
import datetime

now = datetime.datetime.now()


class recoConfigurationImporter(b2.Module):
    ''' importer of the SVD Reconstruction Configuration'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDRecoConfiguration("SVDRecoConfiguration_default_3=6_" +
                                              str(now.isoformat()) + "_INFO:" +
                                              "_" + "CoG3" + "Time" +
                                              "_" + "MaxSample" + "Charge" +
                                              "_" + "CoGOnly" + "Position")

        # cluster time
        payload.setTimeRecoWith6Samples("CoG3")
        payload.setTimeRecoWith3Samples("CoG3")
        # cluster position
        payload.setPositionRecoWith6Samples("CoGOnly")
        payload.setPositionRecoWith3Samples("CoGOnly")
        # cluster charge
        payload.setChargeRecoWith6Samples("MaxSample")
        payload.setChargeRecoWith3Samples("MaxSample")
        # strip time
        payload.setStripTimeRecoWith6Samples("dontdo")
        payload.setStripTimeRecoWith3Samples("dontdo")
        # strip charge
        payload.setStripChargeRecoWith6Samples("MaxSample")
        payload.setStripChargeRecoWith3Samples("MaxSample")
        # grouping
        payload.setStateOfSVDTimeGrouping(6, True)
        payload.setUseOfSVDGroupInfoInSPCreator(6, True)

        Belle2.Database.Instance().storeData(Belle2.SVDRecoConfiguration.name, payload, iov)


class timeGroupingConfigurationImporter(b2.Module):
    ''' importer of the SVDTimeGrouping Configuration'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDTimeGroupingConfiguration("SVDTimeGroupingConfiguration_default_3=6_" +
                                                      str(now.isoformat()))

        payload.setTimeGroupingParameters("CoG3", 6).tRange[0] = -160
        payload.setTimeGroupingParameters("CoG3", 6).tRange[1] = 160
        payload.setTimeGroupingParameters("CoG3", 6).rebinningFactor = 2
        payload.setTimeGroupingParameters("CoG3", 6).fillSigmaN = 3
        payload.setTimeGroupingParameters("CoG3", 6).limitSigma[0] = 1
        payload.setTimeGroupingParameters("CoG3", 6).limitSigma[1] = 15
        payload.setTimeGroupingParameters("CoG3", 6).fitRangeHalfWidth = 5
        payload.setTimeGroupingParameters("CoG3", 6).removeSigmaN = 5
        payload.setTimeGroupingParameters("CoG3", 6).fracThreshold = 0.05
        payload.setTimeGroupingParameters("CoG3", 6).maxGroups = 20
        payload.setTimeGroupingParameters("CoG3", 6).expectedSignalTime[0] = -50
        payload.setTimeGroupingParameters("CoG3", 6).expectedSignalTime[1] = 0
        payload.setTimeGroupingParameters("CoG3", 6).expectedSignalTime[2] = 50
        payload.setTimeGroupingParameters("CoG3", 6).signalLifetime = 30
        payload.setTimeGroupingParameters("CoG3", 6).numberOfSignalGroups = 1
        payload.setTimeGroupingParameters("CoG3", 6).formSingleSignalGroup = False
        payload.setTimeGroupingParameters("CoG3", 6).acceptSigmaN = 5
        payload.setTimeGroupingParameters("CoG3", 6).writeGroupInfo = True
        payload.setTimeGroupingParameters("CoG3", 6).includeOutOfRangeClusters = True
        for item in [3.49898, 2.94008, 3.46766, 5.3746, 6.68848, 7.35446, 7.35983, 7.71601, 10.6172, 13.4805]:
            payload.setTimeGroupingParameters("CoG3", 6).clsSigma[0][0].push_back(item)
        for item in [6.53642, 3.76216, 3.30086, 3.95969, 5.49408, 7.07294, 8.35687, 8.94839, 9.23135, 10.485]:
            payload.setTimeGroupingParameters("CoG3", 6).clsSigma[0][1].push_back(item)

        Belle2.Database.Instance().storeData(Belle2.SVDTimeGroupingConfiguration.name, payload, iov)


main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)


main.add_module(recoConfigurationImporter())
main.add_module(timeGroupingConfigurationImporter())

main.add_module('Progress')

b2.print_path(main)
b2.process(main)
print(b2.statistics)
