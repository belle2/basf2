/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <vector>
#include <Math/Vector3D.h>
#include <Math/Point3D.h>

namespace Belle2 {
  class Particle;
  class TOPLikelihood;
  class ExtHit;
  class TOPBarHit;
  class TOPLikelihoodScanResult;
  namespace Variable {
    namespace TOPVariable {

      //---------------- helpers --------------------

      /**
       * Returns pointer to the related TOPLikelihood or nullptr in case of no relation.
       * Requires TOPLikelihoods.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return pointer to TOP likelihood (nullptr if N/A)
       */
      const TOPLikelihood* getTOPLikelihood(const Particle* particle);

      /**
       * Returns pointer to related ExtHit or nullptr in case of no relation.
       * Requires TOPLikelihoods and ExtHits.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return pointer to extrapolated hit at entrance to TOP (nullptr if N/A)
       */
      const ExtHit* getExtHit(const Particle* particle);

      /**
       * Returns pointer to related TOPBarHit or nullptr in case of no relation.
       * Requires TOPLikelihoods and TOPBarHits.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return pointer to MCParticle hit information at entrance to TOP (nullptr if N/A)
       */
      const TOPBarHit* getBarHit(const Particle* particle);

      /**
       * Returns pointer to the related TOPLikelihoodScanResult or nullptr in case of no relation.
       * Requires TOPLLScanner module in the processing path.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return pointer to TOPLikelihoodScanResult (nullptr if N/A)
       */
      const TOPLikelihoodScanResult* getTOPLikelihoodScanResult(const Particle* particle);


      /**
      * Returns slot ID of the particle at TOP (from ExtHit).
      * Requires TOPLikelihoods and ExtHits.
      * @param particle pointer to Particle object (nullptr is also accepted)
      * @return slot ID (0 if N/A)
      */
      int getSlotID(const Particle* particle);


      //---------------- ExtHit (or TOPBarHit) based --------------------

      /**
       * Returns slot ID of the particle at TOP (from ExtHit).
       * Requires TOPLikelihoods and ExtHits.
       * Variable name: topSlotID
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return slot ID (NaN if N/A)
       */
      double topSlotID(const Particle* particle);

      /**
       * Returns slot ID of the matched MC particle at TOP (from TOPBarHit).
       * Requires TOPLikelihoods and TOPBarHits.
       * Variable name: topSlotIDMCMatch
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return slot ID (NaN if N/A)
       */
      double topSlotIDMCMatch(const Particle* particle);

      /**
       * Returns local position of the particle entry point to the TOP.
       * Requires TOPLikelihoods, ExtHits and TOPGeometry.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param result local position if the return value is true
       * @return true on success
       */
      bool getLocalPosition(const Particle* particle, ROOT::Math::XYZPoint& result);

      /**
       * Returns the X coordinate of the particle entry point to the TOP in the local frame
       * Requires TOPLikelihoods, ExtHits and TOPGeometry.
       * Variable name: topLocalX
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return x coordinate (NaN if N/A)
       */
      double getTOPLocalX(const Particle* particle);

      /**
       * Returns the Y coordinate of the particle entry point to the TOP in the local frame.
       * Requires TOPLikelihoods, ExtHits and TOPGeometry.
       * Variable name: topLocalY
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return y coordinate (NaN if N/A)
       */
      double getTOPLocalY(const Particle* particle);

      /**
       * Returns the Z coordinate of the particle entry point to the TOP in the local frame.
       * Requires TOPLikelihoods, ExtHits and TOPGeometry.
       * Variable name: topLocalZ
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return z coordinate (NaN if N/A)
       */
      double getTOPLocalZ(const Particle* particle);

      /**
       * Returns local position of the matched MC particle entry point to the TOP.
       * Requires TOPLikelihoods, TOPBarHits and TOPGeometry.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param result local position if the return value is true
       * @return true on success
       */
      bool getLocalPositionMCMatch(const Particle* particle, ROOT::Math::XYZPoint& result);

      /**
       * Returns the X coordinate of the matched MC particle entry point to the TOP in the local frame.
       * Requires TOPLikelihoods, TOPBarHits and TOPGeometry.
       * Variable name: topLocalXMCMatch
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return x coordinate (NaN if N/A)
       */
      double getTOPLocalXMCMatch(const Particle* particle);

      /**
       * Returns the Y coordinate of the matched MC particle entry point to the TOP in the local frame.
       * Requires TOPLikelihoods, TOPBarHits and TOPGeometry.
       * Variable name: topLocalYMCMatch
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return y coordinate (NaN if N/A)
       */
      double getTOPLocalYMCMatch(const Particle* particle);

      /**
       * Returns the Z coordinate of the matched MC particle entry point to the TOP in the local frame.
       * Requires TOPLikelihoods, TOPBarHits and TOPGeometry.
       * Variable name: topLocalZMCMatch
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return z coordinate (NaN if N/A)
       */
      double getTOPLocalZMCMatch(const Particle* particle);

      /**
       * Returns the momentum vector in the local frame of the particle at entry point to the TOP.
       * Requires TOPLikelihoods, ExtHits and TOPGeometry.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param result local position if the return value is true
       * @return true on success
       */
      bool getLocalMomentum(const Particle* particle, ROOT::Math::XYZVector& result);

      /**
       * Returns the momentum azimuthal angle in the local frame of the particle at entry point to the TOP.
       * Requires TOPLikelihoods, ExtHits and TOPGeometry.
       * Variable name: topLocalPhi
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return azimuthal angle (NaN if N/A)
       */
      double getTOPLocalPhi(const Particle* particle);

      /**
       * Returns the momentum polar angle in the local frame of the particle at entry point to the TOP.
       * Requires TOPLikelihoods, ExtHits and TOPGeometry.
       * Variable name: topLocalTheta
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return polar angle (NaN if N/A)
       */
      double getTOPLocalTheta(const Particle* particle);

      /**
       * Returns the momentum vector in the local frame of the matched MC particle at entry point to the TOP.
       * Requires TOPLikelihoods, TOPBarHits and TOPGeometry.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param result local momentum vector if the return value is true
       * @return true on success
       */
      bool getLocalMomentumMCMatch(const Particle* particle, ROOT::Math::XYZVector& result);

      /**
       * Returns the momentum azimuthal angle in the local frame of the matched MC particle at entry point to the TOP.
       * Requires TOPLikelihoods, TOPBarHits and TOPGeometry.
       * Variable name: topLocalPhiMCMatch
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return azimuthal angle (NaN if N/A)
       */
      double getTOPLocalPhiMCMatch(const Particle* particle);

      /**
       * Returns the momentum polar angle in the local frame of the matched MC particle at entry point to the TOP.
       * Requires TOPLikelihoods, TOPBarHits and TOPGeometry.
       * Variable name: topLocalThetaMCMatch
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return polar angle (NaN if N/A)
       */
      double getTOPLocalThetaMCMatch(const Particle* particle);

      /**
       * Returns time-of-flight to the TOP for a given PDG.
       * Requires TOPLikelihoods and ExtHits.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param pdg PDG code or 0
       * @return time-of-flight (NaN if N/A)
       */
      double computeTOF(const Particle* particle, int pdg);

      /**
       * Returns time-of-flight of the particle from the origin to the TOP
       * Requires TOPLikelihoods and ExtHits.
       * Variable name: topTOF
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return time-of-flight (NaN if N/A)
       */
      double getTOF(const Particle* particle);

      /**
       * Returns time-of-flight of the matched MC particle from the origin to the TOP
       * Requires TOPLikelihoods and TOPBarHits.
       * Variable name: topTOFMCMatch
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return time-of-flight (NaN if N/A)
       */
      double getTOFMCMatch(const Particle* particle);

      /**
       * Returns time-of-flight from the origin to the TOP for a given PDG code
       * Requires TOPLikelihoods and ExtHits.
       * Variable name: topTOFExpert(pdg)
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param vector of size = 1 containing PDG code
       * @return time-of-flight (NaN if N/A)
       */
      double getTOFExpert(const Particle* particle, const std::vector<double>& vars);


      //---------------- Helix extrapolation --------------------

      /**
       * Returns z coordinate of the track extrapolated to TOP using helix from TrackFitResult.
       * Variable name: extrapTrackToTOPimpactZ
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return z coordinate of a point on helix at R = 120 cm (NaN if N/A)
       */
      double extrapTrackToTOPz(const Particle* particle);

      /**
       * Returns theta coordinate of the track extrapolated to TOP using helix from TrackFitResult.
       * Variable name: extrapTrackToTOPimpactTheta
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return polar angle of a point on helix at R = 120 cm (NaN if N/A)
       */
      double extrapTrackToTOPtheta(const Particle* particle);

      /**
       * Returns phi coordinate of the track extrapolated to TOP using helix from TrackFitResult.
       * Variable name: extrapTrackToTOPimpactPhi
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return azimuthal angle of a point on helix at R = 120 cm (NaN if N/A)
       */
      double extrapTrackToTOPphi(const Particle* particle);


      //---------------- TOPDigit based --------------------

      /**
       * Counts the number of photons in a given time window of the same module as particle.
       * Requires TOPLikelihoods, ExtHits and TOPDigits.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param tmin time window lower limit
       * @param tmax time window upper limit
       * @param clean on true count only good digits
       * @return digits count (NaN if N/A)
       */
      double countHits(const Particle* particle, double tmin, double tmax, bool clean);

      /**
       * Returns the number of good digits in the same module as the particle.
       * Requires TOPLikelihoods, ExtHits and TOPDigits.
       * Variable name: topDigitCount
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return digits count (NaN if N/A)
       */
      double topDigitCount(const Particle* particle);

      /**
       * Returns the number of good digits associated with the matched MC particle and in the same module as the particle.
       * Requires TOPLikelihoods, ExtHits and TOPDigits.
       * Variable name: topDigitCountMCMatch
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return digits count (NaN if N/A)
       */
      double topDigitCountMCMatch(const Particle* particle);

      /**
       * Returns the number of good, background-subtracted digits in interval [0, 50 ns] of the same module as the particle.
       * Background is obtained from the interval [-50 ns, 0].
       * Requires TOPLikelihoods, ExtHits and TOPDigits.
       * Variable name: topDigitCountSignal
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return digits count (NaN if N/A)
       */
      double topSignalDigitCount(const Particle* particle);

      /**
       * Returns the number of good digits in interval [-50 ns, 0] of the same module as the particle.
       * In this interval we measure only beam background.
       * Requires TOPLikelihoods, ExtHits and TOPDigits.
       * Variable name: topDigitCountBkg
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return digits count (NaN if N/A)
       */
      double topBackgroundDigitCount(const Particle* particle);

      /**
       * Returns the number of all digits regardless of hit quality in the same module as the particle.
       * Requires TOPLikelihoods, ExtHits and TOPDigits.
       * Variable name: topDigitCountRaw
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return digits count (NaN if N/A)
       */
      double topRawDigitCount(const Particle* particle);

      /**
       * Returns the number of good digits in a given time interval of the same module as particle.
       * Requires TOPLikelihoods, ExtHits and TOPDigits.
       * Variable name: topDigitCountInterval(tmin, tmax)
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param vars a vector of size = 2 containing time interval (tmin, tmax)
       * @return digits count (NaN if N/A)
       */
      double countTOPHitsInInterval(const Particle* particle, const std::vector<double>& vars);

      /**
       * Returns the number of good digits associated with the matched MC particle in a given time interval of the same module as particle.
       * Requires TOPLikelihoods, ExtHits and TOPDigits.
       * Variable name: topDigitCountIntervalMCMatch(tmin, tmax)
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param vars a vector of size = 2 containing time interval (tmin, tmax)
       * @return digits count (NaN if N/A)
       */
      double countTOPHitsInIntervalMCMatch(const Particle* particle, const std::vector<double>& vars);

      /**
       * Returns the number of all digits in a given time interval of the same module as particle.
       * Requires TOPLikelihoods, ExtHits and TOPDigits.
       * Variable name: topDigitCountIntervalRaw(tmin, tmax)
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param vars a vector of size = 2 containing time interval (tmin, tmax)
       * @return digits count (NaN if N/A)
       */
      double countRawTOPHitsInInterval(const Particle* particle, const std::vector<double>& vars);


      //---------------- TOPLikelihood based --------------------

      /**
       * Returns reconstruction flag.
       * Requires TOPLikelihoods.
       * Variable name: topLogLFlag
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return 1 if log likelihoods are available, otherwise 0
       */
      double getFlag(const Particle* particle);

      /**
       * Returns the number of photons used for the particle likelihood.
       * Requires TOPLikelihoods.
       * Variable name: topLogLPhotonCount
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return number of photons (NaN if N/A)
       */
      double getTOPPhotonCount(const Particle* particle);

      /**
       * Returns the expected number of photons for a given hypothesis.
       * Requires TOPLikelihoods.
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param pdg PDG code of charged stable particle
       * @return expected number of photons (NaN if N/A)
       */
      double expectedPhotonCount(const Particle* particle, int pdg);

      /**
       * Returns the expected number of photons for this particle.
       * Requires TOPLikelihoods.
       * Variable name: topLogLExpectedPhotonCount
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return expected number of photons (NaN if N/A)
       */
      double getExpectedPhotonCount(const Particle* particle);

      /**
       * Returns the expected number of photons for the assigned hypothesis.
       * Requires TOPLikelihoods.
       * Variable name: topLogLExpectedPhotonCountExpert(pdg)
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @param vars vector of size = 1 containing PDG code of charged stable particle
       * @return expected number of photons (NaN if N/A)
       */
      double getExpectedPhotonCountExpert(const Particle* particle,
                                          const std::vector<double>& vars);

      /**
       * Returns the expected background count.
       * Requires TOPLikelihoods.
       * Variable name: topLogLEstimatedBkgCount
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return expected number of background hits (NaN if N/A)
       */
      double getEstimatedBkgCount(const Particle* particle);

      /**
       * Returns electron log likelihood.
       * Requires TOPLikelihoods.
       * Variable name: topLogLElectron
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return log likelihood (NaN if N/A)
       */
      double getElectronLogL(const Particle* particle);

      /**
       * Returns muon log likelihood.
       * Requires TOPLikelihoods.
       * Variable name: topLogLMuon
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return log likelihood (NaN if N/A)
       */
      double getMuonLogL(const Particle* particle);

      /**
       * Returns pion log likelihood.
       * Requires TOPLikelihoods.
       * Variable name: topLogLPion
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return log likelihood (NaN if N/A)
       */
      double getPionLogL(const Particle* particle);

      /**
       * Returns kaon log likelihood.
       * Requires TOPLikelihoods.
       * Variable name: topLogLKaon
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return log likelihood (NaN if N/A)
       */
      double getKaonLogL(const Particle* particle);

      /**
       * Returns proton log likelihood.
       * Requires TOPLikelihoods.
       * Variable name: topLogLProton
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return log likelihood (NaN if N/A)
       */
      double getProtonLogL(const Particle* particle);

      /**
       * Returns deuteron log likelihood.
       * Requires TOPLikelihoods.
       * Variable name: topLogLDeuteron
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return log likelihood (NaN if N/A)
       */
      double getDeuteronLogL(const Particle* particle);


      //---------------- TOPLikelihoodScanResult based --------------------

      /**
       * Returns the mass corresponding to the max LL.
       * Requires TOPLLScanner module in the processing path.
       * Variable name: logLScanMass
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return mass (NaN if N/A)
       */
      double getLogLScanMass(const Particle* particle);

      /**
       * Returns the lower edge of the mass interval.
       * Requires TOPLLScanner module in the processing path.
       * Variable name: logLScanMassUpperInterval
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return lower edge (NaN if N/A)
       */
      double getLogLScanMassLowerInterval(const Particle* particle);

      /**
       * Returns the upper edge of the mass interval.
       * Requires TOPLLScanner module in the processing path.
       * Variable name: logLScanMassLowerInterval
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return upper edge (NaN if N/A)
       */
      double getLogLScanMassUpperInterval(const Particle* particle);

      /**
       * Returns the location of the Cherenkov threshold.
       * Requires TOPLLScanner module in the processing path.
       * Variable name: logLScanThreshold
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return Cherenkov threshold (NaN if N/A)
       */
      double getLogLScanThreshold(const Particle* particle);

      /**
       * Returns the expected signal photon yield at the LL maximum.
       * Requires TOPLLScanner module in the processing path.
       * Variable name: logLScanExpectedSignalPhotons
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return photon yield (NaN if N/A)
       */
      double getLogLScanExpectedSignalPhotons(const Particle* particle);


      //---------------- TOPRecBunch based --------------------

      /**
       * Returns whether the rec bunch is reconstructed.
       * Requires TOPRecBunch.
       * Variable name: topBunchIsReconstructed
       * @param particle unused
       * @return 1 if reconstructed, otherwise 0
       */
      double isTOPRecBunchReconstructed([[maybe_unused]] const Particle* particle);

      /**
       * Returns whether the rec bunch is filled.
       * Requires TOPRecBunch.
       * Variable name: topBunchIsFilled
       * @param particle unused
       * @return 1 if filled, 0 if empty, -1 if unknown
       */
      double isTOPRecBunchFilled([[maybe_unused]] const Particle* particle);

      /**
       * Returns the reconstructed bunch number relative to L1 trigger.
       * Requires TOPRecBunch.
       * Variable name: topBunchNumber
       * @param particle unused
       * @return relative bunch number (NaN if N/A)
       */
      double TOPRecBunchNumber([[maybe_unused]] const Particle* particle);

      /**
       * Returns the reconstructed bucket number within the ring
       * Requires TOPRecBunch.
       * Variable name: topBucketNumber
       * @param particle unused
       * @return buncket number (NaN if N/A)
       */
      double TOPRecBucketNumber([[maybe_unused]] const Particle* particle);

      /**
       * Is the reconstructed bunch number equal to the simulated one?
       * Requires TOPRecBunch.
       * Variable name: topBunchMCMatch
       * @param particle unused
       * @return 1 if yes, 0 otherwise
       */
      double isTOPRecBunchNumberEQsim([[maybe_unused]] const Particle* particle);

      /**
       * Returns the offset to the reconstructed bunch crossing time.
       * Requires TOPRecBunch.
       * Variable name: topBunchOffset
       * @param particle unused
       * @return offset (NaN if N/A)
       */
      double TOPRecBunchCurrentOffset([[maybe_unused]] const Particle* particle);

      /**
       * Returns the number of tracks in the TOP acceptance.
       * Requires TOPRecBunch.
       * Variable name: topBunchTrackCount
       * @param particle unused
       * @return number of tracks (NaN if N/A)
       */
      double TOPRecBunchTrackCount([[maybe_unused]] const Particle* particle);

      /**
       * Returns the number of tracks used in the bunch reconstruction.
       * Requires TOPRecBunch.
       * Variable name: topBunchUsedTrackCount
       * @param particle unused
       * @return number of used tracks (NaN if N/A)
       */
      double TOPRecBunchUsedTrackCount([[maybe_unused]] const Particle* particle);


      //-------------- Event based -----------------------------------

      /**
       * Returns the number of photons in a given slot without cleaning.
       * Requires TOPDigits.
       * Variable name: topRawPhotonsInSlot(id)
       * @param particle unused
       * @param vars vector of size = 1 containig the slot ID
       * @return number of all hits
       */
      double TOPRawPhotonsInSlot([[maybe_unused]] const Particle* particle, const std::vector<double>& vars);

      /**
       * Returns the number of good photons in a given slot.
       * Requires TOPDigits.
       * Variable name: topGoodPhotonsInSlot(id)
       * @param particle unused
       * @param vars vector of size = 1 containig the slot ID
       * @return number of good hits
       */
      double TOPGoodPhotonsInSlot([[maybe_unused]] const Particle* particle, const std::vector<double>& vars);

      /**
       * Returns the number of tracks in the same slot as the particle.
       * Requires TOPDigits, TOPLikelihoods and ExtHits.
       * Variable name: topTracksInSlot
       * @param particle pointer to Particle object (nullptr is also accepted)
       * @return number of tracks (NaN if N/A)
       */
      double TOPTracksInSlot(const Particle* particle);

    } // TOPVariable
  } // Variable
} // Belle2
