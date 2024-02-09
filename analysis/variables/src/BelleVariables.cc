/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/BelleVariables.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

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
    bool goodBelleKshort(const Particle* KS)
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
        return bool(KS->getExtraInfo("goodKs"));
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
        return true;
      } else
        return false;
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
        return Const::doubleNaN;
      }
      return belleTrkExtra->getTrackFirstX();
    }

    double BelleFirstCDCHitY(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getTrackFirstY();
    }

    double BelleFirstCDCHitZ(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getTrackFirstZ();
    }

    double BelleLastCDCHitX(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getTrackLastX();
    }

    double BelleLastCDCHitY(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getTrackLastY();
    }

    double BelleLastCDCHitZ(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
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
        return Const::doubleNaN;

      double invMass = particleInvariantMassFromDaughters(particle);
      double nomMass = particle->getPDGMass();

      return (invMass - nomMass) / sqrt(massErrSquared);
    }

    double BelleTof(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getTof();
    }

    double BelleTofQuality(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getTofQuality();
    }

    double BelleTofSigma(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getTofSigma();
    }

    double BellePathLength(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getPathLength();
    }

    double BelleTofMass(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      double time = belleTrkExtra->getTof();
      double length = belleTrkExtra->getPathLength();
      double p = particle->getP(); //3-momentum
      double tofbeta = length / time / Belle2::Const::speedOfLight;
      double tofmass = p * sqrt(1. / (tofbeta * tofbeta) - 1.); //(GeV)

      return tofmass;
    }

    double BelledEdx(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getdEdx();
    }

    double BelledEdxQuality(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getdEdxQuality();
    }

    double BelleACCnPe(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getACCPe();
    }

    double BelleACCQuality(const Particle* particle)
    {
      auto belleTrkExtra = getBelleTrkExtraInfoFromParticle(particle);
      if (!belleTrkExtra) {
        B2WARNING("Cannot find BelleTrkExtra, did you forget to enable BelleTrkExtra during the conversion?");
        return Const::doubleNaN;
      }
      return belleTrkExtra->getACCQuality();
    }



    VARIABLE_GROUP("Belle Variables");

    REGISTER_VARIABLE("goodBelleKshort", goodBelleKshort, R"DOC(
[Legacy] GoodKs Returns true if a :math:`K_{S}^0\to\pi\pi` candidate passes the Belle algorithm: 
a momentum-binned selection including requirements on impact parameter of, and
angle between the daughter pions as well as separation from the vertex and 
flight distance in the transverse plane.

.. seealso:: `BELLE2-NOTE-PH-2018-017 <https://docs.belle2.org/record/957>`_
)DOC");

    REGISTER_VARIABLE("goodBelleLambda", goodBelleLambda, R"DOC(
[Legacy] Returns 2.0, 1.0, 0.0 as an indication of goodness of :math:`\Lambda^0` candidates, 
based on:

* The distance of the two daughter tracks at their interception at z axis,
* the minimum distance of the daughter tracks and the IP in xy plane,
* the difference of the azimuthal angle of the vertex vector and the momentum vector,
* and the flight distance of the Lambda0 candidates in xy plane.

It reproduces the ``goodLambda()`` function in Belle.

``goodBelleLambda`` selection 1 (selected with: ``goodBelleLambda>0``) maximizes the signal significance after applying
``atcPIDBelle(4,2) > 0.6``, while ``goodBelleLambda`` selection 2 (``goodBelleLambda>1``) is tighter and maximizes the signal
significance of a :math:`\Lambda^0` sample without any proton PID cut. However, it might still be beneficial to apply a proton PID
cut on top of it. Which combination of proton PID cut and ``goodBelleLambda`` selection scenario is ideal, is probably
analysis-dependent.

.. warning:: ``goodBelleLambda`` is not optimized or tested on Belle II data.

See Also:
  * `BN-684`_ Lambda selection at Belle. K F Chen et al.
  * The ``FindLambda`` class can be found at ``/belle_legacy/findLambda/findLambda.h``

.. _BN-684: https://belle.kek.jp/secured/belle_note/gn684/bn684.ps.gz

)DOC");

    REGISTER_VARIABLE("goodBelleGamma", goodBelleGamma, R"DOC(
[Legacy] Returns 1.0 if the photon candidate passes the simple region dependent
energy selection for Belle data and MC.

.. math::

    E > 50 \textrm{ MeV; barrel}\\
    E > 100 \textrm{ MeV; forward endcap}\\
    E > 150 \textrm{ MeV; backward endcap}
)DOC");

    REGISTER_VARIABLE("BelleFirstCDCHitX", BelleFirstCDCHitX, R"DOC(
[Legacy] Returns x component of starting point of the track near the 1st SVD or CDC hit for SVD1 data (exp. 7 - 27) or the 1st CDC hit for SVD2 data (from exp. 31). (Belle only, originally stored in mdst_trk_fit.)

)DOC","cm");

    REGISTER_VARIABLE("BelleFirstCDCHitY", BelleFirstCDCHitY, R"DOC(
[Legacy] Returns y component of starting point of the track near the 1st SVD or CDC hit for SVD1 data (exp. 7 - 27) or the 1st CDC hit for SVD2 data (from exp. 31). (Belle only, originally stored in mdst_trk_fit.)

)DOC","cm");

    REGISTER_VARIABLE("BelleFirstCDCHitZ", BelleFirstCDCHitZ, R"DOC(
[Legacy] Returns z component of starting point of the track near the 1st SVD or CDC hit for SVD1 data (exp. 7 - 27) or the 1st CDC hit for SVD2 data (from exp. 31). (Belle only, originally stored in mdst_trk_fit.)

)DOC","cm");

    REGISTER_VARIABLE("BelleLastCDCHitX", BelleLastCDCHitX, R"DOC(
[Legacy] Returns x component of end point of the track near the last CDC hit. (Belle only, originally stored in mdst_trk_fit.)

)DOC","cm");

    REGISTER_VARIABLE("BelleLastCDCHitY", BelleLastCDCHitY, R"DOC(
[Legacy] Returns y component of end point of the track near the last CDC hit. (Belle only, originally stored in mdst_trk_fit.)

)DOC","cm");

    REGISTER_VARIABLE("BelleLastCDCHitZ", BelleLastCDCHitZ, R"DOC(
[Legacy] Returns z component of end point of the track near the last CDC hit. (Belle only, originally stored in mdst_trk_fit.)

)DOC","cm");

    REGISTER_VARIABLE("BellePi0SigM", BellePi0InvariantMassSignificance, R"DOC(
[Legacy] Returns the significance of the :math:`\pi^0` mass used in the FEI for B2BII.
The significance is calculated as the difference between the reconstructed and the nominal mass divided by the mass uncertainty:

.. math::
      \frac{m_{\gamma\gamma} - m_{\pi^0}^\textrm{PDG}}{\sigma_{m_{\gamma\gamma}}}

Since the :math:`\pi^0`'s covariance matrix for B2BII is empty, the latter is calculated using the photon daughters' covariance matrices.
      )DOC");

    REGISTER_VARIABLE("BelleTof", BelleTof, R"DOC(
[Legacy] Returns the time of flight of a track. (Belle only).

)DOC", "ns");

    REGISTER_VARIABLE("BelleTofQuality", BelleTofQuality, R"DOC(
[Legacy] Returns the quality flag of the time of flight of a track. Original description from the panther table:  0 if consistency between z of hit by charge Q and corrected times, 1 if zhit from Q NOT consistent with zhit from and correct times. (Belle only).
)DOC");

    REGISTER_VARIABLE("BelleTofSigma", BelleTofSigma, R"DOC(
[Legacy] Returns the expected resolution on the time of flight of a track. (Belle only).

)DOC", "ns");

    REGISTER_VARIABLE("BellePathLength", BellePathLength, R"DOC(
[Legacy] Returns the track path length. This is defined from the closest point to the z-axis up to TOF counter. (Belle only).

)DOC", "cm");

    REGISTER_VARIABLE("BelleTofMass", BelleTofMass, R"DOC(
[Legacy] Returns the TOF mass calculated from the time of flight and path length. (Belle only).
)DOC", "GeV/:math:`\\text{c}^2`");

    REGISTER_VARIABLE("BelledEdx", BelledEdx, R"DOC(
[Legacy] Returns the dE/dx measured in the CDC. (Belle only).

)DOC", "keV/cm");

    REGISTER_VARIABLE("BelledEdxQuality", BelledEdxQuality, R"DOC(
[Legacy] Returns the quality flag of the dE/dx measured in the CDC. Sadly no information about the code meaning is given in the original panther tables. (Belle only).
)DOC");

    REGISTER_VARIABLE("BelleACCnPe", BelleACCnPe, R"DOC(
[Legacy] Returns the number of photo-electrons associated to the track in the ACC. (Belle only).
)DOC");

    REGISTER_VARIABLE("BelleACCQuality", BelleACCQuality, R"DOC(
[Legacy] Returns the ACC quality flag. Original definition in the panther tables: if 0 normal, if 1 the track is out of ACC acceptance. (Belle only).
)DOC");


    // this is defined in ECLVariables.{h,cc}
    REGISTER_VARIABLE("clusterBelleQuality", eclClusterDeltaL, R"DOC(
[Legacy] Returns ECL cluster's quality indicating a good cluster in GSIM (stored in deltaL of ECL cluster object).
Belle analysis typically used clusters with quality == 0 in their :math:`E_{\text{extra ECL}}` (Belle only).
)DOC");
  }
}
