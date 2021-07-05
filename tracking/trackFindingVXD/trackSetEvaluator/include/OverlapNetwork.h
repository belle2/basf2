/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vector>


namespace Belle2 {
  /** Hold information about overlap of SpacePointTrackCand. */
  class OverlapNetwork : public RelationsObject {
  public:
    /** Takes an OverlapMatrix in the form, that comes from the OverlapMatrixCreator. */
    explicit OverlapNetwork(const std::vector <std::vector <unsigned short> >& overlapMatrix) :
      m_overlapMatrix(overlapMatrix)
    {}

    /** Returns a vector of indices of SpacePointTrackCand, that overlap with the one at trackIndex.
     *
     *  If a track doesn't have any overlaps with any other track,
     *  the return value will be an empty vector.
     *  In the overlapMatrix, there is a vector for every SpacePointTrackCand, so
     *  in that sense, there is no special treatment of tracks without overlaps.
     */
    std::vector<unsigned short> const& getOverlapForTrackIndex(unsigned short trackIndex) const
    {
      return m_overlapMatrix[trackIndex];
    }

  private:
    std::vector<std::vector <unsigned short> > const m_overlapMatrix;///<Input, format see OverlapMatrixCreator.
    ClassDef(OverlapNetwork, 1)
  };
}
