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
import os
# from hashlib import sha256


def write_tracking_mva_filter_payloads_to_db(dbobj_name: str,
                                             iovList: tuple[int, int, int, int],
                                             weightfile_identifier: str,
                                             cut_value: float) -> str | None:
    """
    This function creates a database entry for a DBObject for MVA weightfiles with a given set of IoVs etc.
    *** WARNING: This function will append a hashed number of the weightfile_identifier and cut_value to the DBObject name
    to avoid overwriting existing different entries in the local DB. ***
    :returns the name of the created DBObject, or None if something went wrong.
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

    if weightfile_identifier.endswith(".xml"):
        with open(weightfile_identifier) as f:
            weight_file_content = f.read()
        weightfile_name = write_mva_weightfile_content_to_db(dbobj_name + "_weightfile", weight_file_content, iovList)
        dbobj.setIdentifierName(weightfile_name)
    # write db object to 'localdb/'
    dbobj_new_name = dbobj_name  # + "_" + sha256((weightfile_identifier + str(cut_value)).encode()).hexdigest()[:8]
    Belle2.Database.Instance().storeData(dbobj_new_name, dbobj, iov)
    return dbobj_new_name


def write_mva_weightfile_content_to_db(dbobj_name: str, content: str, iovList=(0, 0, 0, 0)):
    """Write a weightfile to database, e.g. for use in the tracking MVA filters
    *** WARNING: This function will append a hashed number of the content to the DBObject name
    to avoid overwriting existing different entries in the local DB. ***
    :param dbobj_name Name of the DBObject to identify it in the DB
    :param content Content of the weightfile to be written to the DB, in the format of xml
    :param iovList List of IoVs for which the payload is valid
    """
    # create the iov
    iov = Belle2.IntervalOfValidity(*iovList)

    # create the DBObject and set the content
    db_obj = Belle2.DatabaseRepresentationOfWeightfile()
    db_obj.m_data = content

    # write db object to 'localdb/'
    dbobj_new_name = dbobj_name  # + "_" + sha256(content.encode()).hexdigest()[:8]
    Belle2.Database.Instance().storeData(dbobj_new_name, db_obj, iov)
    return dbobj_new_name


def get_unique_dbobj_name(dbobj_name: str) -> str:
    """
    Check if dbobj_name exists in localdb/database.txt and append a number if needed.
    :param dbobj_name Name of the DBObject to check
    :return Modified name that is unique in the database
    """

    db_file = "localdb/database.txt"
    if not os.path.exists(db_file):
        return dbobj_name

    existing_names = set()
    try:
        with open(db_file, 'r') as f:
            for line in f:
                parts = line.strip().split()
                if parts:
                    existing_names.add(parts[0])
    except Exception as e:
        print(e)
        return dbobj_name

    if dbobj_name not in existing_names:
        return dbobj_name

    counter = 1
    while f"{dbobj_name}_{counter}" in existing_names:
        counter += 1

    return f"{dbobj_name}_{counter}"


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
        write_tracking_mva_filter_payloads_to_db("trackfindingcdc_WireHitBackgroundDetectorParameters",
                                                 iov, "trackfindingcdc_WireHitBackgroundDetector", 0.4)
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
