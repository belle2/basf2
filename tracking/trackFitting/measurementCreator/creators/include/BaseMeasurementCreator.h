/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>
#include <vector>

namespace genfit {
  class TrackPoint;
}

namespace Belle2 {
  class RecoTrack;

  /**
   * Base class for all measurement creators. These creators are used for creating (possible single elemented)
   * lists of TrackPoint-pointers for a given reco track. It is executed by the measurement creator factories used
   * in the MeasurementCreatorModule. Examples for the TrackPoints are coordinate measurements with reconstructed hits
   * or momentum estimations from dEdX in the VXD. For each of these TrackPpoint-Types you can implement a single
   * MeasurementCreator.
   */
  class BaseMeasurementCreator {
  public:
    /** Creates a MeasurementCreator which handles the creation of track points with measurements of a given kind. **/
    BaseMeasurementCreator() {}

    /** Destructor. **/
    virtual ~BaseMeasurementCreator() { }

    /** Overload this method to create track points with measurements.
     * The ordering of the track points in this list has two meanings:
     * 1. Genfit will process the first hit first.
     * 2. Only the first hit will be related to the reco hit information. This means, if this track point is downvoted by genfit, the hit will also be marked.
     * */
    virtual std::vector<genfit::TrackPoint*> createMeasurementPoints(RecoTrack& recoTrack) const = 0;

    /** Use this to set the parameters of the creator. Does nothing per default. */
    virtual void setParameter(const std::string& parameterName, const std::string& parameterValue)
    {
      B2FATAL("A parameter with the name " << parameterName << " and the value " << parameterValue << " could not be set.");
    }
  };
}
