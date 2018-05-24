
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/PhysicsTriggerVariables.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/VariableManager/Manager.h>

// ecl cluster utility
#include <analysis/ClusterUtility/ClusterUtils.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/ParticleList.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TLorentzVector.h>
#include <TVectorF.h>
#include <TVector3.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {


//LE variables
    double nTracksLE(const Particle*)
    {
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      return pionshlt->getListSize();
    }

    double nLongTracksLE(const Particle*)
    {
      int result = 0;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        double tranp = (p->getMomentum()).Pt();
        if (tranp > 0.3) result++;
      }
      return result;
    }


    double nECLMatchTracksLE(const Particle*)
    {
      int result = 0;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        const Track* trk = p->getTrack();
        if (trk->getRelated<ECLCluster>()) result++;
      }
      return result;
    }

    double nKLMMatchTracksLE(const Particle*)
    {
      int result = 0;
      /*
            StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
            for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
              Particle* p = pionshlt->getParticle(i);
              const Track* trk = p->getTrack();
              if (trk->getRelatedFrom<KLMCluster>()) result++;
             //if (p->getKLMCluster()) result++;
       */
      StoreArray<KLMCluster> klmClusters;
      for (int i = 0; i < klmClusters.getEntries(); i++) {
        if (klmClusters[i]->getAssociatedTrackFlag())result++;
      }
      return result;
    }

    const Particle*  T1(const Particle*)
    {
      const  Particle* t = NULL;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      if (pionshlt->getListSize() < 1) return t;
      else {
        double topp = -1.;
        for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
          const Particle* t1 = pionshlt->getParticle(i);
          TLorentzVector V4p1 = t1->get4Vector();
          if ((PCmsLabTransform::labToCms(V4p1)).Rho() > topp)
          {t = t1; topp = (PCmsLabTransform::labToCms(V4p1)).Rho();}
        }
      }
      return t;
    }

    const Particle*  T2(const Particle* part)
    {
      const  Particle* t = NULL;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      if (pionshlt->getListSize() <= 1) return t;

      double sedp = -1.;
      const  Particle* t1 = T1(part);
      TLorentzVector V4p1 = t1->get4Vector();
      double topp = (PCmsLabTransform::labToCms(V4p1)).Rho();
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        const Particle* t2 = pionshlt->getParticle(i);
        TLorentzVector V4p2 = t2->get4Vector();
        double mom = (PCmsLabTransform::labToCms(V4p2)).Rho();
        if (mom > sedp && mom < topp)
        {t = t2; sedp = mom;}
      }
      return t;
    }


    double VisibleEnergyLE(const Particle*)
    {
      double result = 0.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        result += p->getMomentumMagnitude();
      }

      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      for (unsigned int i = 0; i < gammahlt->getListSize(); i++) {
        Particle* p = gammahlt->getParticle(i);
        result += p->getMomentumMagnitude();
      }
      return result;
    }


    double VisiblePzLE(const Particle*)
    {
      double result = 0.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        result += std::abs(p->getPz());
      }

      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      for (unsigned int i = 0; i < gammahlt->getListSize(); i++) {
        Particle* p = gammahlt->getParticle(i);
        result += std::abs(p->getPz());
      }

      return result;

    }

    double AngleTTLE(const Particle* p)
    {
      double result = -10.;
      if (T1(p) && T2(p)) {
        TLorentzVector V4p1 = (T1(p))->get4Vector();
        TLorentzVector V4p2 = (T2(p))->get4Vector();
        const TVector3 V3p1 =  PCmsLabTransform::labToCms(V4p1).Vect();
        const TVector3 V3p2 =  PCmsLabTransform::labToCms(V4p2).Vect();
        result = V3p1.Angle(V3p2);
      }
      return result;
    }

    double ThetaTTLE(const Particle* p)
    {
      double result = -1.;
      if (T1(p) && T2(p)) {
        TLorentzVector V4p1 = (T1(p))->get4Vector();
        TLorentzVector V4p2 = (T2(p))->get4Vector();
        const TVector3 V3p1 =  PCmsLabTransform::labToCms(V4p1).Vect();
        const TVector3 V3p2 =  PCmsLabTransform::labToCms(V4p2).Vect();

        result = std::abs(V3p1.Theta() + V3p2.Theta() - M_PI);
      }
      return result;
    }

    double PhiTTLE(const Particle* p)
    {
      double result = -1.;
      if (T1(p) && T2(p)) {
        TLorentzVector V4p1 = (T1(p))->get4Vector();
        TLorentzVector V4p2 = (T2(p))->get4Vector();
        const TVector3 V3p1 =  PCmsLabTransform::labToCms(V4p1).Vect();
        const TVector3 V3p2 =  PCmsLabTransform::labToCms(V4p2).Vect();
        result = V3p1.DeltaPhi(V3p2);
      }
      return result;
    }


    double P1CMSBhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T1(p)) {
        TLorentzVector V4p1 = (T1(p))->get4Vector();

        result = (PCmsLabTransform::labToCms(V4p1)).Rho();
        //result = (T1(p))->getMomentumMagnitude();
      }
      return result;
    }


    double P1BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T1(p))
        result = (T1(p))->getMomentumMagnitude();
      return result;
    }

    double Pt1BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T1(p)) {
        TVector3 v3 = (T1(p))->getMomentum();
        result = v3.Pt();
      }
      return result;
    }

    double Theta1BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T1(p))
        result = ((T1(p))->getMomentum()).Theta();
      return result;
    }

    double MinusThetaBhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T1(p) && (T1(p)->getCharge() == -1))
        result = ((T1(p))->getMomentum()).Theta();
      else if (T2(p) && (T2(p)->getCharge() == -1))
        result = ((T2(p))->getMomentum()).Theta();
      return result;
    }


    double Phi1BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T1(p))
        result = ((T1(p))->getMomentum()).Phi();
      return result;
    }

    double Charge1BhabhaLE(const Particle* p)
    {
      double result = -10;
      if (T1(p))
        result = (T1(p))->getCharge();
      return result;
    }

    double P2CMSBhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T2(p)) {
        TLorentzVector V4p2 = (T2(p))->get4Vector();
        result = (PCmsLabTransform::labToCms(V4p2)).Rho();
      }
      return result;
    }

    double P12CMSBhabhaLE(const Particle* p)
    {
      return P1CMSBhabhaLE(p) + P2CMSBhabhaLE(p);
    }



    double P2BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T2(p))
        result = (T2(p))->getMomentumMagnitude();
      return result;
    }

    double Pt2BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T2(p)) {
        TVector3 v3 = (T2(p))->getMomentum();
        result = v3.Pt();
      }
      return result;
    }

    double Theta2BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T2(p))
        result = ((T2(p))->getMomentum()).Theta();
      return result;
    }

    double Phi2BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T2(p))
        result = ((T2(p))->getMomentum()).Phi();
      return result;
    }

    double Charge2BhabhaLE(const Particle* p)
    {
      double result = -10;
      if (T2(p))
        result = (T2(p))->getCharge();
      return result;
    }


    double E1BhabhaLE(const Particle* p)
    {
      double result = -1.;
      const Particle* pp = T1(p);
      if (pp) {
        const ECLCluster* eclTrack = (pp->getTrack())->getRelated<ECLCluster>();
        if (eclTrack)
          result = eclTrack->getEnergy();
      }
//if(T1(p)&&T1(p)->getECLCluster())
//result = T1(p)->getECLCluster()->getEnergy();
      return result;
    }

    double E1CMSBhabhaLE(const Particle* p)
    {
      double result = -1.;
      const Particle* pp = T1(p);
      if (pp) {
        const ECLCluster* eclTrack = (pp->getTrack())->getRelated<ECLCluster>();
        if (eclTrack) {
          ClusterUtils C;
          TLorentzVector V4p1 = C.Get4MomentumFromCluster(eclTrack);
          result = (PCmsLabTransform::labToCms(V4p1)).E();

        }
      }
      return result;
    }

    double EoPT1BhabhaLE(const Particle* p)
    {
      double result = -1.;
      const Particle* pp = T1(p);
      if (pp) {
        double mom = pp->getMomentumMagnitude();
        const ECLCluster* eclTrack = (pp->getTrack())->getRelated<ECLCluster>();
        if (eclTrack)
          result = eclTrack->getEnergy() / mom;
      }
      return result;
    }


    double EoPT2BhabhaLE(const Particle* p)
    {
      double result = -1.;
      const Particle* pp = T2(p);
      if (pp) {
        double mom = pp->getMomentumMagnitude();
        const ECLCluster* eclTrack = (pp->getTrack())->getRelated<ECLCluster>();
        if (eclTrack)
          result = eclTrack->getEnergy() / mom;
      }
      return result;
    }

    double E2BhabhaLE(const Particle* p)
    {
      double result = -1.;
      const Particle* pp = T2(p);
      if (pp) {
        const ECLCluster* eclTrack = (pp->getTrack())->getRelated<ECLCluster>();
        if (eclTrack)
          result = eclTrack->getEnergy();
      }

//if(T2(p)&&T2(p)->getECLCluster())
//result = T2(p)->getECLCluster()->getEnergy();
      return result;
    }

    double E2CMSBhabhaLE(const Particle* p)
    {
      double result = -1.;
      const Particle* pp = T2(p);
      if (pp) {
        const ECLCluster* eclTrack = (pp->getTrack())->getRelated<ECLCluster>();
        if (eclTrack) {
          ClusterUtils C;
          TLorentzVector V4p2 = C.Get4MomentumFromCluster(eclTrack);
          result = (PCmsLabTransform::labToCms(V4p2)).E();
        }
      }
      return result;
    }

    double Layer1BhabhaLE(const Particle* p)
    {
      double result = -1.;
      const Particle* pp = T1(p);
      if (pp) {
        const KLMCluster* klmTrack = (pp->getTrack())->getRelatedTo<KLMCluster>();
        if (klmTrack)
          result = klmTrack->getLayers();
      }
      return result;
    }

    double Layer2BhabhaLE(const Particle* p)
    {
      double result = -1.;
      const Particle* pp = T2(p);
      if (pp) {
        const KLMCluster* klmTrack = (pp->getTrack())->getRelated<KLMCluster>();
        if (klmTrack)
          result = klmTrack->getLayers();
      }
      return result;
    }



    double nClustersLE(const Particle*)
    {
      double nClus = 0;
      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      nClus += gammahlt->getListSize();

      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        const Track* trk = p->getTrack();
        const ECLCluster* eclTrack = trk->getRelated<ECLCluster>();
        if (eclTrack)nClus++;

      }

      return nClus;
    }


    double nKLMClustersLE(const Particle*)
    {
      StoreArray<KLMCluster> klmClusters;
      return klmClusters.getEntries();
    }

    double LayerKLMCluster1LE(const Particle*)
    {
      double result = -1.;
      StoreArray<KLMCluster> klmClusters;
      for (int i = 0; i < klmClusters.getEntries(); i++) {
        double tmp = klmClusters[i]->getLayers();
        if (result < tmp) result = tmp;
      }
      return result;
    }

    double LayerKLMCluster2LE(const Particle*)
    {
      double result = -1.;
      StoreArray<KLMCluster> klmClusters;
      int tmpi = -1;
      for (int i = 0; i < klmClusters.getEntries(); i++) {
        double tmp = klmClusters[i]->getLayers();
        if (result < tmp) {result = tmp; tmpi = i;}
      }

      result = -1.;
      for (int i = 0; i < klmClusters.getEntries(); i++) {
        double tmp = klmClusters[i]->getLayers();
        if (result < tmp && tmpi != i) {result = tmp;}
      }
      return result;
    }



    double EtotLE(const Particle*)
    {
      double result = 0.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        /*
        const ECLCluster* cluster = p->getECLCluster();
        if(cluster){
        B2DEBUG(19,"++++"<<"find track Cluster"<<"++++");
        const Track* track = cluster->getRelated<Track>();
        if(track)
        B2DEBUG(19,"++++"<<"Track Match Cluster"<<"++++");

        }
        */
        const Track* trk = p->getTrack();
        const ECLCluster* eclTrack = trk->getRelated<ECLCluster>();
        if (!eclTrack) continue;
        if (eclTrack->getEnergy() > 0.1)
          result += eclTrack->getEnergy();
//     B2DEBUG(19,"======="<<"Match Cluster"<<"=======");
      }

      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      for (unsigned int i = 0; i < gammahlt->getListSize(); i++) {
        Particle* p = gammahlt->getParticle(i);
        result += p->getEnergy();
      }
      return result;
    }

    double maxAngleTTLE(const Particle*)
    {
      double result = -10.;
      double temp = -10.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      if (pionshlt->getListSize() < 2) return result;
      for (unsigned int i = 0; i < pionshlt->getListSize() - 1; i++) {
        Particle* par1 = pionshlt->getParticle(i);
        for (unsigned int j = i + 1; j < pionshlt->getListSize(); j++) {
          Particle* par2 = pionshlt->getParticle(j);
          TLorentzVector V4p1 = par1->get4Vector();
          TLorentzVector V4p2 = par2->get4Vector();
          const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
          const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
          //const TVector3 V3p1 = V4p1.Vect();
          //const TVector3 V3p2 = V4p2.Vect();
          temp = V3p1.Angle(V3p2);
          if (result < temp) result = temp;
        }
      }
      return result;
    }


    double maxPhiTTLE(const Particle*)
    {
      double result = -10.;
      double temp = -10.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      if (pionshlt->getListSize() < 2) return result;
      for (unsigned int i = 0; i < pionshlt->getListSize() - 1; i++) {
        Particle* par1 = pionshlt->getParticle(i);
        for (unsigned int j = i + 1; j < pionshlt->getListSize(); j++) {
          Particle* par2 = pionshlt->getParticle(j);
          TLorentzVector V4p1 = par1->get4Vector();
          TLorentzVector V4p2 = par2->get4Vector();
          const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
          const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
          //const TVector3 V3p1 = V4p1.Vect();
          //const TVector3 V3p2 = V4p2.Vect();
          temp = V3p1.DeltaPhi(V3p2);
          if (result < temp) result = temp;
        }
      }
      return result;
    }


    double maxThetaTTLE(const Particle*)
    {
      double result = -10.;
      double temp = -10.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      if (pionshlt->getListSize() < 2) return result;
      for (unsigned int i = 0; i < pionshlt->getListSize() - 1; i++) {
        Particle* par1 = pionshlt->getParticle(i);
        for (unsigned int j = i + 1; j < pionshlt->getListSize(); j++) {
          Particle* par2 = pionshlt->getParticle(j);
          TLorentzVector V4p1 = par1->get4Vector();
          TLorentzVector V4p2 = par2->get4Vector();
          const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
          const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
          //const TVector3 V3p1 = V4p1.Vect();
          //const TVector3 V3p2 = V4p2.Vect();
          temp = std::abs(V3p1.Theta() + V3p2.Theta() - M_PI);
          if (result < temp) result = temp;
        }
      }
      return result;
    }

    double maxAngleMMLE(const Particle*)
    {
      double result = -10.;
      StoreArray<KLMCluster> klmClusters;
      for (int i = 0; i < klmClusters.getEntries() - 1; i++) {
        TLorentzVector v1 = klmClusters[i]->getMomentum();
        for (int j = i + 1; j < klmClusters.getEntries(); j++) {
          TLorentzVector v2 = klmClusters[j]->getMomentum();
          double temp = (v1.Vect()).Angle(v2.Vect());
          if (result < temp) result = temp;
        }
      }
      return result ;
    }

    double maxAngleTMLE(const Particle*)
    {
      double result = -10.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      StoreArray<KLMCluster> klmClusters;
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        TLorentzVector v1 = p->get4Vector();
        for (int j = 0; j < klmClusters.getEntries(); j++) {
          TLorentzVector v2 = klmClusters[j]->getMomentum();
          double temp = (v1.Vect()).Angle(v2.Vect());
          if (result < temp) result = temp;
        }
      }
      return result ;
    }

    const ECLCluster*  ECLClusterNeutralLE(const Particle*)
    {
      const ECLCluster*  ecle = NULL;
      double E1 = -1.;
      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      ClusterUtils C;
      for (unsigned  int i = 0; i < gammahlt->getListSize(); i++) {
        Particle* p;
        const ECLCluster* ecle_tmp;
        p = gammahlt->getParticle(i);
        ecle_tmp = p->getECLCluster();
        TLorentzVector V4p = C.Get4MomentumFromCluster(ecle_tmp);
        double e = (PCmsLabTransform::labToCms(V4p)).Rho();
        if (E1 < e) {E1 = e; ecle = ecle_tmp;}
      }
      return ecle;
    }

    double  ENeutralLE(const Particle* particle)
    {
      if (ECLClusterNeutralLE(particle))
        return ECLClusterNeutralLE(particle)->getEnergy();
      else return -1.;
    }

    double  ENeutralCMSLE(const Particle* particle)
    {
      double result = -1.;
      const ECLCluster* cc = ECLClusterNeutralLE(particle);
      if (cc) {
        ClusterUtils C;
        TLorentzVector VNe = C.Get4MomentumFromCluster(cc);
        result = (PCmsLabTransform::labToCms(VNe)).E();
      }
      return result;
    }

    double  ThetaNeutralLE(const Particle* particle)
    {
      if (ECLClusterNeutralLE(particle))
        return ECLClusterNeutralLE(particle)->getTheta();
      else return -1.;
    }

    double  PhiNeutralLE(const Particle* particle)
    {
      if (ECLClusterNeutralLE(particle))
        return ECLClusterNeutralLE(particle)->getPhi();
      else return -1.;
    }

    double RNeutralLE(const Particle* particle)
    {
      if (ECLClusterNeutralLE(particle))
        return ECLClusterNeutralLE(particle)->getR();
      else return -1.;
    }

    const ECLCluster*  ECLClusterC1LE(const Particle*)
    {
      const ECLCluster*  ecle = NULL;
      double E1 = -1.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      ClusterUtils C;
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
        TLorentzVector V4p1 = C.Get4MomentumFromCluster(ecle_tmp);
        double e = (PCmsLabTransform::labToCms(V4p1)).Rho();
        if (E1 < e) {E1 = e; ecle = ecle_tmp;}
      }
      return ecle;
    }

    double EC1LE(const Particle* particle)
    {
      if (ECLClusterC1LE(particle))
        return ECLClusterC1LE(particle)->getEnergy();
      else return -1.;
    }

    double EC1CMSLE(const Particle* p)
    {
      double result = -1.;
      if (ECLClusterC1LE(p)) {
        ClusterUtils C;
        TLorentzVector V4p2 = C.Get4MomentumFromCluster(ECLClusterC1LE(p));
        result = (PCmsLabTransform::labToCms(V4p2)).Rho();
      }
      return result;
    }


    double EC12LE(const Particle* particle)
    {
      return EC1LE(particle) + EC2LE(particle);
    }

    double ThetaC1LE(const Particle* particle)
    {
      if (ECLClusterC1LE(particle))
        return ECLClusterC1LE(particle)->getTheta();
      else return -1.;
    }

    double PhiC1LE(const Particle* particle)
    {
      if (ECLClusterC1LE(particle))
        return ECLClusterC1LE(particle)->getPhi();
      else return -1.;
    }

    double RC1LE(const Particle* particle)
    {
      if (ECLClusterC1LE(particle))
        return ECLClusterC1LE(particle)->getR();
      else return -1.;
    }


    const ECLCluster*  ECLClusterC2LE(const Particle* particle)
    {
      const ECLCluster*  ecle = NULL;
      double result = -1.;
      double Ehigh = -1.;
      if (ECLClusterC1LE(particle)) Ehigh = EC1CMSLE(particle);
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
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
        ClusterUtils C;
        TLorentzVector V4p2 = C.Get4MomentumFromCluster(ecle_tmp);
        double e = (PCmsLabTransform::labToCms(V4p2)).Rho();
        if (e >= Ehigh) continue;
        else if (result < e) {result = e; ecle = ecle_tmp;}
      }
      return ecle;

    }


    double EC2LE(const Particle* particle)
    {
      if (ECLClusterC2LE(particle))
        return ECLClusterC2LE(particle)->getEnergy();
      else return -1.;
    }


    double EC2CMSLE(const Particle* p)
    {
      double result = -1.;
      if (ECLClusterC2LE(p)) {
        ClusterUtils C;
        TLorentzVector V4p2 = C.Get4MomentumFromCluster(ECLClusterC2LE(p));
        result = (PCmsLabTransform::labToCms(V4p2)).Rho();
      }
      return result;
    }

    double EC12CMSLE(const Particle* p)
    {
      return EC1CMSLE(p) + EC2CMSLE(p);
    }


    double ThetaC2LE(const Particle* particle)
    {
      if (ECLClusterC2LE(particle))
        return ECLClusterC2LE(particle)->getTheta();
      else return -1.;
    }

    double PhiC2LE(const Particle* particle)
    {
      if (ECLClusterC2LE(particle))
        return ECLClusterC2LE(particle)->getPhi();
      else return -1.;
    }

    double RC2LE(const Particle* particle)
    {
      if (ECLClusterC2LE(particle))
        return ECLClusterC2LE(particle)->getR();
      else return -1.;
    }

    double AngleGGLE(const Particle* particle)
    {
      double result = -10.;
      if (ECLClusterC1LE(particle) && ECLClusterC2LE(particle)) {
        const ECLCluster* E1 =  ECLClusterC1LE(particle);
        const ECLCluster* E2 =  ECLClusterC2LE(particle);
        ClusterUtils C;
        TLorentzVector V4p1 = C.Get4MomentumFromCluster(E1);
        TLorentzVector V4p2 = C.Get4MomentumFromCluster(E2);
        const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
        const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();

        result = V3p1.Angle(V3p2);
      }
      return result;
    }

    double AngleGTLE(const Particle* particle)
    {
      double result = -10.;

      if (ECLClusterNeutralLE(particle)) {
        ClusterUtils C;
        TLorentzVector V4g1 = C.Get4MomentumFromCluster(ECLClusterNeutralLE(particle));
        //const TVector3 V3g1 = (PCmsLabTransform::labToCms(V4g1)).Vect();
        if (T1(particle)) {
          TLorentzVector V4p1 = T1(particle)->get4Vector();
          //  const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
          double theta1 = (V4g1.Vect()).Angle(V4p1.Vect());
          if (result < theta1) result = theta1;
        }
        if (T2(particle)) {
          TLorentzVector V4p2 = T2(particle)->get4Vector();
          //const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
          double theta2 = (V4g1.Vect()).Angle(V4p2.Vect());
          if (result < theta2) result = theta2;
        }
      }
      return result ;
    }


    double AngleG1T1LE(const Particle* particle)
    {
      double result = -10.;

      if (ECLClusterNeutralLE(particle)) {
        ClusterUtils C;
        TLorentzVector V4g1 = C.Get4MomentumFromCluster(ECLClusterNeutralLE(particle));
        //const TVector3 V3g1 = (PCmsLabTransform::labToCms(V4g1)).Vect();
        if (T1(particle)) {
          TLorentzVector V4p1 = T1(particle)->get4Vector();
          //  const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
          result = (V4g1.Vect()).Angle(V4p1.Vect());
        }
      }
      return result ;
    }


    double AngleG1T2LE(const Particle* particle)
    {
      double result = -10.;

      if (ECLClusterNeutralLE(particle)) {
        ClusterUtils C;
        TLorentzVector V4g1 = C.Get4MomentumFromCluster(ECLClusterNeutralLE(particle));
        //const TVector3 V3g1 = (PCmsLabTransform::labToCms(V4g1)).Vect();
        if (T2(particle)) {
          TLorentzVector V4p2 = T2(particle)->get4Vector();
          //const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
          result = (V4g1.Vect()).Angle(V4p2.Vect());
        }
      }
      return result ;
    }



    double AngleT1C1LE(const Particle* particle)
    {
      double result = -10.;
      if (T1(particle) && ECLClusterC1LE(particle)) {
        TLorentzVector V4p1 = T1(particle)->get4Vector();
        ClusterUtils C;
        TLorentzVector V4e1 =  C.Get4MomentumFromCluster(ECLClusterC1LE(particle));
        //const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
        //const TVector3 V3e1 = (PCmsLabTransform::labToCms(V4e1)).Vect();
        //result = V3p1.Angle(V3e1);
        result = (V4p1.Vect()).Angle(V4e1.Vect());
      }
      return result;
    }

    double AngleT1C2LE(const Particle* particle)
    {
      double result = -10.;
      if (T1(particle) && ECLClusterC2LE(particle)) {
        TLorentzVector V4p1 = T1(particle)->get4Vector();
        const ECLCluster* E2 =  ECLClusterC2LE(particle);
        ClusterUtils C;
        TLorentzVector V4e2 = C.Get4MomentumFromCluster(E2);
//     const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
//    const TVector3 V3e2 = (PCmsLabTransform::labToCms(V4e2)).Vect();
        result = (V4p1.Vect()).Angle(V4e2.Vect());
      }
      return result;
    }

    double AngleT2C1LE(const Particle* particle)
    {
      double result = -10.;
      if (T2(particle) && ECLClusterC1LE(particle)) {
        TLorentzVector V4p2 = T2(particle)->get4Vector();
        const ECLCluster* E1 =  ECLClusterC1LE(particle);
        ClusterUtils C;
        TLorentzVector V4e1 = C.Get4MomentumFromCluster(E1);
//     const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
//    const TVector3 V3e1 = (PCmsLabTransform::labToCms(V4e1)).Vect();
        result = (V4p2.Vect()).Angle(V4e1.Vect());
      }
      return result;
    }


    double AngleT2C2LE(const Particle* particle)
    {
      double result = -10.;
      if (T2(particle) && ECLClusterC2LE(particle)) {
        TLorentzVector V4p2 = T2(particle)->get4Vector();
        const ECLCluster* E2 =  ECLClusterC2LE(particle);
        ClusterUtils C;
        TLorentzVector V4e2 = C.Get4MomentumFromCluster(E2);
//     const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
//     const TVector3 V3e2 = (PCmsLabTransform::labToCms(V4e2)).Vect();
        result = (V4p2.Vect()).Angle(V4e2.Vect());
      }
      return result;
    }

    double nEidLE(const Particle*)
    {
      int result = 0;

      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        double mom  = p->getMomentumMagnitude();
        TLorentzVector V4p1 = p->get4Vector();
        double Pcms = (PCmsLabTransform::labToCms(V4p1)).Rho();
        const ECLCluster* eclTrack = (p->getTrack())->getRelated<ECLCluster>();
        double eop = -1.;
        if (eclTrack)
          eop = eclTrack->getEnergy() / mom;
        if (Pcms > 5.0 && eop > 0.8) result++;
      }
      return result;
    }



    //Variables for LE
    VARIABLE_GROUP("PhysicsTrigger");
    REGISTER_VARIABLE("NtHLT",  nTracksLE,  "[Eventbased] the number of good tracks in the event");
    REGISTER_VARIABLE("NltHLT",  nLongTracksLE,  "[Eventbased] the number of long good tracks in the event: Pt>0.3 GeV");
    REGISTER_VARIABLE("NtcHLT",  nECLMatchTracksLE,
                      "[Eventbased] the number of tracks matched to ECL clusters in the event");
    REGISTER_VARIABLE("NtmHLT",  nKLMMatchTracksLE,
                      "[Eventbased] the number of tracks matched to KLM clusters in the event");
    REGISTER_VARIABLE("NEidHLT",  nEidLE,
                      "[Eventbased] the number of tracks which are identified as e: Pcms>5.0 GeV and E/P>0.8");
    REGISTER_VARIABLE("PT1HLT",  P1BhabhaLE,
                      "[Eventbased] the momentum of trk1, where trk1 is the track with the largest momentum");
    REGISTER_VARIABLE("PT2HLT",  P2BhabhaLE,
                      "[Eventbased] the momentum of trk2, where trk2 is the track with the secondary largest momentum");
    REGISTER_VARIABLE("PT1CMSHLT",  P1CMSBhabhaLE,
                      "[Eventbased] the momentum of trk1 in e+e- center of mass system (CMS)");
    REGISTER_VARIABLE("PT2CMSHLT",  P2CMSBhabhaLE,
                      "[Eventbased] the momentum of trk2 in CMS");
    REGISTER_VARIABLE("PT12CMSHLT",  P12CMSBhabhaLE,
                      "[Eventbased] the total momentum of trk2 and trk1 in CMS");
    REGISTER_VARIABLE("PtT1HLT",  Pt1BhabhaLE,
                      "[Eventbased] the transverse momentum of trk1");
    REGISTER_VARIABLE("PtT2HLT",  Pt2BhabhaLE,
                      "[Eventbased] the transverse momentum of trk2");

    REGISTER_VARIABLE("ThetaT1HLT",  Theta1BhabhaLE,  "[Eventbased] the polar angle of trk1");
    REGISTER_VARIABLE("ThetaT2HLT",  Theta2BhabhaLE,  "[Eventbased] the polar angle of trk2");
    REGISTER_VARIABLE("MinusThetaHLT",  MinusThetaBhabhaLE,  "[Eventbased] the polar angle of trk with negative charge");

    REGISTER_VARIABLE("PhiT1HLT",  Phi1BhabhaLE,  "[Eventbased] the azimuthal angle of trk1");
    REGISTER_VARIABLE("PhiT2HLT",  Phi2BhabhaLE,  "[Eventbased] the azimuthal angle of trk2");
    REGISTER_VARIABLE("ChargeT1HLT",  Charge1BhabhaLE,  "[Eventbased] the charge of trk1");
    REGISTER_VARIABLE("ChargeT2HLT",  Charge2BhabhaLE,  "[Eventbased] the charge of trk2");

    REGISTER_VARIABLE("ET1HLT",  E1BhabhaLE,  "[Eventbased] the energy of ECL cluster matched to trk1");
    REGISTER_VARIABLE("ET1CMSHLT",  E1CMSBhabhaLE,  "[Eventbased] the energy of ECL cluster matched to trk1 in CMS");
    REGISTER_VARIABLE("ET2HLT",  E2BhabhaLE,  "[Eventbased] the energy of ECL cluster matched to trk2");
    REGISTER_VARIABLE("ET2CMSHLT",  E2CMSBhabhaLE,  "[Eventbased] the energy of ECL cluster matched to trk2 in CMS");
    REGISTER_VARIABLE("EoPT1HLT",  EoPT1BhabhaLE,
                      "[Eventbased] the energy of ECL cluster matched to trk1 over the momentum of trak1");
    REGISTER_VARIABLE("EoPT2HLT",  EoPT2BhabhaLE,
                      "[Eventbased] the energy of ECL cluster matched to trk2 over the momentum of trak2");

    REGISTER_VARIABLE("LayersT1HLT",  Layer1BhabhaLE,  "[Eventbased] the layers of KLM cluster matched to trk1");
    REGISTER_VARIABLE("LayersT2HLT",  Layer2BhabhaLE,  "[Eventbased] the layers of KLM cluster matched to trk2");

    REGISTER_VARIABLE("NcHLT",  nClustersLE,  "[Eventbased] the number of ECL clusters");
    REGISTER_VARIABLE("EtotHLT", EtotLE, "[Eventbased] the total energy of ECL clusters");
    REGISTER_VARIABLE("VisiblePzHLT", VisiblePzLE, "[Eventbased] the sum of the absolute momentum in z-coordinate in the event");

    REGISTER_VARIABLE("VisibleEnergyHLT", VisibleEnergyLE,
                      "[Eventbased] total visible energy (the sum of the charged track's momenta and the ECL cluster's energies)");
    REGISTER_VARIABLE("maxAngTTCMSHLT", maxAngleTTLE,
                      "[Eventbased] the maximum angle between two charged tracks in CMS");
    REGISTER_VARIABLE("maxPhiTTCMSHLT", maxPhiTTLE,
                      "[Eventbased] the maximum azimuthal angle between two charged tracks in CMS");
    REGISTER_VARIABLE("maxThetaTTCMSHLT", maxThetaTTLE,
                      "[Eventbased] the maximum accollinearity angle between two charged tracks in CMS");

    REGISTER_VARIABLE("AngTTCMSHLT", AngleTTLE,
                      "[Eventbased] the 3-D angle between trk1 and trk2 in CMS");
    REGISTER_VARIABLE("PhiTTCMSHLT", PhiTTLE,
                      "[Eventbased] the azimuthal angle between trk1 and trk2 in CMS");
    REGISTER_VARIABLE("ThetaTTCMSHLT", ThetaTTLE,
                      "[Eventbased] the accollinearity angle between trk1 and trk2 in CMS");

    REGISTER_VARIABLE("maxAngMMHLT", maxAngleMMLE, "[Eventbased] the maximum angle between two KLM Clusters");
    REGISTER_VARIABLE("maxAngTMHLT", maxAngleTMLE, "[Eventbased] the maximum angle between CDC tracks and  KLM Clusters");
    REGISTER_VARIABLE("NkHLT", nKLMClustersLE, "[Eventbased] the number of KLM clusters");
    REGISTER_VARIABLE("Layer1KLMHLT", LayerKLMCluster1LE, "[Eventbased] the largest layers of KLM clusters");
    REGISTER_VARIABLE("Layer2KLMHLT", LayerKLMCluster2LE, "[Eventbased] the secondary largest layers of KLM clusters");
    REGISTER_VARIABLE("EC1HLT", EC1LE, "[Eventbased] the most energetic ECL cluster, the cluster is denoted as C1");
    REGISTER_VARIABLE("EC1CMSHLT", EC1CMSLE, "[Eventbased] the most energetic ECL cluster in CMS, the cluster is denoted as C1");
    REGISTER_VARIABLE("ThetaC1HLT", ThetaC1LE, "[Eventbased] the pular angle of C1");
    REGISTER_VARIABLE("PhiC1HLT", PhiC1LE, "[Eventbased] the azimuthal angle of C1");
    REGISTER_VARIABLE("RC1HLT", RC1LE, "[Eventbased] the distance between C1 and interation point (IP)");
    REGISTER_VARIABLE("EC2HLT", EC2LE, "[Eventbased] the second most energetic ECL cluster, the cluster is denoted as C2");
    REGISTER_VARIABLE("EC2CMSHLT", EC2CMSLE,
                      "[Eventbased] the secondary most energetic ECL cluster in CMS, the cluster is denoted as C2");
    REGISTER_VARIABLE("ThetaC2HLT", ThetaC2LE, "[Eventbased] the polar of C2");
    REGISTER_VARIABLE("PhiC2HLT", PhiC2LE, "[Eventbased] the azimuthal angle of C2");
    REGISTER_VARIABLE("RC2HLT", RC2LE, "[Eventbased] the distance between C2 and IP");
    REGISTER_VARIABLE("EC12HLT", EC12LE, "[Eventbased] the total energy of C1 and C2");
    REGISTER_VARIABLE("EC12CMSHLT", EC12CMSLE, "[Eventbased] the total energy of C1 and C2 in CMS");
    REGISTER_VARIABLE("AngGGCMSHLT", AngleGGLE, "[Eventbased] the angle between C1 and C2");

    REGISTER_VARIABLE("ThetaNeutralHLT", ThetaNeutralLE, "[Eventbased] the polar of the neutral cluster with the largest energy");
    REGISTER_VARIABLE("RNeutralHLT", RNeutralLE, "[Eventbased] the distance between the neutral cluster and IP");
    REGISTER_VARIABLE("PhiNeutralHLT", PhiNeutralLE, "[Eventbased] the azimuthal angle of the neutral cluster with the largest energy");
    REGISTER_VARIABLE("ENeutralHLT", ENeutralLE, "[Eventbased] the largest energy of the neutral cluster");
    REGISTER_VARIABLE("ENeutralCMSHLT", ENeutralCMSLE, "[Eventbased] the secondary largest energy of the neutral cluster in CMS");

    REGISTER_VARIABLE("AngGTHLT", AngleGTLE, "[Eventbased] the max angle between the largest neutral cluster and T1 (T2) ");
    REGISTER_VARIABLE("AngG1T1HLT", AngleG1T1LE, "[Eventbased] the angle between the largest neutral cluster and T1 ");
    REGISTER_VARIABLE("AngG1T2HLT", AngleG1T2LE, "[Eventbased] the angle between the largest neutral cluster and T2 ");
    REGISTER_VARIABLE("AngC1T1HLT", AngleT1C1LE, "[Eventbased] the angle between T1 and C1 ");
    REGISTER_VARIABLE("AngC2T1HLT", AngleT1C2LE, "[Eventbased] the angle between T1 and C2 ");
    REGISTER_VARIABLE("AngC1T2HLT", AngleT2C1LE, "[Eventbased] the angle between T2 and C1 ");
    REGISTER_VARIABLE("AngC2T2HLT", AngleT2C2LE, "[Eventbased] the angle between T2 and C2 ");

  }
}

