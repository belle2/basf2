/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationsObject.h>
#include <utility>

namespace Belle2 {
  class V0 : public RelationsObject {
  public:
    /** Constructor without arguments; needed for I/O. */
    V0();

    /** Constructor taking a pair of a Track and a TrackFitResult.
     *
     *  By taking such a pair the likelihood is minimized, that later people give the Track and the
     *  TrackFitResult in mixed order to the V0.
     *  @param trackPairPositive  A pair of the Track forming one part of the V0 and the result of the refitting.
     *  @param trackPairNegative  @sa trackPairPositive
     */
    V0(const std::pair<Track, TrackFitResult>& trackPairPositive,
       const std::pair<Track, TrackFitResult>& trackPairNegative);



  private:
    /** Indicates which positively charged track was used for this V0. */
    short m_trackIndexPositive;

    /** Indicates which negatively charged track was used for this V0. */
    short m_trackIndexNegative;

    /** Points to the new TrackFitResult of the positive Track.
     *
     *  As the V0 particle may have traveled through material and may have added hits,
     *  that are unlikely really part of the track, a new TrackFitResult has to be created,
     *  that contains the information extrapolated to the area, where the two tracks from
     *  this V0 intersect.
     */
    short m_trackFitResultIndexPositive;

    /** Points to the new TrackFitResult of the positive Track.
     *
     *  @sa m_trackFitResultIndexPositive
     */
    short m_trackFitResultIndexNegative;

    ClassDef(V0, 1);
  };
}
