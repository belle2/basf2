#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Importer of the SVD Reconstruction Configuration
"""
import basf2
from ROOT import Belle2
import datetime

now = datetime.datetime.now()

# cluster reconstruction algorithms:
algorithmFor_timeRecoWith6Samples = "CoG3"
algorithmFor_timeRecoWith3Samples = "CoG3"
algorithmFor_positionRecoWith6Samples = "CoGOnly"
algorithmFor_positionRecoWith3Samples = "CoGOnly"
algorithmFor_chargeRecoWith6Samples = "MaxSample"
algorithmFor_chargeRecoWith3Samples = "MaxSample"
# strip reconstruction algorithms:
algorithmFor_stripTimeRecoWith6Samples = "dontdo"
algorithmFor_stripTimeRecoWith3Samples = "dontdo"
algorithmFor_stripChargeRecoWith6Samples = "MaxSample"
algorithmFor_stripChargeRecoWith3Samples = "MaxSample"
# grouping
grouping_stateOfTimeGroupingInClusterizerIn6Samples = True  # in 6 Samples DAQ Mode
grouping_useOfSVDGroupInfoInSPCreatorIn6Samples = True
grouping_stateOfTimeGroupingInClusterizerIn3Samples = False  # in 3 Samples DAQ Mode
grouping_useOfSVDGroupInfoInSPCreatorIn3Samples = False
# cut on sample SNR
snr_useOfSVDSpacePointSNRSelectionFunctionIn6Samples = False
snr_useOfSVDSpacePointSNRSelectionFunctionIn3Samples = False


class recoConfigurationImporter(basf2.Module):
    ''' importer of the SVD Reconstruction Configuration'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        uniqueID = "SVDRecoConfiguration_default_3=6_" + \
            str(now.isoformat()) + "_INFO:" + \
            "_" + str(algorithmFor_timeRecoWith6Samples) + "Time" + \
            "_" + str(algorithmFor_chargeRecoWith6Samples) + "Charge" + \
            "_" + str(algorithmFor_positionRecoWith6Samples) + "Position"

        groupingSpecificString = ""
        if grouping_stateOfTimeGroupingInClusterizerIn6Samples and grouping_stateOfTimeGroupingInClusterizerIn3Samples:
            if grouping_useOfSVDGroupInfoInSPCreatorIn6Samples or grouping_useOfSVDGroupInfoInSPCreatorIn3Samples:
                groupingSpecificString += "_groupSelectionON"
            else:
                groupingSpecificString += "_groupIdON"
        elif grouping_stateOfTimeGroupingInClusterizerIn6Samples:
            if grouping_useOfSVDGroupInfoInSPCreatorIn6Samples:
                groupingSpecificString += "_6groupSelectionON"
            else:
                groupingSpecificString += "_6groupIdON"
        elif grouping_stateOfTimeGroupingInClusterizerIn3Samples:
            if grouping_useOfSVDGroupInfoInSPCreatorIn3Samples:
                groupingSpecificString += "_3groupSelectionON"
            else:
                groupingSpecificString += "_3groupIdON"

        uniqueID += groupingSpecificString

        cutOnSampleSNRSpecificString = ""
        if snr_useOfSVDSpacePointSNRSelectionFunctionIn6Samples and snr_useOfSVDSpacePointSNRSelectionFunctionIn3Samples:
            cutOnSampleSNRSpecificString = "_cutOnSampleSNRFractionON"
        elif snr_useOfSVDSpacePointSNRSelectionFunctionIn6Samples:
            cutOnSampleSNRSpecificString = "_cutOn6SampleSNRFractionON"
        elif snr_useOfSVDSpacePointSNRSelectionFunctionIn3Samples:
            cutOnSampleSNRSpecificString = "_cutOn3SampleSNRFractionON"

        uniqueID += cutOnSampleSNRSpecificString

        payload = Belle2.SVDRecoConfiguration(uniqueID)

        # cluster time
        payload.setTimeRecoWith6Samples(algorithmFor_timeRecoWith6Samples)
        payload.setTimeRecoWith3Samples(algorithmFor_timeRecoWith3Samples)
        # cluster position
        payload.setPositionRecoWith6Samples(algorithmFor_positionRecoWith6Samples)
        payload.setPositionRecoWith3Samples(algorithmFor_positionRecoWith3Samples)
        # cluster charge
        payload.setChargeRecoWith6Samples(algorithmFor_chargeRecoWith6Samples)
        payload.setChargeRecoWith3Samples(algorithmFor_chargeRecoWith3Samples)
        # strip time
        payload.setStripTimeRecoWith6Samples(algorithmFor_stripTimeRecoWith6Samples)
        payload.setStripTimeRecoWith3Samples(algorithmFor_stripTimeRecoWith3Samples)
        # strip charge
        payload.setStripChargeRecoWith6Samples(algorithmFor_stripChargeRecoWith6Samples)
        payload.setStripChargeRecoWith3Samples(algorithmFor_stripChargeRecoWith3Samples)
        # SVDTimeGrouping
        payload.setStateOfSVDTimeGrouping(6, grouping_stateOfTimeGroupingInClusterizerIn6Samples)
        payload.setUseOfSVDGroupInfoInSPCreator(6, grouping_useOfSVDGroupInfoInSPCreatorIn6Samples)
        payload.setStateOfSVDTimeGrouping(3, grouping_stateOfTimeGroupingInClusterizerIn3Samples)
        payload.setUseOfSVDGroupInfoInSPCreator(3, grouping_useOfSVDGroupInfoInSPCreatorIn3Samples)
        # Cut on sample SNR
        payload.setUseOfSVDSpacePointSNRSelectionFunction(6, snr_useOfSVDSpacePointSNRSelectionFunctionIn6Samples)
        payload.setUseOfSVDSpacePointSNRSelectionFunction(3, snr_useOfSVDSpacePointSNRSelectionFunctionIn3Samples)

        Belle2.Database.Instance().storeData(Belle2.SVDRecoConfiguration.name, payload, iov)


main = basf2.create_path()

# Event info setter - execute single event
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module(recoConfigurationImporter())

# Process events
basf2.process(main)
