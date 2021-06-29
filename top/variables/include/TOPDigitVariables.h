/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Jan Strube (jan.strube@desy.de)                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <vector>

class TVector3;
namespace Belle2 {
  class Particle;
  class TOPLikelihood;
  class ExtHit;
  namespace Variable {
    // contains a couple of helper functions that are related to TOP variables
    namespace TOPVariable {
      /**
       * Returns pointer to related TOPLikelihood or nullptr in case of no relation
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return pointer to TOP likelihood or nullptr
       */
      const TOPLikelihood* getTOPLikelihood(const Particle* particle);

      /**
       * Returns pointer to related ExtHit or nullptr in case of no relation
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return pointer to extrapolated hit at entrance to TOP or nullptr
       */
      const ExtHit* getExtHit(const Particle* particle);

      /**
       * Returns slot ID of a particle at TOP
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return slot ID or 0 if particle doesn't hit any slot
       */
      double getSlotID(const Particle* particle);

      /**
       * returns the local coordinate of the particle's entry point to the TOP
       */
      TVector3 getLocalPosition(const Particle* particle);

      /**
       * returns the local coordinate of the MC particle's entry point to the TOP
       */
      TVector3 getLocalPositionMCMatch(const Particle* particle);

      /**
       * returns the local coordinates of the particles momentum in the TOP
       */
      TVector3 getLocalMomentum(const Particle* particle);

      /**
       * returns the time of flight from the origin to the TOP
       */
      double getTOF(const Particle* particle);

      /**
       * returns the time of flight from the origin to the TOP under a given hypothesis
       */
      double getTOFExpert(const Particle* particle, const std::vector<double>& vars);

      /**
       * returns the average time of the first 5 (good) digits
       */
      double getAverageTimeOfFirst5(const Particle* particle);

      /**
       * counts the number of photons in the TOP in a given time frame
       * if tmin < 0, count from the time of the first photon
       */
      int countHits(const Particle* particle, double tmin, double tmax, bool clean = true);

      /**
       * counts the number of photons regardless of hit quality
       */
      int countRawHits(const Particle* particle, double tmin, double tmax);

      /**
       * returns the expected number of photons for a given hypothesis
       */
      double getExpectedPhotonCount(const Particle* particle, int pdg = 0);

      /**
       * returns the number of digits in the same module as the particle
       */
      double topDigitCount(const Particle* particle);

      /**
       * z coordinate of the track extrapolated to TOP using helix data from TrackFitResult
       */
      double extrapTrackToTOPz(const Particle* particle);

      /**
       * theta coordinate of the track extrapolated to TOP using helix data from TrackFitResult
       */
      double extrapTrackToTOPtheta(const Particle* particle);

      /**
       * phi coordinate of the track extrapolated to TOP using helix data from TrackFitResult
       */
      double extrapTrackToTOPphi(const Particle* particle);

      /**
       * returns the number of digits in all other module as the particle
       */
      double topBackgroundDigitCount(const Particle* particle);

      /**
       * returns the number of digits in all other module as the particle
       */
      double topBackgroundDigitCountRaw(const Particle* particle);

      /**
       * returns the number of all digits regardless of hit quality in the same module as the particle
       */
      double topRawDigitCount(const Particle* particle);

      /**
       * returns the largest time between to subsequent digits in the same module as the particle
       */
      double topDigitGapSize(const Particle* particle);

      /**
       * The number of reflected digits is defined as the number of digits after the gap
       * This method is a helper function to count the top digits after the largest gap
       * between subsequent hits, under the constraints gap > minGap and gap < maxGap
       */
      double topCountPhotonsAfterLargesGapWithin(const Particle* particle,
                                                 double min, double max);

      /**
       * returns the number of reflected digits in the same module as the particle
       */
      double topReflectedDigitCount(const Particle* particle);

      /**
       * returns the X coordinate of the particle entry point to the TOP in the local frame
       */
      double getTOPLocalX(const Particle* particle);

      /**
       * returns the Y coordinate of the particle entry point to the TOP in the local frame
       */
      double getTOPLocalY(const Particle* particle);

      /**
       * returns the Z coordinate of the particle entry point to the TOP in the local frame
       */
      double getTOPLocalZ(const Particle* particle);

      /**
       * returns the X coordinate of the MC particle entry point to the TOP in the local frame
       */
      double getTOPLocalXMCMatch(const Particle* particle);

      /**
       * returns the Y coordinate of the MC particle entry point to the TOP in the local frame
       */
      double getTOPLocalYMCMatch(const Particle* particle);

      /**
       * returns the Z coordinate of the MC particle entry point to the TOP in the local frame
       */
      double getTOPLocalZMCMatch(const Particle* particle);

      /**
       * returns the local phi component of the particle's momentum in the TOP
       */
      double getTOPLocalPhi(const Particle* particle);

      /**
       * returns the local theta component of the particle's momentum in the TOP
       */
      double getTOPLocalTheta(const Particle* particle);

      /**
       * returns the number of photons associated with the particle likelihood
       */
      double getTOPPhotonCount(const Particle* particle);

      /**
       * returns the expected number of photons for the assigned hypothesis
       */
      double getExpectedTOPPhotonCount(const Particle* particle,
                                       const std::vector<double>& vars);

      /**
       * returns the number of TOP photons in the given time interval
       */
      double countTOPHitsInInterval(const Particle* particle,
                                    const std::vector<double>& vars);

      /**
       * returns the number of TOP photons in the first 20 ns
       */
      double countTOPHitsInFirst20ns(const Particle* particle);

      /**
       * returns the number of raw TOP photons in the given time interval
       */
      double countRawTOPHitsInInterval(const Particle* particle,
                                       const std::vector<double>& vars);

      /**
       * returns reconstruction flag
       */
      double getFlag(const Particle* particle);

      /**
       * returns electron log likelihood
       */
      double getElectronLogL(const Particle* particle);

      /**
       * returns muon log likelihood
       */
      double getMuonLogL(const Particle* particle);

      /**
       * returns pion log likelihood
       */
      double getPionLogL(const Particle* particle);

      /**
       * returns kaon log likelihood
       */
      double getKaonLogL(const Particle* particle);

      /**
       * returns proton log likelihood
       */
      double getProtonLogL(const Particle* particle);


      /**
       * returns the mass corresponding to the max LL
       */
      double getLogLScanMass(const Particle* particle);

      /**
       * returns the lower edge of the mass interval
       */
      double getLogLScanMassLowerInterval(const Particle* particle);

      /**
       * returns the upper edge of the mass interval
       */
      double getLogLScanMassUpperInterval(const Particle* particle);



      //---------------- TOPRecBunch related --------------------

      /**
       * returns whether the rec bunch is reconstructed
       */
      double isTOPRecBunchReconstructed([[maybe_unused]] const Particle* particle);

      /**
       * returns the bunch number. Use -9999 to indicate error
       */
      double TOPRecBunchNumber([[maybe_unused]] const Particle* particle);

      /**
       * returns the current offset
       */
      double TOPRecBunchCurrentOffset([[maybe_unused]] const Particle* particle);

      /**
       * returns the number of tracks in the TOP acceptance
       */
      double TOPRecBunchTrackCount([[maybe_unused]] const Particle* particle);

      /**
       * returns the number of tracks used in the bunch reconstruction
       */
      double TOPRecBunchUsedTrackCount([[maybe_unused]] const Particle* particle);

      //-------------- Event based -----------------------------------

      /**
       * returns the number of photons in a given slot without cleaning
       */
      double TOPRawPhotonsInSlot([[maybe_unused]] const Particle* particle,
                                 const std::vector<double>& vars);

      /**
       * returns the number of good photons in a given slot
       */
      double TOPGoodPhotonsInSlot([[maybe_unused]] const Particle* particle,
                                  const std::vector<double>& vars);

      /**
       * returns the number of tracks in the same slot as the particle
       */
      double TOPTracksInSlot([[maybe_unused]] const Particle* particle);
    } // TOPVariable
  } // Variable
} // Belle2
