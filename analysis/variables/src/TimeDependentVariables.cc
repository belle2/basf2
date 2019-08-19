/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi, Fernando Abudinen                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/variables/TimeDependentVariables.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <framework/dbobjects/BeamParameters.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/TagVertex.h>

#include <mdst/dataobjects/MCParticle.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TLorentzVector.h>
#include <TRandom.h>
#include <TVectorF.h>
#include <TVector3.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    //   ############################################## Time Dependent CPV Analysis Variables  ###############################################


    // TagV x, y, z

    double particleTagVx(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVertex().X();

      return result;
    }

    double particleTagVy(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVertex().Y();

      return result;
    }

    double particleTagVz(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVertex().Z();

      return result;
    }

    double particleTruthTagVx(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getMCTagVertex().X();

      return result;
    }

    double particleTruthTagVy(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getMCTagVertex().Y();

      return result;
    }

    double particleTruthTagVz(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getMCTagVertex().Z();

      return result;
    }

    double particleTagVxErr(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert) {
        TMatrixFSym TagVErr = vert->getTagVertexErrMatrix();
        result = sqrt(TagVErr(0, 0));
      }

      return result;
    }

    double particleTagVyErr(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert) {
        TMatrixFSym TagVErr = vert->getTagVertexErrMatrix();
        result = sqrt(TagVErr(1, 1));
      }

      return result;
    }

    double particleTagVzErr(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert) {
        TMatrixFSym TagVErr = vert->getTagVertexErrMatrix();
        result = sqrt(TagVErr(2, 2));
      }

      return result;
    }

    double particleTagVpVal(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVertexPval();

      return result;
    }

    double particleTagVNTracks(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return -1111.0;
      return vert->getNTracks();
    }

    double particleTagVType(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return -1111.0;
      return vert->getFitType();
    }

    double particleTagVNDF(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return -1111.0;
      return vert->getTagVNDF();
    }

    double particleTagVChi2(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return -1111.0;
      return vert->getTagVChi2();
    }

    double particleTagVChi2IP(const Particle* particle)
    {
      auto* vert = particle->getRelatedTo<TagVertex>();
      if (!vert)
        return -1111.0;
      return vert->getTagVChi2IP();
    }


    // Delta t and related

    double particleDeltaT(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getDeltaT();

      return result;
    }

    double particleDeltaTErr(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getDeltaTErr();

      return result;
    }

    double particleMCDeltaT(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getMCDeltaT();

      return result;
    }

    double particleDeltaZ(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert)
        result = particle->getZ() - vert->getTagVertex().Z();

      return result;
    }

    double particleDeltaZErr(const Particle* particle)
    {
      double result = -1111.0;

      auto* vert = particle->getRelatedTo<TagVertex>();

      if (vert) {
        double zVariance = particle->getVertexErrorMatrix()(2, 2);
        double TagVZVariance = vert->getTagVertexErrMatrix()(2, 2);
        result = sqrt(zVariance + TagVZVariance);
        if (std::isnan(result) or std::isinf(result)) result = -1111.0;
      }
      return result;
    }

    double particleDeltaB(const Particle* particle)
    {
      double result = -1111.0;

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
      double result = -1111.0;

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
      if (mcPart == nullptr) return -1111;
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
      if (mcPart == nullptr) return -1111;
      TVector3 pos = mcPart->getDecayVertex();
      double l = pos.Dot(orthBoostDir);

      return l;
    }


    double vertexErrBoostDirection(const Particle* part)
    {
      PCmsLabTransform T;

      TVector3 boost = T.getBoostVector();
      TVector3 boostDir = boost.Unit();

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
      double result = -1111.0;

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVl();

      return result;
    }


    double tagVOrthogonalBoostDirection(const Particle* part)
    {
      double result = -1111.0;

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVol();

      return result;
    }


    double tagVTruthBoostDirection(const Particle* part)
    {
      double result = -1111.0;

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTruthTagVl();

      return result;
    }


    double tagVTruthOrthogonalBoostDirection(const Particle* part)
    {
      double result = -1111.0;

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTruthTagVol();

      return result;
    }

    double tagVErrBoostDirection(const Particle* part)
    {
      double result = -1111.0;

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVlErr();

      return result;
    }


    double tagVErrOrthogonalBoostDirection(const Particle* part)
    {
      double result = -1111.0;

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getTagVolErr();

      return result;
    }


    double particleInternalTagVMCFlavor(const Particle* part)
    {
      double result = 1000.0;

      auto* vert = part->getRelatedTo<TagVertex>();

      if (vert)
        result = vert->getMCTagBFlavor();

      return result;
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
  }
}

