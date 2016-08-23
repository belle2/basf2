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
    OverlapNetwork(const std::vector <std::vector <unsigned short> >& overlapMatrix) :
      m_overlapMatrix(overlapMatrix)
    {}

    /** Returns a vector of indices of SpacePointTrackCand, that overlap with the one at trackIndex. */
    std::vector<unsigned short>& getOverlapForTrackIndex(unsigned short trackIndex)
    {
      return m_overlapMatrix[trackIndex];
    }

  private:
    std::vector<std::vector <unsigned short> > m_overlapMatrix;///<Input, format see OverlapMatrixCreator.
    ClassDef(OverlapNetwork, 1)
  };
}
