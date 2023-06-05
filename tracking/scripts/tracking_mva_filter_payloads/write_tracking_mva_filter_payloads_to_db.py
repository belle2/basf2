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
    write_tracking_mva_filter_payloads_to_db("TrackQualityEstimatorParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_TrackQualityIndicator", 0.7)
    write_tracking_mva_filter_payloads_to_db("TrackQualityEstimatorParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_TrackQualityIndicator", 0.7)
    write_tracking_mva_filter_payloads_to_db("TrackQualityEstimatorParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_TrackQualityIndicator", 0.7)

    write_tracking_mva_filter_payloads_to_db("SegmentTrackFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_SegmentTrackFilter", 0.74)
    write_tracking_mva_filter_payloads_to_db("SegmentTrackFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_SegmentTrackFilter", 0.74)
    write_tracking_mva_filter_payloads_to_db("SegmentTrackFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_SegmentTrackFilter", 0.74)

    write_tracking_mva_filter_payloads_to_db("TrackFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_TrackFilter", 0.1)
    write_tracking_mva_filter_payloads_to_db("TrackFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_TrackFilter", 0.1)
    write_tracking_mva_filter_payloads_to_db("TrackFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_TrackFilter", 0.1)
    write_tracking_mva_filter_payloads_to_db("ClusterFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_BackgroundClusterFilter", 0.2)
    write_tracking_mva_filter_payloads_to_db("ClusterFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_BackgroundClusterFilter", 0.2)
    write_tracking_mva_filter_payloads_to_db("ClusterFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_BackgroundClusterFilter", 0.2)
    write_tracking_mva_filter_payloads_to_db("FacetFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_FacetFilter", 2.805)
    write_tracking_mva_filter_payloads_to_db("FacetFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_FacetFilter", 2.805)
    write_tracking_mva_filter_payloads_to_db("FacetFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_FacetFilter", 2.805)
    write_tracking_mva_filter_payloads_to_db("RealisticSegmentPairRelationFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_RealisticSegmentPairRelationFilter", 0.22)
    write_tracking_mva_filter_payloads_to_db("RealisticSegmentPairRelationFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_RealisticSegmentPairRelationFilter", 0.22)
    write_tracking_mva_filter_payloads_to_db("RealisticSegmentPairRelationFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_RealisticSegmentPairRelationFilter", 0.22)
    write_tracking_mva_filter_payloads_to_db("FeasibleSegmentPairFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_FeasibleSegmentPairFilter", 0.004)
    write_tracking_mva_filter_payloads_to_db("FeasibleSegmentPairFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_FeasibleSegmentPairFilter", 0.004)
    write_tracking_mva_filter_payloads_to_db("FeasibleSegmentPairFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_FeasibleSegmentPairFilter", 0.004)
    write_tracking_mva_filter_payloads_to_db("RealisticSegmentPairFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_RealisticSegmentPairFilter", 0.02)
    write_tracking_mva_filter_payloads_to_db("RealisticSegmentPairFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_RealisticSegmentPairFilter", 0.02)
    write_tracking_mva_filter_payloads_to_db("RealisticSegmentPairFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_RealisticSegmentPairFilter", 0.02)
    write_tracking_mva_filter_payloads_to_db("FeasibleTrackRelationFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_FeasibleTrackRelationFilter", 0.03)
    write_tracking_mva_filter_payloads_to_db("FeasibleTrackRelationFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_FeasibleTrackRelationFilter", 0.03)
    write_tracking_mva_filter_payloads_to_db("FeasibleTrackRelationFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_FeasibleTrackRelationFilter", 0.03)
    write_tracking_mva_filter_payloads_to_db("RealisticTrackRelationFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_RealisticTrackRelationFilter", 0.80)
    write_tracking_mva_filter_payloads_to_db("RealisticTrackRelationFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_RealisticTrackRelationFilter", 0.80)
    write_tracking_mva_filter_payloads_to_db("RealisticTrackRelationFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_RealisticTrackRelationFilter", 0.80)
    write_tracking_mva_filter_payloads_to_db("FeasibleAxialSegmentPairFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_FeasibleAxialSegmentPairFilter", 0.04)
    write_tracking_mva_filter_payloads_to_db("FeasibleAxialSegmentPairFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_FeasibleAxialSegmentPairFilter", 0.04)
    write_tracking_mva_filter_payloads_to_db("FeasibleAxialSegmentPairFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_FeasibleAxialSegmentPairFilter", 0.04)
    write_tracking_mva_filter_payloads_to_db("RealisticAxialSegmentPairFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_RealisticAxialSegmentPairFilter", 0.49)
    write_tracking_mva_filter_payloads_to_db("RealisticAxialSegmentPairFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_RealisticAxialSegmentPairFilter", 0.49)
    write_tracking_mva_filter_payloads_to_db("RealisticAxialSegmentPairFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_RealisticAxialSegmentPairFilter", 0.49)
    write_tracking_mva_filter_payloads_to_db("FeasibleSegmentRelationFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_FeasibleSegmentRelationFilter", 0.04)
    write_tracking_mva_filter_payloads_to_db("FeasibleSegmentRelationFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_FeasibleSegmentRelationFilter", 0.04)
    write_tracking_mva_filter_payloads_to_db("FeasibleSegmentRelationFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_FeasibleSegmentRelationFilter", 0.04)
    write_tracking_mva_filter_payloads_to_db("RealisticSegmentRelationFilterParameters",
                                             (0,    0,    0, -1), "trackfindingcdc_RealisticSegmentRelationFilter", 0.805)
    write_tracking_mva_filter_payloads_to_db("RealisticSegmentRelationFilterParameters",
                                             (1002, 0, 1002, -1), "trackfindingcdc_RealisticSegmentRelationFilter", 0.805)
    write_tracking_mva_filter_payloads_to_db("RealisticSegmentRelationFilterParameters",
                                             (1003, 0, 1003, -1), "trackfindingcdc_RealisticSegmentRelationFilter", 0.805)

    write_tracking_mva_filter_payloads_to_db("CDCSVDStateFilter_1Parameters",
                                             (0,    0,    0, -1), "ckf_CDCSVDStateFilter_1", 0.1)
    write_tracking_mva_filter_payloads_to_db("CDCSVDStateFilter_1Parameters",
                                             (1002, 0, 1002, -1), "ckf_CDCSVDStateFilter_1", 0.1)
    write_tracking_mva_filter_payloads_to_db("CDCSVDStateFilter_1Parameters",
                                             (1003, 0, 1003, -1), "ckf_CDCSVDStateFilter_1", 0.1)
    write_tracking_mva_filter_payloads_to_db("CDCSVDStateFilter_2Parameters",
                                             (0,    0,    0, -1), "ckf_CDCSVDStateFilter_2", 0.1)
    write_tracking_mva_filter_payloads_to_db("CDCSVDStateFilter_2Parameters",
                                             (1002, 0, 1002, -1), "ckf_CDCSVDStateFilter_2", 0.1)
    write_tracking_mva_filter_payloads_to_db("CDCSVDStateFilter_2Parameters",
                                             (1003, 0, 1003, -1), "ckf_CDCSVDStateFilter_2", 0.1)
    write_tracking_mva_filter_payloads_to_db("CDCSVDStateFilter_3Parameters",
                                             (0,    0,    0, -1), "ckf_CDCSVDStateFilter_3", 0.1)
    write_tracking_mva_filter_payloads_to_db("CDCSVDStateFilter_3Parameters",
                                             (1002, 0, 1002, -1), "ckf_CDCSVDStateFilter_3", 0.1)
    write_tracking_mva_filter_payloads_to_db("CDCSVDStateFilter_3Parameters",
                                             (1003, 0, 1003, -1), "ckf_CDCSVDStateFilter_3", 0.1)
    write_tracking_mva_filter_payloads_to_db("CDCToSVDResultParameters",
                                             (0,    0,    0, -1), "ckf_CDCToSVDResult", 0.2)
    write_tracking_mva_filter_payloads_to_db("CDCToSVDResultParameters",
                                             (1002, 0, 1002, -1), "ckf_CDCToSVDResult", 0.2)
    write_tracking_mva_filter_payloads_to_db("CDCToSVDResultParameters",
                                             (1003, 0, 1003, -1), "ckf_CDCToSVDResult", 0.2)
    write_tracking_mva_filter_payloads_to_db("ToPXDStateFilter_1Parameters",
                                             (0,    0,    0, -1), "ckf_ToPXDStateFilter_1", 0.03)
    write_tracking_mva_filter_payloads_to_db("ToPXDStateFilter_1Parameters",
                                             (1002, 0, 1002, -1), "ckf_ToPXDStateFilter_1", 0.03)
    write_tracking_mva_filter_payloads_to_db("ToPXDStateFilter_1Parameters",
                                             (1003, 0, 1003, -1), "ckf_ToPXDStateFilter_1", 0.03)
    write_tracking_mva_filter_payloads_to_db("ToPXDStateFilter_2Parameters",
                                             (0,    0,    0, -1), "ckf_ToPXDStateFilter_2", 0.03)
    write_tracking_mva_filter_payloads_to_db("ToPXDStateFilter_2Parameters",
                                             (1002, 0, 1002, -1), "ckf_ToPXDStateFilter_2", 0.03)
    write_tracking_mva_filter_payloads_to_db("ToPXDStateFilter_2Parameters",
                                             (1003, 0, 1003, -1), "ckf_ToPXDStateFilter_2", 0.03)
    write_tracking_mva_filter_payloads_to_db("ToPXDStateFilter_3Parameters",
                                             (0,    0,    0, -1), "ckf_ToPXDStateFilter_3", 0.03)
    write_tracking_mva_filter_payloads_to_db("ToPXDStateFilter_3Parameters",
                                             (1002, 0, 1002, -1), "ckf_ToPXDStateFilter_3", 0.03)
    write_tracking_mva_filter_payloads_to_db("ToPXDStateFilter_3Parameters",
                                             (1003, 0, 1003, -1), "ckf_ToPXDStateFilter_3", 0.03)
    write_tracking_mva_filter_payloads_to_db("PXDTrackCombinationParameters",
                                             (0,    0,    0, -1), "PXDTrackCombination", 0.2)
    write_tracking_mva_filter_payloads_to_db("PXDTrackCombinationParameters",
                                             (1002, 0, 1002, -1), "PXDTrackCombination", 0.2)
    write_tracking_mva_filter_payloads_to_db("PXDTrackCombinationParameters",
                                             (1003, 0, 1003, -1), "PXDTrackCombination", 0.2)
