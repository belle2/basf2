/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/TimeDependentVariables.h>

#include <framework/dbobjects/BeamParameters.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/TagVertex.h>
#include <mdst/dataobjects/MCParticle.h>

//utilities
#include <analysis/utility/DistanceTools.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/RotationTools.h>
#include <analysis/utility/ReferenceFrame.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/geometry/B2Vector3.h>

#include <TMatrixD.h>
#include <TVectorD.h>

//#include <iostream>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    // from RotationTools.h
    using RotationTools::getUnitOrthogonal;
    using RotationTools::toVec;

    static const B2Vector3D vecNaN(Const::doubleNaN, Const::doubleNaN, Const::doubleNaN);

    //   ############################################## Time Dependent CPV Analysis Variables  ###############################################

    // TagV x, y, z
    double particleTagVx(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVertex().X();
    }

    double particleTagVy(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVertex().Y();
    }

    double particleTagVz(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVertex().Z();
    }

    double particleTruthTagVx(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getMCTagVertex().X();
    }

    double particleTruthTagVy(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getMCTagVertex().Y();
    }

    double particleTruthTagVz(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getMCTagVertex().Z();
    }

    double particleTagVxErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      TMatrixDSym TagVErr = vert->getTagVertexErrMatrix();
      return sqrt(TagVErr(0, 0));
    }

    double particleTagVyErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      TMatrixDSym TagVErr = vert->getTagVertexErrMatrix();
      return sqrt(TagVErr(1, 1));
    }

    double particleTagVzErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      TMatrixDSym TagVErr = vert->getTagVertexErrMatrix();
      return sqrt(TagVErr(2, 2));
    }

    double particleTagVpVal(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVertexPval();
    }

    double particleTagVNTracks(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getNTracks();
    }

    double particleTagVNFitTracks(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getNFitTracks();
    }

    double particleTagVType(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getFitType();
    }

    double particleTagVNDF(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVNDF();
    }

    double particleTagVChi2(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVChi2();
    }

    double particleTagVChi2IP(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVChi2IP();
    }

    // Delta t and related

    double particleDeltaT(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getDeltaT();
    }

    double particleDeltaTErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getDeltaTErr();
    }

    double particleDeltaTRes(const Particle* particle)
    {
      return particleDeltaT(particle) - particleMCDeltaT(particle);
    }

    double particleDeltaTBelle(const Particle* particle)
    {
      double beta = PCmsLabTransform().getBoostVector().Mag();
      double bg = beta / sqrt(1 - beta * beta);
      double c = Const::speedOfLight / 1000.; // cm ps-1
      return particleDeltaZ(particle) / bg / c;
    }

    double particleMCDeltaTau(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getMCDeltaTau();
    }

    double particleMCDeltaT(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getMCDeltaT();
    }

    double particleMCDeltaBoost(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      double beta = PCmsLabTransform().getBoostVector().Mag();
      double bg = beta / sqrt(1 - beta * beta);
      double c = Const::speedOfLight / 1000.; // cm ps-1
      return vert->getMCDeltaT() * bg * c;
    }

    double particleDeltaZ(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return particle->getZ() - vert->getTagVertex().Z();
    }

    double particleDeltaZErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      double zVariance     = particle->getVertexErrorMatrix()(2, 2);
      double TagVZVariance = vert->getTagVertexErrMatrix()(2, 2);
      double result = sqrt(zVariance + TagVZVariance);
      if (!std::isfinite(result)) return Const::doubleNaN;

      return result;
    }

    double particleDeltaB(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      double beta = PCmsLabTransform().getBoostVector().Mag();
      double bg = beta / sqrt(1 - beta * beta);
      double c = Const::speedOfLight / 1000.; // cm ps-1
      return vert->getDeltaT() * bg * c;
    }

    double particleDeltaBErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      double beta = PCmsLabTransform().getBoostVector().Mag();
      double bg = beta / sqrt(1 - beta * beta);
      double c = Const::speedOfLight / 1000.; // cm ps-1
      return vert->getDeltaTErr() * bg * c;
    }

    // Vertex boost direction

    double vertexBoostDirection(const Particle* part)
    {
      B2Vector3D boostDir = PCmsLabTransform().getBoostVector().Unit();
      B2Vector3F pos = part->getVertex();
      return pos.Dot(boostDir);
    }

    double vertexOrthogonalBoostDirection(const Particle* part)
    {
      B2Vector3D boost = PCmsLabTransform().getBoostVector();
      B2Vector3D orthBoostDir = getUnitOrthogonal(boost);

      B2Vector3F pos = part->getVertex();
      return pos.Dot(orthBoostDir);
    }

    double vertexTruthBoostDirection(const Particle* part)
    {
      static DBObjPtr<BeamParameters> beamParamsDB;
      B2Vector3D boostDir = -(beamParamsDB->getHER() + beamParamsDB->getLER()).BoostToCM().Unit();

      const MCParticle* mcPart = part->getMCParticle();
      if (!mcPart) return Const::doubleNaN;
      B2Vector3D pos = mcPart->getDecayVertex();
      return pos.Dot(boostDir);
    }

    double vertexTruthOrthogonalBoostDirection(const Particle* part)
    {
      static DBObjPtr<BeamParameters> beamParamsDB;
      B2Vector3D boost = -(beamParamsDB->getHER() + beamParamsDB->getLER()).BoostToCM();
      B2Vector3D orthBoostDir = getUnitOrthogonal(boost);

      const MCParticle* mcPart = part->getMCParticle();
      if (!mcPart) return Const::doubleNaN;
      B2Vector3D pos = mcPart->getDecayVertex();
      return pos.Dot(orthBoostDir);
    }

    double vertexErrBoostDirection(const Particle* part)
    {
      TVectorD bDir = toVec(PCmsLabTransform().getBoostVector().Unit());
      // sqrt(bDir^T * Mat * bDir)
      return sqrt(part->getVertexErrorMatrix().Similarity(bDir));
    }

    double vertexErrOrthBoostDirection(const Particle* part)
    {
      TVectorD oDir = toVec(getUnitOrthogonal(PCmsLabTransform().getBoostVector()));
      // sqrt(oDir^T * Mat * oDir)
      return sqrt(part->getVertexErrorMatrix().Similarity(oDir));
    }

    // TagV boost direction

    double tagVBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVl();
    }

    double tagVOrthogonalBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVol();
    }

    double tagVTruthBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTruthTagVl();
    }

    double tagVTruthOrthogonalBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTruthTagVol();
    }

    double tagVErrBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVlErr();
    }

    double tagVErrOrthogonalBoostDirection(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getTagVolErr();
    }

    // cosTheta boost direction

    double particleCosThetaBoostDirection(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      B2Vector3D boost = PCmsLabTransform().getBoostVector();
      ROOT::Math::PxPyPzEVector pxpypze = frame.getMomentum(part);
      B2Vector3D momentum(pxpypze.Px(), pxpypze.Py(), pxpypze.Pz());
      return cos(momentum.Angle(boost));
    }

    double particleInternalTagVMCFlavor(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      return vert->getMCTagBFlavor();
    }

    double tagTrackMomentum(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackP(trackIndexInt).R();
    }

    double tagTrackMomentumX(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackP(trackIndexInt).X();
    }

    double tagTrackMomentumY(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackP(trackIndexInt).Y();
    }

    double tagTrackMomentumZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackP(trackIndexInt).Z();
    }

    double tagTrackZ0(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackZ0(trackIndexInt);
    }

    double tagTrackD0(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackD0(trackIndexInt);
    }

    double tagTrackRaveWeight(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getRaveWeight(trackIndexInt);
    }

    double tagTrackDistanceToConstraint(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      if (vert->getConstraintType() == "noConstraint") return Const::doubleNaN;
      const Particle* tagParticle(vert->getVtxFitParticle(trackIndexInt));
      if (!tagParticle) return Const::doubleNaN;

      return DistanceTools::trackToVtxDist(tagParticle->getTrackFitResult()->getPosition(),
                                           tagParticle->getMomentum(),
                                           vert->getConstraintCenter());
    }

    double tagTrackDistanceToConstraintErr(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      if (vert->getConstraintType() == "noConstraint") return Const::doubleNaN;
      const Particle* tagParticle(vert->getVtxFitParticle(trackIndexInt));
      if (!tagParticle) return Const::doubleNaN;

      //recover the covariance matrix associated to the position of the tag track
      TMatrixDSym trackPosCovMat = tagParticle->getVertexErrorMatrix();

      return DistanceTools::trackToVtxDistErr(tagParticle->getTrackFitResult()->getPosition(),
                                              tagParticle->getMomentum(),
                                              vert->getConstraintCenter(),
                                              trackPosCovMat,
                                              vert->getConstraintCov());
    }

    double tagTrackDistanceToConstraintSignificance(const Particle* part, const std::vector<double>& trackIndex)
    {
      double val = tagTrackDistanceToConstraint(part, trackIndex);
      if (isinf(val) || isnan(val)) return Const::doubleNaN;
      double err = tagTrackDistanceToConstraintErr(part, trackIndex);
      if (isinf(err) || isnan(err)) return Const::doubleNaN;

      return val / err;
    }

    double tagVDistanceToConstraint(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (vert->getConstraintType() == "noConstraint") return Const::doubleNaN;

      return DistanceTools::vtxToVtxDist(vert->getConstraintCenter(),
                                         vert->getTagVertex());
    }

    double tagVDistanceToConstraintErr(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (vert->getConstraintType() == "noConstraint") return Const::doubleNaN;

      //To compute the uncertainty, the tag vtx uncertainty is NOT taken into account
      //The error computed is the the one used  in the chi2.
      //To change that, emptyMat has to be replaced by m_TagVertexErrMatrix
      TMatrixDSym emptyMat(3);

      return DistanceTools::vtxToVtxDistErr(vert->getConstraintCenter(),
                                            vert->getTagVertex(),
                                            vert->getConstraintCov(),
                                            emptyMat);
    }

    double tagVDistanceToConstraintSignificance(const Particle* part)
    {
      double val = tagVDistanceToConstraint(part);
      if (isinf(val) || isnan(val)) return Const::doubleNaN;
      double err = tagVDistanceToConstraintErr(part);
      if (isinf(err) || isnan(err)) return Const::doubleNaN;

      return val / err;
    }

    double tagTrackDistanceToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      const Particle* particle = vert->getVtxFitParticle(trackIndexInt);
      if (!particle) return Const::doubleNaN;

      return DistanceTools::trackToVtxDist(particle->getTrackFitResult()->getPosition(),
                                           particle->getMomentum(),
                                           vert->getTagVertex());
    }

    double tagTrackDistanceToTagVErr(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      const Particle* tagParticle(vert->getVtxFitParticle(trackIndexInt));
      if (!tagParticle) return Const::doubleNaN;

      //recover the covariance matrix associated to the position of the tag track
      TMatrixDSym trackPosCovMat = tagParticle->getVertexErrorMatrix();

      //To compute the uncertainty, the tag vtx uncertainty is NOT taken into account
      //The error computed is then the one in the chi2.
      //To change that, emptyMat has to be replaced by m_TagVertexErrMatrix

      TMatrixDSym emptyMat(3);

      return DistanceTools::trackToVtxDistErr(tagParticle->getTrackFitResult()->getPosition(),
                                              tagParticle->getMomentum(),
                                              vert->getTagVertex(),
                                              trackPosCovMat,
                                              emptyMat);

    }

    double tagTrackDistanceToTagVSignificance(const Particle* part, const std::vector<double>& trackIndex)
    {
      double val = tagTrackDistanceToTagV(part, trackIndex);
      if (isinf(val) || isnan(val)) return Const::doubleNaN;
      double err = tagTrackDistanceToTagVErr(part, trackIndex);
      if (isinf(err) || isnan(err)) return Const::doubleNaN;

      return val / err;
    }

    double tagTrackTrueDistanceToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;

      if (trackIndex.size() != 1) return Const::doubleNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      const MCParticle* mcParticle(vert->getVtxFitMCParticle(trackIndexInt));
      if (!mcParticle) return Const::doubleNaN;

      B2Vector3D mcTagV = vert->getMCTagVertex();
      if (mcTagV(0)  == Const::doubleNaN)                                        return Const::doubleNaN;
      if (mcTagV(0)  == 0 && mcTagV(1) == 0 && mcTagV(2) == 0)          return Const::doubleNaN;

      return DistanceTools::trackToVtxDist(mcParticle->getProductionVertex(),
                                           mcParticle->getMomentum(),
                                           mcTagV);
    }

    B2Vector3D tagTrackTrueVecToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return vecNaN;

      if (trackIndex.size() != 1) return vecNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      const MCParticle* mcParticle(vert->getVtxFitMCParticle(trackIndexInt));
      if (!mcParticle) return vecNaN;

      B2Vector3D mcTagV = vert->getMCTagVertex();
      if (mcTagV(0)  == Const::doubleNaN)                                        return vecNaN;
      if (mcTagV(0)  == 0 && mcTagV(1) == 0 && mcTagV(2) == 0)          return vecNaN;

      return DistanceTools::trackToVtxVec(mcParticle->getProductionVertex(),
                                          mcParticle->getMomentum(),
                                          mcTagV);
    }

    double tagTrackTrueVecToTagVX(const Particle* part, const std::vector<double>& trackIndex)
    {
      B2Vector3D result = tagTrackTrueVecToTagV(part, trackIndex);
      return result(0);
    }

    double tagTrackTrueVecToTagVY(const Particle* part, const std::vector<double>& trackIndex)
    {
      B2Vector3D result = tagTrackTrueVecToTagV(part, trackIndex);
      return result(1);
    }

    double tagTrackTrueVecToTagVZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      B2Vector3D result = tagTrackTrueVecToTagV(part, trackIndex);
      return result(2);
    }

    B2Vector3D tagTrackTrueMomentum(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return vecNaN;

      if (trackIndex.size() != 1) return vecNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      const MCParticle* mcParticle(vert->getVtxFitMCParticle(trackIndexInt));
      if (!mcParticle) return vecNaN;

      return mcParticle->getMomentum();
    }

    double tagTrackTrueMomentumX(const Particle* part, const std::vector<double>& trackIndex)
    {
      B2Vector3D pTrue = tagTrackTrueMomentum(part, trackIndex);
      return pTrue(0);
    }

    double tagTrackTrueMomentumY(const Particle* part, const std::vector<double>& trackIndex)
    {
      B2Vector3D pTrue = tagTrackTrueMomentum(part, trackIndex);
      return pTrue(1);
    }

    double tagTrackTrueMomentumZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      B2Vector3D pTrue = tagTrackTrueMomentum(part, trackIndex);
      return pTrue(2);
    }

    B2Vector3D tagTrackTrueOrigin(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return vecNaN;

      if (trackIndex.size() != 1) return vecNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      const MCParticle* mcParticle(vert->getVtxFitMCParticle(trackIndexInt));
      if (!mcParticle) return vecNaN;

      return mcParticle->getProductionVertex();
    }

    double tagTrackTrueOriginX(const Particle* part, const std::vector<double>& trackIndex)
    {
      B2Vector3D origin = tagTrackTrueOrigin(part, trackIndex);
      return origin(0);
    }

    double tagTrackTrueOriginY(const Particle* part, const std::vector<double>& trackIndex)
    {
      B2Vector3D origin = tagTrackTrueOrigin(part, trackIndex);
      return origin(1);
    }

    double tagTrackTrueOriginZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      B2Vector3D origin = tagTrackTrueOrigin(part, trackIndex);
      return origin(2);
    }

    int fitTruthStatus(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return -1;
      return vert->getFitTruthStatus();
    }

    int rollbackStatus(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return -1;
      return vert->getRollBackStatus();
    }

    //**********************************
    //Meta variables
    //**********************************

    /**
     * returns a pointer to a function from its name. This is useful to combine the individual information
     * related to each tag track into average, min, max, etc.
     *
     */
    TagTrFPtr getTagTrackFunctionFromName(string const& name)
    {
      if (name == "TagTrackMomentum") return tagTrackMomentum;
      else if (name == "TagTrackMomentumX") return tagTrackMomentumX;
      else if (name == "TagTrackMomentumY") return tagTrackMomentumY;
      else if (name == "TagTrackMomentumZ") return tagTrackMomentumZ;
      else if (name == "TagTrackZ0") return tagTrackZ0;
      else if (name == "TagTrackD0") return tagTrackD0;
      else if (name == "TagTrackRaveWeight") return tagTrackRaveWeight;
      else if (name == "TagTrackDistanceToConstraint") return tagTrackDistanceToConstraint;
      else if (name == "TagTrackDistanceToConstraintErr") return tagTrackDistanceToConstraintErr;
      else if (name == "TagTrackDistanceToConstraintSignificance") return tagTrackDistanceToConstraintSignificance;
      else if (name == "TagTrackDistanceToTagV") return tagTrackDistanceToTagV;
      else if (name == "TagTrackDistanceToTagVErr") return tagTrackDistanceToTagVErr;
      else if (name == "TagTrackDistanceToTagVSignificance") return tagTrackDistanceToTagVSignificance;
      else if (name == "TagTrackTrueDistanceToTagV") return tagTrackTrueDistanceToTagV;
      else if (name == "TagTrackTrueVecToTagVX") return tagTrackTrueVecToTagVX;
      else if (name == "TagTrackTrueVecToTagVY") return tagTrackTrueVecToTagVY;
      else if (name == "TagTrackTrueVecToTagVZ") return tagTrackTrueVecToTagVZ;
      else if (name == "TagTrackTrueMomentumX") return tagTrackTrueMomentumX;
      else if (name == "TagTrackTrueMomentumY") return tagTrackTrueMomentumY;
      else if (name == "TagTrackTrueMomentumZ") return tagTrackTrueMomentumZ;
      else if (name == "TagTrackTrueOriginX") return tagTrackTrueOriginX;
      else if (name == "TagTrackTrueOriginY") return tagTrackTrueOriginY;
      else if (name == "TagTrackTrueOriginZ") return tagTrackTrueOriginZ;

      B2ERROR("From TimeDependentVariables: Trying to access unknown tagTrack function");
      return nullptr;
    }

    double cumulate(const Particle* part,  const std::vector<std::string>& variable, double start,
                    std::function<double(double, double, double)> fun)
    {
      TagTrFPtr fptr(getTagTrackFunctionFromName(variable.at(0)));
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return Const::doubleNaN;
      int nTracks = vert->getNFitTracks();

      //calculate cumulative quantity
      double acum = start;
      for (int i = 0; i < nTracks; ++i) {
        vector<double> indx(1, i);
        double w = tagTrackRaveWeight(part, indx);
        double f = (*fptr)(part, indx);
        if (w > 0) {
          acum = fun(acum, f, w);
        }
      }
      return acum;
    }

    Manager::FunctionPtr tagTrackAverage(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackAverage");
        return nullptr;
      }

      return [variable](const Particle * part) -> double {
        double sum = cumulate(part, variable, 0, [](double s, double f, double) {return s + f;});
        double tot = cumulate(part, variable, 0, [](double s, double, double) {return s + 1;});
        return (tot > 0) ? sum / tot : Const::doubleNaN;
      };
    }

    Manager::FunctionPtr tagTrackAverageSquares(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackAverageSquares");
        return nullptr;
      }

      return [variable](const Particle * part) -> double {
        double sum = cumulate(part, variable, 0, [](double s, double f, double) {return s + f * f;});
        double tot = cumulate(part, variable, 0, [](double s, double, double) {return s + 1;});
        return (tot > 0) ? sum / tot : Const::doubleNaN;
      };
    }


    Manager::FunctionPtr tagTrackMax(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackMax");
        return nullptr;
      }

      return [variable](const Particle * part) -> double {
        double Max = cumulate(part, variable, -DBL_MAX, [](double s, double f, double) {return std::max(s, f);});
        return (Max != -DBL_MAX) ? Max : Const::doubleNaN;
      };
    }

    Manager::FunctionPtr tagTrackMin(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackMin");
        return nullptr;
      }

      return [variable](const Particle * part) -> double {
        double Min = cumulate(part, variable, +DBL_MAX, [](double s, double f, double) {return std::min(s, f);});
        return (Min != DBL_MAX) ? Min : Const::doubleNaN;
      };
    }

    Manager::FunctionPtr tagTrackWeightedAverage(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackWeightedAverage");
        return nullptr;
      }

      return [variable](const Particle * part) -> double {
        double num = cumulate(part, variable, 0, [](double s, double f, double w) {return s + w * f;});
        double den = cumulate(part, variable, 0, [](double s, double, double w) {return s + w;});
        return (den > 0) ? num / den : Const::doubleNaN;
      };
    }

    Manager::FunctionPtr tagTrackWeightedAverageSquares(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackWeightedAverageSquares");
        return nullptr;
      }

      return [variable](const Particle * part) -> double {
        double num = cumulate(part, variable, 0, [](double s, double f, double w) {return s + w * f * f;});
        double den = cumulate(part, variable, 0, [](double s, double, double w) {return s + w;});
        return (den > 0) ? num / den : Const::doubleNaN;
      };
    }

    Manager::FunctionPtr tagTrackSum(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackSum");
        return nullptr;
      }

      return [variable](const Particle * part) -> double {
        return cumulate(part, variable, 0, [](double s, double f, double) {return s + f;});
      };
    }

    //**********************************
    //VARIABLE REGISTRATION
    //**********************************

    VARIABLE_GROUP("Time Dependent CPV Analysis Variables");

    REGISTER_VARIABLE("TagVx", particleTagVx, "Tag vertex X component\n\n", "cm");
    REGISTER_VARIABLE("TagVy", particleTagVy, "Tag vertex Y component\n\n", "cm");
    REGISTER_VARIABLE("TagVz", particleTagVz, "Tag vertex Z component\n\n", "cm");
    REGISTER_VARIABLE("mcTagVx", particleTruthTagVx, "Generated Tag vertex X component\n\n", "cm");
    REGISTER_VARIABLE("mcTagVy", particleTruthTagVy, "Generated Tag vertex Y component\n\n", "cm");
    REGISTER_VARIABLE("mcTagVz", particleTruthTagVz, "Generated Tag vertex Z component\n\n", "cm");
    REGISTER_VARIABLE("TagVxErr", particleTagVxErr, "Tag vertex X component uncertainty\n\n", "cm");
    REGISTER_VARIABLE("TagVyErr", particleTagVyErr, "Tag vertex Y component uncertainty\n\n", "cm");
    REGISTER_VARIABLE("TagVzErr", particleTagVzErr, "Tag vertex Z component uncertainty\n\n", "cm");
    REGISTER_VARIABLE("TagVpVal", particleTagVpVal, "Tag vertex p-Value");
    REGISTER_VARIABLE("TagVNTracks", particleTagVNTracks, "Number of tracks in the tag vertex");
    REGISTER_VARIABLE("TagVType", particleTagVType,
                      R"DOC(Type of algorithm for the tag vertex. -1: failed (1,2: single track, deprecated), 3: standard, 4: standard_PXD, 5: no constraint)DOC");
    REGISTER_VARIABLE("TagVNDF", particleTagVNDF, "Number of degrees of freedom in the tag vertex fit");
    REGISTER_VARIABLE("TagVChi2", particleTagVChi2, "chi2 value of the tag vertex fit");
    REGISTER_VARIABLE("TagVChi2IP", particleTagVChi2IP, "IP component of chi2 value of the tag vertex fit");

    REGISTER_VARIABLE("DeltaT", particleDeltaT, R"DOC(
                      Proper decay time difference :math:`\Delta t` between signal B-meson :math:`(B_{rec})` and tag B-meson :math:`(B_{tag})`.

                      )DOC", "ps");
    REGISTER_VARIABLE("DeltaTErr", particleDeltaTErr, R"DOC(
                      Proper decay time difference :math:`\Delta t` uncertainty

                      )DOC", "ps");
    REGISTER_VARIABLE("DeltaTRes", particleDeltaTRes, R"DOC(
                      :math:`\Delta t` residual, to be used for resolution function studies

                      )DOC", "ps");
    REGISTER_VARIABLE("DeltaTBelle", particleDeltaTBelle, R"DOC(
                      [Legacy] :math:`\Delta t`, as it was used in Belle

                      )DOC", "ps");
    REGISTER_VARIABLE("mcDeltaTau", particleMCDeltaTau,
                      R"DOC(Generated proper decay time difference :math:`\Delta t`: :math:`\tau(B_{\rm rec})-\tau(B_{\rm tag})`

)DOC", "ps");
    REGISTER_VARIABLE("mcDeltaT", particleMCDeltaT,
                      R"DOC(Generated proper decay time difference (in z-difference approximation) :math:`\Delta t`:
                      :math:`(l(B_{\rm rec}) - l(B_{\rm tag}))/\beta_{\Upsilon(4S)}\gamma_{\Upsilon(4S)}`

)DOC","ps");
    REGISTER_VARIABLE("mcDeltaBoost", particleMCDeltaBoost,
                      R"DOC(True difference of decay vertex boost-direction components between signal B-meson :math:`(B_{rec})` and tag B-meson :math:`(B_{tag})`:
:math:`\Delta l = l(B_{rec}) - l(B_{tag})`

)DOC", "cm");
    REGISTER_VARIABLE("DeltaZ", particleDeltaZ,
                      R"DOC(Difference of decay vertex longitudinal components between signal B-meson :math:`(B_{rec})` and tag B-meson :math:`(B_{tag})`:
:math:`\Delta z = z(B_{rec}) - z(B_{tag})`

)DOC", "cm");
    REGISTER_VARIABLE("DeltaZErr", particleDeltaZErr,
                      R"DOC(Uncertainty of the difference :math:`z(B_{rec}) - z(B_{tag})`

)DOC", "cm");
    REGISTER_VARIABLE("DeltaBoost", particleDeltaB, R"DOC(:math:`\Delta z` in the boost direction

)DOC", "cm");
    REGISTER_VARIABLE("DeltaBoostErr", particleDeltaBErr, R"DOC(Uncertainty of :math:`\Delta z` in the boost direction

)DOC", "cm");

    REGISTER_VARIABLE("LBoost", vertexBoostDirection,
                      "Returns the vertex component in the boost direction\n\n", "cm");
    REGISTER_VARIABLE("OBoost", vertexOrthogonalBoostDirection,
                      "Returns the vertex component in the direction orthogonal to the boost\n\n", "cm");
    REGISTER_VARIABLE("mcLBoost", vertexTruthBoostDirection,
                      "Returns the MC vertex component in the boost direction\n\n", "cm");
    REGISTER_VARIABLE("mcOBoost", vertexTruthOrthogonalBoostDirection,
                      "Returns the MC vertex component in the direction orthogonal to the boost\n\n", "cm");
    REGISTER_VARIABLE("LBoostErr", vertexErrBoostDirection,
                      "Returns the error of the vertex in the boost direction\n\n", "cm");
    REGISTER_VARIABLE("OBoostErr", vertexErrOrthBoostDirection,
                      "Returns the error of the vertex in the direction orthogonal to the boost\n\n", "cm");

    REGISTER_VARIABLE("TagVLBoost", tagVBoostDirection,
                      "Returns the TagV component in the boost direction\n\n", "cm");
    REGISTER_VARIABLE("TagVOBoost", tagVOrthogonalBoostDirection,
                      "Returns the TagV component in the direction orthogonal to the boost\n\n", "cm");
    REGISTER_VARIABLE("mcTagVLBoost", tagVTruthBoostDirection,
                      "Returns the MC TagV component in the boost direction\n\n", "cm");
    REGISTER_VARIABLE("mcTagVOBoost", tagVTruthOrthogonalBoostDirection,
                      "Returns the MC TagV component in the direction orthogonal to the boost\n\n", "cm");
    REGISTER_VARIABLE("TagVLBoostErr", tagVErrBoostDirection,
                      "Returns the error of TagV in the boost direction\n\n", "cm");
    REGISTER_VARIABLE("TagVOBoostErr", tagVErrOrthogonalBoostDirection,
                      "Returns the error of TagV in the direction orthogonal to the boost\n\n", "cm");

    REGISTER_VARIABLE("cosAngleBetweenMomentumAndBoostVector", particleCosThetaBoostDirection,
                      "cosine of the angle between momentum and boost vector");

    REGISTER_VARIABLE("internalTagVMCFlavor", particleInternalTagVMCFlavor,
                      "[Expert] [Debugging] This variable is only for internal checks of the TagV module by developers. \n"
                      "It returns the internal mc flavor information of the tag-side B provided by the TagV module.");

    REGISTER_VARIABLE("TagTrackMomentum(i) ", tagTrackMomentum,
                      "Returns the magnitude of the momentum of the ith track used in the tag vtx fit.\n\n", "GeV/c");
    REGISTER_VARIABLE("TagTrackMomentumX(i) ", tagTrackMomentumX,
                      "Returns the X component of the momentum of the ith track used in the tag vtx fit.\n\n", "GeV/c");
    REGISTER_VARIABLE("TagTrackMomentumY(i) ", tagTrackMomentumY,
                      "Returns the Y component of the momentum of the ith track used in the tag vtx fit.\n\n", "GeV/c");
    REGISTER_VARIABLE("TagTrackMomentumZ(i) ", tagTrackMomentumZ,
                      "Returns the Z component of the momentum of the ith track used in the tag vtx fit.\n\n", "GeV/c");

    REGISTER_VARIABLE("TagTrackZ0(i)", tagTrackZ0, "Returns the z0 parameter of the ith track used in the tag vtx fit\n\n", "cm");
    REGISTER_VARIABLE("TagTrackD0(i)", tagTrackD0, "Returns the d0 parameter of the ith track used in the tag vtx fit\n\n", "cm");


    REGISTER_VARIABLE("TagTrackRaveWeight(i)", tagTrackRaveWeight, "Returns the weight assigned by Rave to track i");

    REGISTER_VARIABLE("TagVNFitTracks", particleTagVNFitTracks,
                      "returns the number of tracks used by rave to fit the vertex (not counting the ones coming from Kshorts)");

    REGISTER_VARIABLE("TagTrackDistanceToConstraint(i)", tagTrackDistanceToConstraint,
                      "Returns the measured distance between the ith tag track and the centre of the constraint.\n\n", "cm");

    REGISTER_VARIABLE("TagTrackDistanceToConstraintErr(i)", tagTrackDistanceToConstraintErr,
                      "Returns the estimated error on the distance between the ith tag track and the centre of the constraint.\n\n", "cm");

    REGISTER_VARIABLE("TagTrackDistanceToConstraintSignificance(i)", tagTrackDistanceToConstraintSignificance,
                      "Returns the significance of the distance between the centre of the constraint and the tag track indexed by track index (computed as distance / uncertainty)");


    REGISTER_VARIABLE("TagVDistanceToConstraint", tagVDistanceToConstraint,
                      "returns the measured distance between the tag vtx and the centre of the constraint.\n\n", "cm");

    REGISTER_VARIABLE("TagVDistanceToConstraintErr", tagVDistanceToConstraintErr,
                      "returns the estimated error on the distance between the tag vtx and the centre of the constraint.\n\n", "cm");

    REGISTER_VARIABLE("TagVDistanceToConstraintSignificance", tagVDistanceToConstraintSignificance,
                      "returns the significance of the distance between the tag vtx and the centre of the constraint (computed as distance / uncertainty)");

    REGISTER_VARIABLE("TagTrackDistanceToTagV(i)", tagTrackDistanceToTagV,
                      "Returns the measured distance between the ith tag track and the tag vtx.\n\n", "cm");

    REGISTER_VARIABLE("TagTrackDistanceToTagVErr(i)", tagTrackDistanceToTagVErr, R"DOC(
Returns the estimated error on the distance between the ith tag track and the tag vtx.

.. warning:: Only the uncertainties on the track position parameters are taken into account.

)DOC", "cm");

    REGISTER_VARIABLE("TagTrackDistanceToTagVSignificance(i)", tagTrackDistanceToTagVSignificance,
                      "Returns the significance of the distance between the tag vtx and the tag track indexed by trackIndex (computed as distance / uncertainty)");

    REGISTER_VARIABLE("TagTrackTrueDistanceToTagV(i)", tagTrackTrueDistanceToTagV,
                      "Returns the true distance between the true B Tag decay vertex and the particle corresponding to the ith tag vtx track.\n\n", "cm");

    REGISTER_VARIABLE("TagTrackTrueVecToTagVX(i)", tagTrackTrueVecToTagVX,
                      "Returns the X coordinate of the vector between the mc particle corresponding to the ith tag vtx track and the true tag B decay vertex.\n\n",
                      "cm");

    REGISTER_VARIABLE("TagTrackTrueVecToTagVY(i)", tagTrackTrueVecToTagVY,
                      "Returns the Y coordinate of the vector between the mc particle corresponding to the ith tag vtx track and the true tag B decay vertex.\n\n",
                      "cm");

    REGISTER_VARIABLE("TagTrackTrueVecToTagVZ(i)", tagTrackTrueVecToTagVZ,
                      "Returns the Z coordinate of the vector between the mc particle corresponding to the ith tag vtx track and the true tag B decay vertex.\n\n",
                      "cm");

    REGISTER_VARIABLE("TagTrackTrueMomentumX(i)", tagTrackTrueMomentumX,
                      "Returns the X component of the true momentum of the MC particle corresponding to the ith tag vtx track.\n\n", "GeV/c");

    REGISTER_VARIABLE("TagTrackTrueMomentumY(i)", tagTrackTrueMomentumY,
                      "Returns the Y component of the true momentum of the MC particle corresponding to the ith tag vtx track.\n\n", "GeV/c");

    REGISTER_VARIABLE("TagTrackTrueMomentumZ(i)", tagTrackTrueMomentumZ,
                      "Returns the Z component of the true momentum of the MC particle corresponding to the ith tag vtx track.\n\n", "GeV/c");

    REGISTER_VARIABLE("TagTrackTrueOriginX(i)", tagTrackTrueOriginX,
                      "Returns the X component of the true origin of the MC particle corresponding to the ith tag vtx track.\n\n", "cm");

    REGISTER_VARIABLE("TagTrackTrueOriginY(i)", tagTrackTrueOriginY,
                      "Returns the Y component of the true origin of the MC particle corresponding to the ith tag vtx track.\n\n", "cm");

    REGISTER_VARIABLE("TagTrackTrueOriginZ(i)", tagTrackTrueOriginZ,
                      "Returns the Z component of the true origin of the MC particle corresponding to the ith tag vtx track.\n\n", "cm");

    REGISTER_VARIABLE("TagVFitTruthStatus", fitTruthStatus, R"DOC(
Returns the status of the fit performed with the truth info. Possible values are:

- -1: no related TagVertex found
- 0: fit performed with measured parameters
- 1: fit performed with true parameters
- 2: unable to recover truth parameters
)DOC");

    REGISTER_VARIABLE("TagVRollBackStatus", rollbackStatus, R"DOC(
Returns the status of the fit performed with rolled back tracks. Possible values are:

- -1: no related TagVertex found
- 0: fit performed with measured parameters
- 1: fit performed with true parameters
- 2: unable to recover truth parameters
)DOC");

    REGISTER_METAVARIABLE("TagTrackMax(var)", tagTrackMax,
                          "return the maximum value of the variable ``var`` evaluated for each tag track. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackMax(TagTrackDistanceToConstraint)``. The tracks that are assigned a zero weight are ignored.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("TagTrackMin(var)", tagTrackMin,
                          "return the minimum value of the variable ``var`` evaluated for each tag track. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackMin(TagTrackDistanceToConstraint)``. The tracks that are assigned a zero weight are ignored.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("TagTrackAverage(var)", tagTrackAverage,
                          "return the average over the tag tracks of the variable ``var``. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackAverage(TagTrackDistanceToConstraint)``. The tracks that are assigned a zero weight are ignored.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("TagTrackAverageSquares(var)", tagTrackAverageSquares,
                          "return the average over the tag tracks of the square of the variable ``var``. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackAverageSquares(TagTrackDistanceToConstraint)``. The tracks that are assigned a zero weight are ignored.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("TagTrackWeightedAverage(var)", tagTrackWeightedAverage,
                          "return the average over the tag tracks of the variable ``var``, weighted by weights of the tag vertex fitter. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackWeightedAverage(TagTrackDistanceToConstraint)``.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("TagTrackWeightedAverageSquares(var)", tagTrackWeightedAverageSquares,
                          "return the average over the tag tracks of the variable ``var``, weighted by weights of the tag vertex fitter. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackWeightedAverageSquares(TagTrackDistanceToConstraint)``.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("TagTrackSum(var)", tagTrackSum,
                          "Returns the sum of the provided variable for all tag tracks. The variable must be one of the TagTrackXXX variables, "
                          "for example ``TagTrackSum(TagTrackD0)``. The tracks that are assigned a zero weight are ignored.",
                          Manager::VariableDataType::c_double);
  }
}




