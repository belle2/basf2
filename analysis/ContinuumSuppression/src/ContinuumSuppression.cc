/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig,                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/ContinuumSuppression/ContinuumSuppression.h>
#include <analysis/ContinuumSuppression/Thrust.h>
#include <analysis/ContinuumSuppression/KsfwMoments.h>
#include <analysis/ContinuumSuppression/FoxWolfram.h>
#include <analysis/ContinuumSuppression/CleoCones.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <framework/datastore/StoreArray.h>

#include <vector>


namespace Belle2 {

  void addContinuumSuppression(const Particle* particle, const std::string& maskName)
  {
    // Output
    StoreArray<ContinuumSuppression> qqArray;
    // Create ContinuumSuppression object
    ContinuumSuppression* qqVars = qqArray.appendNew();

    // Create relation: Particle <-> ContinuumSuppression
    particle->addRelationTo(qqVars);

    std::vector<TVector3> p3_cms_sigB, p3_cms_roe, p3_cms_all;

    std::vector<std::pair<TVector3, int>> p3_cms_q_sigA;
    std::vector<std::pair<TVector3, int>> p3_cms_q_sigB;
    std::vector<std::pair<TVector3, int>> p3_cms_q_roe;

    std::vector<float> ksfwFS0;
    std::vector<float> ksfwFS1;

    std::vector<float> cleoConesAll;
    //std::vector<float> cleoConesRoe;

    double et[2];

    TVector3 thrustB;
    TVector3 thrustO;

    float thrustBm = -1;
    float thrustOm = -1;
    float cosTBTO  = -1;
    float cosTBz   = -1;
    float R2       = -1;

    // Kinematically allowed maximum momentum for mbc>5.2
    // sqrt(5.29^2 - 5.2^2) ~ 1.0GeV
    // 10.58/4 + 1.0/2 = 3.145
    static const double P_MAX(3.2);

    // -- B Cand --------------------------------------------------------------------------
    PCmsLabTransform T;
    double BeamEnergy = T.getCMSEnergy() / 2;

    TLorentzVector p_cms_missA(0, 0, 0, 2 * BeamEnergy);
    TLorentzVector p_cms_missB(0, 0, 0, 2 * BeamEnergy);
    et[0] = et[1] = 0;

    // -- SIG A --- Use B primary daughters - (Belle: use_finalstate_for_sig == 0) --------
    std::vector<Belle2::Particle*> sigDau = particle->getDaughters();

    for (unsigned i = 0; i < sigDau.size(); i++) {
      PCmsLabTransform T;
      TLorentzVector p_cms = T.rotateLabToCms() * sigDau[i]->get4Vector();

      p3_cms_q_sigA.push_back({p_cms.Vect(), sigDau[i]->getCharge()});

      p_cms_missA -= p_cms;
      et[0] += p_cms.Perp();
    }

    // -- SIG B --- Use B final-state daughters - (Belle: use_finalstate_for_sig == 1) ----
    std::vector<const Belle2::Particle*> sigFsp = particle->getFinalStateDaughters();

    for (unsigned i = 0; i < sigFsp.size(); i++) {
      PCmsLabTransform T;
      TLorentzVector p_cms = T.rotateLabToCms() * sigFsp[i]->get4Vector();

      p3_cms_all.push_back(p_cms.Vect());
      p3_cms_sigB.push_back(p_cms.Vect());

      p3_cms_q_sigB.push_back({p_cms.Vect(), sigFsp[i]->getCharge()});

      p_cms_missB -= p_cms;
      et[1] += p_cms.Perp();
    }

    // -- ROE -----------------------------------------------------------------------------
    const RestOfEvent* roe = particle->getRelated<RestOfEvent>();

    if (roe) {

      // Charged tracks -> Pion
      //
      std::vector<const Track*> roeTracks = roe->getTracks(maskName);

      const Const::ChargedStable charged = Const::pion;

      for (const Track* track : roeTracks) {

        // TODO: Add helix and KVF with IpProfile once available. Port from L163-199 of:
        // /belle/b20090127_0910/src/anal/ekpcontsuppress/src/ksfwmoments.cc

        // Create particle from track with pion hypothesis
        Particle particle(track, charged);
        if (particle.getParticleType() == Particle::c_Track) {
          PCmsLabTransform T;
          TLorentzVector p_cms = T.rotateLabToCms() * particle.get4Vector();
          if (p_cms.Rho() > P_MAX) continue;

          p3_cms_all.push_back(p_cms.Vect());
          p3_cms_roe.push_back(p_cms.Vect());

          p3_cms_q_roe.push_back({p_cms.Vect(), particle.getCharge()});

          p_cms_missA -= p_cms;
          p_cms_missB -= p_cms;
          et[0] += p_cms.Perp();
          et[1] += p_cms.Perp();
        }
      }

      // ECLCluster -> Gamma
      //
      std::vector<const ECLCluster*> roeECLClusters = roe->getECLClusters(maskName);

      for (const ECLCluster* cluster : roeECLClusters) {

        if (cluster->isNeutral()) {

          // Create particle from ECLCluster with gamma hypothesis
          Particle particle(cluster);

          TLorentzVector p_lab = particle.get4Vector();
          if (p_lab.Rho() < 0.05) continue;
          PCmsLabTransform T;
          TLorentzVector p_cms = T.rotateLabToCms() * p_lab;
          if (p_cms.Rho() > P_MAX) continue;
          p3_cms_all.push_back(p_cms.Vect());
          p3_cms_roe.push_back(p_cms.Vect());

          p3_cms_q_roe.push_back({p_cms.Vect(), particle.getCharge()});

          p_cms_missA -= p_cms;
          p_cms_missB -= p_cms;
          et[0] += p_cms.Perp();
          et[1] += p_cms.Perp();
        }
      }

      // Thrust variables
      thrustB = Thrust::calculateThrust(p3_cms_sigB);
      thrustO = Thrust::calculateThrust(p3_cms_roe);
      thrustBm = thrustB.Mag();
      thrustOm = thrustO.Mag();
      cosTBTO  = fabs(cos(thrustB.Angle(thrustO)));
      cosTBz   = fabs(thrustB.CosTheta());

      // Cleo Cones
      // TODO: make the following option configurable:
      //       calculate the momentum flow for all particles in the event
      //       vs only those in the roe.
      CleoCones cc(p3_cms_all, p3_cms_roe, thrustB, true, true);
      cleoConesAll = cc.cleo_cone_with_all();
      //cleoConesRoe = cc.cleo_cone_with_roe();

      // Fox-Wolfram Moments: Uses all final-state tracks (= sigB + ROE)
      FoxWolfram FW(p3_cms_all);
      R2 = FW.R(2);

      // KSFW moments
      TLorentzVector p_cms_B = T.rotateLabToCms() * particle->get4Vector();
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
    qqVars->addThrustO(thrustO);
    qqVars->addThrustBm(thrustBm);
    qqVars->addThrustOm(thrustOm);
    qqVars->addCosTBTO(cosTBTO);
    qqVars->addCosTBz(cosTBz);
    qqVars->addR2(R2);
    qqVars->addKsfwFS0(ksfwFS0);
    qqVars->addKsfwFS1(ksfwFS1);
    qqVars->addCleoCones(cleoConesAll);
    // TODO: add cleo cones calculated from roe only.
  }
}
