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

      int getSlotID(const Particle* particle)
      {
        const auto* extHit = getExtHit(particle);
        return extHit ? extHit->getCopyID() : 0;
      }


      //---------------- ExtHit (or TOPBarHit) based --------------------

      double topSlotID(const Particle* particle)
      {
        const auto* extHit = getExtHit(particle);
        return extHit ? extHit->getCopyID() : std::numeric_limits<double>::quiet_NaN();
      }

      double topSlotIDMCMatch(const Particle* particle)
      {
        const auto* barHit = getBarHit(particle);
        return barHit ? barHit->getModuleID() : std::numeric_limits<double>::quiet_NaN();
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

      double countHits(const Particle* particle, double tmin, double tmax, bool clean)
      {
        int slotID = getSlotID(particle);
        if (slotID == 0) return std::numeric_limits<double>::quiet_NaN();

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
        int slotID = getSlotID(particle);
        if (slotID == 0) return std::numeric_limits<double>::quiet_NaN();

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
        int slotID = getSlotID(particle);
        if (slotID == 0) return std::numeric_limits<double>::quiet_NaN();

        auto* trk = particle->getTrack();
        if (not trk) return std::numeric_limits<double>::quiet_NaN();

        auto* mcParticle = trk->getRelated<MCParticle>();
        if (not mcParticle) return std::numeric_limits<double>::quiet_NaN();

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
        int slotID = getSlotID(particle);
        if (slotID == 0) return std::numeric_limits<double>::quiet_NaN();

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
        int slotID = getSlotID(particle);
        if (slotID == 0) return std::numeric_limits<double>::quiet_NaN();

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
        int slotID = getSlotID(particle);
        if (slotID == 0) return std::numeric_limits<double>::quiet_NaN();

        auto* trk = particle->getTrack();
        if (not trk) return std::numeric_limits<double>::quiet_NaN();

        auto* mcParticle = trk->getRelated<MCParticle>();
        if (not mcParticle) return std::numeric_limits<double>::quiet_NaN();

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
        return topLikelihood ? topLikelihood->getNphot() : std::numeric_limits<double>::quiet_NaN();
      }

      double expectedPhotonCount(const Particle* particle, int pdg)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return std::numeric_limits<double>::quiet_NaN();

        pdg = pdg != 0 ? pdg : particle->getPDGCode();
        const auto& chargedStable = Const::chargedStableSet.find(abs(pdg));
        if (chargedStable == Const::invalidParticle) return std::numeric_limits<double>::quiet_NaN();

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
        return topLikelihood ? topLikelihood->getEstBkg() : std::numeric_limits<double>::quiet_NaN();
      }

      double getElectronLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return std::numeric_limits<double>::quiet_NaN();
        return topLikelihood->getLogL_e();
      }

      double getMuonLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return std::numeric_limits<double>::quiet_NaN();
        return topLikelihood->getLogL_mu();
      }

      double getPionLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return std::numeric_limits<double>::quiet_NaN();
        return topLikelihood->getLogL_pi();
      }

      double getKaonLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return std::numeric_limits<double>::quiet_NaN();
        return topLikelihood->getLogL_K();
      }

      double getProtonLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return std::numeric_limits<double>::quiet_NaN();
        return topLikelihood->getLogL_p();
      }

      double getDeuteronLogL(const Particle* particle)
      {
        const auto* topLikelihood = getTOPLikelihood(particle);
        if (not topLikelihood) return std::numeric_limits<double>::quiet_NaN();
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

      double TOPRecBucketNumber([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return std::numeric_limits<double>::quiet_NaN();
        auto bucket = recBunch->getBucketNumber();
        if (bucket == TOPRecBunch::c_Unknown) return std::numeric_limits<double>::quiet_NaN();
        return bucket;
      }

      double isTOPRecBunchFilled([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return std::numeric_limits<double>::quiet_NaN();
        return recBunch->getBucketFillStatus();
      }

      double isTOPRecBunchNumberEQsim([[maybe_unused]] const Particle* particle)
      {
        StoreObjPtr<TOPRecBunch> recBunch;
        if (not recBunch.isValid()) return 0;
        if (not recBunch->isReconstructed()) return 0;
        if (not recBunch->isSimulated()) return 0;
        return (recBunch->getBunchNo() == recBunch->getMCBunchNo());
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
        int slotID = getSlotID(particle);
        if (slotID == 0) return std::numeric_limits<double>::quiet_NaN();

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


    VARIABLE_GROUP("TOP Variables (cdst needed)");
    REGISTER_VARIABLE("topSlotID", TOPVariable::topSlotID,
                      "slot ID of the particle");
    REGISTER_VARIABLE("topSlotIDMCMatch", TOPVariable::topSlotIDMCMatch,
                      "slot ID of the matched MC particle");

    REGISTER_VARIABLE("topLocalX", TOPVariable::getTOPLocalX,
                      "x coordinate of the particle entry point to TOP in the local frame");
    REGISTER_VARIABLE("topLocalY", TOPVariable::getTOPLocalY,
                      "y coordinate of the particle entry point to TOP in the local frame");
    REGISTER_VARIABLE("topLocalZ", TOPVariable::getTOPLocalZ,
                      "z coordinate of the particle entry point to TOP in the local frame");
    REGISTER_VARIABLE("topLocalXMCMatch", TOPVariable::getTOPLocalXMCMatch,
                      "x coordinate of the matched MC particle entry point to TOP in the local frame");
    REGISTER_VARIABLE("topLocalYMCMatch", TOPVariable::getTOPLocalYMCMatch,
                      "y coordinate of the matched MC particle entry point to TOP in the local frame");
    REGISTER_VARIABLE("topLocalZMCMatch", TOPVariable::getTOPLocalZMCMatch,
                      "z coordinate of the matched MC particle entry point to TOP in the local frame");
    REGISTER_VARIABLE("topLocalPhi", TOPVariable::getTOPLocalPhi,
                      "momentum azimuthal angle of the particle at entry point to TOP in the local frame");
    REGISTER_VARIABLE("topLocalTheta", TOPVariable::getTOPLocalTheta,
                      "momentum polar angle of the particle at entry point to the TOP in the local frame");
    REGISTER_VARIABLE("topLocalPhiMCMatch", TOPVariable::getTOPLocalPhiMCMatch,
                      "momentum azimuthal angle of the matched MC particle at entry point to TOP in the local frame");
    REGISTER_VARIABLE("topLocalThetaMCMatch", TOPVariable::getTOPLocalThetaMCMatch,
                      "momentum polar angle of the matched MC particle at entry point to TOP in the local frame");
    REGISTER_VARIABLE("topTOF", TOPVariable::getTOF,
                      "time-of-flight of the particle from the origin to TOP");
    REGISTER_VARIABLE("topTOFMCMatch", TOPVariable::getTOFMCMatch,
                      "time-of-flight of the matched MC particle from the origin to TOP");
    REGISTER_VARIABLE("topTOFExpert(pdg)", TOPVariable::getTOFExpert,
                      "time-of-flight from the origin to TOP for a given PDG code");

    REGISTER_VARIABLE("extrapTrackToTOPimpactZ", TOPVariable::extrapTrackToTOPz,
                      "z coordinate of the track extrapolated to R = 120 cm using helix from TrackFitResult");
    REGISTER_VARIABLE("extrapTrackToTOPimpactTheta", TOPVariable::extrapTrackToTOPtheta,
                      "theta coordinate of the track extrapolated to R = 120 cm using helix from TrackFitResult");
    REGISTER_VARIABLE("extrapTrackToTOPimpactPhi", TOPVariable::extrapTrackToTOPphi,
                      "phi coordinate of the track extrapolated to R = 120 cm using helix from TrackFitResult");

    REGISTER_VARIABLE("topDigitCount", TOPVariable::topDigitCount,
                      "number of good digits in the same module as particle");
    REGISTER_VARIABLE("topDigitCountSignal", TOPVariable::topSignalDigitCount,
                      "number of good, background-subtracted digits in interval [0, 50 ns] of the same module as particle");
    REGISTER_VARIABLE("topDigitCountBkg", TOPVariable::topBackgroundDigitCount,
                      "number of good digits in interval [-50 ns, 0] of the same module as particle");
    REGISTER_VARIABLE("topDigitCountRaw", TOPVariable::topRawDigitCount,
                      "number of all digits (regardless of hit quality) in the same module as particle");
    REGISTER_VARIABLE("topDigitCountInterval(tmin, tmax)", TOPVariable::countTOPHitsInInterval,
                      "number of good digits in a given time interval of the same module as particle");
    REGISTER_VARIABLE("topDigitCountIntervalRaw(tmin, tmax)", TOPVariable::countRawTOPHitsInInterval,
                      "number of all digits (regardless of hit quality) in a given time interval of the same module as particle");
    REGISTER_VARIABLE("topDigitCountMCMatch", TOPVariable::topDigitCountMCMatch,
                      "number of good digits associated with the matched MC particle and in the same module as particle");
    REGISTER_VARIABLE("topDigitCountIntervalMCMatch(tmin, tmax)", TOPVariable::countTOPHitsInIntervalMCMatch,
                      "number of good digits associated with the matched MC particle in a given time interval and in the same module as particle");

    REGISTER_VARIABLE("topLogLFlag", TOPVariable::getFlag,
                      "reconstruction flag: 1 if log likelihoods available, 0 otherwise");
    REGISTER_VARIABLE("topLogLPhotonCount", TOPVariable::getTOPPhotonCount,
                      "number of photons used for log likelihood calculation");
    REGISTER_VARIABLE("topLogLExpectedPhotonCount", TOPVariable::getExpectedPhotonCount,
                      "expected number of photons (including bkg) for this particle");
    REGISTER_VARIABLE("topLogLExpectedPhotonCountExpert(pdg)", TOPVariable::getExpectedPhotonCountExpert,
                      "expected number of photons (including bkg) for a given PDG code");
    REGISTER_VARIABLE("topLogLEstimatedBkgCount", TOPVariable::getEstimatedBkgCount,
                      "estimated number of background photons");
    REGISTER_VARIABLE("topLogLElectron", TOPVariable::getElectronLogL,
                      "electron log likelihood");
    REGISTER_VARIABLE("topLogLMuon", TOPVariable::getMuonLogL,
                      "muon log likelihood");
    REGISTER_VARIABLE("topLogLPion", TOPVariable::getPionLogL,
                      "pion log likelihood");
    REGISTER_VARIABLE("topLogLKaon", TOPVariable::getKaonLogL,
                      "kaon log likelihood");
    REGISTER_VARIABLE("topLogLProton", TOPVariable::getProtonLogL,
                      "proton log likelihood");
    REGISTER_VARIABLE("topLogLDeuteron", TOPVariable::getDeuteronLogL,
                      "deuteron log likelihood");

    REGISTER_VARIABLE("logLScanMass", TOPVariable::getLogLScanMass,
                      "mass at the logL maximum from the LL scan. Requires TOPLLScanner in the processing path.");
    REGISTER_VARIABLE("logLScanMassUpperInterval", TOPVariable::getLogLScanMassUpperInterval,
                      "Upper edge of the mass interval determined by the LL scan. Requires TOPLLScanner in the processing path.");
    REGISTER_VARIABLE("logLScanMassLowerInterval", TOPVariable::getLogLScanMassLowerInterval,
                      "Lower edge of the mass interval determined by the LL scan. Requires TOPLLScanner in the processing path.");
    REGISTER_VARIABLE("logLScanThreshold", TOPVariable::getLogLScanThreshold,
                      "Cherenkov threshold determind by the LL scan. Requires TOPLLScanner in the processing path.");
    REGISTER_VARIABLE("logLScanExpectedSignalPhotons", TOPVariable::getLogLScanExpectedSignalPhotons,
                      "Expected signal photon yield at the LL maximum. Requires TOPLLScanner in the processing path.");

    REGISTER_VARIABLE("topBunchIsReconstructed", TOPVariable::isTOPRecBunchReconstructed,
                      "reconstruction flag: 1 if reconstructed, 0 otherwise");
    REGISTER_VARIABLE("topBunchIsFilled", TOPVariable::isTOPRecBunchFilled,
                      "bunch fill status: 0 empty, 1 filled, -1 unknown");
    REGISTER_VARIABLE("topBunchNumber", TOPVariable::TOPRecBunchNumber,
                      "reconstructed bunch number relative to L1 trigger");
    REGISTER_VARIABLE("topBucketNumber", TOPVariable::TOPRecBucketNumber,
                      "reconstructed bucket number within the ring");
    REGISTER_VARIABLE("topBunchMCMatch", TOPVariable::isTOPRecBunchNumberEQsim,
                      "MC matching status: 1 if reconstructed bunch equal to simulated bunch, 0 otherwise");
    REGISTER_VARIABLE("topBunchOffset", TOPVariable::TOPRecBunchCurrentOffset,
                      "current offset to the reconstructed bunch crossing time");
    REGISTER_VARIABLE("topBunchTrackCount", TOPVariable::TOPRecBunchTrackCount,
                      "number of tracks selected for the bunch reconstruction");
    REGISTER_VARIABLE("topBunchUsedTrackCount", TOPVariable::TOPRecBunchUsedTrackCount,
                      "number of tracks actually used in the bunch reconstruction");

    REGISTER_VARIABLE("topRawPhotonsInSlot(id)", TOPVariable::TOPRawPhotonsInSlot,
                      "number of all photons in the given slot id");
    REGISTER_VARIABLE("topGoodPhotonsInSlot(id)", TOPVariable::TOPGoodPhotonsInSlot,
                      "number of good photons in the given slot id");
    REGISTER_VARIABLE("topTracksInSlot", TOPVariable::TOPTracksInSlot,
                      "number of tracks in the same slot as particle");
  } // Variable
} // Belle2
