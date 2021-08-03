/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <top/variables/TOPDigitVariables.h>

// // framework
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

// // dataobjects
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/Track.h>
#include <analysis/dataobjects/Particle.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <top/dataobjects/TOPLikelihoodScanResult.h>

#include <top/dataobjects/TOPBarHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>


#include <algorithm> // for sort
using namespace std;

namespace Belle2 {
  // contains a couple of helper functions that are related to TOP variables
  namespace Variable {
    namespace TOPVariable {
      // returns the TOP likelihood that is associated with a given particle
      const TOPLikelihood* getTOPLikelihood(const Particle* particle)
      {
        if (not particle) return nullptr;
        const auto* track = particle->getTrack();
        return track ? track->getRelated<TOPLikelihood>() : nullptr;
      }

      // returns the ExtHit that is associated with a given particle
      const ExtHit* getExtHit(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        return topLikelihood ? topLikelihood->getRelated<ExtHit>() : nullptr;
      }

      // returns the TOP slot ID of the particle
      double getSlotID(const Particle* particle)
      {
        const auto* extHit = getExtHit(particle);
        // zero is invalid slot ID, e.g. particle didn't hit the TOP
        return extHit ? extHit->getCopyID() : 0;
      }

      // returns the local coordinate of the particle's entry point to the TOP
      TVector3 getLocalPosition(const Particle* particle)
      {
        const auto* extHit = getExtHit(particle);
        if (not extHit) return TVector3(0, 0, 0);
        int slotID = extHit->getCopyID();
        const auto& position = extHit->getPosition(); // TVector3
        const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
        if (not geo or not geo->isModuleIDValid(slotID)) return TVector3(0, 0, 0);
        const auto& module = geo->getModule(slotID);
        return module.pointToLocal(position); // TVector3
      }

      // returns the local coordinate of the MC particle's entry point to the TOP
      TVector3 getLocalPositionMCMatch(const Particle* particle)
      {
        const MCParticle* mcparticle = particle->getRelatedTo<MCParticle>();
        if (mcparticle == nullptr) {
          return TVector3(0, 0, 0);
        }
        const auto* barHit = mcparticle->getRelated<TOPBarHit>();
        if (!barHit) {
          return TVector3(0, 0, 0);
        }
        int slotID = barHit->getModuleID();
        const auto& position = barHit->getPosition(); // TVector3
        const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
        if (not geo or not geo->isModuleIDValid(slotID)) return TVector3(0, 0, 0);
        const auto& module = geo->getModule(slotID);
        return module.pointToLocal(position); // TVector3
      }

      // returns the local coordinates of the particles momentum in the TOP
      TVector3 getLocalMomentum(const Particle* particle)
      {
        const auto* extHit = getExtHit(particle);
        if (not extHit) return TVector3(0, 0, 0);
        int slotID = extHit->getCopyID();
        const auto& momentum = extHit->getMomentum(); // TVector3
        const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
        if ((not geo) or (not geo->isModuleIDValid(slotID))) return TVector3(0, 0, 0);
        const auto& module = geo->getModule(slotID);
        return module.momentumToLocal(momentum); // TVector3
      }

      // helper function to compute the TOF for an arbitrary hypothesis
      double computeTOF(const Particle* particle, int pdg)
      {
        const auto* extHit = getExtHit(particle);
        if (not extHit) return 0;
        auto extPDGCode = abs(extHit->getPdgCode());
        double pmom = particle->getMomentumMagnitude();
        double massExtHit = Const::ChargedStable(extPDGCode).getMass();
        double betaExtHit = pmom / sqrt(pmom * pmom + massExtHit * massExtHit);
        double mass = pdg == 0 ? particle->getMass() : Const::ChargedStable(abs(pdg)).getMass();
        double beta = pmom / sqrt(pmom * pmom + mass * mass);
        return extHit->getTOF() * betaExtHit / beta;
      }

      // returns the time of flight from the origin to the TOP
      double getTOF(const Particle* particle)
      {
        return computeTOF(particle, 0);
      }

      // returns the time of flight from the origin to the TOP under a given hypothesis
      double getTOFExpert(const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 1) {
          B2FATAL("Need exactly one parameter (pdg id).");
        }
        int pdg = static_cast<int>(vars[0]);
        return computeTOF(particle, pdg);
      }

      // returns the average time of the first 5 (good) digits
      double getAverageTimeOfFirst5(const Particle* particle)
      {
        int slotID = static_cast<int>(getSlotID(particle));
        StoreArray<TOPDigit> digits;
        vector<double> digitTimes;
        for (const auto& digit : digits) {
          if (digit.getModuleID() != slotID) continue;
          // skip bad digits only when we want to clean
          if (digit.getHitQuality() != TOPDigit::c_Good) continue;
          digitTimes.push_back(digit.getTime());
        }
        if (digitTimes.empty()) return 0;
        sort(digitTimes.begin(), digitTimes.end());
        double T0 = 0;
        size_t count = 0;
        for (auto t : digitTimes) {
          T0 += t;
          count += 1;
          if (count == 5) break;
        }
        return T0 / count;
      }

      // counts the number of photons in the TOP in a given time frame
      // if tmin < 0, count from the time of the first photon
      int countHits(const Particle* particle, double tmin, double tmax, bool clean)
      {
        int slotID = static_cast<int>(getSlotID(particle));
        StoreArray<TOPDigit> digits;
        vector<double> digitTimes;
        for (const auto& digit : digits) {
          if (digit.getModuleID() != slotID) continue;
          // skip bad digits only when we want to clean
          if (clean && digit.getHitQuality() != TOPDigit::c_Good) continue;
          digitTimes.push_back(digit.getTime());
        }
        if (digitTimes.empty()) return 0;
        sort(digitTimes.begin(), digitTimes.end());
        int count = 0;
        if (tmin < 0) tmin = digitTimes[0];
        for (auto t : digitTimes) {
          if (t > tmax) break;
          if (t >= tmin) ++count;
        }
        return count;
      }

      // counts the number of photons regardless of hit quality
      int countRawHits(const Particle* particle, double tmin, double tmax)
      {
        return countHits(particle, tmin, tmax, false);
      }

      // returns the expected number of photons for a given hypothesis
      double getExpectedPhotonCount(const Particle* particle, int pdg)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return 0;
        // if the user does select a hypothesis, use the particle's pdg code
        pdg = pdg != 0 ? pdg : particle->getPDGCode();
        const auto& chargedStable = Const::chargedStableSet.find(abs(pdg));
        if (chargedStable == Const::invalidParticle) return 0; // PDG code not one of e, mu, pi, K, p, d
        return topLikelihood->getEstPhot(chargedStable);
      }
      //! @returns the number of digits in the same module as the particle
      double topDigitCount(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) {
          return -1.0;
        }
        auto extHits = trk->getRelationsWith<ExtHit>();
        int thisModuleID = static_cast<int>(getSlotID(particle));
        if (thisModuleID == 0) return 0;
        StoreArray<TOPDigit> topDigits;
        int count = 0;
        for (const auto& t : topDigits) {
          if (t.getModuleID() != thisModuleID) continue; // catch the case where one of the module IDs is negative
          if (t.getHitQuality() != TOPDigit::c_Good) continue;
          count += 1;
        }
        return count;
      }

      //! @returns the number of digits in all other module as the particle
      double topBackgroundDigitCount(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) {
          return -1.0;
        }
        auto extHits = trk->getRelationsWith<ExtHit>();
        int thisModuleID = static_cast<int>(getSlotID(particle));
        if (thisModuleID == 0) return 0;
        StoreArray<TOPDigit> topDigits;
        int count = 0;
        for (const auto& t : topDigits) {
          if (abs(t.getModuleID()) == abs(thisModuleID)) continue; // catch the case where one of the module IDs is negative
          if (t.getHitQuality() != TOPDigit::c_Good) continue;
          count += 1;
        }
        return count;
      }

      //! @returns the number of digits in all other module as the particle
      double topBackgroundDigitCountRaw(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) {
          return -1.0;
        }
        auto extHits = trk->getRelationsWith<ExtHit>();
        int thisModuleID = static_cast<int>(getSlotID(particle));
        if (thisModuleID == 0) return 0;
        StoreArray<TOPDigit> topDigits;
        int count = 0;
        for (const auto& t : topDigits) {
          if (abs(t.getModuleID()) == abs(thisModuleID)) continue; // catch the case where one of the module IDs is negative
          count += 1;
        }
        return count;
      }

      //! @returns the number of all digits regardless of hit quality in the same module as the particle
      double topRawDigitCount(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) {
          return -1.0;
        }
        auto extHits = trk->getRelationsWith<ExtHit>();
        int thisModuleID = static_cast<int>(getSlotID(particle));
        if (thisModuleID == 0) return 0;
        StoreArray<TOPDigit> topDigits;
        int count = 0;
        for (const auto& t : topDigits) {
          if (abs(t.getModuleID()) != abs(thisModuleID)) continue; // catch the case where one of the module IDs is negative
          count += 1;
        }
        return count;
      }

      //! @returns the largest time between to subsequent digits in the same module as the particle
      double topDigitGapSize(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) {
          return -1.0;
        }
        auto extHits = trk->getRelationsWith<ExtHit>();
        int thisModuleID = static_cast<int>(getSlotID(particle));
        if (thisModuleID == 0) return 0;
        StoreArray<TOPDigit> topDigits;
        double maxGap = 0; // the largest time difference between two consecutive hits
        vector<double> digitTimes; // all digits in the module that the track entered
        for (const auto& t : topDigits) {
          if (abs(t.getModuleID()) != abs(thisModuleID)) continue;
          if (t.getHitQuality() != TOPDigit::c_Good) continue;
          digitTimes.push_back(t.getTime());
        }
        if (digitTimes.empty()) {
          return -1.0;
        }
        sort(digitTimes.begin(), digitTimes.end());
        for (size_t i = 0; i < digitTimes.size() - 1; ++i) {
          double gap = digitTimes[i + 1] - digitTimes[i];
          if (gap > maxGap) {
            maxGap = gap;
          }
        }
        return maxGap;
      }

      // The number of reflected digits is defined as the number of digits after the gap
      // This method is a helper function to count the top digits after the largest gap
      // between subsequent hits, under the constraints gap > minGap and gap < maxGap
      double topCountPhotonsAfterLargesGapWithin(const Particle* particle, double minGap, double maxGap)
      {
        auto trk = particle->getTrack();
        if (not trk) {
          return -1.0;
        }
        auto extHits = trk->getRelationsWith<ExtHit>();
        int thisModuleID = static_cast<int>(getSlotID(particle));
        if (thisModuleID == 0) return 0;
        StoreArray<TOPDigit> topDigits;
        vector<double> digitTimes; // the times for all digits in the module that the track entered
        for (const auto& t : topDigits) {
          if (abs(t.getModuleID()) != abs(thisModuleID)) continue;
          if (t.getHitQuality() != TOPDigit::c_Good) continue;
          digitTimes.push_back(t.getTime());
        }
        if (digitTimes.empty()) {
          return -1.0;
        }

        double currentMaxGap = -1; // the largest time difference between two consecutive hits
        size_t maxGapIndex = 0; // the index of the first hit *after* the gap
        sort(digitTimes.begin(), digitTimes.end());
        for (size_t i = 0; i < digitTimes.size() - 1; ++i) {
          double gap = digitTimes[i + 1] - digitTimes[i];
          if ((gap > minGap) and (gap < maxGap) and (gap > currentMaxGap)) {
            currentMaxGap = gap;
            maxGapIndex = i + 1;
          }
        }
        return digitTimes.size() - maxGapIndex;
      }

      //! @z coordinate of the track extrapolated to TOP using helix data from TrackFitResult
      double extrapTrackToTOPz(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) {
          return std::numeric_limits<double>::quiet_NaN();
        }
        auto trkfit = trk->getTrackFitResultWithClosestMass(Belle2::Const::ChargedStable(std::abs(particle->getPDGCode())));
        auto top = trkfit->getHelix();
        double arcLength = top.getArcLength2DAtCylindricalR(120);
        const auto& result = top.getPositionAtArcLength2D(arcLength);
        return result.z();
      }

      //! @theta coordinate of the track extrapolated to TOP using helix data from TrackFitResult
      double extrapTrackToTOPtheta(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) {
          return std::numeric_limits<double>::quiet_NaN();
        }
        auto trkfit = trk->getTrackFitResultWithClosestMass(Belle2::Const::ChargedStable(std::abs(particle->getPDGCode())));
        auto top = trkfit->getHelix();
        double arcLength = top.getArcLength2DAtCylindricalR(120);
        const auto& result = top.getPositionAtArcLength2D(arcLength);
        return result.Theta();
      }

      //! @phi coordinate of the track extrapolated to TOP using helix data from TrackFitResult
      double extrapTrackToTOPphi(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) {
          return std::numeric_limits<double>::quiet_NaN();
        }
        auto trkfit = trk->getTrackFitResultWithClosestMass(Belle2::Const::ChargedStable(std::abs(particle->getPDGCode())));
        auto top = trkfit->getHelix();
        double arcLength = top.getArcLength2DAtCylindricalR(120);
        const auto& result = top.getPositionAtArcLength2D(arcLength);
        return result.Phi();
      }

      //! @returns the number of reflected digits in the same module as the particle
      double topReflectedDigitCount(const Particle* particle)
      {
        return topCountPhotonsAfterLargesGapWithin(particle, 0, 10000);
      }

      double topReflectedDigitCountExpert(const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 2) {
          B2FATAL("Need exactly two parameters (min, max)");
        }
        return topCountPhotonsAfterLargesGapWithin(particle, vars[0], vars[1]);
      }

      //! @returns the X coordinate of the particle entry point to the TOP in the local frame
      double getTOPLocalX(const Particle* particle)
      {
        return TOPVariable::getLocalPosition(particle).X();
      }

      //! @returns the Y coordinate of the particle entry point to the TOP in the local frame
      double getTOPLocalY(const Particle* particle)
      {
        return TOPVariable::getLocalPosition(particle).Y();
      }

      //! @returns the Z coordinate of the particle entry point to the TOP in the local frame
      double getTOPLocalZ(const Particle* particle)
      {
        return TOPVariable::getLocalPosition(particle).Z();
      }

      //! @returns the X coordinate of the MC particle entry point to the TOP in the local frame
      double getTOPLocalXMCMatch(const Particle* particle)
      {
        return TOPVariable::getLocalPositionMCMatch(particle).X();
      }

      //! @returns the Y coordinate of the MC particle entry point to the TOP in the local frame
      double getTOPLocalYMCMatch(const Particle* particle)
      {
        return TOPVariable::getLocalPositionMCMatch(particle).Y();
      }

      //! @returns the Z coordinate of the MC particle entry point to the TOP in the local frame
      double getTOPLocalZMCMatch(const Particle* particle)
      {
        return TOPVariable::getLocalPositionMCMatch(particle).Z();
      }

      //! @returns the local phi component of the particle's momentum in the TOP
      double getTOPLocalPhi(const Particle* particle)
      {
        return TOPVariable::getLocalMomentum(particle).Phi();
      }

      //! @returns the local theta component of the particle's momentum in the TOP
      double getTOPLocalTheta(const Particle* particle)
      {
        return TOPVariable::getLocalMomentum(particle).Theta();
      }

      //! @returns the number of photons associated with the particle likelihood
      double getTOPPhotonCount(const Particle* particle)
      {
        const auto* topLikelihood = TOPVariable::getTOPLikelihood(particle);
        return topLikelihood ? topLikelihood->getNphot() : 0;
      }

      //! @returns the expected number of photons for the assigned hypothesis
      double getExpectedTOPPhotonCount(const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 1) {
          B2FATAL("Need exactly one parameter (pdg id).");
        }
        return TOPVariable::getExpectedPhotonCount(particle, static_cast<int>(vars[0]));
      }

      //! @returns the number of TOP photons in the given time interval
      double countTOPHitsInInterval(const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 2) {
          B2FATAL("Need exactly two parameters (tmin, tmax)");
        }
        return TOPVariable::countHits(particle, vars[0], vars[1]);
      }

      //! @returns the number of TOP photons in the first 20 ns
      double countTOPHitsInFirst20ns(const Particle* particle)
      {
        return TOPVariable::countHits(particle, -1.0, 20.0);
      }

      //! @returns the number of raw TOP photons in the given time interval
      double countRawTOPHitsInInterval([[maybe_unused]] const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 2) {
          B2FATAL("Need exactly two parameters (tmin, tmax)");
        }
        return TOPVariable::countRawHits(particle, vars[0], vars[1]);
      }

      double getFlag(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return 0;
        return topLikelihood->getFlag();
      }

      double getElectronLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return 0;
        return topLikelihood->getLogL_e();
      }

      double getMuonLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return 0;
        return topLikelihood->getLogL_mu();
      }

      double getPionLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return 0;
        return topLikelihood->getLogL_pi();
      }

      double getKaonLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return 0;
        return topLikelihood->getLogL_K();
      }

      double getProtonLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return 0;
        return topLikelihood->getLogL_p();
      }


      double getLogLScanMass(const Particle* particle)
      {
        const auto* track = particle->getTrack();
        if (!track) return -1;
        auto scanRes = track->getRelated<TOPLikelihoodScanResult>();
        if (!scanRes) {
          B2WARNING("No TOPLikelihoodScanResult objcte found. Are you sure you added TOPLLScanner to the path?");
          return -1;
        }
        return scanRes->getMostLikelyMass();
      }

      double getLogLScanMassUpperInterval(const Particle* particle)
      {
        const auto* track = particle->getTrack();
        if (!track) return -1;
        auto scanRes = track->getRelated<TOPLikelihoodScanResult>();
        if (!scanRes) {
          B2WARNING("No TOPLikelihoodScanResult object found. Are you sure you added TOPLLScanner to the path?");
          return -1;
        }
        return scanRes->getMostLikelyMassIntervalUp();
      }


      double getLogLScanMassLowerInterval(const Particle* particle)
      {
        const auto* track = particle->getTrack();
        if (!track) return -1;
        auto scanRes = track->getRelated<TOPLikelihoodScanResult>();
        if (!scanRes) {
          B2WARNING("No TOPLikelihoodScanResult object found. Are you sure you added TOPLLScanner to the path?");
          return -1;
        }
        return scanRes->getMostLikelyMassIntervalLow();
      }

      double getLogLScanThreshold(const Particle* particle)
      {
        const auto* track = particle->getTrack();
        if (!track) return -1;
        auto scanRes = track->getRelated<TOPLikelihoodScanResult>();
        if (!scanRes) {
          B2WARNING("No TOPLikelihoodScanResult objcte found. Are you sure you added TOPLLScanner to the path?");
          return -1;
        }
        return scanRes->getThreshold();
      }


      double getLogLScanExpectedSignalPhotons(const Particle* particle)
      {
        const auto* track = particle->getTrack();
        if (!track) return -1;
        auto scanRes = track->getRelated<TOPLikelihoodScanResult>();
        if (!scanRes) {
          B2WARNING("No TOPLikelihoodScanResult objcte found. Are you sure you added TOPLLScanner to the path?");
          return -1;
        }
        return scanRes->getMostLikelySignalPhotonCount();
      }


      //---------------- TOPRecBunch related --------------------

      //! @returns whether the rec bunch is reconstructed
      double isTOPRecBunchReconstructed([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        // Attention! 0.0 is false, everything else is true
        // returning -1, like for the others will most likely lead to bugs
        // if the caller is not careful about return values.
        if (not recBunch.isValid()) return 0.0;
        return recBunch->isReconstructed();
      }

      //! returns the bunch number. Use -9999 to indicate error
      double TOPRecBunchNumber([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return -9999.0;
        return recBunch->getBunchNo();
      }

      //! returns the current offset
      double TOPRecBunchCurrentOffset([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return -9999.0;
        return recBunch->getCurrentOffset();
      }

      //! returns the number of tracks in the TOP acceptance
      double TOPRecBunchTrackCount([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return -9999.0;
        return recBunch->getNumTracks();
      }

      //! returns the number of tracks used in the bunch reconstruction
      double TOPRecBunchUsedTrackCount([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return -9999.0;
        return recBunch->getUsedTracks();
      }
      //-------------- Event based -----------------------------------

      //! returns the number of photons in a given slot without cleaning
      double TOPRawPhotonsInSlot([[maybe_unused]] const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 1) { B2FATAL("Need exactly one parameter (slot id).");}
        StoreArray<TOPDigit> topDigits;
        int thisModuleID = static_cast<int>(vars[0]);
        size_t count = 0;
        for (const auto& t : topDigits) {
          if (abs(t.getModuleID()) != abs(thisModuleID)) continue;
          count += 1;
        }
        return count;
      }

      //! returns the number of good photons in a given slot
      double TOPGoodPhotonsInSlot([[maybe_unused]] const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 1) { B2FATAL("Need exactly one parameter (slot id).");}
        StoreArray<TOPDigit> topDigits;
        int thisModuleID = static_cast<int>(vars[0]);
        size_t count = 0;
        for (const auto& t : topDigits) {
          if (abs(t.getModuleID()) != abs(thisModuleID)) continue;
          if (t.getHitQuality() != TOPDigit::c_Good) continue;
          count += 1;
        }
        return count;
      }

      //! returns the number of tracks in the same slot as the particle
      double TOPTracksInSlot([[maybe_unused]] const Particle* particle)
      {
        const auto* trk = particle->getTrack();
        if (not trk) {
          return -1.0;
        }
        int thisModuleID = static_cast<int>(getSlotID(particle));
        if (thisModuleID == 0) return 0;
        StoreArray<Track> tracks;
        int nTracks = 0;
        for (const auto& t : tracks) {
          const auto* tl = t.getRelated<TOPLikelihood>();
          if (not tl) continue;
          const auto* te = tl->getRelated<ExtHit>();
          if (not te) continue;
          if (te->getCopyID() != thisModuleID) continue;
          nTracks += 1;
        }
        return nTracks;
      }

    } // TOPVariable

    VARIABLE_GROUP("TOP Calibration");
    REGISTER_VARIABLE("extrapTrackToTOPimpactZ", TOPVariable::extrapTrackToTOPz,
                      "[calibration] z coordinate of the impact point of the track extrapolated to TOP using helix data from TrackFitResult",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("extrapTrackToTOPimpactTheta", TOPVariable::extrapTrackToTOPtheta,
                      "[calibration] theta coordinate of the impact point of the track extrapolated to TOP using helix data from TrackFitResult",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("extrapTrackToTOPimpactPhi", TOPVariable::extrapTrackToTOPphi,
                      "[calibration] phi coordinate of the impact point of the track extrapolated to TOP using helix data from TrackFitResult",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topDigitCount", TOPVariable::topDigitCount,
                      "[calibration] The number of TOPDigits in the module to which the track was extrapolated", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topBackgroundDigitCount", TOPVariable::topBackgroundDigitCount,
                      "[calibration] The number of TOPDigits in all modules except the one to which the track was extrapolated",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topBackgroundDigitCountRaw", TOPVariable::topBackgroundDigitCountRaw,
                      "[calibration] The number of TOPDigits in all modules except the one to which the track was extrapolated",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topDigitCountRaw", TOPVariable::topDigitCount,
                      "[calibration] The number of TOPDigits in the module to which the track was extrapolated, regardless of hit quality",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topReflectedDigitCount", TOPVariable::topReflectedDigitCount,
                      "[calibration] The number of reflected photons in the same module", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topReflectedDigitCountExpert(minGap, maxGap)", TOPVariable::topReflectedDigitCountExpert,
                      "[calibration] The number of photons after the largest gap between minGap and maxGap", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topDigitGapSize", TOPVariable::topDigitGapSize,
                      "[calibration] The largest time difference between two consecutive hits in the same module", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topLocalX", TOPVariable::getTOPLocalX,
                      "[calibration] The local x coordinate of the particle's entry point to the TOP module", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topLocalY", TOPVariable::getTOPLocalY,
                      "[calibration] The local y coordinate of the particle's entry point to the TOP module", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topLocalZ", TOPVariable::getTOPLocalZ,
                      "[calibration] The local z coordinate of the particle's entry point to the TOP module", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topLocalXMCMatch", TOPVariable::getTOPLocalXMCMatch,
                      "[calibration] The local x coordinate of the MC particle's entry point to the TOP module", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topLocalYMCMatch", TOPVariable::getTOPLocalYMCMatch,
                      "[calibration] The local y coordinate of the MC particle's entry point to the TOP module", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topLocalZMCMatch", TOPVariable::getTOPLocalZMCMatch,
                      "[calibration] The local z coordinate of the MC particle's entry point to the TOP module", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topLocalPhi", TOPVariable::getTOPLocalPhi,
                      "[calibration] The local phi coordinate of the particle's momentum in the TOP module", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topLocalTheta", TOPVariable::getTOPLocalTheta,
                      "[calibration] The local phi coordinate of the particle's momentum in the TOP module", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topTOF", TOPVariable::getTOF,
                      "[calibration] The time of flight from the origin to the TOP", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topTOFExpert(pdg)", TOPVariable::getTOFExpert,
                      "[calibration] The time of flight from the origin to the TOP under the given hypothesis", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topAverageTimeOfFirst5", TOPVariable::getAverageTimeOfFirst5,
                      "[calibration] The average time of the first (up to) 5 hits in the module with the track", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topSlotID", TOPVariable::getSlotID,
                      "[calibration] The ID of the TOP slot that was hit by the particle", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topExpectedPhotonCount(pdg)", TOPVariable::getExpectedTOPPhotonCount,
                      "[calibration] The expected number of photons in the TOP for the particle under the given hypothesis",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topPhotonCount", TOPVariable::getTOPPhotonCount,
                      "[calibration] The number of (bg-subtracted) TOP photons in for the given particle", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("countTOPHitsInInterval(tmin, tmax)", TOPVariable::countTOPHitsInInterval,
                      "[calibration] The number of photons in the given interval", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("countTOPHitsInFirst20ns", TOPVariable::countTOPHitsInFirst20ns,
                      "[calibration] The number of photons in the first 20 ns after the first photon", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("countRawTOPHitsInInterval(tmin, tmax)", TOPVariable::countRawTOPHitsInInterval,
                      "[calibration] The number of photons in the given interval", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topFlag", TOPVariable::getFlag,
                      "[calibration] reconstruction flag, log likelihoods are valid if flag==1", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topElectronLogL", TOPVariable::getElectronLogL,
                      "[calibration] electron log likelihood", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topMuonLogL", TOPVariable::getMuonLogL,
                      "[calibration] muon log likelihood", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topPionLogL", TOPVariable::getPionLogL,
                      "[calibration] pion log likelihood", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topKaonLogL", TOPVariable::getKaonLogL,
                      "[calibration] kaon log likelihood", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topProtonLogL", TOPVariable::getProtonLogL,
                      "[calibration] proton log likelihood", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("logLScanMass", TOPVariable::getLogLScanMass,
                      "[calibration] mass at the logL maximum from the LL scan");
    REGISTER_VARIABLE("logLScanMassUpperInterval", TOPVariable::getLogLScanMassUpperInterval,
                      "[calibration] Upper edge of the mass interval determined by the LL scan");
    REGISTER_VARIABLE("logLScanMassLowerInterval", TOPVariable::getLogLScanMassLowerInterval,
                      "[calibration] Lower edge of the mass interval determined by the LL scan");
    REGISTER_VARIABLE("logLScanThreshold", TOPVariable::getLogLScanThreshold,
                      "[calibration] Cherenkov threshold determind by the LL scan");
    REGISTER_VARIABLE("logLScanExpectedSignalPhotons", TOPVariable::getLogLScanExpectedSignalPhotons,
                      "[calibration] Expected signal photon yeild at the LL maximum");
    REGISTER_VARIABLE("topRecBunchUsedTrackCount", TOPVariable::TOPRecBunchUsedTrackCount,
                      "[calibration] The number of tracks used in the bunch reconstruction", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topRecBunchTrackCount", TOPVariable::TOPRecBunchTrackCount,
                      "[calibration] The number of tracks in the TOP acceptance", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topRecBunchCurrentOffset", TOPVariable::TOPRecBunchCurrentOffset,
                      "[calibration] The current offset", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topRecBunchNumber", TOPVariable::TOPRecBunchNumber,
                      "[calibration] The number of the bunch relative to the interaction", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("isTopRecBunchReconstructed", TOPVariable::isTOPRecBunchReconstructed,
                      "[calibration] Flag to indicate whether the bunch was reconstructed", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topRawPhotonsInSlot(id)", TOPVariable::TOPRawPhotonsInSlot,
                      "[calibration] The number of all photons in the given slot", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topGoodPhotonsInSlot(id)", TOPVariable::TOPGoodPhotonsInSlot,
                      "[calibration] The number of good photons in the given slot", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("topTracksInSlot", TOPVariable::TOPTracksInSlot,
                      "[calibration] The number of tracks in the same slot as the particle", Manager::VariableDataType::c_double);
  } // Variable
} // Belle2
