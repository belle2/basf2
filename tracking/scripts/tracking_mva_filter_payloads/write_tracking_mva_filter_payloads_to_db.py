#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2
import basf2


def write_tracking_mva_filter_payloads_to_db(dbobj_name: str = None,
                                             iovList=(0, 0, 0, 0),
                                             weightfile_identifier: str = None,
                                             cut_value: float = None):
    """
    This function creates a database entry for a DBObject for MVA weightfiles with a given set of IoVs etc.
    :param dbobj_name Name of the DBObject to identify it in the DB
    :param iovList List of IoVs for which the payload is valid
    :param weightfile_identifier Name of the weightfile which already is on the DB
    :param cut_value Cut value
    """

    # Create the DBObject
    dbobj = Belle2.TrackingMVAFilterParameters()
    # Just a small sanity check for a valid IoV (expLow, runLow, expHigh, runHigh) and valid parameters
    if (dbobj_name is None) or \
       (len(iovList) != 4) or \
       (weightfile_identifier is None) or \
       (cut_value is None):
        basf2.B2ERROR(
            f"Ooops, something went wrong creating the DB payload for DBObject name {dbobj_name}. \
              Please check for valid arguments.")
        return

    # create the iov
    iov = Belle2.IntervalOfValidity(*iovList)
    # then set the parameters it contains
    dbobj.setIdentifierName(weightfile_identifier)
    dbobj.setCutValue(cut_value)

    # write db object to 'localdb/'
    Belle2.Database.Instance().storeData(dbobj_name, dbobj, iov)


if __name__ == "__main__":

    validIoVs = [(0, 0, 0, -1), (1003, 0, 1003, -1), (1004, 0, 1004, -1)]
    for iov in validIoVs:

        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_TrackQualityEstimatorParameters",
                                                 iov, "trackfindingcdc_TrackQualityIndicator", 0.7)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_SegmentTrackFilterParameters",
                                                 iov, "trackfindingcdc_SegmentTrackFilter", 0.74)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_TrackFilterParameters",
                                                 iov, "trackfindingcdc_TrackFilter", 0.1)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_ClusterFilterParameters",
                                                 iov, "trackfindingcdc_BackgroundClusterFilter", 0.2)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_FacetFilterParameters",
                                                 iov, "trackfindingcdc_FacetFilter", 2.805)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_RealisticSegmentPairRelationFilterParameters",
                                                 iov, "trackfindingcdc_RealisticSegmentPairRelationFilter", 0.22)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_FeasibleSegmentPairFilterParameters",
                                                 iov, "trackfindingcdc_FeasibleSegmentPairFilter", 0.004)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_RealisticSegmentPairFilterParameters",
                                                 iov, "trackfindingcdc_RealisticSegmentPairFilter", 0.02)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_FeasibleTrackRelationFilterParameters",
                                                 iov, "trackfindingcdc_FeasibleTrackRelationFilter", 0.03)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_RealisticTrackRelationFilterParameters",
                                                 iov, "trackfindingcdc_RealisticTrackRelationFilter", 0.80)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_FeasibleAxialSegmentPairFilterParameters",
                                                 iov, "trackfindingcdc_FeasibleAxialSegmentPairFilter", 0.04)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_RealisticAxialSegmentPairFilterParameters",
                                                 iov, "trackfindingcdc_RealisticAxialSegmentPairFilter", 0.49)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_FeasibleSegmentRelationFilterParameters",
                                                 iov, "trackfindingcdc_FeasibleSegmentRelationFilter", 0.04)
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_RealisticSegmentRelationFilterParameters",
                                                 iov, "trackfindingcdc_RealisticSegmentRelationFilter", 0.805)

        write_tracking_mva_filter_payloads_to_db("ckf_CDCSVDStateFilter_1_backward_Parameters",
                                                 iov, "ckf_CDCSVDStateFilter_1", 0.1)
        write_tracking_mva_filter_payloads_to_db("ckf_CDCSVDStateFilter_2_backward_Parameters",
                                                 iov, "ckf_CDCSVDStateFilter_2", 0.1)
        write_tracking_mva_filter_payloads_to_db("ckf_CDCSVDStateFilter_3_backward_Parameters",
                                                 iov, "ckf_CDCSVDStateFilter_3", 0.1)
        write_tracking_mva_filter_payloads_to_db("ckf_CDCSVDStateFilter_1_forward_Parameters",
                                                 iov, "ckf_CDCSVDStateFilter_1", 0.01)
        write_tracking_mva_filter_payloads_to_db("ckf_CDCSVDStateFilter_2_forward_Parameters",
                                                 iov, "ckf_CDCSVDStateFilter_2", 0.01)
        write_tracking_mva_filter_payloads_to_db("ckf_CDCSVDStateFilter_3_forward_Parameters",
                                                 iov, "ckf_CDCSVDStateFilter_3", 0.01)
        write_tracking_mva_filter_payloads_to_db("ckf_CDCToSVDResultParameters",
                                                 iov, "ckf_CDCToSVDResult", 0.2)
        write_tracking_mva_filter_payloads_to_db("ckf_ToPXDStateFilter_1Parameters",
                                                 iov, "ckf_ToPXDStateFilter_1", 0.03)
        write_tracking_mva_filter_payloads_to_db("ckf_ToPXDStateFilter_2Parameters",
                                                 iov, "ckf_ToPXDStateFilter_2", 0.03)
        write_tracking_mva_filter_payloads_to_db("ckf_ToPXDStateFilter_3Parameters",
                                                 iov, "ckf_ToPXDStateFilter_3", 0.03)
        write_tracking_mva_filter_payloads_to_db("ckf_PXDTrackCombinationParameters",
                                                 iov, "ckf_PXDTrackCombination", 0.2)
        write_tracking_mva_filter_payloads_to_db("ckf_SeededCDCToSVDResultParameters",
                                                 iov, "ckf_SeededCDCToSVDResult", 0.6)
