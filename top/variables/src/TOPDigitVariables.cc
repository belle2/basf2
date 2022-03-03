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
#include <framework/dataobjects/MCInitialParticles.h>

#include <algorithm> // for sort

using namespace std;

namespace Belle2 {
  namespace Variable {
    namespace TOPVariable {

      //---------------- helpers --------------------

      const TOPLikelihood* getTOPLikelihood(const Particle* particle)
      {
        if (not particle) return nullptr;
        const auto* track = particle->getTrack();
        return track ? track->getRelated<TOPLikelihood>() : nullptr;
      }

      const ExtHit* getExtHit(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        return topLikelihood ? topLikelihood->getRelated<ExtHit>() : nullptr;
      }

      const TOPBarHit* getBarHit(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        return topLikelihood ? topLikelihood->getRelated<TOPBarHit>() : nullptr;
      }

      const TOPLikelihoodScanResult* getTOPLikelihoodScanResult(const Particle* particle)
      {
        if (not particle) return nullptr;
        const auto* track = particle->getTrack();
        if (not track) return nullptr;
        auto scanRes = track->getRelated<TOPLikelihoodScanResult>();
        if (not scanRes) {
          B2WARNING("No TOPLikelihoodScanResult object found. Are you sure you added TOPLLScanner to the path?");
          return nullptr;
        }
        return scanRes;
      }

      //---------------- ExtHit (or TOPBarHit) based --------------------

      double getSlotID(const Particle* particle)
      {
        const auto* extHit = getExtHit(particle);
        return extHit ? extHit->getCopyID() : 0;
      }

      double getSlotIDMCMatch(const Particle* particle)
      {
        const auto* barHit = getBarHit(particle);
        return barHit ? barHit->getModuleID() : 0;
      }

      bool getLocalPosition(const Particle* particle, TVector3& result)
      {
        const auto* extHit = getExtHit(particle);
        if (not extHit) return false;
        int slotID = extHit->getCopyID();
        const auto position = extHit->getPosition();
        const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
        if (not geo or not geo->isModuleIDValid(slotID)) return false;
        const auto& module = geo->getModule(slotID);
        result = module.pointToLocal(position);
        return true;
      }

      double getTOPLocalX(const Particle* particle)
      {
        TVector3 position;
        bool ok = TOPVariable::getLocalPosition(particle, position);
        if (not ok) return std::numeric_limits<double>::quiet_NaN();
        return position.X();
      }

      double getTOPLocalY(const Particle* particle)
      {
        TVector3 position;
        bool ok = TOPVariable::getLocalPosition(particle, position);
        if (not ok) return std::numeric_limits<double>::quiet_NaN();
        return position.Y();
      }

      double getTOPLocalZ(const Particle* particle)
      {
        TVector3 position;
        bool ok = TOPVariable::getLocalPosition(particle, position);
        if (not ok) return std::numeric_limits<double>::quiet_NaN();
        return position.Z();
      }

      bool getLocalPositionMCMatch(const Particle* particle, TVector3& result)
      {
        const auto* barHit = getBarHit(particle);
        if (not barHit) return false;
        int slotID = barHit->getModuleID();
        const auto& position = barHit->getPosition();
        const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
        if (not geo or not geo->isModuleIDValid(slotID)) return false;
        const auto& module = geo->getModule(slotID);
        result = module.pointToLocal(position);
        return true;
      }

      double getTOPLocalXMCMatch(const Particle* particle)
      {
        TVector3 position;
        bool ok = TOPVariable::getLocalPositionMCMatch(particle, position);
        if (not ok) return std::numeric_limits<double>::quiet_NaN();
        return position.X();
      }

      double getTOPLocalYMCMatch(const Particle* particle)
      {
        TVector3 position;
        bool ok = TOPVariable::getLocalPositionMCMatch(particle, position);
        if (not ok) return std::numeric_limits<double>::quiet_NaN();
        return position.Y();
      }

      double getTOPLocalZMCMatch(const Particle* particle)
      {
        TVector3 position;
        bool ok = TOPVariable::getLocalPositionMCMatch(particle, position);
        if (not ok) return std::numeric_limits<double>::quiet_NaN();
        return position.Z();
      }

      bool getLocalMomentum(const Particle* particle, TVector3& result)
      {
        const auto* extHit = getExtHit(particle);
        if (not extHit) return false;
        int slotID = extHit->getCopyID();
        const auto& momentum = extHit->getMomentum();
        const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
        if ((not geo) or (not geo->isModuleIDValid(slotID))) return false;
        const auto& module = geo->getModule(slotID);
        result = module.momentumToLocal(momentum);
        return true;
      }

      double getTOPLocalPhi(const Particle* particle)
      {
        TVector3 momentum;
        bool ok = TOPVariable::getLocalMomentum(particle, momentum);
        if (not ok) return std::numeric_limits<double>::quiet_NaN();
        return momentum.Phi();
      }

      double getTOPLocalTheta(const Particle* particle)
      {
        TVector3 momentum;
        bool ok = TOPVariable::getLocalMomentum(particle, momentum);
        if (not ok) return std::numeric_limits<double>::quiet_NaN();
        return momentum.Theta();
      }

      bool getLocalMomentumMCMatch(const Particle* particle, TVector3& result)
      {
        const auto* barHit = getBarHit(particle);
        if (not barHit) return false;
        int slotID = barHit->getModuleID();
        const auto& momentum = barHit->getMomentum();
        const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
        if ((not geo) or (not geo->isModuleIDValid(slotID))) return false;
        const auto& module = geo->getModule(slotID);
        result = module.momentumToLocal(momentum);
        return true;
      }

      double getTOPLocalPhiMCMatch(const Particle* particle)
      {
        TVector3 momentum;
        bool ok = TOPVariable::getLocalMomentumMCMatch(particle, momentum);
        if (not ok) return std::numeric_limits<double>::quiet_NaN();
        return momentum.Phi();
      }

      double getTOPLocalThetaMCMatch(const Particle* particle)
      {
        TVector3 momentum;
        bool ok = TOPVariable::getLocalMomentumMCMatch(particle, momentum);
        if (not ok) return std::numeric_limits<double>::quiet_NaN();
        return momentum.Theta();
      }

      double computeTOF(const Particle* particle, int pdg)
      {
        const auto* extHit = getExtHit(particle);
        if (not extHit) return std::numeric_limits<double>::quiet_NaN();
        auto extPDGCode = abs(extHit->getPdgCode());
        double pmom = particle->getMomentumMagnitude();
        double massExtHit = Const::ChargedStable(extPDGCode).getMass();
        double betaExtHit = pmom / sqrt(pmom * pmom + massExtHit * massExtHit);
        double mass = pdg == 0 ? particle->getMass() : Const::ChargedStable(abs(pdg)).getMass();
        double beta = pmom / sqrt(pmom * pmom + mass * mass);
        return extHit->getTOF() * betaExtHit / beta;
      }

      double getTOF(const Particle* particle)
      {
        return computeTOF(particle, 0);
      }

      double getTOFMCMatch(const Particle* particle)
      {
        const auto* barHit = getBarHit(particle);
        if (not barHit) return std::numeric_limits<double>::quiet_NaN();
        StoreObjPtr<MCInitialParticles> mcInitialParticles;
        double trueEventT0 = 0;
        if (mcInitialParticles.isValid()) trueEventT0 = mcInitialParticles->getTime();
        return barHit->getTime() - trueEventT0;
      }

      double getTOFExpert(const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 1) {
          B2FATAL("topTOFExpert(pdg): Need exactly one parameter (PDG code).");
        }
        int pdg = static_cast<int>(vars[0]);
        return computeTOF(particle, pdg);
      }

      //---------------- Helix extrapolation --------------------

      double extrapTrackToTOPz(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) return std::numeric_limits<double>::quiet_NaN();
        auto trkfit = trk->getTrackFitResultWithClosestMass(Const::ChargedStable(std::abs(particle->getPDGCode())));
        if (not trkfit) return std::numeric_limits<double>::quiet_NaN();
        auto helix = trkfit->getHelix();
        double arcLength = helix.getArcLength2DAtCylindricalR(120);
        const auto& result = helix.getPositionAtArcLength2D(arcLength);
        return result.z();
      }

      double extrapTrackToTOPtheta(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) return std::numeric_limits<double>::quiet_NaN();
        auto trkfit = trk->getTrackFitResultWithClosestMass(Const::ChargedStable(std::abs(particle->getPDGCode())));
        if (not trkfit) return std::numeric_limits<double>::quiet_NaN();
        auto helix = trkfit->getHelix();
        double arcLength = helix.getArcLength2DAtCylindricalR(120);
        const auto& result = helix.getPositionAtArcLength2D(arcLength);
        return result.Theta();
      }

      double extrapTrackToTOPphi(const Particle* particle)
      {
        auto trk = particle->getTrack();
        if (not trk) return std::numeric_limits<double>::quiet_NaN();
        auto trkfit = trk->getTrackFitResultWithClosestMass(Const::ChargedStable(std::abs(particle->getPDGCode())));
        if (not trkfit) return std::numeric_limits<double>::quiet_NaN();
        auto helix = trkfit->getHelix();
        double arcLength = helix.getArcLength2DAtCylindricalR(120);
        const auto& result = helix.getPositionAtArcLength2D(arcLength);
        return result.Phi();
      }

      //---------------- TOPDigit based --------------------

      int countHits(const Particle* particle, double tmin, double tmax, bool clean)
      {
        int slotID = static_cast<int>(getSlotID(particle));
        if (slotID == 0) return 0;
        StoreArray<TOPDigit> digits;
        int count = 0;
        for (const auto& digit : digits) {
          if (digit.getModuleID() != slotID) continue;
          // skip bad digits only when we want to clean
          if (clean and digit.getHitQuality() != TOPDigit::c_Good) continue;
          if (digit.getTime() < tmin or digit.getTime() > tmax) continue;
          count++;
        }
        return count;
      }

      double topDigitCount(const Particle* particle)
      {
        int slotID = static_cast<int>(getSlotID(particle));
        if (slotID == 0) return 0;
        StoreArray<TOPDigit> topDigits;
        int count = 0;
        for (const auto& t : topDigits) {
          if (t.getModuleID() != slotID) continue;
          if (t.getHitQuality() != TOPDigit::c_Good) continue;
          count++;
        }
        return count;
      }

      double topDigitCountMCMatch(const Particle* particle)
      {
        int slotID = static_cast<int>(getSlotID(particle));
        if (slotID == 0) return 0;

        auto* trk = particle->getTrack();
        if (not trk) return 0;
        auto* mcParticle = trk->getRelated<MCParticle>();
        if (not mcParticle) return 0;
        auto digits = mcParticle->getRelationsWith<TOPDigit>();

        int count = 0;
        for (const auto& digit : digits) {
          if (digit.getModuleID() != slotID) continue;
          if (digit.getHitQuality() != TOPDigit::c_Good) continue;
          count++;
        }
        return count;
      }

      double topSignalDigitCount(const Particle* particle)
      {
        int slotID = static_cast<int>(getSlotID(particle));
        if (slotID == 0) return 0;
        StoreArray<TOPDigit> digits;
        int count = 0;
        for (const auto& digit : digits) {
          if (digit.getModuleID() != slotID) continue;
          if (digit.getHitQuality() != TOPDigit::c_Good) continue;
          if (abs(digit.getTime()) > 50) continue;
          if (digit.getTime() > 0) count++;
          else count--;
        }
        return count;
      }

      double topBackgroundDigitCount(const Particle* particle)
      {
        return TOPVariable::countHits(particle, -50, 0, true);
      }

      double topRawDigitCount(const Particle* particle)
      {
        int slotID = static_cast<int>(getSlotID(particle));
        if (slotID == 0) return 0;
        StoreArray<TOPDigit> topDigits;
        int count = 0;
        for (const auto& t : topDigits) {
          if (t.getModuleID() != slotID) continue;
          count++;
        }
        return count;
      }

      double countTOPHitsInInterval(const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 2) {
          B2FATAL("countTOPHitsInInterval(tmin, tmax): Need exactly two parameters (tmin, tmax)");
        }
        return TOPVariable::countHits(particle, vars[0], vars[1], true);
      }

      double countRawTOPHitsInInterval(const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 2) {
          B2FATAL("countRawTOPHitsInInterval(tmin, tmax): Need exactly two parameters (tmin, tmax)");
        }
        return TOPVariable::countHits(particle, vars[0], vars[1], false);
      }

      double countTOPHitsInIntervalMCMatch(const Particle* particle, const std::vector<double>& vars)
      {
        if (vars.size() != 2) {
          B2FATAL("countTOPHitsInIntervalMCMatch(tmin, tmax): Need exactly two parameters (tmin, tmax)");
        }
        int slotID = static_cast<int>(getSlotID(particle));
        if (slotID == 0) return 0;

        auto* trk = particle->getTrack();
        if (not trk) return 0;
        auto* mcParticle = trk->getRelated<MCParticle>();
        if (not mcParticle) return 0;
        auto digits = mcParticle->getRelationsWith<TOPDigit>();

        int count = 0;
        for (const auto& digit : digits) {
          if (digit.getModuleID() != slotID) continue;
          if (digit.getHitQuality() != TOPDigit::c_Good) continue;
          if (digit.getTime() < vars[0] or digit.getTime() > vars[1]) continue;
          count++;
        }
        return count;
      }


      //---------------- TOPLikelihood based --------------------

      double getFlag(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return 0;
        return (topLikelihood->getFlag() == 1);
      }

      double getTOPPhotonCount(const Particle* particle)
      {
        const auto* topLikelihood = TOPVariable::getTOPLikelihood(particle);
        return topLikelihood ? topLikelihood->getNphot() : 0;
      }

      double expectedPhotonCount(const Particle* particle, int pdg)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return 0;
        // if the user does select a hypothesis, use the particle's pdg code
        pdg = pdg != 0 ? pdg : particle->getPDGCode();
        const auto& chargedStable = Const::chargedStableSet.find(abs(pdg));
        if (chargedStable == Const::invalidParticle) return 0; // PDG code not one of e, mu, pi, K, p, d
        return topLikelihood->getEstPhot(chargedStable);
      }

      double getExpectedPhotonCount(const Particle* particle)
      {
        return TOPVariable::expectedPhotonCount(particle, 0);
      }

      double getExpectedPhotonCountExpert(const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 1) {
          B2FATAL("Need exactly one parameter (pdg id).");
        }
        return TOPVariable::expectedPhotonCount(particle, static_cast<int>(vars[0]));
      }

      double getEstimatedBkgCount(const Particle* particle)
      {
        const auto* topLikelihood = TOPVariable::getTOPLikelihood(particle);
        return topLikelihood ? topLikelihood->getEstBkg() : 0;
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

      double getDeuteronLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return 0;
        return topLikelihood->getLogL(Const::deuteron);
      }

      //---------------- TOPLikelihoodScanResult based --------------------

      double getLogLScanMass(const Particle* particle)
      {
        auto* scanRes = getTOPLikelihoodScanResult(particle);
        return scanRes ? scanRes->getMostLikelyMass() : std::numeric_limits<double>::quiet_NaN();
      }

      double getLogLScanMassUpperInterval(const Particle* particle)
      {
        auto* scanRes = getTOPLikelihoodScanResult(particle);
        return scanRes ? scanRes->getMostLikelyMassIntervalUp() : std::numeric_limits<double>::quiet_NaN();
      }

      double getLogLScanMassLowerInterval(const Particle* particle)
      {
        auto* scanRes = getTOPLikelihoodScanResult(particle);
        return scanRes ? scanRes->getMostLikelyMassIntervalLow() : std::numeric_limits<double>::quiet_NaN();
      }

      double getLogLScanThreshold(const Particle* particle)
      {
        auto* scanRes = getTOPLikelihoodScanResult(particle);
        return scanRes ? scanRes->getThreshold() : std::numeric_limits<double>::quiet_NaN();
      }

      double getLogLScanExpectedSignalPhotons(const Particle* particle)
      {
        auto* scanRes = getTOPLikelihoodScanResult(particle);
        return scanRes ? scanRes->getMostLikelySignalPhotonCount() : std::numeric_limits<double>::quiet_NaN();
      }

      //---------------- TOPRecBunch based --------------------

      double isTOPRecBunchReconstructed([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return 0;
        return recBunch->isReconstructed();
      }

      double TOPRecBunchNumber([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return std::numeric_limits<double>::quiet_NaN();
        if (not recBunch->isReconstructed()) return std::numeric_limits<double>::quiet_NaN();
        return recBunch->getBunchNo();
      }

      double isTOPRecBunchNumberEQsim([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return 0;
        if (not recBunch->isReconstructed()) return 0;
        if (not recBunch->isSimulated()) return 0;
        if (recBunch->getBunchNo() != recBunch->getMCBunchNo()) return 0;
        return 1;
      }

      double TOPRecBunchCurrentOffset([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return std::numeric_limits<double>::quiet_NaN();
        if (not recBunch->isReconstructed()) return std::numeric_limits<double>::quiet_NaN();
        return recBunch->getCurrentOffset();
      }

      double TOPRecBunchTrackCount([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return std::numeric_limits<double>::quiet_NaN();
        return recBunch->getNumTracks();
      }

      double TOPRecBunchUsedTrackCount([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return std::numeric_limits<double>::quiet_NaN();
        return recBunch->getUsedTracks();
      }

      //-------------- Event based -----------------------------------

      double TOPRawPhotonsInSlot([[maybe_unused]] const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 1) { B2FATAL("Need exactly one parameter (slot id).");}
        StoreArray<TOPDigit> topDigits;
        int slotID = static_cast<int>(vars[0]);
        size_t count = 0;
        for (const auto& t : topDigits) {
          if (t.getModuleID() != slotID) continue;
          count += 1;
        }
        return count;
      }

      double TOPGoodPhotonsInSlot([[maybe_unused]] const Particle* particle, const vector<double>& vars)
      {
        if (vars.size() != 1) { B2FATAL("Need exactly one parameter (slot id).");}
        StoreArray<TOPDigit> topDigits;
        int slotID = static_cast<int>(vars[0]);
        size_t count = 0;
        for (const auto& t : topDigits) {
          if (t.getModuleID() != slotID) continue;
          if (t.getHitQuality() != TOPDigit::c_Good) continue;
          count += 1;
        }
        return count;
      }

      double TOPTracksInSlot(const Particle* particle)
      {
        int slotID = static_cast<int>(getSlotID(particle));
        if (slotID == 0) return 0;
        StoreArray<Track> tracks;
        int nTracks = 0;
        for (const auto& t : tracks) {
          const auto* tl = t.getRelated<TOPLikelihood>();
          if (not tl) continue;
          const auto* te = tl->getRelated<ExtHit>();
          if (not te) continue;
          if (te->getCopyID() != slotID) continue;
          nTracks += 1;
        }
        return nTracks;
      }

    } // TOPVariable


    VARIABLE_GROUP("TOP Calibration");
    REGISTER_VARIABLE("topSlotID", TOPVariable::getSlotID,
                      "[calibration] slot ID of the particle (0 if N/A)");
    REGISTER_VARIABLE("topSlotIDMCMatch", TOPVariable::getSlotIDMCMatch,
                      "[calibration] slot ID of the matched MC particle (0 if N/A)");

    REGISTER_VARIABLE("topLocalX", TOPVariable::getTOPLocalX,
                      "[calibration] x coordinate of the particle entry point to TOP in the local frame (NaN if N/A)");
    REGISTER_VARIABLE("topLocalY", TOPVariable::getTOPLocalY,
                      "[calibration] y coordinate of the particle entry point to TOP in the local frame (NaN if N/A)");
    REGISTER_VARIABLE("topLocalZ", TOPVariable::getTOPLocalZ,
                      "[calibration] z coordinate of the particle entry point to TOP in the local frame (NaN if N/A)");
    REGISTER_VARIABLE("topLocalXMCMatch", TOPVariable::getTOPLocalXMCMatch,
                      "[calibration] x coordinate of the matched MC particle entry point to TOP in the local frame (NaN if N/A)");
    REGISTER_VARIABLE("topLocalYMCMatch", TOPVariable::getTOPLocalYMCMatch,
                      "[calibration] y coordinate of the matched MC particle entry point to TOP in the local frame (NaN if N/A)");
    REGISTER_VARIABLE("topLocalZMCMatch", TOPVariable::getTOPLocalZMCMatch,
                      "[calibration] z coordinate of the matched MC particle entry point to TOP in the local frame (NaN if N/A)");
    REGISTER_VARIABLE("topLocalPhi", TOPVariable::getTOPLocalPhi,
                      "[calibration] momentum azimuthal angle of the particle at entry point to TOP in the local frame (NaN if N/A)");
    REGISTER_VARIABLE("topLocalTheta", TOPVariable::getTOPLocalTheta,
                      "[calibration] momentum polar angle of the particle at entry point to the TOP in the local frame (NaN if N/A)");
    REGISTER_VARIABLE("topLocalPhiMCMatch", TOPVariable::getTOPLocalPhiMCMatch,
                      "[calibration] momentum azimuthal angle of the matched MC particle at entry point to TOP in the local frame (NaN if N/A)");
    REGISTER_VARIABLE("topLocalThetaMCMatch", TOPVariable::getTOPLocalThetaMCMatch,
                      "[calibration] momentum polar angle of the matched MC particle at entry point to TOP in the local frame (NaN if N/A)");
    REGISTER_VARIABLE("topTOF", TOPVariable::getTOF,
                      "[calibration] time-of-flight of the particle from the origin to TOP (NaN if N/A)");
    REGISTER_VARIABLE("topTOFMCMatch", TOPVariable::getTOFMCMatch,
                      "[calibration] time-of-flight of the matched MC particle from the origin to TOP (NaN if N/A)");
    REGISTER_VARIABLE("topTOFExpert(pdg)", TOPVariable::getTOFExpert,
                      "[calibration] time-of-flight from the origin to TOP for a given PDG code (NaN if N/A)");

    REGISTER_VARIABLE("extrapTrackToTOPimpactZ", TOPVariable::extrapTrackToTOPz,
                      "[calibration] z coordinate of the track extrapolated to R = 120 cm using helix from TrackFitResult (NaN if N/A)");
    REGISTER_VARIABLE("extrapTrackToTOPimpactTheta", TOPVariable::extrapTrackToTOPtheta,
                      "[calibration] theta coordinate of the track extrapolated to R = 120 cm using helix from TrackFitResult (NaN if N/A)");
    REGISTER_VARIABLE("extrapTrackToTOPimpactPhi", TOPVariable::extrapTrackToTOPphi,
                      "[calibration] phi coordinate of the track extrapolated to R = 120 cm using helix from TrackFitResult (NaN if N/A)");

    REGISTER_VARIABLE("topDigitCount", TOPVariable::topDigitCount,
                      "[calibration] number of good digits in the same module as particle (0 if N/A)");
    REGISTER_VARIABLE("topDigitCountSignal", TOPVariable::topSignalDigitCount,
                      "[calibration] number of good, background-subtracted digits in interval [0, 50 ns] of the same module as particle (0 if N/A)");
    REGISTER_VARIABLE("topDigitCountBkg", TOPVariable::topBackgroundDigitCount,
                      "[calibration] number of good digits in interval [-50 ns, 0] of the same module as particle (0 if N/A)");
    REGISTER_VARIABLE("topDigitCountRaw", TOPVariable::topRawDigitCount,
                      "[calibration] number of all digits (regardless of hit quality) in the same module as particle (0 if N/A)");
    REGISTER_VARIABLE("topDigitCountInterval(tmin, tmax)", TOPVariable::countTOPHitsInInterval,
                      "[calibration] number of good digits in a given time interval of the same module as particle (0 if N/A)");
    REGISTER_VARIABLE("topDigitCountIntervalRaw(tmin, tmax)", TOPVariable::countRawTOPHitsInInterval,
                      "[calibration] number of all digits (regardless of hit quality) in a given time interval of the same module as particle (0 if N/A)");
    REGISTER_VARIABLE("topDigitCountMCMatch", TOPVariable::topDigitCountMCMatch,
                      "[calibration] number of good digits associated with the matched MC particle and in the same module as particle (0 if N/A)");
    REGISTER_VARIABLE("topDigitCountIntervalMCMatch(tmin, tmax)", TOPVariable::countTOPHitsInIntervalMCMatch,
                      "[calibration] number of good digits associated with the matched MC particle in a given time interval and in the same module as particle (0 if N/A)");

    REGISTER_VARIABLE("topLogLFlag", TOPVariable::getFlag,
                      "[calibration] reconstruction flag: 1 if log likelihoods available, 0 otherwise");
    REGISTER_VARIABLE("topLogLPhotonCount", TOPVariable::getTOPPhotonCount,
                      "[calibration] number of photons used for log likelihood calculation (0 if N/A)");
    REGISTER_VARIABLE("topLogLExpectedPhotonCount", TOPVariable::getExpectedPhotonCount,
                      "[calibration] expected number of photons (including bkg) for this particle (0 if N/A)");
    REGISTER_VARIABLE("topLogLExpectedPhotonCountExpert(pdg)", TOPVariable::getExpectedPhotonCountExpert,
                      "[calibration] expected number of photons (including bkg) for a given PDG code (0 if N/A)");
    REGISTER_VARIABLE("topLogLEstimatedBkgCount", TOPVariable::getEstimatedBkgCount,
                      "[calibration] estimated number of background photons (0 if N/A)");
    REGISTER_VARIABLE("topLogLElectron", TOPVariable::getElectronLogL,
                      "[calibration] electron log likelihood (0 if N/A)");
    REGISTER_VARIABLE("topLogLMuon", TOPVariable::getMuonLogL,
                      "[calibration] muon log likelihood (0 if N/A)");
    REGISTER_VARIABLE("topLogLPion", TOPVariable::getPionLogL,
                      "[calibration] pion log likelihood (0 if N/A)");
    REGISTER_VARIABLE("topLogLKaon", TOPVariable::getKaonLogL,
                      "[calibration] kaon log likelihood (0 if N/A)");
    REGISTER_VARIABLE("topLogLProton", TOPVariable::getProtonLogL,
                      "[calibration] proton log likelihood (0 if N/A)");
    REGISTER_VARIABLE("topLogLDeuteron", TOPVariable::getDeuteronLogL,
                      "[calibration] deuteron log likelihood (0 if N/A)");

    REGISTER_VARIABLE("logLScanMass", TOPVariable::getLogLScanMass,
                      "[calibration] mass at the logL maximum from the LL scan (NaN if N/A). Requires TOPLLScanner in the processing path.");
    REGISTER_VARIABLE("logLScanMassUpperInterval", TOPVariable::getLogLScanMassUpperInterval,
                      "[calibration] Upper edge of the mass interval determined by the LL scan (NaN if N/A). Requires TOPLLScanner in the processing path.");
    REGISTER_VARIABLE("logLScanMassLowerInterval", TOPVariable::getLogLScanMassLowerInterval,
                      "[calibration] Lower edge of the mass interval determined by the LL scan (NaN if N/A). Requires TOPLLScanner in the processing path.");
    REGISTER_VARIABLE("logLScanThreshold", TOPVariable::getLogLScanThreshold,
                      "[calibration] Cherenkov threshold determind by the LL scan (NaN if N/A). Requires TOPLLScanner in the processing path.");
    REGISTER_VARIABLE("logLScanExpectedSignalPhotons", TOPVariable::getLogLScanExpectedSignalPhotons,
                      "[calibration] Expected signal photon yeild at the LL maximum (NaN if N/A). Requires TOPLLScanner in the processing path.");

    REGISTER_VARIABLE("topBunchIsReconstructed", TOPVariable::isTOPRecBunchReconstructed,
                      "[calibration] reconstruction flag: 1 if reconstructed, 0 otherwise");
    REGISTER_VARIABLE("topBunchNumber", TOPVariable::TOPRecBunchNumber,
                      "[calibration] reconstructed bunch number relative to L1 trigger (NaN if N/A)");
    REGISTER_VARIABLE("topBunchMCMatch", TOPVariable::isTOPRecBunchNumberEQsim,
                      "[calibration] MC matching status: 1 if reconstructed bunch equal to simulated bunch, 0 otherwise");
    REGISTER_VARIABLE("topBunchOffset", TOPVariable::TOPRecBunchCurrentOffset,
                      "[calibration] current offset to the reconstructed bunch crossing time (NaN if N/A)");
    REGISTER_VARIABLE("topBunchTrackCount", TOPVariable::TOPRecBunchTrackCount,
                      "[calibration] number of tracks selected for the bunch reconstruction (NaN if N/A)");
    REGISTER_VARIABLE("topBunchUsedTrackCount", TOPVariable::TOPRecBunchUsedTrackCount,
                      "[calibration] number of tracks actually used in the bunch reconstruction (NaN if N/A)");

    REGISTER_VARIABLE("topRawPhotonsInSlot(id)", TOPVariable::TOPRawPhotonsInSlot,
                      "[calibration] number of all photons in the given slot id (0 if N/A)");
    REGISTER_VARIABLE("topGoodPhotonsInSlot(id)", TOPVariable::TOPGoodPhotonsInSlot,
                      "[calibration] number of good photons in the given slot id (0 if N/A)");
    REGISTER_VARIABLE("topTracksInSlot", TOPVariable::TOPTracksInSlot,
                      "[calibration] number of tracks in the same slot as particle (0 if N/A)");
  } // Variable
} // Belle2
