/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
#include <analysis/utility/RotationTools.h>

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

    static const double realNaN = std::numeric_limits<double>::quiet_NaN();
    static const B2Vector3D vecNaN(realNaN, realNaN, realNaN);

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
      if (!vert) return realNaN;
      return vert->getMCDeltaTau();
    }

    double particleMCDeltaT(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;
      return vert->getMCDeltaT();
    }

    double particleMCDeltaBoost(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      double beta = PCmsLabTransform().getBoostVector().Mag();
      double bg = beta / sqrt(1 - beta * beta);
      double c = Const::speedOfLight / 1000.; // cm ps-1
      return vert->getMCDeltaT() * bg * c;
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

      double zVariance     = particle->getVertexErrorMatrix()(2, 2);
      double TagVZVariance = vert->getTagVertexErrMatrix()(2, 2);
      double result = sqrt(zVariance + TagVZVariance);
      if (!std::isfinite(result)) return realNaN;

      return result;
    }

    double particleDeltaB(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      double beta = PCmsLabTransform().getBoostVector().Mag();
      double bg = beta / sqrt(1 - beta * beta);
      double c = Const::speedOfLight / 1000.; // cm ps-1
      return vert->getDeltaT() * bg * c;
    }

    double particleDeltaBErr(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

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
      B2Vector3D boostDir = (beamParamsDB->getHER() + beamParamsDB->getLER()).BoostToCM().Unit();

      const MCParticle* mcPart = part->getMCParticle();
      if (!mcPart) return realNaN;
      B2Vector3D pos = mcPart->getDecayVertex();
      return pos.Dot(boostDir);
    }

    double vertexTruthOrthogonalBoostDirection(const Particle* part)
    {
      static DBObjPtr<BeamParameters> beamParamsDB;
      B2Vector3D boost = (beamParamsDB->getHER() + beamParamsDB->getLER()).BoostToCM();
      B2Vector3D orthBoostDir = getUnitOrthogonal(boost);

      const MCParticle* mcPart = part->getMCParticle();
      if (!mcPart) return realNaN;
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

    double tagTrackMomentum(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackP(trackIndexInt).R();
    }

    double tagTrackMomentumX(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackP(trackIndexInt).x();
    }

    double tagTrackMomentumY(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackP(trackIndexInt).y();
    }

    double tagTrackMomentumZ(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackP(trackIndexInt).z();
    }

    double tagTrackZ0(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackZ0(trackIndexInt);
    }

    double tagTrackD0(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getVtxFitTrackD0(trackIndexInt);
    }

    double tagTrackRaveWeight(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      return vert->getRaveWeight(trackIndexInt);
    }

    double tagTrackDistanceToConstraint(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      if (vert->getConstraintType() == "noConstraint") return realNaN;
      const Particle* tagParticle(vert->getVtxFitParticle(trackIndexInt));
      if (!tagParticle) return realNaN;

      return DistanceTools::trackToVtxDist(tagParticle->getTrackFitResult()->getPosition(),
                                           tagParticle->getMomentum(),
                                           vert->getConstraintCenter());
    }

    double tagTrackDistanceToConstraintErr(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      if (vert->getConstraintType() == "noConstraint") return realNaN;
      const Particle* tagParticle(vert->getVtxFitParticle(trackIndexInt));
      if (!tagParticle) return realNaN;

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
      if (isinf(val) || isnan(val)) return realNaN;
      double err = tagTrackDistanceToConstraintErr(part, trackIndex);
      if (isinf(err) || isnan(err)) return realNaN;

      return val / err;
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

    double tagVDistanceToConstraintSignificance(const Particle* part)
    {
      double val = tagVDistanceToConstraint(part);
      if (isinf(val) || isnan(val)) return realNaN;
      double err = tagVDistanceToConstraintErr(part);
      if (isinf(err) || isnan(err)) return realNaN;

      return val / err;
    }

    double tagTrackDistanceToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      const Particle* particle = vert->getVtxFitParticle(trackIndexInt);
      if (!particle) return realNaN;

      return DistanceTools::trackToVtxDist(particle->getTrackFitResult()->getPosition(),
                                           particle->getMomentum(),
                                           vert->getTagVertex());
    }

    double tagTrackDistanceToTagVErr(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      const Particle* tagParticle(vert->getVtxFitParticle(trackIndexInt));
      if (!tagParticle) return realNaN;

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
      if (isinf(val) || isnan(val)) return realNaN;
      double err = tagTrackDistanceToTagVErr(part, trackIndex);
      if (isinf(err) || isnan(err)) return realNaN;

      return val / err;
    }

    double tagTrackTrueDistanceToTagV(const Particle* part, const std::vector<double>& trackIndex)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return realNaN;

      if (trackIndex.size() != 1) return realNaN;
      unsigned trackIndexInt = trackIndex.at(0);

      const MCParticle* mcParticle(vert->getVtxFitMCParticle(trackIndexInt));
      if (!mcParticle) return realNaN;

      B2Vector3D mcTagV = vert->getMCTagVertex();
      if (mcTagV(0)  == realNaN)                                        return realNaN;
      if (mcTagV(0)  == 0 && mcTagV(1) == 0 && mcTagV(2) == 0)          return realNaN;

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
      if (mcTagV(0)  == realNaN)                                        return vecNaN;
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
      if (!vert) return std::numeric_limits<int>::quiet_NaN();
      return vert->getFitTruthStatus();
    }

    int rollbackStatus(const Particle* part)
    {
      auto* vert = part->getRelatedTo<TagVertex>();
      if (!vert) return std::numeric_limits<int>::quiet_NaN();
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
      if (!vert) return realNaN;
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

      return [variable](const Particle * part) {
        double sum = cumulate(part, variable, 0, [](double s, double f, double) {return s + f;});
        double tot = cumulate(part, variable, 0, [](double s, double  , double) {return s + 1;});
        return (tot > 0) ? sum / tot : realNaN;
      };
    }

    Manager::FunctionPtr tagTrackAverageSquares(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackAverageSquares");
        return nullptr;
      }

      return [variable](const Particle * part) {
        double sum = cumulate(part, variable, 0, [](double s, double f, double) {return s + f * f;});
        double tot = cumulate(part, variable, 0, [](double s, double  , double) {return s + 1;});
        return (tot > 0) ? sum / tot : realNaN;
      };
    }


    Manager::FunctionPtr tagTrackMax(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackMax");
        return nullptr;
      }

      return [variable](const Particle * part) {
        double Max = cumulate(part, variable, -DBL_MAX, [](double s, double f, double) {return std::max(s, f);});
        return (Max != -DBL_MAX) ? Max : realNaN;
      };
    }

    Manager::FunctionPtr tagTrackMin(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackMin");
        return nullptr;
      }

      return [variable](const Particle * part) {
        double Min = cumulate(part, variable, +DBL_MAX, [](double s, double f, double) {return std::min(s, f);});
        return (Min != DBL_MAX) ? Min : realNaN;
      };
    }

    Manager::FunctionPtr tagTrackWeightedAverage(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackWeightedAverage");
        return nullptr;
      }

      return [variable](const Particle * part) {
        double num = cumulate(part, variable, 0, [](double s, double f, double w) {return s + w * f;});
        double den = cumulate(part, variable, 0, [](double s, double  , double w) {return s + w;});
        return (den > 0) ? num / den : realNaN;
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
        double den = cumulate(part, variable, 0, [](double s, double  , double w) {return s + w;});
        return (den > 0) ? num / den : realNaN;
      };
    }

    Manager::FunctionPtr tagTrackSum(const std::vector<std::string>& variable)
    {
      if (variable.size() != 1) {
        B2FATAL("Wrong number of arguments for meta function tagTrackSum");
        return nullptr;
      }

      return [variable](const Particle * part) {
        return cumulate(part, variable, 0, [](double s, double f, double) {return s + f;});
      };
    }


    //**********************************
    //VARIABLE REGISTRATION
    //**********************************

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
    REGISTER_VARIABLE("DeltaTRes", particleDeltaTRes,
                      R"DOC(:math:`\Delta t` residual in ps, to be used for resolution function studies)DOC");
    REGISTER_VARIABLE("DeltaTBelle", particleDeltaTBelle,
                      R"DOC([Legacy] :math:`\Delta t` in ps, as it was used in Belle)DOC");
    REGISTER_VARIABLE("mcDeltaTau", particleMCDeltaTau,
                      R"DOC(Generated proper decay time difference :math:`\Delta t` in ps: :math:`\tau(B_{\rm rec})-\tau(B_{\rm tag})`)DOC");
    REGISTER_VARIABLE("mcDeltaT", particleMCDeltaT,
                      R"DOC(Generated proper decay time difference (in z-difference approximation) :math:`\Delta t` in ps: :math:`(l(B_{\rm rec}) - l(B_{\rm tag}))/\beta_{\Upsilon(4S)}\gamma_{\Upsilon(4S)}`)DOC");
    REGISTER_VARIABLE("mcDeltaBoost", particleMCDeltaBoost,
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
    REGISTER_VARIABLE("mcTagVLBoost", tagVTruthBoostDirection,
                      "Returns the MC TagV component in the boost direction");
    REGISTER_VARIABLE("mcTagVOBoost", tagVTruthOrthogonalBoostDirection,
                      "Returns the MC TagV component in the direction orthogonal to the boost");
    REGISTER_VARIABLE("TagVLBoostErr", tagVErrBoostDirection,
                      "Returns the error of TagV in the boost direction");
    REGISTER_VARIABLE("TagVOBoostErr", tagVErrOrthogonalBoostDirection,
                      "Returns the error of TagV in the direction orthogonal to the boost");

    REGISTER_VARIABLE("internalTagVMCFlavor", particleInternalTagVMCFlavor,
                      "[Expert] [Debugging] This variable is only for internal checks of the TagV module by developers. \n"
                      "It returns the internal mc flavor information of the tag-side B provided by the TagV module.");

    REGISTER_VARIABLE("TagTrackMomentum(i) ", tagTrackMomentum,
                      "return the magnitude of the momentum of the ith track used in the tag vtx fit.");
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

    REGISTER_VARIABLE("TagTrackDistanceToConstraintSignificance(i)", tagTrackDistanceToConstraintSignificance,
                      "returns the significance of the distance between the centre of the constraint and the tag track indexed by track index (computed as distance / uncertainty)");


    REGISTER_VARIABLE("TagVDistanceToConstraint", tagVDistanceToConstraint,
                      "returns the measured distance between the tag vtx and the centre of the constraint.");

    REGISTER_VARIABLE("TagVDistanceToConstraintErr", tagVDistanceToConstraintErr,
                      "returns the estimated error on the distance between the tag vtx and the centre of the constraint.");

    REGISTER_VARIABLE("TagVDistanceToConstraintSignificance", tagVDistanceToConstraintSignificance,
                      "returns the significance of the distance between the tag vtx and the centre of the constraint (computed as distance / uncertainty)");

    REGISTER_VARIABLE("TagTrackDistanceToTagV(i)", tagTrackDistanceToTagV,
                      "returns the measured distance between the ith tag track and the tag vtx.");

    REGISTER_VARIABLE("TagTrackDistanceToTagVErr(i)", tagTrackDistanceToTagVErr,
                      "returns the estimated error on the distance between the ith tag track and the tag vtx. Warning: only the uncertainties on the track position parameters are taken into account.");

    REGISTER_VARIABLE("TagTrackDistanceToTagVSignificance(i)", tagTrackDistanceToTagVSignificance,
                      "returns the significance of the distance between the tag vtx and the tag track indexed by trackIndex (computed as distance / uncertainty)");

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

    REGISTER_VARIABLE("TagVRollBackStatus", rollbackStatus,
                      "Returns the status of the fit performed with rolled back tracks. Possible values are: 0: fit performed with measured parameters, 1: fit performed with rolled back tracks, 2: unable to recover truth parameters")

    REGISTER_VARIABLE("TagTrackMax(var)", tagTrackMax,
                      "return the maximum value of the variable ``var`` evaluated for each tag track. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackMax(TagTrackDistanceToConstraint)``. The tracks that are assigned a zero weight are ignored.")

    REGISTER_VARIABLE("TagTrackMin(var)", tagTrackMin,
                      "return the minimum value of the variable ``var`` evaluated for each tag track. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackMin(TagTrackDistanceToConstraint)``. The tracks that are assigned a zero weight are ignored.")

    REGISTER_VARIABLE("TagTrackAverage(var)", tagTrackAverage,
                      "return the average over the tag tracks of the variable ``var``. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackAverage(TagTrackDistanceToConstraint)``. The tracks that are assigned a zero weight are ignored.")

    REGISTER_VARIABLE("TagTrackAverageSquares(var)", tagTrackAverageSquares,
                      "return the average over the tag tracks of the square of the variable ``var``. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackAverageSquares(TagTrackDistanceToConstraint)``. The tracks that are assigned a zero weight are ignored.")

    REGISTER_VARIABLE("TagTrackWeightedAverage(var)", tagTrackWeightedAverage,
                      "return the average over the tag tracks of the variable ``var``, weighted by weights of the tag vertex fitter. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackWeightedAverage(TagTrackDistanceToConstraint)``.")

    REGISTER_VARIABLE("TagTrackWeightedAverageSquares(var)", tagTrackWeightedAverageSquares,
                      "return the average over the tag tracks of the variable ``var``, weighted by weights of the tag vertex fitter. ``var`` must be one of the TagTrackXXX variables, for example: ``TagTrackWeightedAverageSquares(TagTrackDistanceToConstraint)``.")
  }
}




