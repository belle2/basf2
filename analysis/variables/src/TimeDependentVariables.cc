/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi, Fernando Abudinen, Thibaud Humair         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/variables/TimeDependentVariables.h>
#include <analysis/VariableManager/Manager.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <framework/dbobjects/BeamParameters.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/TagVertex.h>
#include <mdst/dataobjects/MCParticle.h>

//utilities
#include <analysis/utility/DistanceTools.h>

// framework aux
#include <framework/gearbox/Const.h>

#include <TLorentzVector.h>
#include <TMatrixD.h>
#include <TVector3.h>

#include <iostream>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    //   ############################################## Time Dependent CPV Analysis Variables  ###############################################


    // TagV x, y, z

    double particleTagVx(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVertex().X();

      return result;
    }

    double particleTagVy(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVertex().Y();

      return result;
    }

    double particleTagVz(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVertex().Z();

      return result;
    }

    double particleTruthTagVx(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getMCTagVertex().X();

      return result;
    }

    double particleTruthTagVy(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getMCTagVertex().Y();

      return result;
    }

    double particleTruthTagVz(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getMCTagVertex().Z();

      return result;
    }

    double particleTagVxErr(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert) {
        TMatrixFSym TagVErr = vert->getTagVertexErrMatrix();
        result = sqrt(TagVErr(0, 0));
      }

      return result;
    }

    double particleTagVyErr(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert) {
        TMatrixFSym TagVErr = vert->getTagVertexErrMatrix();
        result = sqrt(TagVErr(1, 1));
      }

      return result;
    }

    double particleTagVzErr(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert) {
        TMatrixFSym TagVErr = vert->getTagVertexErrMatrix();
        result = sqrt(TagVErr(2, 2));
      }

      return result;
    }

    double particleTagVpVal(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVertexPval();

      return result;
    }

    double particleTagVNTracks(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return std::numeric_limits<float>::quiet_NaN();
      return vert->getNTracks();
    }

    double particleTagVNFitTracks(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return std::numeric_limits<float>::quiet_NaN();
      return vert->getNFitTracks();
    }

    double particleTagVType(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return std::numeric_limits<float>::quiet_NaN();
      return vert->getFitType();
    }

    double particleTagVNDF(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return std::numeric_limits<float>::quiet_NaN();
      return vert->getTagVNDF();
    }

    double particleTagVChi2(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return std::numeric_limits<float>::quiet_NaN();
      return vert->getTagVChi2();
    }

    double particleTagVChi2IP(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return std::numeric_limits<float>::quiet_NaN();
      return vert->getTagVChi2IP();
    }


    // Delta t and related

    double particleDeltaT(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getDeltaT();

      return result;
    }

    double particleDeltaTErr(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getDeltaTErr();

      return result;
    }

    double particleMCDeltaT(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getMCDeltaT();

      return result;
    }

    double particleDeltaZ(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = particle->getZ() - vert->getTagVertex().Z();

      return result;
    }

    double particleDeltaZErr(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert) {
        double zVariance = particle->getVertexErrorMatrix()(2, 2);
        double TagVZVariance = vert->getTagVertexErrMatrix()(2, 2);
        result = sqrt(zVariance + TagVZVariance);
        if (std::isnan(result) or std::isinf(result)) result = std::numeric_limits<float>::quiet_NaN();
      }
      return result;
    }

    double particleDeltaB(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert) {
        PCmsLabTransform T;
        TVector3 boost = T.getBoostVector();
        double bg = boost.Mag() / TMath::Sqrt(1 - boost.Mag2());
        double c = Const::speedOfLight / 1000.; // cm ps-1
        result = vert->getDeltaT() * bg * c;
      }
      return result;
    }

    double particleDeltaBErr(const Particle* particle)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert) {
        PCmsLabTransform T;
        TVector3 boost = T.getBoostVector();
        double bg = boost.Mag() / TMath::Sqrt(1 - boost.Mag2());
        double c = Const::speedOfLight / 1000.; // cm ps-1
        result = vert->getDeltaTErr() * bg * c;
      }
      return result;
    }

    // Vertex boost direction

    double vertexBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;

      TVector3 boost = T.getBoostVector();
      TVector3 boostDir = boost.Unit();

      TVector3 pos = part->getVertex();
      double l = pos.Dot(boostDir);

      return l;
    }

    double vertexOrthogonalBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;

      TVector3 boost = T.getBoostVector();

      TVector3 orthBoost(boost.Z(), 0, -1 * boost.X());
      TVector3 orthBoostDir = orthBoost.Unit();

      TVector3 pos = part->getVertex();
      double l = pos.Dot(orthBoostDir);

      return l;
    }

    double vertexTruthBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;

      TVector3 boost = T.getBoostVector();
      TVector3 boostDir = boost.Unit();

      const MCParticle* mcPart = part->getRelated<MCParticle>();
      if (mcPart == nullptr) return std::numeric_limits<float>::quiet_NaN();
      TVector3 pos = mcPart->getDecayVertex();
      double l = pos.Dot(boostDir);

      return l;
    }

    double vertexTruthOrthogonalBoostDirection(const Particle* part)
    {
      static DBObjPtr<BeamParameters> beamParamsDB;
      TLorentzVector trueBeamEnergy = beamParamsDB->getHER() + beamParamsDB->getLER();
      TVector3 boost = trueBeamEnergy.BoostVector();

      TVector3 orthBoost(boost.Z(), 0, -1 * boost.X());
      TVector3 orthBoostDir = orthBoost.Unit();

      const MCParticle* mcPart = part->getRelated<MCParticle>();
      if (mcPart == nullptr) return std::numeric_limits<float>::quiet_NaN();
      TVector3 pos = mcPart->getDecayVertex();
      double l = pos.Dot(orthBoostDir);

      return l;
    }


    double vertexErrBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;

      TVector3 boost = T.getBoostVector();

      double cy = boost.Z() / TMath::Sqrt(boost.Z() * boost.Z() + boost.X() * boost.X());
      double sy = boost.X() / TMath::Sqrt(boost.Z() * boost.Z() + boost.X() * boost.X());
      double cx = TMath::Sqrt(boost.Z() * boost.Z() + boost.X() * boost.X()) / boost.Mag();
      double sx = boost.Y() / boost.Mag();

      TMatrixD RotY(3, 3);
      RotY(0, 0) = cy;  RotY(0, 1) = 0;   RotY(0, 2) = -sy;
      RotY(1, 0) = 0;   RotY(1, 1) = 1;   RotY(1, 2) = 0;
      RotY(2, 0) = sy;  RotY(2, 1) = 0;   RotY(2, 2) = cy;

      TMatrixD RotX(3, 3);
      RotX(0, 0) = 1;   RotX(0, 1) = 0;   RotX(0, 2) = 0;
      RotX(1, 0) = 0;   RotX(1, 1) = cx;  RotX(1, 2) = -sx;
      RotX(2, 0) = 0;   RotX(2, 1) = sx;  RotX(2, 2) = cx;

      TMatrixD Rot = RotY * RotX;
      TMatrixD RotCopy = Rot;
      TMatrixD RotInv = Rot.Invert();

      TMatrixD RR = (TMatrixD)part->getVertexErrorMatrix();
      TMatrixD RotErr = RotInv * RR * RotCopy;

      double VbErr = sqrt(RotErr(2, 2));


      return VbErr;
    }


    double vertexErrOrthBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;

      TVector3 boost = T.getBoostVector();
      TVector3 orthBoost(boost.Z(), 0, -1 * boost.X());
      TVector3 orthBoostDir = orthBoost.Unit();

      double cy = orthBoostDir.Z() / TMath::Sqrt(orthBoostDir.Z() * orthBoostDir.Z() + orthBoostDir.X() * orthBoostDir.X());
      double sy = orthBoostDir.X() / TMath::Sqrt(orthBoostDir.Z() * orthBoostDir.Z() + orthBoostDir.X() * orthBoostDir.X());
      double cx = TMath::Sqrt(orthBoostDir.Z() * orthBoostDir.Z() + orthBoostDir.X() * orthBoostDir.X()) / orthBoostDir.Mag();
      double sx = orthBoostDir.Y() / orthBoostDir.Mag();

      TMatrixD RotY(3, 3);
      RotY(0, 0) = cy;  RotY(0, 1) = 0;   RotY(0, 2) = -sy;
      RotY(1, 0) = 0;   RotY(1, 1) = 1;   RotY(1, 2) = 0;
      RotY(2, 0) = sy;  RotY(2, 1) = 0;   RotY(2, 2) = cy;

      TMatrixD RotX(3, 3);
      RotX(0, 0) = 1;   RotX(0, 1) = 0;   RotX(0, 2) = 0;
      RotX(1, 0) = 0;   RotX(1, 1) = cx;  RotX(1, 2) = -sx;
      RotX(2, 0) = 0;   RotX(2, 1) = sx;  RotX(2, 2) = cx;

      TMatrixD Rot = RotY * RotX;
      TMatrixD RotCopy = Rot;
      TMatrixD RotInv = Rot.Invert();

      TMatrixD RR = (TMatrixD)part->getVertexErrorMatrix();
      TMatrixD RotErr = RotInv * RR * RotCopy;

      double VbErr = sqrt(RotErr(2, 2));


      return VbErr;
    }




    // TagV boost direction

    double tagVBoostDirection(const Particle* part)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVl();

      return result;
    }


    double tagVOrthogonalBoostDirection(const Particle* part)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVol();

      return result;
    }


    double tagVTruthBoostDirection(const Particle* part)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTruthTagVl();

      return result;
    }


    double tagVTruthOrthogonalBoostDirection(const Particle* part)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTruthTagVol();

      return result;
    }

    double tagVErrBoostDirection(const Particle* part)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVlErr();

      return result;
    }


    double tagVErrOrthogonalBoostDirection(const Particle* part)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVolErr();

      return result;
    }


    double particleInternalTagVMCFlavor(const Particle* part)
    {
      double result = std::numeric_limits<float>::quiet_NaN();

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getMCTagBFlavor();

      return result;
    }

    double tagTrackMomentumX(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getVtxFitTrackPComponent(trackIndexInt, 0);

      return result;
    }

    double tagTrackMomentumY(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getVtxFitTrackPComponent(trackIndexInt, 1);

      return result;
    }

    double tagTrackMomentumZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getVtxFitTrackPComponent(trackIndexInt, 2);

      return result;
    }

    double tagTrackZ0(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getVtxFitTrackZ0(trackIndexInt);

      return result;
    }

    double tagTrackD0(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getVtxFitTrackD0(trackIndexInt);

      return result;
    }

    double tagTrackRaveWeight(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getRaveWeight(trackIndexInt);

      return result;
    }

    double tagTrackDistanceToConstraint(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();

      if (!vert) return result;
      if (vert->getConstraintType() == "noConstraint") return result;
      const TrackFitResult* tagTrack(vert->getVtxFitTrackResultPtr(trackIndexInt));
      if (!tagTrack) return result;

      result =  DistanceTools::trackToVtxDist(tagTrack -> getPosition(),
                                              tagTrack -> getMomentum(),
                                              vert -> getConstraintCenter());

      return result;
    }

    double tagTrackDistanceToConstraintErr(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();

      if (!vert) return result;
      if (vert->getConstraintType() == "noConstraint") return result;
      const TrackFitResult* tagTrack(vert->getVtxFitTrackResultPtr(trackIndexInt));
      if (!tagTrack) return result;

      //recover the covariance matrix associated to the position of the tag track

      TMatrixDSym trackPosCovMat(tagTrack->getCovariance6().GetSub(0, 2, 0, 2));

      result = DistanceTools::trackToVtxDistErr(tagTrack -> getPosition(),
                                                tagTrack -> getMomentum(),
                                                vert -> getConstraintCenter(),
                                                trackPosCovMat,
                                                vert -> getConstraintCov());

      return result;

    }

    double tagVDistanceToConstraint(const Particle* part)
    {
      double result(std::numeric_limits<float>::quiet_NaN());

      auto* vert = part->getRelatedTo<TagVertex>();

      if (!vert) return result;
      if (vert->getConstraintType() == "noConstraint") return result;

      result =  DistanceTools::vtxToVtxDist(vert -> getConstraintCenter(),
                                            vert -> getTagVertex());
      return result;
    }

    double tagVDistanceToConstraintErr(const Particle* part)
    {
      double result(std::numeric_limits<float>::quiet_NaN());

      auto* vert = part->getRelatedTo<TagVertex>();

      if (!vert) return result;
      if (vert->getConstraintType() == "noConstraint") return result;

      //To compute the uncertainty, the tag vtx uncertainty is NOT taken into account
      //The error computed is the the one used  in the chi2.
      //To change that, emptyMat has to be replaced by m_TagVertexErrMatrix
      TMatrixDSym emptyMat(3);

      result = DistanceTools::vtxToVtxDistErr(vert -> getConstraintCenter(),
                                              vert -> getTagVertex(),
                                              vert -> getConstraintCov(),
                                              emptyMat);

      return result;
    }

    double tagTrackDistanceToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();

      if (!vert) return result;

      const TrackFitResult* tagTrack(vert->getVtxFitTrackResultPtr(trackIndexInt));
      if (!tagTrack) return result;

      result = DistanceTools::trackToVtxDist(tagTrack -> getPosition(),
                                             tagTrack -> getMomentum(),
                                             vert -> getTagVertex());


      return result;
    }

    double tagTrackDistanceToTagVErr(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return result;

      const TrackFitResult* tagTrack(vert->getVtxFitTrackResultPtr(trackIndexInt));
      if (!tagTrack) return result;

      //recover the covariance matrix associated to the position of the tag track

      TMatrixDSym trackPosCovMat(tagTrack->getCovariance6().GetSub(0, 2, 0, 2));

      //To compute the uncertainty, the tag vtx uncertainty is NOT taken into account
      //The error computed is then the one in the chi2.
      //To change that, emptyMat has to be replaced by m_TagVertexErrMatrix

      TMatrixDSym emptyMat(3);

      result = DistanceTools::trackToVtxDistErr(tagTrack -> getPosition(),
                                                tagTrack -> getMomentum(),
                                                vert -> getTagVertex(),
                                                trackPosCovMat,
                                                emptyMat);

      return result;
    }

    double tagTrackTrueDistanceToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      double result(std::numeric_limits<float>::quiet_NaN());
      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return result;

      const MCParticle* mcParticle(vert -> getVtxFitTrackMCParticle(trackIndexInt));
      if (!mcParticle) return result;

      TVector3 mcTagV(vert->getMCTagVertex());

      if (mcTagV(0)  == -111 && mcTagV(1) == -111 && mcTagV(2) == -111) return std::numeric_limits<float>::quiet_NaN();
      if (mcTagV(0)  == std::numeric_limits<float>::quiet_NaN()) return std::numeric_limits<float>::quiet_NaN();
      if (mcTagV(0)  == 0 && mcTagV(1) == 0 && mcTagV(2) == 0) return std::numeric_limits<float>::quiet_NaN();

      result = DistanceTools::trackToVtxDist(mcParticle -> getProductionVertex(),
                                             mcParticle -> getMomentum(),
                                             mcTagV);

      return result;
    }

    TVector3 tagTrackTrueVecToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result(std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::quiet_NaN());

      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return result;

      const MCParticle* mcParticle(vert -> getVtxFitTrackMCParticle(trackIndexInt));
      if (!mcParticle) return result;

      TVector3 mcTagV(vert->getMCTagVertex());

      if (mcTagV(0)  == -111 && mcTagV(1) == -111 && mcTagV(2) == -111) return result;
      if (mcTagV(0)  == std::numeric_limits<float>::quiet_NaN()) return result;
      if (mcTagV(0)  == 0 && mcTagV(1) == 0 && mcTagV(2) == 0) return result;

      result = DistanceTools::trackToVtxVec(mcParticle -> getProductionVertex(),
                                            mcParticle -> getMomentum(),
                                            mcTagV);

      return result;
    }

    double tagTrackTrueVecToTagVX(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result;
      result = tagTrackTrueVecToTagV(part, trackIndex);

      return result(0);
    }

    double tagTrackTrueVecToTagVY(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result;
      result = tagTrackTrueVecToTagV(part, trackIndex);

      return result(1);
    }

    double tagTrackTrueVecToTagVZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result;
      result = tagTrackTrueVecToTagV(part, trackIndex);

      return result(2);
    }

    TVector3 tagTrackTrueMomentum(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result(std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::quiet_NaN());

      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return result;

      const MCParticle* mcParticle(vert -> getVtxFitTrackMCParticle(trackIndexInt));
      if (!mcParticle) return result;

      return mcParticle -> getMomentum();
    }

    double tagTrackTrueMomentumX(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result;
      result = tagTrackTrueMomentum(part, trackIndex);

      return result(0);
    }

    double tagTrackTrueMomentumY(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result;
      result = tagTrackTrueMomentum(part, trackIndex);

      return result(1);
    }

    double tagTrackTrueMomentumZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result;
      result = tagTrackTrueMomentum(part, trackIndex);

      return result(2);
    }

    TVector3 tagTrackTrueOrigin(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result(std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::quiet_NaN());

      if (trackIndex.size() != 1) return result;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return result;

      const MCParticle* mcParticle(vert -> getVtxFitTrackMCParticle(trackIndexInt));
      if (!mcParticle) return result;

      return mcParticle -> getProductionVertex();
    }

    double tagTrackTrueOriginX(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result;
      result = tagTrackTrueOrigin(part, trackIndex);

      return result(0);
    }

    double tagTrackTrueOriginY(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result;
      result = tagTrackTrueOrigin(part, trackIndex);

      return result(1);
    }

    double tagTrackTrueOriginZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result;
      result = tagTrackTrueOrigin(part, trackIndex);

      return result(2);
    }

    int fitTruthStatus(const Particle* part)
    {
      int result(-1);
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return result;
      return vert->getFitTruthStatus();
    }

    VARIABLE_GROUP("Time Dependent CPV Analysis Variables");

    REGISTER_VARIABLE("TagVx", particleTagVx, "Tag vertex X component");
    REGISTER_VARIABLE("TagVy", particleTagVy, "Tag vertex Y component");
    REGISTER_VARIABLE("TagVz", particleTagVz, "Tag vertex Z component");
    REGISTER_VARIABLE("mcTagVx", particleTruthTagVx, "Generated Tag vertex X component");
    REGISTER_VARIABLE("mcTagVy", particleTruthTagVy, "Generated Tag vertex Y component");
    REGISTER_VARIABLE("mcTagVz", particleTruthTagVz, "Generated Tag vertex Z component");
    REGISTER_VARIABLE("TagVxErr", particleTagVxErr, "Tag vertex X component uncertainty");
    REGISTER_VARIABLE("TagVyErr", particleTagVyErr, "Tag vertex Y component uncertainty");
    REGISTER_VARIABLE("TagVzErr", particleTagVzErr, "Tag vertex Z component uncertainty");
    REGISTER_VARIABLE("TagVpVal", particleTagVpVal, "Tag vertex p-Value");
    REGISTER_VARIABLE("TagVNTracks", particleTagVNTracks, "Number of tracks in the tag vertex");
    REGISTER_VARIABLE("TagVType", particleTagVType, "Fit type of the tag vertex");
    REGISTER_VARIABLE("TagVNDF", particleTagVNDF, "Number of degrees of freedom in the tag vertex fit");
    REGISTER_VARIABLE("TagVChi2", particleTagVChi2, "chi2 value of the tag vertex fit");
    REGISTER_VARIABLE("TagVChi2IP", particleTagVChi2IP, "IP component of chi2 value of the tag vertex fit");

    REGISTER_VARIABLE("DeltaT", particleDeltaT,
                      R"DOC(Proper decay time difference :math:`\Delta t` between signal B-meson :math:`(B_{rec})` and tag B-meson :math:`(B_{tag})` in ps.)DOC");
    REGISTER_VARIABLE("DeltaTErr", particleDeltaTErr,
                      R"DOC(Proper decay time difference :math:`\Delta t` uncertainty in ps)DOC");
    REGISTER_VARIABLE("MCDeltaT", particleMCDeltaT,
                      R"DOC(Generated proper decay time difference :math:`\Delta t` in ps)DOC");
    REGISTER_VARIABLE("DeltaZ", particleDeltaZ,
                      R"DOC(Difference of decay vertex longitudinal components between signal B-meson :math:`(B_{rec})` and tag B-meson :math:`(B_{tag})`:
:math:`\Delta z = z(B_{rec}) - z(B_{tag})`)DOC");
    REGISTER_VARIABLE("DeltaZErr", particleDeltaZErr,
                      R"DOC(Uncertainty of the difference :math:`z(B_{rec}) - z(B_{tag})`)DOC");
    REGISTER_VARIABLE("DeltaBoost", particleDeltaB, R"DOC(:math:`\Delta z` in the boost direction)DOC");
    REGISTER_VARIABLE("DeltaBoostErr", particleDeltaBErr, R"DOC(Uncertanty of :math:`\Delta z` in the boost direction)DOC");

    REGISTER_VARIABLE("LBoost", vertexBoostDirection,
                      "Returns the vertex component in the boost direction");
    REGISTER_VARIABLE("OBoost", vertexOrthogonalBoostDirection,
                      "Returns the vertex component in the direction orthogonal to the boost");
    REGISTER_VARIABLE("mcLBoost", vertexTruthBoostDirection,
                      "Returns the MC vertex component in the boost direction");
    REGISTER_VARIABLE("mcOBoost", vertexTruthOrthogonalBoostDirection,
                      "Returns the MC vertex component in the direction orthogonal to the boost");
    REGISTER_VARIABLE("LBoostErr", vertexErrBoostDirection,
                      "Returns the error of the vertex in the boost direction");
    REGISTER_VARIABLE("OBoostErr", vertexErrOrthBoostDirection,
                      "Returns the error of the vertex in the direction orthogonal to the boost");

    REGISTER_VARIABLE("TagVLBoost", tagVBoostDirection,
                      "Returns the TagV component in the boost direction");
    REGISTER_VARIABLE("TagVOBoost", tagVOrthogonalBoostDirection,
                      "Returns the TagV component in the direction orthogonal to the boost");
    REGISTER_VARIABLE("TagVmcLBoost", tagVTruthBoostDirection,
                      "Returns the MC TagV component in the boost direction");
    REGISTER_VARIABLE("TagVmcOBoost", tagVTruthOrthogonalBoostDirection,
                      "Returns the MC TagV component in the direction orthogonal to the boost");
    REGISTER_VARIABLE("TagVLBoostErr", tagVErrBoostDirection,
                      "Returns the error of TagV in the boost direction");
    REGISTER_VARIABLE("TagVOBoostErr", tagVErrOrthogonalBoostDirection,
                      "Returns the error of TagV in the direction orthogonal to the boost");

    REGISTER_VARIABLE("internalTagVMCFlavor", particleInternalTagVMCFlavor,
                      "[Expert] [Debugging] This variable is only for internal checks of the TagV module by developers. \n"
                      "It returns the internal mc flavor information of the tag-side B provided by the TagV module.");

    REGISTER_VARIABLE("TagTrackMomentumX(i) ", tagTrackMomentumX,
                      "return the X component of the momentum of the ith track used in the tag vtx fit.");
    REGISTER_VARIABLE("TagTrackMomentumY(i) ", tagTrackMomentumY,
                      "return the Y component of the momentum of the ith track used in the tag vtx fit.");
    REGISTER_VARIABLE("TagTrackMomentumZ(i) ", tagTrackMomentumZ,
                      "return the Z component of the momentum of the ith track used in the tag vtx fit.");

    REGISTER_VARIABLE("TagTrackZ0(i)", tagTrackZ0, "return the z0 parameter of the ith track used in the tag vtx fit");
    REGISTER_VARIABLE("TagTrackD0(i)", tagTrackD0, "return the d0 parameter of the ith track used in the tag vtx fit");


    REGISTER_VARIABLE("TagTrackRaveWeight(i)", tagTrackRaveWeight, "returns the weight assigned by Rave to track i");

    REGISTER_VARIABLE("TagVNFitTracks", particleTagVNFitTracks,
                      "returns the number of tracks used by rave to fit the vertex (not counting the ones coming from Kshorts)");

    REGISTER_VARIABLE("TagTrackDistanceToConstraint(i)", tagTrackDistanceToConstraint,
                      "returns the measured distance between the ith tag track and the centre of the constraint.");

    REGISTER_VARIABLE("TagTrackDistanceToConstraintErr(i)", tagTrackDistanceToConstraintErr,
                      "returns the estimated error on the distance between the ith tag track and the centre of the constraint.");


    REGISTER_VARIABLE("TagVDistanceToConstraint", tagVDistanceToConstraint,
                      "returns the measured distance between the tag vtx and the centre of the constraint.");

    REGISTER_VARIABLE("TagVDistanceToConstraintErr", tagVDistanceToConstraintErr,
                      "returns the estimated error on the distance between the tag vtx and the centre of the constraint.");

    REGISTER_VARIABLE("TagTrackDistanceToTagV(i)", tagTrackDistanceToTagV,
                      "returns the measured distance between the ith tag track and the tag vtx.");

    REGISTER_VARIABLE("TagTrackDistanceToTagVErr(i)", tagTrackDistanceToTagVErr,
                      "returns the estimated error on the distance between the ith tag track and the tag vtx. Warning: only the uncertainties on the track position parameters are taken into account.");

    REGISTER_VARIABLE("TagTrackTrueDistanceToTagV(i)", tagTrackTrueDistanceToTagV,
                      "return the true distance between the true B Tag decay vertex and the p'cle corresponding to the ith tag vtx track.")

    REGISTER_VARIABLE("TagTrackTrueVecToTagVX(i)", tagTrackTrueVecToTagVX,
                      "return the X coordinate of the vector between the mc particle corresponding to the ith tag vtx track and the true tag B decay vertex.")

    REGISTER_VARIABLE("TagTrackTrueVecToTagVY(i)", tagTrackTrueVecToTagVY,
                      "return the Y coordinate of the vector between the mc particle corresponding to the ith tag vtx track and the true tag B decay vertex.")

    REGISTER_VARIABLE("TagTrackTrueVecToTagVZ(i)", tagTrackTrueVecToTagVZ,
                      "return the Z coordinate of the vector between the mc particle corresponding to the ith tag vtx track and the true tag B decay vertex.")

    REGISTER_VARIABLE("TagTrackTrueMomentumX(i)", tagTrackTrueMomentumX,
                      "return the X component of the true momentum of the MC particle corresponding to the ith tag vtx track.")

    REGISTER_VARIABLE("TagTrackTrueMomentumY(i)", tagTrackTrueMomentumY,
                      "return the Y component of the true momentum of the MC particle corresponding to the ith tag vtx track.")

    REGISTER_VARIABLE("TagTrackTrueMomentumZ(i)", tagTrackTrueMomentumZ,
                      "return the Z component of the true momentum of the MC particle corresponding to the ith tag vtx track.")

    REGISTER_VARIABLE("TagTrackTrueOriginX(i)", tagTrackTrueOriginX,
                      "return the X component of the true origin of the MC particle corresponding to the ith tag vtx track.")

    REGISTER_VARIABLE("TagTrackTrueOriginY(i)", tagTrackTrueOriginY,
                      "return the Y component of the true origin of the MC particle corresponding to the ith tag vtx track.")

    REGISTER_VARIABLE("TagTrackTrueOriginZ(i)", tagTrackTrueOriginZ,
                      "return the Z component of the true origin of the MC particle corresponding to the ith tag vtx track.")

    REGISTER_VARIABLE("TagVFitTruthStatus", fitTruthStatus,
                      "Returns the status of the fit performed with the truth info.")
  }
}

