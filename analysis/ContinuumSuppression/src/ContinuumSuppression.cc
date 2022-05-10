/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/ContinuumSuppression/ContinuumSuppression.h>
#include <analysis/ContinuumSuppression/Thrust.h>
#include <analysis/ContinuumSuppression/KsfwMoments.h>
#include <analysis/ContinuumSuppression/FoxWolfram.h>
#include <analysis/ContinuumSuppression/CleoCones.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <vector>
#include <Math/Vector3D.h>

namespace Belle2 {

  void addContinuumSuppression(const Particle* particle, const std::string& maskName)
  {
    // Output
    StoreArray<ContinuumSuppression> qqArray(maskName);
    // Create ContinuumSuppression object
    ContinuumSuppression* qqVars = qqArray.appendNew();

    // Create relation: Particle <-> ContinuumSuppression
    particle->addRelationTo(qqVars);

    std::vector<ROOT::Math::XYZVector> p3_cms_sigB, p3_cms_roe, p3_cms_all;

    std::vector<std::pair<ROOT::Math::XYZVector, int>> p3_cms_q_sigA;
    std::vector<std::pair<ROOT::Math::XYZVector, int>> p3_cms_q_sigB;
    std::vector<std::pair<ROOT::Math::XYZVector, int>> p3_cms_q_roe;

    std::vector<float> ksfwFS0;
    std::vector<float> ksfwFS1;

    std::vector<float> cleoConesAll;
    std::vector<float> cleoConesROE;

    double et[2];

    ROOT::Math::XYZVector thrustB;
    ROOT::Math::XYZVector thrustO;

    float thrustBm = -1;
    float thrustOm = -1;
    float cosTBTO  = -1;
    float cosTBz   = -1;
    float R2       = -1;


    // -- B Cand --------------------------------------------------------------------------
    PCmsLabTransform T;
    double BeamEnergy = T.getCMSEnergy() / 2;

    ROOT::Math::PxPyPzEVector p_cms_missA(0, 0, 0, 2 * BeamEnergy);
    ROOT::Math::PxPyPzEVector p_cms_missB(0, 0, 0, 2 * BeamEnergy);
    et[0] = et[1] = 0;

    // -- SIG A --- Use B primary daughters - (Belle: use_finalstate_for_sig == 0) --------
    std::vector<Belle2::Particle*> signalDaughters = particle->getDaughters();

    for (const Belle2::Particle* sigFS0 : signalDaughters) {
      ROOT::Math::PxPyPzEVector p_cms = T.rotateLabToCms() * sigFS0->get4Vector();

      p3_cms_q_sigA.emplace_back(p_cms.Vect(), sigFS0->getCharge());

      p_cms_missA -= p_cms;
      et[0] += p_cms.Pt();
    }

    // -- SIG B --- Use B final-state daughters - (Belle: use_finalstate_for_sig == 1) ----
    std::vector<const Belle2::Particle*> signalFSParticles = particle->getFinalStateDaughters();

    for (const Belle2::Particle* sigFS1 : signalFSParticles) {
      ROOT::Math::PxPyPzEVector p_cms = T.rotateLabToCms() * sigFS1->get4Vector();

      p3_cms_all.push_back(p_cms.Vect());
      p3_cms_sigB.push_back(p_cms.Vect());

      p3_cms_q_sigB.emplace_back(p_cms.Vect(), sigFS1->getCharge());

      p_cms_missB -= p_cms;
      et[1] += p_cms.Pt();
    }

    // -- ROE -----------------------------------------------------------------------------
    const RestOfEvent* roe = particle->getRelated<RestOfEvent>();

    if (roe) {

      // Charged tracks
      //
      std::vector<const Particle*> chargedROEParticles = roe->getChargedParticles(maskName);

      for (const Particle* chargedROEParticle : chargedROEParticles) {

        // TODO: Add helix and KVF with IpProfile once available. Port from L163-199 of:
        // /belle/b20090127_0910/src/anal/ekpcontsuppress/src/ksfwmoments.cc

        ROOT::Math::PxPyPzEVector p_cms = T.rotateLabToCms() * chargedROEParticle->get4Vector();

        p3_cms_all.push_back(p_cms.Vect());
        p3_cms_roe.push_back(p_cms.Vect());

        p3_cms_q_roe.emplace_back(p_cms.Vect(), chargedROEParticle->getCharge());

        p_cms_missA -= p_cms;
        p_cms_missB -= p_cms;
        et[0] += p_cms.Pt();
        et[1] += p_cms.Pt();
      }

      // ECLCluster
      //
      std::vector<const Particle*> roePhotons = roe->getPhotons(maskName);

      for (const Particle* photon : roePhotons) {

        if (photon->getECLClusterEHypothesisBit() == ECLCluster::EHypothesisBit::c_nPhotons) {

          ROOT::Math::PxPyPzEVector p_cms = T.rotateLabToCms() * photon->get4Vector();
          p3_cms_all.push_back(p_cms.Vect());
          p3_cms_roe.push_back(p_cms.Vect());

          p3_cms_q_roe.emplace_back(p_cms.Vect(), photon->getCharge());

          p_cms_missA -= p_cms;
          p_cms_missB -= p_cms;
          et[0] += p_cms.Pt();
          et[1] += p_cms.Pt();
        }
      }

      // Thrust variables
      thrustB = Thrust::calculateThrust(p3_cms_sigB);
      thrustO = Thrust::calculateThrust(p3_cms_roe);
      thrustBm = thrustB.R();
      thrustOm = thrustO.R();
      cosTBTO  = fabs(thrustB.Unit().Dot(thrustO.Unit()));
      cosTBz   = fabs(cos(thrustB.Theta()));

      // Cleo Cones
      CleoCones cc(p3_cms_all, p3_cms_roe, thrustB, true, true);
      cleoConesAll = cc.cleo_cone_with_all();
      cleoConesROE = cc.cleo_cone_with_roe();

      // Fox-Wolfram Moments: Uses all final-state tracks (= sigB + ROE)
      FoxWolfram FW(p3_cms_all);
      FW.calculateBasicMoments();
      R2 = FW.getR(2);

      // KSFW moments
      ROOT::Math::PxPyPzEVector p_cms_B = T.rotateLabToCms() * particle->get4Vector();
      double Hso0_max(2 * (2 * BeamEnergy - p_cms_B.E()));
      KsfwMoments KsfwM(Hso0_max,
                        p3_cms_q_sigA,
                        p3_cms_q_sigB,
                        p3_cms_q_roe,
                        p_cms_missA,
                        p_cms_missB,
                        et);
      // use_finalstate_for_sig == 0
      KsfwM.usefinal(0);
      ksfwFS0.push_back(KsfwM.mm2());
      ksfwFS0.push_back(KsfwM.et());
      ksfwFS0.push_back(KsfwM.Hso(0, 0));
      ksfwFS0.push_back(KsfwM.Hso(0, 1));
      ksfwFS0.push_back(KsfwM.Hso(0, 2));
      ksfwFS0.push_back(KsfwM.Hso(0, 3));
      ksfwFS0.push_back(KsfwM.Hso(0, 4));
      ksfwFS0.push_back(KsfwM.Hso(1, 0));
      ksfwFS0.push_back(KsfwM.Hso(1, 2));
      ksfwFS0.push_back(KsfwM.Hso(1, 4));
      ksfwFS0.push_back(KsfwM.Hso(2, 0));
      ksfwFS0.push_back(KsfwM.Hso(2, 2));
      ksfwFS0.push_back(KsfwM.Hso(2, 4));
      ksfwFS0.push_back(KsfwM.Hoo(0));
      ksfwFS0.push_back(KsfwM.Hoo(1));
      ksfwFS0.push_back(KsfwM.Hoo(2));
      ksfwFS0.push_back(KsfwM.Hoo(3));
      ksfwFS0.push_back(KsfwM.Hoo(4));
      // use_finalstate_for_sig == 1
      KsfwM.usefinal(1);
      ksfwFS1.push_back(KsfwM.mm2());
      ksfwFS1.push_back(KsfwM.et());
      ksfwFS1.push_back(KsfwM.Hso(0, 0));
      ksfwFS1.push_back(KsfwM.Hso(0, 1));
      ksfwFS1.push_back(KsfwM.Hso(0, 2));
      ksfwFS1.push_back(KsfwM.Hso(0, 3));
      ksfwFS1.push_back(KsfwM.Hso(0, 4));
      ksfwFS1.push_back(KsfwM.Hso(1, 0));
      ksfwFS1.push_back(KsfwM.Hso(1, 2));
      ksfwFS1.push_back(KsfwM.Hso(1, 4));
      ksfwFS1.push_back(KsfwM.Hso(2, 0));
      ksfwFS1.push_back(KsfwM.Hso(2, 2));
      ksfwFS1.push_back(KsfwM.Hso(2, 4));
      ksfwFS1.push_back(KsfwM.Hoo(0));
      ksfwFS1.push_back(KsfwM.Hoo(1));
      ksfwFS1.push_back(KsfwM.Hoo(2));
      ksfwFS1.push_back(KsfwM.Hoo(3));
      ksfwFS1.push_back(KsfwM.Hoo(4));

      // TODO: The following is from the original belle ksfwmoments.cc module.
      //       Not sure if necessary here (i.e., will we be using rooksfw in belle II in the same way?).
      //       printf("rooksfw::rooksfw: mm2=%f et=%f hoo2=%f hso02=%f\n",
      //       m_mm2[0], et[0], m_Hoo[0][2], m_Hso[0][0][2]);
    }

    // Fill ContinuumSuppression with content
    qqVars->addThrustB(thrustB);
    qqVars->addThrustO(thrustO);
    qqVars->addThrustBm(thrustBm);
    qqVars->addThrustOm(thrustOm);
    qqVars->addCosTBTO(cosTBTO);
    qqVars->addCosTBz(cosTBz);
    qqVars->addR2(R2);
    qqVars->addKsfwFS0(ksfwFS0);
    qqVars->addKsfwFS1(ksfwFS1);
    qqVars->addCleoConesALL(cleoConesAll);
    qqVars->addCleoConesROE(cleoConesROE);
  }
}
