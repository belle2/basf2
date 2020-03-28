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

    static const TVector3 vecNaN(std::numeric_limits<float>::quiet_NaN(),
                                 std::numeric_limits<float>::quiet_NaN(),
                                 std::numeric_limits<float>::quiet_NaN());
    static const double realNaN = std::numeric_limits<float>::quiet_NaN();

    //   ############################################## Time Dependent CPV Analysis Variables  ###############################################


    // TagV x, y, z

    double particleTagVx(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVertex().X();
    }

    double particleTagVy(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVertex().Y();
    }

    double particleTagVz(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVertex().Z();
    }

    double particleTruthTagVx(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getMCTagVertex().X();
    }

    double particleTruthTagVy(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getMCTagVertex().Y();
    }

    double particleTruthTagVz(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getMCTagVertex().Z();
    }

    double particleTagVxErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      TMatrixDSym TagVErr = vert->getTagVertexErrMatrix();
      return sqrt(TagVErr(0, 0));
    }

    double particleTagVyErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      TMatrixDSym TagVErr = vert->getTagVertexErrMatrix();
      return sqrt(TagVErr(1, 1));
    }

    double particleTagVzErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      TMatrixDSym TagVErr = vert->getTagVertexErrMatrix();
      return sqrt(TagVErr(2, 2));
    }

    double particleTagVpVal(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVertexPval();
    }

    double particleTagVNTracks(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getNTracks();
    }

    double particleTagVNFitTracks(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getNFitTracks();
    }

    double particleTagVType(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getFitType();
    }

    double particleTagVNDF(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVNDF();
    }

    double particleTagVChi2(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVChi2();
    }

    double particleTagVChi2IP(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVChi2IP();
    }


    // Delta t and related

    double particleDeltaT(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getDeltaT();
    }

    double particleDeltaTErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getDeltaTErr();
    }

    double particleMCDeltaT(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getMCDeltaT();
    }

    double particleMCDeltaTapprox(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getMCDeltaTapprox();
    }

    double particleMCDeltaL(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      PCmsLabTransform T;
      TVector3 boost = T.getBoostVector();
      double bg = boost.Mag() / sqrt(1 - boost.Mag2());
      double c = Const::speedOfLight / 1000.; // cm ps-1
      return vert->getMCDeltaTapprox() * bg * c;
    }


    double particleDeltaZ(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return particle->getZ() - vert->getTagVertex().Z();
    }

    double particleDeltaZErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      double zVariance = particle->getVertexErrorMatrix()(2, 2);
      double TagVZVariance = vert->getTagVertexErrMatrix()(2, 2);
      double result = sqrt(zVariance + TagVZVariance);
      if (std::isnan(result) || std::isinf(result)) return realNaN;

      return result;
    }

    double particleDeltaB(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      PCmsLabTransform T;
      TVector3 boost = T.getBoostVector();
      double bg = boost.Mag() / sqrt(1 - boost.Mag2());
      double c = Const::speedOfLight / 1000.; // cm ps-1
      return vert->getDeltaT() * bg * c;
    }

    double particleDeltaBErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      PCmsLabTransform T;
      TVector3 boost = T.getBoostVector();
      double bg = boost.Mag() / sqrt(1 - boost.Mag2());
      double c = Const::speedOfLight / 1000.; // cm ps-1
      return vert->getDeltaTErr() * bg * c;
    }

    // Vertex boost direction

    double vertexBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;
      TVector3 boostDir = T.getBoostVector().Unit();

      TVector3 pos = part->getVertex();
      return pos.Dot(boostDir);
    }

    double vertexOrthogonalBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;
      TVector3 boost = T.getBoostVector();

      TVector3 orthBoost(boost.Z(), 0, -boost.X());
      TVector3 orthBoostDir = orthBoost.Unit();

      TVector3 pos = part->getVertex();
      return pos.Dot(orthBoostDir);
    }

    double vertexTruthBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;
      TVector3 boostDir = T.getBoostVector().Unit();

      const MCParticle* mcPart = part->getRelated<MCParticle>();
      if (!mcPart) return realNaN;
      TVector3 pos = mcPart->getDecayVertex();
      return pos.Dot(boostDir);
    }

    double vertexTruthOrthogonalBoostDirection(const Particle* part)
    {
      static DBObjPtr<BeamParameters> beamParamsDB;
      TLorentzVector trueBeamEnergy = beamParamsDB->getHER() + beamParamsDB->getLER();
      TVector3 boost = trueBeamEnergy.BoostVector();

      TVector3 orthBoost(boost.Z(), 0, -boost.X());
      TVector3 orthBoostDir = orthBoost.Unit();

      const MCParticle* mcPart = part->getRelated<MCParticle>();
      if (!mcPart) return realNaN;
      TVector3 pos = mcPart->getDecayVertex();
      return pos.Dot(orthBoostDir);
    }


    //TODO : unify with implementation in analysis/modules/TagVertex/src/TagVertexModule.cc
    static TMatrixD toMatrix(TRotation r)
    {
      TMatrixD rM(3, 3);
      for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
          rM(i, j) = r(i, j);
      return rM;
    }

    //  Ry * Rx
    static TMatrixD getRotationMatrixXY(double angleX, double angleY)
    {
      TRotation r;
      r.RotateX(angleX);
      r.RotateY(angleY);
      return toMatrix(r);
    }


    double vertexErrBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;
      TVector3 boost = T.getBoostVector();

      double zxD = sqrt(boost.Z() * boost.Z() + boost.X() * boost.X());
      double angleY = atan2(boost.X(), boost.Z());
      double angleX = atan2(boost.Y(), zxD);
      TMatrixD Rot = getRotationMatrixXY(angleX, -angleY);
      TMatrixD RotT = Rot; RotT.T();

      TMatrixD RR = (TMatrixD)part->getVertexErrorMatrix();
      TMatrixD RotErr = RotT * RR * Rot;

      return sqrt(RotErr(2, 2));
    }


    double vertexErrOrthBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;

      TVector3 boost = T.getBoostVector();
      TVector3 orthBoost(boost.Z(), 0, -boost.X());
      TVector3 orthBoostDir = orthBoost.Unit();

      double zxD = sqrt(orthBoostDir.Z() * orthBoostDir.Z() + orthBoostDir.X() * orthBoostDir.X());
      double angleY = atan2(orthBoostDir.X(), orthBoostDir.Z());
      double angleX = atan2(orthBoostDir.Y(), zxD);
      TMatrixD Rot = getRotationMatrixXY(angleX, -angleY);
      TMatrixD RotT = Rot; RotT.T();

      TMatrixD RR = (TMatrixD)part->getVertexErrorMatrix();
      TMatrixD RotErr = RotT * RR * Rot;

      return sqrt(RotErr(2, 2));
    }




    // TagV boost direction

    double tagVBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVl();
    }


    double tagVOrthogonalBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVol();
    }


    double tagVTruthBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTruthTagVl();
    }


    double tagVTruthOrthogonalBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTruthTagVol();
    }

    double tagVErrBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVlErr();
    }


    double tagVErrOrthogonalBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getTagVolErr();
    }


    double particleInternalTagVMCFlavor(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getMCTagBFlavor();
    }

    double tagTrackMomentumX(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      return vert->getVtxFitTrackPComponent(trackIndexInt, 0);
    }

    double tagTrackMomentumY(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      return vert->getVtxFitTrackPComponent(trackIndexInt, 1);
    }

    double tagTrackMomentumZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      return vert->getVtxFitTrackPComponent(trackIndexInt, 2);
    }

    double tagTrackZ0(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      return vert->getVtxFitTrackZ0(trackIndexInt);
    }

    double tagTrackD0(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      return vert->getVtxFitTrackD0(trackIndexInt);
    }

    double tagTrackRaveWeight(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      return vert->getRaveWeight(trackIndexInt);
    }

    double tagTrackDistanceToConstraint(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      if (vert->getConstraintType() == "noConstraint") return realNaN;
      const Particle* tagParticle(vert->getVtxFitParticle(trackIndexInt));
      if (!tagParticle) return realNaN;

      return  DistanceTools::trackToVtxDist(tagParticle->getTrackFitResult() -> getPosition(),
                                            tagParticle->getMomentum(),
                                            vert->getConstraintCenter());
    }

    double tagTrackDistanceToConstraintErr(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();

      if (!vert) return realNaN;
      if (vert->getConstraintType() == "noConstraint") return realNaN;
      const Particle* tagParticle(vert->getVtxFitParticle(trackIndexInt));
      if (!tagParticle) return realNaN;

      //recover the covariance matrix associated to the position of the tag track

      TMatrixDSym trackPosCovMat(tagParticle->getVertexErrorMatrix());

      return DistanceTools::trackToVtxDistErr(tagParticle->getTrackFitResult() -> getPosition(),
                                              tagParticle->getMomentum(),
                                              vert->getConstraintCenter(),
                                              trackPosCovMat,
                                              vert->getConstraintCov());


    }

    double tagVDistanceToConstraint(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (vert->getConstraintType() == "noConstraint") return realNaN;

      return DistanceTools::vtxToVtxDist(vert->getConstraintCenter(),
                                         vert->getTagVertex());
    }

    double tagVDistanceToConstraintErr(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (vert->getConstraintType() == "noConstraint") return realNaN;

      //To compute the uncertainty, the tag vtx uncertainty is NOT taken into account
      //The error computed is the the one used  in the chi2.
      //To change that, emptyMat has to be replaced by m_TagVertexErrMatrix
      TMatrixDSym emptyMat(3);

      return DistanceTools::vtxToVtxDistErr(vert->getConstraintCenter(),
                                            vert->getTagVertex(),
                                            vert->getConstraintCov(),
                                            emptyMat);
    }

    double tagTrackDistanceToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      const Particle* particle(vert->getVtxFitParticle(trackIndexInt));
      if (!particle) return realNaN;

      return DistanceTools::trackToVtxDist(particle -> getTrackFitResult() -> getPosition(),
                                           particle -> getMomentum(),
                                           vert -> getTagVertex());
    }

    double tagTrackDistanceToTagVErr(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      const Particle* tagParticle(vert->getVtxFitParticle(trackIndexInt));
      if (!tagParticle) return realNaN;

      //recover the covariance matrix associated to the position of the tag track

      TMatrixDSym trackPosCovMat(tagParticle->getVertexErrorMatrix());

      //To compute the uncertainty, the tag vtx uncertainty is NOT taken into account
      //The error computed is then the one in the chi2.
      //To change that, emptyMat has to be replaced by m_TagVertexErrMatrix

      TMatrixDSym emptyMat(3);

      return  DistanceTools::trackToVtxDistErr(tagParticle->getTrackFitResult()->getPosition(),
                                               tagParticle->getMomentum(),
                                               vert->getTagVertex(),
                                               trackPosCovMat,
                                               emptyMat);

    }

    double tagTrackTrueDistanceToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return realNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      const MCParticle* mcParticle(vert -> getVtxFitMCParticle(trackIndexInt));
      if (!mcParticle) return realNaN;

      TVector3 mcTagV(vert->getMCTagVertex());

      if (mcTagV(0)  == -111 && mcTagV(1) == -111 && mcTagV(2) == -111) return realNaN;
      if (mcTagV(0)  == std::numeric_limits<float>::quiet_NaN())        return realNaN;
      if (mcTagV(0)  == 0 && mcTagV(1) == 0 && mcTagV(2) == 0)          return realNaN;

      return DistanceTools::trackToVtxDist(mcParticle->getProductionVertex(),
                                           mcParticle->getMomentum(),
                                           mcTagV);
    }

    TVector3 tagTrackTrueVecToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return vecNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return vecNaN;

      const MCParticle* mcParticle(vert -> getVtxFitMCParticle(trackIndexInt));
      if (!mcParticle) return vecNaN;

      TVector3 mcTagV(vert->getMCTagVertex());

      if (mcTagV(0)  == -111 && mcTagV(1) == -111 && mcTagV(2) == -111) return vecNaN;
      if (mcTagV(0)  == std::numeric_limits<float>::quiet_NaN())        return vecNaN;
      if (mcTagV(0)  == 0 && mcTagV(1) == 0 && mcTagV(2) == 0)          return vecNaN;

      return DistanceTools::trackToVtxVec(mcParticle->getProductionVertex(),
                                          mcParticle->getMomentum(),
                                          mcTagV);
    }

    double tagTrackTrueVecToTagVX(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result = tagTrackTrueVecToTagV(part, trackIndex);
      return result(0);
    }

    double tagTrackTrueVecToTagVY(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result = tagTrackTrueVecToTagV(part, trackIndex);
      return result(1);
    }

    double tagTrackTrueVecToTagVZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result = tagTrackTrueVecToTagV(part, trackIndex);
      return result(2);
    }

    TVector3 tagTrackTrueMomentum(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return vecNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return vecNaN;

      const MCParticle* mcParticle(vert -> getVtxFitMCParticle(trackIndexInt));
      if (!mcParticle) return vecNaN;

      return mcParticle->getMomentum();
    }

    double tagTrackTrueMomentumX(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result = tagTrackTrueMomentum(part, trackIndex);
      return result(0);
    }

    double tagTrackTrueMomentumY(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result = tagTrackTrueMomentum(part, trackIndex);
      return result(1);
    }

    double tagTrackTrueMomentumZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result = tagTrackTrueMomentum(part, trackIndex);
      return result(2);
    }

    TVector3 tagTrackTrueOrigin(const Particle* part, const std::vector<double>& trackIndex)
    {
      if (trackIndex.size() != 1) return vecNaN;
      unsigned int trackIndexInt(trackIndex.at(0));

      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return vecNaN;

      const MCParticle* mcParticle(vert -> getVtxFitMCParticle(trackIndexInt));
      if (!mcParticle) return vecNaN;

      return mcParticle->getProductionVertex();
    }

    double tagTrackTrueOriginX(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result = tagTrackTrueOrigin(part, trackIndex);
      return result(0);
    }

    double tagTrackTrueOriginY(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result = tagTrackTrueOrigin(part, trackIndex);
      return result(1);
    }

    double tagTrackTrueOriginZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      TVector3 result = tagTrackTrueOrigin(part, trackIndex);
      return result(2);
    }

    int fitTruthStatus(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return std::numeric_limits<int>::quiet_NaN();
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
    REGISTER_VARIABLE("TagVType", particleTagVType,
                      R"DOC(Type of algo for the tag vertex. -1: failed (1,2: single track, deprecated), 3: standard, 4: standard_PXD, 5: no constraint)DOC");
    REGISTER_VARIABLE("TagVNDF", particleTagVNDF, "Number of degrees of freedom in the tag vertex fit");
    REGISTER_VARIABLE("TagVChi2", particleTagVChi2, "chi2 value of the tag vertex fit");
    REGISTER_VARIABLE("TagVChi2IP", particleTagVChi2IP, "IP component of chi2 value of the tag vertex fit");

    REGISTER_VARIABLE("DeltaT", particleDeltaT,
                      R"DOC(Proper decay time difference :math:`\Delta t` between signal B-meson :math:`(B_{rec})` and tag B-meson :math:`(B_{tag})` in ps.)DOC");
    REGISTER_VARIABLE("DeltaTErr", particleDeltaTErr,
                      R"DOC(Proper decay time difference :math:`\Delta t` uncertainty in ps)DOC");
    REGISTER_VARIABLE("mcDeltaT", particleMCDeltaT,
                      R"DOC(Generated proper decay time difference :math:`\Delta t` in ps)DOC");
    REGISTER_VARIABLE("mcDeltaTapprox", particleMCDeltaTapprox,
                      R"DOC(Generated proper decay time difference (in z-difference approximation):math:`\Delta t` in ps)DOC");
    REGISTER_VARIABLE("mcDeltaL", particleMCDeltaL,
                      R"DOC(True difference of decay vertex boost-direction components between signal B-meson :math:`(B_{rec})` and tag B-meson :math:`(B_{tag})`:
:math:`\Delta l = l(B_{rec}) - l(B_{tag})`)DOC");
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
                      "Returns the status of the fit performed with the truth info. Possible values are: 0: fit performed with measured parameters, 1: fit performed with true parameters, 2: unable to recover truth parameters")
  }
}

