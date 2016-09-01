/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <skim/softwaretrigger/calculations/HLTCalculator.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
// TODO: Also cache it
#include <analysis/utility/PCmsLabTransform.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    void HLTCalculator::requireStoreArrays()
    {
      m_pionParticles.isRequired();
      m_gammaParticles.isRequired();
    };

    const ECLCluster* ECLClusterNeutralLE(const StoreObjPtr<ParticleList>& gammas)
    {
      const ECLCluster* ecle = nullptr;
      double E1 = -1.;
      for (unsigned int i = 0; i < gammas->getListSize(); i++) {
        Particle* p = gammas->getParticle(i);
        const ECLCluster* ecle_tmp = p->getECLCluster();
        TLorentzVector V4p = ecle_tmp->get4Vector();
        double e = (PCmsLabTransform::labToCms(V4p)).Rho();
        if (E1 < e) {
          E1 = e;
          ecle = ecle_tmp;
        }
      }
      return ecle;
    }

    const Particle* T1(const StoreObjPtr<ParticleList>& pions)
    {
      if (pions->getListSize() < 1) {
        return nullptr;
      } else {
        const Particle* t = nullptr;
        double topp = -1.;
        for (unsigned int i = 0; i < pions->getListSize(); i++) {
          const Particle* t1 = pions->getParticle(i);
          TLorentzVector V4p1 = t1->get4Vector();
          if ((PCmsLabTransform::labToCms(V4p1)).Rho() > topp) {
            t = t1;
            topp = (PCmsLabTransform::labToCms(V4p1)).Rho();
          }
        }
        return t;
      }
    }

    const Particle*  T2(const StoreObjPtr<ParticleList>& pionshlt)
    {
      if (pionshlt->getListSize() <= 1) {
        return nullptr;
      } else {
        const  Particle* t = nullptr;
        double sedp = -1.;
        const Particle* t1 = T1(pionshlt);
        TLorentzVector V4p1 = t1->get4Vector();
        double topp = (PCmsLabTransform::labToCms(V4p1)).Rho();
        for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
          const Particle* t2 = pionshlt->getParticle(i);
          TLorentzVector V4p2 = t2->get4Vector();
          double mom = (PCmsLabTransform::labToCms(V4p2)).Rho();
          if (mom > sedp && mom < topp) {
            t = t2;
            sedp = mom;
          }
        }
        return t;
      }
    }

    const ECLCluster*  ECLClusterC1LE(const StoreObjPtr<ParticleList>& pionshlt, const StoreObjPtr<ParticleList>& gammahlt)
    {
      const ECLCluster*  ecle = nullptr;
      double E1 = -1.;
      for (unsigned  int i = 0; i < (pionshlt->getListSize() + gammahlt->getListSize()); i++) {
        Particle* p;
        const ECLCluster* ecle_tmp;
        if (i < pionshlt->getListSize()) {
          p = pionshlt->getParticle(i);
          const Track* trk = p->getTrack();
          ecle_tmp = trk->getRelated<ECLCluster>();
          if (!ecle_tmp) { continue;}

        } else {
          p = gammahlt->getParticle(i - pionshlt->getListSize());
          ecle_tmp = p->getECLCluster();
        }
        TLorentzVector V4p1 = ecle_tmp->get4Vector();
        double e = (PCmsLabTransform::labToCms(V4p1)).Rho();
        if (E1 < e) {E1 = e; ecle = ecle_tmp;}
      }
      return ecle;
    }

    const ECLCluster*  ECLClusterC2LE(const double Ehigh, const StoreObjPtr<ParticleList>& pionshlt,
                                      const StoreObjPtr<ParticleList>& gammahlt)
    {
      const ECLCluster*  ecle = nullptr;
      double result = -1.;
      for (unsigned  int i = 0; i < (pionshlt->getListSize() + gammahlt->getListSize()); i++) {
        Particle* p;
        const ECLCluster* ecle_tmp;
        if (i < pionshlt->getListSize()) {
          p = pionshlt->getParticle(i);
          const Track* trk = p->getTrack();
          ecle_tmp = trk->getRelated<ECLCluster>();
          if (!ecle_tmp) { continue;}

        } else {
          p = gammahlt->getParticle(i - pionshlt->getListSize());
          ecle_tmp = p->getECLCluster();
        }
        TLorentzVector V4p2 = ecle_tmp->get4Vector();
        double e = (PCmsLabTransform::labToCms(V4p2)).Rho();
        if (e >= Ehigh) continue;
        else if (result < e) {result = e; ecle = ecle_tmp;}
      }
      return ecle;

    }

    void HLTCalculator::doCalculation(SoftwareTriggerObject& calculationResult) const
    {
      // AngleGTLE
      double angleGTLE = -10.;
      const ECLCluster* eclClusterNeutral = ECLClusterNeutralLE(m_gammaParticles);
      const Particle* t1 = T1(m_pionParticles);
      const Particle* t2 = T2(m_pionParticles);

      if (eclClusterNeutral) {
        TLorentzVector V4g1 = eclClusterNeutral->get4Vector();
        if (t1) {
          const TLorentzVector& V4p1 = t1->get4Vector();
          const double theta1 = (V4g1.Vect()).Angle(V4p1.Vect());
          if (angleGTLE < theta1) angleGTLE = theta1;
        }
        if (t2) {
          const TLorentzVector& V4p2 = t2->get4Vector();
          const double theta2 = (V4g1.Vect()).Angle(V4p2.Vect());
          if (angleGTLE < theta2) angleGTLE = theta2;
        }
      }

      calculationResult["AngleGTLE"] = angleGTLE;

      // EC1CMSLE
      double ec1CMSLE = -1.;
      const ECLCluster* eclCluster1 = ECLClusterC1LE(m_pionParticles, m_gammaParticles);
      if (eclCluster1) {
        const TLorentzVector& V4p2 = eclCluster1->get4Vector();
        ec1CMSLE = (PCmsLabTransform::labToCms(V4p2)).Rho();
      }
      calculationResult["EC1CMSLE"] = ec1CMSLE;

      // EC2CMSLE
      double ec2CMSLE = -1.;
      const ECLCluster* eclCluster2 = ECLClusterC2LE(ec1CMSLE, m_pionParticles, m_gammaParticles);
      if (eclCluster2) {
        const TLorentzVector& V4p2 = eclCluster2->get4Vector();
        ec2CMSLE = (PCmsLabTransform::labToCms(V4p2)).Rho();
      }
      calculationResult["EC2CMSLE"] = ec2CMSLE;

      // EC12CMSLE
      calculationResult["EC12CMSLE"] = ec1CMSLE + ec2CMSLE;

      // ENeutralLE
      double eNeutralLE = -1;
      if (eclClusterNeutral) {
        eNeutralLE = eclClusterNeutral->getEnergy();
      }

      calculationResult["ENeutralLE"] = eNeutralLE;

      // maxAngleTTLE
      double maxAngleTTLE = -10.;
      if (m_pionParticles->getListSize() >= 2) {
        ;
        for (unsigned int i = 0; i < m_pionParticles->getListSize() - 1; i++) {
          Particle* par1 = m_pionParticles->getParticle(i);
          for (unsigned int j = i + 1; j < m_pionParticles->getListSize(); j++) {
            Particle* par2 = m_pionParticles->getParticle(j);
            TLorentzVector V4p1 = par1->get4Vector();
            TLorentzVector V4p2 = par2->get4Vector();
            const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
            const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
            const double temp = V3p1.Angle(V3p2);
            if (maxAngleTTLE < temp) maxAngleTTLE = temp;
          }
        }
      }

      calculationResult["maxAngleTTLE"] = maxAngleTTLE;

      // nEidLE
      int nEidLE = 0;

      for (unsigned int i = 0; i < m_pionParticles->getListSize(); i++) {
        Particle* p = m_pionParticles->getParticle(i);
        double mom  = p->getMomentumMagnitude();
        TLorentzVector V4p1 = p->get4Vector();
        double Pcms = (PCmsLabTransform::labToCms(V4p1)).Rho();
        const ECLCluster* eclTrack = (p->getTrack())->getRelated<ECLCluster>();
        double eop = -1.;
        if (eclTrack)
          eop = eclTrack->getEnergy() / mom;
        if (Pcms > 5.0 && eop > 0.8) nEidLE++;
      }

      calculationResult["nEidLE"] = nEidLE;

      // nECLMatchTracksLE
      int nECLMatchTracksLE = 0;
      for (unsigned int i = 0; i < m_pionParticles->getListSize(); i++) {
        Particle* p = m_pionParticles->getParticle(i);
        const Track* trk = p->getTrack();
        if (trk->getRelated<ECLCluster>()) nECLMatchTracksLE++;
      }

      calculationResult["nECLMatchTracksLE"] = nECLMatchTracksLE;

      // nTracksLE
      calculationResult["nTracksLE"] = m_pionParticles->getListSize();

      // P1CMSBhabhaLE
      double p1CMSBhabhaLE = -1;
      if (t1) {
        const TLorentzVector& V4p1 = t1->get4Vector();
        p1CMSBhabhaLE = (PCmsLabTransform::labToCms(V4p1)).Rho();
      }
      calculationResult["P1CMSBhabhaLE"] = p1CMSBhabhaLE;

      // P2CMSBhabhaLE
      double p2CMSBhabhaLE = -1;
      if (t2) {
        const TLorentzVector& V4p2 = t2->get4Vector();
        p2CMSBhabhaLE = (PCmsLabTransform::labToCms(V4p2)).Rho();
      }
      calculationResult["P2CMSBhabhaLE"] = p2CMSBhabhaLE;

      // P12CMSBhabhaLE
      calculationResult["P12CMSBhabhaLE"] = p1CMSBhabhaLE + p2CMSBhabhaLE;

      // VisibleEnergyLE
      double VisibleEnergyLE = 0.;
      for (unsigned int i = 0; i < m_pionParticles->getListSize(); i++) {
        Particle* p = m_pionParticles->getParticle(i);
        VisibleEnergyLE += p->getMomentumMagnitude();
      }

      for (unsigned int i = 0; i < m_gammaParticles->getListSize(); i++) {
        Particle* p = m_gammaParticles->getParticle(i);
        VisibleEnergyLE += p->getMomentumMagnitude();
      }

      calculationResult["VisibleEnergyLE"] = VisibleEnergyLE;

      // EtotLE
      double EtotLE = 0.;
      for (unsigned int i = 0; i < m_pionParticles->getListSize(); i++) {
        Particle* p = m_pionParticles->getParticle(i);
        const Track* trk = p->getTrack();
        const ECLCluster* eclTrack = trk->getRelated<ECLCluster>();
        if (!eclTrack) continue;
        if (eclTrack->getEnergy() > 0.1)
          EtotLE += eclTrack->getEnergy();
      }

      for (unsigned int i = 0; i < m_gammaParticles->getListSize(); i++) {
        Particle* p = m_gammaParticles->getParticle(i);
        EtotLE += p->getEnergy();
      }

      calculationResult["EtotLE"] = EtotLE;
    }
  }
}
