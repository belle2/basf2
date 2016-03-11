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

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <TVector3.h>
#include <TVectorD.h>
#include <TMatrixDSym.h>

namespace Belle2 {

  /**
   * WARNING
   *
   * using old TrackLetFilters for seed-calculation as a workaround for the time being.
   * Their input-containers expect local errors for sigmaU and sigmaV too, which are actually never used in the TrackletFilters (except for Debug-output) and will therefore set to 0.
   */

  /** small class to take simple vectors of SpacePoints and convert them to real SpacePointTrackCand including realistic seed */
  template<class SPTCContainerType>
  struct SpacePointTrackCandCreator {

    /** takes simple vectors of SpacePoints and convert them to real SpacePointTrackCand including realistic seed,
     * returns number of TCs successfully created. */
    unsigned int createSPTCs(SPTCContainerType& tcContainer, std::vector<std::vector<const SpacePoint*> > allPaths,
                             bool removeVirtualIP)
    {
      auto seedGenerator = TrackletFilters();
      seedGenerator.resetMagneticField(bFieldValue);

      unsigned int nTCsCreated = 0;
      int tempPDG = (stdPdgCode > 0 ? stdPdgCode : -stdPdgCode);
      double chargeVal = double((stdPdgCode > 0 ? 1 : -1) * (chargeSignFactor));
      if (tempPDG > 10 and tempPDG < 18) {
        // in this case, its a lepton. since leptons with positive sign have got negative codes, this must be taken into account
        chargeSignFactor = 1;
      } else { chargeSignFactor = -1; }
      TVectorD stateSeed(6); //(x,y,z,px,py,pz)
      TMatrixDSym covSeed(6);
      covSeed(0, 0) = 0.01 ; covSeed(1, 1) = 0.01 ; covSeed(2, 2) = 0.04 ; // 0.01 = 0.1^2 = dx*dx =dy*dy. 0.04 = 0.2^2 = dz*dz
      covSeed(3, 3) = 0.01 ; covSeed(4, 4) = 0.01 ; covSeed(5, 5) = 0.04 ;


      for (std::vector<const SpacePoint*>& aPath : allPaths) {
        nTCsCreated++;

        std::vector<PositionInfo> convertedPathrawData;
        convertedPathrawData.reserve(aPath.size());
        std::vector<PositionInfo*> convertedPath;
        convertedPath.reserve(aPath.size());
        // collecting actual hits
        for (const SpacePoint* aNode : aPath) { // is a const SpacePoint* here
//      B2WARNING("SpacePointTrackCandCreator: spacePoint has got PositionError: " << aNode->getPositionError().PrintString())
          PositionInfo convertedHit{
            TVector3(aNode->getPosition()),
            TVector3(aNode->getPositionError()),
            0,
            0};
          convertedPathrawData.push_back(std::move(convertedHit));
          convertedPath.push_back(&convertedPathrawData.back());
        }

        seedGenerator.resetValues(&convertedPath);

        std::pair<TVector3, int> seedValue; // first is momentum vector, second is signCurvature

        // parameter means: false -> take last it as seed hit (true would be other end), 0: do not artificially force a momentum value onto the seed).
        seedValue = seedGenerator.calcMomentumSeed(false, 0);

        int pdgCode = seedValue.second * stdPdgCode * chargeSignFactor; // improved one for curved tracks

        // remove virtualIP in tc if wanted to:
        if (removeVirtualIP) {
          std::vector<const SpacePoint*> newPath;
          for (const SpacePoint* aNode : aPath) {
            if (aNode->getType() == VXD::SensorInfoBase::SensorType::VXD) continue;
            newPath.push_back(aNode);
          }
          stateSeed(0) = (*newPath.back()).X(); stateSeed(1) = (*newPath.back()).Y(); stateSeed(2) = (*newPath.back()).Z();
          stateSeed(3) = seedValue.first[0]; stateSeed(4) = seedValue.first[1]; stateSeed(5) = seedValue.first[2];
          tcContainer.appendNew(newPath, pdgCode, chargeVal);
        } else { // keep vIP
          stateSeed(0) = (*aPath.back()).X(); stateSeed(1) = (*aPath.back()).Y(); stateSeed(2) = (*aPath.back()).Z();
          stateSeed(3) = seedValue.first[0]; stateSeed(4) = seedValue.first[1]; stateSeed(5) = seedValue.first[2];
          tcContainer.appendNew(aPath, pdgCode, chargeVal);
        }
      }

      return nTCsCreated;
    }


    /** WARNING hardcoded values so far, should be passed by parameter! */

    double bFieldValue = 1.5; /**< magnetic field. */

    int stdPdgCode = 211; /**< standard value for pdg code used. */

    int chargeSignFactor = 0; /**< == 1 if pdg code is for a lepton, -1 if not. */
  };

} //Belle2 namespace
