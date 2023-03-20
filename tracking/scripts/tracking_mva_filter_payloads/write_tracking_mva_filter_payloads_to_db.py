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
