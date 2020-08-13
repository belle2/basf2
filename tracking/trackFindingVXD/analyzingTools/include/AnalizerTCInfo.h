/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// tracking:
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/spacePointCreation/MCVXDPurityInfo.h>
#include <tracking/trackFindingVXD/analyzingTools/TCType.h>

// root:
#include <TVector3.h>

// stl:
#include <vector>
#include <utility> // std::pair

namespace Belle2 {
  /** simple class storing infos relevant for a TC for analizing it. */
  class AnalizerTCInfo {
  public:


    /** carries the global coordinates of the position of the seed hit (typically the innermost hit) */
    TVector3 posSeed;

    /** carries the momentum vector at the position of the seed hit (typically the innermost hit) */
    TVector3 momSeed;

    /** a link to the TC itself */
    const SpacePointTrackCand* tC;

    /** for reference TC: best test TC found, for test TC, compatible reference TC found */
    AnalizerTCInfo* assignedTC;

    /** stores the iD of the particle and knows the purity for it */
    MCVXDPurityInfo assignedID;

    /** classifies attached TC */
    TCType::Type tcType;


    /** constructor, makes sure that pointers are on nullptr until set */
    AnalizerTCInfo() : tC(nullptr), assignedTC(nullptr), tcType(TCType::Unclassified) {}


    /** static function for correctly creating TrackCandidates */
    static AnalizerTCInfo createTC(bool isReference, const MCVXDPurityInfo& iD, SpacePointTrackCand& aTC)
    {
      AnalizerTCInfo newTC;

      newTC.posSeed = aTC.getPosSeed();
      newTC.momSeed = aTC.getMomSeed();
      newTC.tC = &aTC;
      newTC.assignedID = iD;
      newTC.tcType = isReference ? TCType::Reference : TCType::Unclassified;
      return newTC;
    }


    /** find unpaired tcs and mark them with given type */
    static void markUnused(std::vector<AnalizerTCInfo>& tcs, TCType::Type newType)
    {
      for (AnalizerTCInfo& aTC : tcs) {
        if (aTC.assignedTC == nullptr) {
          aTC.tcType = newType;
        }
      }
    }


    /** for given pair of TCs their compatibility will be checked and the testTC classified, the given threshold will be used to draw the line between Contaminated and Ghost */
    static TCType::Type classifyTC(AnalizerTCInfo& referenceTC, AnalizerTCInfo& testTC, double purityThreshold,
                                   unsigned int ndfThreshold)
    {
      std::pair<int, float> testPurity = testTC.assignedID.getPurity();
      std::pair<int, float> refPurity = referenceTC.assignedID.getPurity();
      // catch ill case
      if (testPurity.first != refPurity.first) { return TCType::Unclassified; }

      if (testPurity.second < purityThreshold) { return TCType::Ghost; }

      if (testTC.assignedID.getNDFTotal() < ndfThreshold) { return TCType::SmallStump; }

      if (testPurity.second < 1.f) { return TCType::Contaminated; }

      if (testTC.assignedID.getNClustersTotal() < referenceTC.assignedID.getNClustersTotal()) { return TCType::Clean; }

      return TCType::Perfect;
    }


    /** function for discarding the old TC.
     *
     * the old TC attached to this will be marked as Clone and discarded
     */
    void discardTC()
    {
      assignedTC->tcType = TCType::Clone;
      assignedTC = nullptr;
    }


    /** function for assigning TCs to each other */
    void assignTCs(AnalizerTCInfo* otherTC)
    {
      assignedTC = otherTC;
      otherTC->assignedTC = this;
    }


    /** links otherTC to this one */
    void pairUp(AnalizerTCInfo* otherTC)
    {
      // case: this TC was not assigned before
      if (assignedTC == nullptr) {
        assignTCs(otherTC);
        return;
      }

      // case: was already assigned, but old one was not as good as the new one
      if (assignedTC->tcType < otherTC->tcType) {
        discardTC();
        assignTCs(otherTC);
        return;
      }

      // case: was already assigned, but old one was better.
      otherTC->assignedTC = this;
      otherTC->tcType = TCType::Clone;
    }


    /** a type-identifier function */
    TCType::Type getType() const { return tcType; }
  };
}
