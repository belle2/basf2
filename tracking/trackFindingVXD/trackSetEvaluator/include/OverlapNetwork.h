/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
