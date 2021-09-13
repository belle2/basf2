/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/variables/BelleVariables.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/variables/Variables.h>
#include <analysis/variables/VertexVariables.h>
#include <analysis/variables/ECLVariables.h>
#include <analysis/variables/TrackVariables.h>
#include <analysis/variables/V0DaughterTrackVariables.h>
#include <mdst/dataobjects/Track.h>
#include <analysis/variables/VertexVariables.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>

#include <framework/datastore/StoreArray.h>
#include <b2bii/dataobjects/BelleTrkExtra.h>

#include <TVectorF.h>

#include <limits>

namespace Belle2 {
  namespace Variable {
    double goodBelleKshort(const Particle* KS)
    {
      // check input
      if (KS->getNDaughters() != 2) {
        B2WARNING("goodBelleKshort is only defined for a particle with two daughters");
        return false;
      }
      const Particle* d0 = KS->getDaughter(0);
      const Particle* d1 = KS->getDaughter(1);
      if ((d0->getCharge() == 0) || (d1->getCharge() == 0)) {
        B2WARNING("goodBelleKshort is only defined for a particle with charged daughters");
        return false;
      }
      if (abs(KS->getPDGCode()) != Const::Kshort.getPDGCode())
        B2WARNING("goodBelleKshort is being applied to a candidate with PDG " << KS->getPDGCode());

      // If goodKs exists, return the value
      if (KS->hasExtraInfo("goodKs")) {
        return KS->getExtraInfo("goodKs");
      }

      // Belle selection
      double p = particleP(KS);
      double fl = particleDRho(KS);
      double dphi = acos(((particleDX(KS) * particlePx(KS)) + (particleDY(KS) * particlePy(KS))) / (fl * sqrt(particlePx(KS) * particlePx(
                           KS) + particlePy(KS) * particlePy(KS))));
      // particleDRho returns track d0 relative to IP for tracks
      double dr = std::min(abs(particleDRho(d0)), abs(particleDRho(d1)));
      double zdist = v0DaughterZ0Diff(KS);

      bool low = p < 0.5 && abs(zdist) < 0.8 && dr > 0.05 && dphi < 0.3;
      bool mid = p < 1.5 && p > 0.5 && abs(zdist) < 1.8 && dr > 0.03 && dphi < 0.1 && fl > .08;
      bool high = p > 1.5 && abs(zdist) < 2.4 && dr > 0.02 && dphi < 0.03 && fl > .22;

      if (low || mid || high) {
        return 1.0;
      } else
        return 0.0;
    }


    double goodBelleLambda(const Particle* Lambda)
    {
      if (Lambda->getNDaughters() != 2) {
        B2WARNING("goodBelleLambda is only defined for a particle with two daughters");
        return 0.;
      }
      const Particle* d0 = Lambda->getDaughter(0);
      const Particle* d1 = Lambda->getDaughter(1);
      if ((d0->getCharge() == 0) || (d1->getCharge() == 0)) {
        B2WARNING("goodBelleLambda is only defined for a particle with charged daughters");
        return 0.;
      }
      if (abs(Lambda->getPDGCode()) != Const::Lambda.getPDGCode()) {
        B2WARNING("goodBelleLambda is being applied to a candidate with PDG " << Lambda->getPDGCode());
      }

      if (Lambda->hasExtraInfo("goodLambda"))
        return Lambda->getExtraInfo("goodLambda");

      double p = particleP(Lambda);
      double dr = std::min(abs(particleDRho(d0)), abs(particleDRho(d1)));
      double zdist = v0DaughterZ0Diff(Lambda);
      double dphi = acos(cosAngleBetweenMomentumAndVertexVectorInXYPlane(Lambda));
      // Flight distance of Lambda0 in xy plane
      double fl = particleDRho(Lambda);

      // goodBelleLambda == 1 (optimized for proton PID > 0.6)
      bool high1 = p >= 1.5 && abs(zdist) < 12.9 && dr > 0.008 && dphi < 0.09 && fl > 0.22;
      bool mid1 = p >= 0.5 && p < 1.5 && abs(zdist) < 9.8 && dr > 0.01 && dphi < 0.18 && fl > 0.16;
      bool low1 = p < 0.5 && abs(zdist) < 2.4 && dr > 0.027 && dphi < 1.2 && fl > 0.11;

      // goodBelleLambda == 2 (optimized without PID selection)
      bool high2 = p >= 1.5 && abs(zdist) < 7.7 && dr > 0.018 && dphi < 0.07 && fl > 0.35;
      bool mid2 = p >= 0.5 && p < 1.5 && abs(zdist) < 2.1 && dr > 0.033 && dphi < 0.10 && fl > 0.24;
      bool low2 = p < 0.5 && abs(zdist) < 1.9 && dr > 0.059 && dphi < 0.6 && fl > 0.17;

      if (low2 || mid2 || high2) {
        return 2.0;
      } else if (low1 || mid1 || high1) {
        return 1.0;
      } else {
        return 0.0;
      }
    }


    bool isGoodBelleGamma(int region, double energy)
    {
      bool goodGammaRegion1, goodGammaRegion2, goodGammaRegion3;
      goodGammaRegion1 = region == 1 && energy > 0.100;
      goodGammaRegion2 = region == 2 && energy > 0.050;
      goodGammaRegion3 = region == 3 && energy > 0.150;

      return goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3;
    }

    bool goodBelleGamma(const Particle* particle)
    {
      double energy = eclClusterE(particle);
      int region = eclClusterDetectionRegion(particle);

      return isGoodBelleGamma(region, energy);
    }

    BelleTrkExtra* getBelleTrkExtraInfoFromParticle(Particle const* particle)
    {
      const Track* track = particle->getTrack();
      if (!track) {
        return nullptr;
      }
      auto belleTrkExtra = track->getRelatedTo<BelleTrkExtra>();
      if (!belleTrkExtra) {
        return nullptr;
      }
      return belleTrkExtra;
    }

    double BelleFirstCDCHitX(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return std::numeric_limits<double>::quiet_NaN();
      }
      return belleTrkExtra->getTrackFirstX();
    }

    double BelleFirstCDCHitY(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return std::numeric_limits<double>::quiet_NaN();
      }
      return belleTrkExtra->getTrackFirstY();
    }

    double BelleFirstCDCHitZ(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return std::numeric_limits<double>::quiet_NaN();
      }
      return belleTrkExtra->getTrackFirstZ();
    }

    double BelleLastCDCHitX(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return std::numeric_limits<double>::quiet_NaN();
      }
      return belleTrkExtra->getTrackLastX();
    }

    double BelleLastCDCHitY(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return std::numeric_limits<double>::quiet_NaN();
      }
      return belleTrkExtra->getTrackLastY();
    }

    double BelleLastCDCHitZ(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return std::numeric_limits<double>::quiet_NaN();
      }
      return belleTrkExtra->getTrackLastZ();
    }

    double BellePi0InvariantMassSignificance(const Particle* particle)
    {
      TMatrixFSym covarianceMatrix(Particle::c_DimMomentum);
      for (auto daughter : particle->getDaughters()) {
        covarianceMatrix += daughter->getMomentumErrorMatrix();
      }

      TVectorF jacobian(Particle::c_DimMomentum);
      jacobian[0] = -1.0 * particle->getPx() / particle->getMass();
      jacobian[1] = -1.0 * particle->getPy() / particle->getMass();
      jacobian[2] = -1.0 * particle->getPz() / particle->getMass();
      jacobian[3] = 1.0 * particle->getEnergy() / particle->getMass();

      double massErrSquared = jacobian * (covarianceMatrix * jacobian);

      if (massErrSquared < 0.0)
        return std::numeric_limits<double>::quiet_NaN();

      double invMass = particleInvariantMassFromDaughters(particle);
      double nomMass = particle->getPDGMass();

      return (invMass - nomMass) / sqrt(massErrSquared);
    }

    VARIABLE_GROUP("Belle Variables");

    REGISTER_VARIABLE("goodBelleKshort", goodBelleKshort, R"DOC(
[Legacy] GoodKs Returns true if a :math:`K_{S}^0\to\pi\pi` candidate passes the Belle algorithm: 
a momentum-binned selection including requirements on impact parameter of, and
angle between the daughter pions as well as separation from the vertex and 
flight distance in the transverse plane.

.. seealso:: `BELLE2-NOTE-PH-2018-017 <https://docs.belle2.org/record/957>`_
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("goodBelleLambda", goodBelleLambda, R"DOC(
[Legacy] Returns 2.0, 1.0, 0.0 as an indication of goodness of :math:`\Lambda^0` candidates, 
based on:

    * The distance of the two daughter tracks at their interception at z axis,
    * the minimum distance of the daughter tracks and the IP in xy plane,
    * the difference of the azimuthal angle of the vertex vector and the momentum vector,
    * and the flight distance of the Lambda0 candidates in xy plane.

It reproduces the ``goodLambda()`` function in Belle.

``goodBelleLambda`` selection 1 (selected with: ``goodBelleLambda>0``) should be used with ``atcPIDBelle(4,2) > 0.6``,
and ``goodBelleLambda`` selection 2 (``goodBelleLambda>1``) can be used without a proton PID cut. 
The former cut is looser than the latter.". 

.. warning:: ``goodBelleLambda`` is not optimized or tested on Belle II data.

See Also:
  * `BN-684`_ Lambda selection at Belle. K F Chen et al.
  * The ``FindLambda`` class can be found at ``/belle_legacy/findLambda/findLambda.h``

.. _BN-684: https://belle.kek.jp/secured/belle_note/gn684/bn684.ps.gz

)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("goodBelleGamma", goodBelleGamma, R"DOC(
[Legacy] Returns 1.0 if the photon candidate passes the simple region dependent
energy selection for Belle data and MC.

.. math::

    E > 50 \textrm{ MeV; barrel}\\
    E > 100 \textrm{ MeV; forward endcap}\\
    E > 150 \textrm{ MeV; backward endcap}
)DOC", Manager::VariableDataType::c_bool);

    REGISTER_VARIABLE("BelleFirstCDCHitX", BelleFirstCDCHitX, R"DOC(
[Legacy] Returns x component of starting point of the track near the 1st SVD or CDC hit for SVD1 data (exp. 7 - 27) or the 1st CDC hit for SVD2 data (from exp. 31). (Belle only, originally stored in mdst_trk_fit.)
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("BelleFirstCDCHitY", BelleFirstCDCHitY, R"DOC(
[Legacy] Returns y component of starting point of the track near the 1st SVD or CDC hit for SVD1 data (exp. 7 - 27) or the 1st CDC hit for SVD2 data (from exp. 31). (Belle only, originally stored in mdst_trk_fit.)
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("BelleFirstCDCHitZ", BelleFirstCDCHitZ, R"DOC(
[Legacy] Returns z component of starting point of the track near the 1st SVD or CDC hit for SVD1 data (exp. 7 - 27) or the 1st CDC hit for SVD2 data (from exp. 31). (Belle only, originally stored in mdst_trk_fit.)
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("BelleLastCDCHitX", BelleLastCDCHitX, R"DOC(
[Legacy] Returns x component of end point of the track near the last CDC hit. (Belle only, originally stored in mdst_trk_fit.)
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("BelleLastCDCHitY", BelleLastCDCHitY, R"DOC(
[Legacy] Returns y component of end point of the track near the last CDC hit. (Belle only, originally stored in mdst_trk_fit.)
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("BelleLastCDCHitZ", BelleLastCDCHitZ, R"DOC(
[Legacy] Returns z component of end point of the track near the last CDC hit. (Belle only, originally stored in mdst_trk_fit.)
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("BellePi0SigM", BellePi0InvariantMassSignificance, R"DOC(
[Legacy] Returns the significance of the :math:`\pi^0` mass used in the FEI for B2BII.
The significance is calculated as the difference between the reconstructed and the nominal mass divided by the mass uncertainty:

.. math::
      \frac{m_{\gamma\gamma} - m_{\pi^0}^\textrm{PDG}}{\sigma_{m_{\gamma\gamma}}}

Since the :math:`\pi^0`'s covariance matrix for B2BII is empty, the latter is calculated using the photon daughters' covariance matrices.
      )DOC", Manager::VariableDataType::c_double);

    // this is defined in ECLVariables.{h,cc}
    REGISTER_VARIABLE("clusterBelleQuality", eclClusterDeltaL, R"DOC(
[Legacy] Returns ECL cluster's quality indicating a good cluster in GSIM (stored in deltaL of ECL cluster object).
Belle analysis typically used clusters with quality == 0 in their :math:`E_{\text{extra ECL}}` (Belle only).
)DOC", Manager::VariableDataType::c_double);
  }
}
