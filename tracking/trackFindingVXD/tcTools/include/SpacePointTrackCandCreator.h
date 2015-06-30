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

#include <tracking/vxdCaTracking/SharedFunctions.h> // e.g. PositionInfo
#include <tracking/vxdCaTracking/TrackletFilters.h>

#include <tracking/trackFindingVXD/segmentNetwork/Segment.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <TVector3.h>

namespace Belle2 {

  /**
   * WARNING
   *
   * using old TrackLetFilters for seed-calculation as a workaround for the time being.
   * Their input-containers expect local errors for sigmaU and sigmaV too, which are actually never used in the TrackletFilters (except for Debug-output) and will therefore set to 0.
   */

  /** bla */
  template<class SPTCContainerType, class HitType>
  struct SpacePointTrackCandCreator {
    /** bla, returns number of TCs successfully created */
    unsigned int createSPTCs(SPTCContainerType& tcContainer, std::vector<std::vector<Segment<HitType>*> > allPaths)
    {
      auto seedGenerator = TrackletFilters();
      seedGenerator.resetMagneticField(bFieldValue);

      unsigned int nTCsCreated = 0;
      int tempPDG = (stdPdgCode > 0 ? stdPdgCode : -stdPdgCode);
      double chargeVal = double((stdPdgCode > 0 ? 1 : -1) * (chargeSignFactor));
      if (tempPDG > 10
          and tempPDG <
          18) { // in this case, its a lepton. since leptons with positive sign have got negative codes, this must be taken into account
        chargeSignFactor = 1;
      } else { chargeSignFactor = -1; }
      TVectorD stateSeed(6); //(x,y,z,px,py,pz)
      TMatrixDSym covSeed(6);
      covSeed(0, 0) = 0.01 ; covSeed(1, 1) = 0.01 ; covSeed(2, 2) = 0.04 ; // 0.01 = 0.1^2 = dx*dx =dy*dy. 0.04 = 0.2^2 = dz*dz
      covSeed(3, 3) = 0.01 ; covSeed(4, 4) = 0.01 ; covSeed(5, 5) = 0.04 ;


      for (auto& aPath : allPaths) {
        nTCsCreated++;

        std::vector<PositionInfo> convertedPathrawData;
        std::vector<PositionInfo*> convertedPath;
        std::vector<const SpacePoint*> realHits;
        // collecting actual hits
        for (auto* aNode : aPath) { // is a segment here
          PositionInfo convertedHit{
            TVector3(aNode->getOuterHit()->spacePoint->getPosition()),
            TVector3(aNode->getOuterHit()->spacePoint->getPositionError()),
            0,
            0};
          convertedPathrawData.push_back(std::move(convertedHit));
          convertedPath.push_back(&convertedPathrawData.back());
          realHits.push_back(aNode->getOuterHit()->spacePoint);
        }
        // ... and the innermost hit separately:
        PositionInfo convertedHit{
          TVector3(aPath.back()->getInnerHit()->spacePoint->getPosition()),
          TVector3(aPath.back()->getInnerHit()->spacePoint->getPositionError()),
          0,
          0};
        convertedPathrawData.push_back(std::move(convertedHit));
        convertedPath.push_back(&convertedPathrawData.back());
        realHits.push_back(aPath.back()->getInnerHit()->spacePoint);


        seedGenerator.resetValues(&convertedPath);

        std::pair<TVector3, int> seedValue; // first is momentum vector, second is signCurvature

        // parameter means: false -> take last it as seed hit (true would be other end), 0: do not artificially force a momentum value onto the seed).
        seedValue = seedGenerator.calcMomentumSeed(false, 0);


        int pdgCode = seedValue.second * stdPdgCode * chargeSignFactor; // improved one for curved tracks

        stateSeed(0) = (*realHits.back()).X(); stateSeed(1) = (*realHits.back()).Y(); stateSeed(2) = (*realHits.back()).Z();
        stateSeed(3) = seedValue.first[0]; stateSeed(4) = seedValue.first[1]; stateSeed(5) = seedValue.first[2];

        tcContainer.appendNew(realHits, pdgCode, chargeVal);
      }

      return nTCsCreated;
    }


    /** WARNING hardcoded values so far, should be passed by parameter! */

    double bFieldValue = 1.5;

    int stdPdgCode = 211;

    int chargeSignFactor = 0;
  };

} //Belle2 namespace
