/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>


namespace Belle2 {

  /** Trivial struct used by the SecMapTrainerXYModules to store/cluster the results of one measurement.
   * extra infos allows to uniquely identify tracks again.*/
  struct SecMapTrainDataSet {

    /** number of the experiment this dataset is taken from. */
    unsigned expNo;

    /** number of the run this dataset is taken from. */
    unsigned runNo;

    /** number of the event this dataset is taken from. */
    unsigned evtNo;

    /** number of the reference track this dataset is taken from (its ID in the datastore). */
    unsigned trackNo;

    /** Pdg given by reference track. */
    int pdg;

    /** a human readable string for the filterID (does make it easier to read when studying the data). */
    std::string filterID;

    /** human readable strings for the sectorIDs. */
    std::vector<std::string> sectorIDs;

    /** the value determined using the filter listed above. */
    double value;
  };

}

