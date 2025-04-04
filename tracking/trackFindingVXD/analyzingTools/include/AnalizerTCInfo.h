/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

// tracking:
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/spacePointCreation/MCVXDPurityInfo.h>
#include <tracking/trackFindingVXD/analyzingTools/TCType.h>

// root:
#include <Math/Vector3D.h>

// stl:
#include <vector>
#include <utility> // std::pair

namespace Belle2 {
  /** simple class storing infos relevant for a TC for analyzing it. */
  class AnalizerTCInfo {
  public:


    /** carries the global coordinates of the position of the seed hit (typically the innermost hit) */
    ROOT::Math::XYZVector m_posSeed;

    /** carries the momentum vector at the position of the seed hit (typically the innermost hit) */
    ROOT::Math::XYZVector m_momSeed;

    /** a link to the TC itself */
    const SpacePointTrackCand* m_tC = nullptr;

    /** for reference TC: best test TC found, for test TC, compatible reference TC found */
    AnalizerTCInfo* m_assignedTC = nullptr;

    /** stores the iD of the particle and knows the purity for it */
    MCVXDPurityInfo m_assignedID;

    /** classifies attached TC */
    TCType::Type m_tcType;


    /** constructor, makes sure that pointers are on nullptr until set */
    AnalizerTCInfo() : m_tcType(TCType::Unclassified) {}


    /** static function for correctly creating TrackCandidates */
    static AnalizerTCInfo createTC(bool isReference, const MCVXDPurityInfo& iD, SpacePointTrackCand& aTC)
    {
      AnalizerTCInfo newTC;

      newTC.m_posSeed = aTC.getPosSeed();
      newTC.m_momSeed = aTC.getMomSeed();
      newTC.m_tC = &aTC;
      newTC.m_assignedID = iD;
      newTC.m_tcType = isReference ? TCType::Reference : TCType::Unclassified;
      return newTC;
    }


    /** find unpaired tcs and mark them with given type */
    static void markUnused(std::vector<AnalizerTCInfo>& tcs, TCType::Type newType)
    {
      for (AnalizerTCInfo& aTC : tcs) {
        if (aTC.m_assignedTC == nullptr) {
          aTC.m_tcType = newType;
        }
      }
    }


    /** for given pair of TCs their compatibility will be checked and the testTC classified, the given threshold will be used to draw the line between Contaminated and Ghost */
    static TCType::Type classifyTC(AnalizerTCInfo& referenceTC, AnalizerTCInfo& testTC, double purityThreshold,
                                   unsigned int ndfThreshold)
    {
      std::pair<int, float> testPurity = testTC.m_assignedID.getPurity();
      std::pair<int, float> refPurity = referenceTC.m_assignedID.getPurity();
      // catch ill case
      if (testPurity.first != refPurity.first) { return TCType::Unclassified; }

      if (testPurity.second < purityThreshold) { return TCType::Ghost; }

      if (testTC.m_assignedID.getNDFTotal() < ndfThreshold) { return TCType::SmallStump; }

      if (testPurity.second < 1.f) { return TCType::Contaminated; }

      if (testTC.m_assignedID.getNClustersTotal() < referenceTC.m_assignedID.getNClustersTotal()) { return TCType::Clean; }

      return TCType::Perfect;
    }


    /** function for discarding the old TC.
     *
     * the old TC attached to this will be marked as Clone and discarded
     */
    void discardTC()
    {
      m_assignedTC->m_tcType = TCType::Clone;
      m_assignedTC = nullptr;
    }


    /** function for assigning TCs to each other */
    void assignTCs(AnalizerTCInfo* otherTC)
    {
      m_assignedTC = otherTC;
      otherTC->m_assignedTC = this;
    }


    /** links otherTC to this one */
    void pairUp(AnalizerTCInfo* otherTC)
    {
      // case: this TC was not assigned before
      if (m_assignedTC == nullptr) {
        assignTCs(otherTC);
        return;
      }

      // case: was already assigned, but old one was not as good as the new one
      if (m_assignedTC->m_tcType < otherTC->m_tcType) {
        discardTC();
        assignTCs(otherTC);
        return;
      }

      // case: was already assigned, but old one was better.
      otherTC->m_assignedTC = this;
      otherTC->m_tcType = TCType::Clone;
    }


    /** a type-identifier function */
    TCType::Type getType() const { return m_tcType; }
  };
}
