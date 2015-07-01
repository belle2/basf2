/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  //! Store the Level 3 trigger information
  class L3Tag : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    L3Tag();

    //! Constructor with summary
    explicit L3Tag(int summaryWord);

    //! Constructor with summary energy sum and number of tracks
    L3Tag(int summaryWord, double Esum, int nTrks);

    //! Destructor
    virtual ~L3Tag() {}

    //! returns Level 3 Trigger flag
    bool getTriggerFlag() const { return m_SummaryWord & 0x1; }

    //! returns Level 3 Trigger bit information
    int getSummaryWord() const { return m_SummaryWord; }

    //! returns Energy sum of good ECL clusters
    double getEnergySum() const { return m_EnergySum; }

    //! returns number of good CDC Tracks
    int getNTracks() const { return m_nTracks; }

    //! assigns Level 3 Trigger information
    void setSummaryWord(int i) { m_SummaryWord = i; }

    //! assigns Energy sum of good ECL clusters
    void setEnergySum(double e) { m_EnergySum = e; }

    //! assigns number of good CDC Tracks
    void setNTracks(int n) { m_nTracks = n; }

  private:

    //! Level 3 Trigger information
    // Same information is also stored in HLTTag::m_HLTAlgoInfo[HLTTag::Level3]
    // Bit 0:  Summary bit   : taken = 1 , discarded = 0
    // Bit 1:  TrackTrigger  : taken = 1 , discarded = 0
    // Bit 2:  EnergyTrigger : taken = 1 , discarded = 0
    // Bit 3:  algorithm 3   : taken = 1 , discarded = 0
    // Bit 4:  algorithm 4   : taken = 1 , discarded = 0
    int m_SummaryWord;

    //! ECL cluster energy sum
    double m_EnergySum;

    //! number of good CDC tracks
    int m_nTracks;

    //
    ClassDef(L3Tag, 1)

  };
}
