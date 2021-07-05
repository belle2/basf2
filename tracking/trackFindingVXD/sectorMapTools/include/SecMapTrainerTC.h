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
#include <tracking/trackFindingVXD/sectorMapTools/SecMapTrainerHit.h>

#include <vector>


namespace Belle2 {

  /** simple Hit class used for sectorMap-training. */
  class SecMapTrainerTC {
  public:

    /** Constructor of class SecMapTrainerTC*/
    SecMapTrainerTC(int trackID, float pT, int pdg):
      m_index(trackID),
      m_pT(pT),
      m_pdg(pdg) {}

    /** typedef for more readable iterator-type */
    using ConstIterator = typename std::vector<SecMapTrainerHit>::const_iterator;

    /** adds hit to Track.
     * Always add from outer to inner hits, the TC-Class can not detect that by itself! */
    void addHit(SecMapTrainerHit hit) { m_hits.push_back(hit); }

    /** returns hits of track. */
    const std::vector<SecMapTrainerHit>& getHits() const { return m_hits; }

    /** returns the iterator-position pointing to the outermost hit.
     * '++' moves the iterator inwards. */
    ConstIterator outermostHit() const { return m_hits.begin(); }

    /** returns the iterator-position pointing _after_ the innermost hit. */
    ConstIterator innerEnd() const { return m_hits.end(); }

    /** returns indexNumber of current track. */
    int getTrackID() const { return m_index; }

    /** returns pdgCode of current track. */
    int getPDG() const { return m_pdg; }

    /** returns number of hits attached to curren track. */
    unsigned size() const { return m_hits.size(); }

    /** get Pt of track. */
    float getPt() const { return m_pT; }

  protected:
    int m_index; /**< stores index number of track. */
    float m_pT; /**< stores transverse momentum value for track classification. */
    int m_pdg; /**< stores pdGCode of particle. */
    std::vector<SecMapTrainerHit> m_hits; /**< contains hits attached to track. */
  };
}

