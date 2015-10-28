
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
      else if (pionshlt->getListSize() == 1)
        t = pionshlt->getParticle(0);
      else if (pionshlt->getListSize() > 1) {
        Particle* t1 = pionshlt->getParticle(0);
        Particle* t2 = pionshlt->getParticle(1);

        if (t1->getMomentumMagnitude() > t2->getMomentumMagnitude())
          t = t1;
        else
          t = t2;
      }
      return t;
    }

    const Particle*  T2(const Particle*)
    {
      const  Particle* t = NULL;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      if (pionshlt->getListSize() <= 1) return t;
      else if (pionshlt->getListSize() > 1) {
        Particle* t1 = pionshlt->getParticle(0);
        Particle* t2 = pionshlt->getParticle(1);

        if (t1->getMomentumMagnitude() > t2->getMomentumMagnitude())
          t = t2;
        else
          t = t1;
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
        const ECLCluster* shower = p->getECLCluster();
        if(shower){
        std::cout<<"++++"<<"find track Cluster"<<"++++"<<std::endl;
        const Track* track = shower->getRelated<Track>();
        if(track)
        std::cout<<"++++"<<"Track Match Cluster"<<"++++"<<std::endl;

        }
        */
        const Track* trk = p->getTrack();
        const ECLCluster* eclTrack = trk->getRelated<ECLCluster>();
        if (!eclTrack) continue;
        if (eclTrack->getEnergy() > 0.1)
          result += eclTrack->getEnergy();
//     std::cout<<"======="<<"Match Cluster"<<"======="<<std::endl;
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
          temp = V3p1.Angle(V3p2);
          if (result < temp) result = temp;
        }
      }
      result = result / 3.1415926 * 180.;
      return result;
    }

    double maxAngleMMLE(const Particle*)
    {
      double result = -1.;
      StoreArray<KLMCluster> klmClusters;
      for (int i = 0; i < klmClusters.getEntries() - 1; i++) {
        TLorentzVector v1 = klmClusters[i]->getMomentum();
        for (int j = i + 1; j < klmClusters.getEntries(); j++) {
          TLorentzVector v2 = klmClusters[j]->getMomentum();
          double temp = (v1.Vect()).Angle(v2.Vect());
          if (result < temp) result = temp;
        }
      }
      return result / 3.1415926 * 180.;
    }

    double maxAngleTMLE(const Particle*)
    {
      double result = -1.;
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
      return result / 3.1415926 * 180.;
    }

    const ECLCluster*  ECLClusterNeutralLE(const Particle*)
    {
      const ECLCluster*  ecle = NULL;
      double E1 = -1.;
      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      for (unsigned  int i = 0; i < gammahlt->getListSize(); i++) {
        Particle* p;
        const ECLCluster* ecle_tmp;
        p = gammahlt->getParticle(i);
        ecle_tmp = p->getECLCluster();
        double e = ecle_tmp->getEnergy();
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
        double e = ecle_tmp->getEnergy();
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
        TLorentzVector V4p2 = (ECLClusterC1LE(p))->get4Vector();
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
      if (ECLClusterC1LE(particle))Ehigh = EC1LE(particle);
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
        double e = ecle_tmp->getEnergy();
        if ((float)Ehigh == (float)e)continue;
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
        TLorentzVector V4p2 = (ECLClusterC2LE(p))->get4Vector();
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
      double result = -1.;
      if (ECLClusterC1LE(particle) && ECLClusterC2LE(particle)) {
        const ECLCluster* E1 =  ECLClusterC1LE(particle);
        const ECLCluster* E2 =  ECLClusterC2LE(particle);
        TVector3 v1 = E1->getMomentum();
        TVector3 v2 = E2->getMomentum();
        TLorentzVector V4p1(v1, v1.Mag());
        TLorentzVector V4p2(v2, v2.Mag());
        const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
        const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();

        result = V3p1.Angle(V3p2);
        result = result / 3.1415926 * 180.;
      }
      return result;
    }

    double AngleGTLE(const Particle* particle)
    {
      double result = -1.;

      if (ECLClusterNeutralLE(particle)) {
        TVector3 cv1 = ECLClusterNeutralLE(particle)->getMomentum();
        if (T1(particle)) {
          TVector3 tv1 = (T1(particle))->getMomentum();
          double theta1 = tv1.Angle(cv1);
          if (result < theta1) result = theta1;
        }
        if (T2(particle)) {
          TVector3 tv2 = (T2(particle))->getMomentum();
          double theta2 = tv2.Angle(cv1);
          if (result < theta2) result = theta2;
        }

      }

      return result / 3.1415926 * 180.;

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
    REGISTER_VARIABLE("NtHLT",  nTracksLE,  "[Eventbased] number of tracks in the event after LE good tracks requirment");
    REGISTER_VARIABLE("NltHLT",  nLongTracksLE,  "[Eventbased] number of long tracks in the event after LE good tracks requirment");
    REGISTER_VARIABLE("NtcHLT",  nECLMatchTracksLE,
                      "[Eventbased] number of tracks matched to ECL clusters in the event after LE good tracks requirment");
    REGISTER_VARIABLE("NtmHLT",  nKLMMatchTracksLE,
                      "[Eventbased] number of tracks matched to KLM clusters in the event after LE good tracks requirment");
    REGISTER_VARIABLE("NEidHLT",  nEidLE,
                      "[Eventbased] number of tracks which are identified as e");
    REGISTER_VARIABLE("PT1HLT",  P1BhabhaLE,
                      "[Eventbased] the  momentum of trk1, where trk1 is the track with the largest momentum");
    REGISTER_VARIABLE("PT2HLT",  P2BhabhaLE,
                      "[Eventbased] the momentum of trk2, where trk2 is the track with the largest momentum");
    REGISTER_VARIABLE("PT1CMSHLT",  P1CMSBhabhaLE,
                      "[Eventbased] the CMS momentum of trk1, where trk1 is the track with the largest momentum");
    REGISTER_VARIABLE("PT2CMSHLT",  P2CMSBhabhaLE,
                      "[Eventbased] the CMS momentum of trk2, where trk2 is the track with the largest momentum");
    REGISTER_VARIABLE("PT12CMSHLT",  P12CMSBhabhaLE,
                      "[Eventbased] the total CMS momentum of trk2 and trk1");
    REGISTER_VARIABLE("PtT1HLT",  Pt1BhabhaLE,
                      "[Eventbased] the transverse momentum of trk1, where trk1 is the track with the largest momentum");
    REGISTER_VARIABLE("PtT2HLT",  Pt2BhabhaLE,
                      "[Eventbased] the transverse momentum of trk2, where trk2 is the track with the largest momentum");

    REGISTER_VARIABLE("ThetaT1HLT",  Theta1BhabhaLE,  "[Eventbased] the polar angle of trk1");
    REGISTER_VARIABLE("ThetaT2HLT",  Theta2BhabhaLE,  "[Eventbased] the polar angle of trk2");
    REGISTER_VARIABLE("MinusThetaHLT",  MinusThetaBhabhaLE,  "[Eventbased] the polar angle of trk with negative charge");

    REGISTER_VARIABLE("PhiT1HLT",  Phi1BhabhaLE,  "[Eventbased] the azimuthal angle of trk1");
    REGISTER_VARIABLE("PhiT2HLT",  Phi2BhabhaLE,  "[Eventbased] the azimuthal angle of trk2");
    REGISTER_VARIABLE("ChargeT1HLT",  Charge1BhabhaLE,  "[Eventbased] the charge of trk1");
    REGISTER_VARIABLE("ChargeT2HLT",  Charge2BhabhaLE,  "[Eventbased] the charge of trk2");

    REGISTER_VARIABLE("ET1HLT",  E1BhabhaLE,  "[Eventbased] the energy of ECL cluster matched to trk1");
    REGISTER_VARIABLE("ET2HLT",  E2BhabhaLE,  "[Eventbased] the energy of ECL cluster matched to trk2");
    REGISTER_VARIABLE("EoPT1HLT",  EoPT1BhabhaLE,
                      "[Eventbased] the energy of ECL cluster matched to trk1 over the lab momentum of trak1");
    REGISTER_VARIABLE("EoPT2HLT",  EoPT2BhabhaLE,
                      "[Eventbased] the energy of ECL cluster matched to trk2 over the lab momentum of trak2");

    REGISTER_VARIABLE("LayersT1HLT",  Layer1BhabhaLE,  "[Eventbased] the layers of KLM cluster matched to trk1");
    REGISTER_VARIABLE("LayersT2HLT",  Layer2BhabhaLE,  "[Eventbased] the layers of KLM cluster matched to trk2");

    REGISTER_VARIABLE("NcHLT",  nClustersLE,  "[Eventbased] number of ECL clusters");
    REGISTER_VARIABLE("EtotHLT", EtotLE, "[Eventbased] the total ECL energy");
    REGISTER_VARIABLE("VisiblePzHLT", VisiblePzLE, "[Eventbased] sum of the absolute momentum in z-coordinate in the event");

    REGISTER_VARIABLE("VisibleEnergyHLT", VisibleEnergyLE,
                      "[Eventbased] total visible energy (the sum of the charged track's momenta and the ECL cluster's energies)");
    REGISTER_VARIABLE("maxAngTTHLT", maxAngleTTLE,
                      "[Eventbased] the maximum angle between two charged tracks in e+e- center-of-mass frame");

    REGISTER_VARIABLE("maxAngMMHLT", maxAngleMMLE, "[Eventbased] the maximum angle between two KLM Clusters");
    REGISTER_VARIABLE("maxAngTMHLT", maxAngleTMLE, "[Eventbased] the maximum angle between CDC tracks and  KLM Clusters");
    REGISTER_VARIABLE("NkHLT", nKLMClustersLE, "[Eventbased] the number of KLM clusters");
    REGISTER_VARIABLE("Layer1KLMHLT", LayerKLMCluster1LE, "[Eventbased] the largest layers of KLM clusters");
    REGISTER_VARIABLE("Layer2KLMHLT", LayerKLMCluster2LE, "[Eventbased] the second largest layers of KLM clusters");
    REGISTER_VARIABLE("EC1HLT", EC1LE, "[Eventbased] the most energetic ECL cluster in the rest frame, the cluster is denoted as C1");
    REGISTER_VARIABLE("EC1CMSHLT", EC1CMSLE, "[Eventbased] the most energetic ECL cluster in CMS, the cluster is denoted as C1");
    REGISTER_VARIABLE("ThetaC1HLT", ThetaC1LE, "[Eventbased] the pular angle of C1");
    REGISTER_VARIABLE("PhiC1HLT", PhiC1LE, "[Eventbased] the azimuthal angle of C1");
    REGISTER_VARIABLE("RC1HLT", RC1LE, "[Eventbased] the distance between C1 and IP");
    REGISTER_VARIABLE("EC2HLT", EC2LE, "[Eventbased] the second most energetic ECL cluster, the cluster is denoted as C2");
    REGISTER_VARIABLE("EC2CMSHLT", EC2CMSLE, "[Eventbased] the second most energetic ECL cluster in CMS, the cluster is denoted as C2");
    REGISTER_VARIABLE("ThetaC2HLT", ThetaC2LE, "[Eventbased] the polar of C2");
    REGISTER_VARIABLE("PhiC2HLT", PhiC2LE, "[Eventbased] the azimuthal angle of C2");
    REGISTER_VARIABLE("RC2HLT", RC2LE, "[Eventbased] the distance between C2 and IP");
    REGISTER_VARIABLE("EC12HLT", EC12LE, "[Eventbased] the total energy of C1 and C2");
    REGISTER_VARIABLE("EC12CMSHLT", EC12CMSLE, "[Eventbased] the total energy of C1 and C2 in CMS");
    REGISTER_VARIABLE("AngGGHLT", AngleGGLE, "[Eventbased] the angle between C1 and C2");

    REGISTER_VARIABLE("ThetaNeutralHLT", ThetaNeutralLE, "[Eventbased] the polar of the neutral cluster with the largest energy");
    REGISTER_VARIABLE("RNeutralHLT", RNeutralLE, "[Eventbased] the distance between the neutral cluster and IP");
    REGISTER_VARIABLE("PhiNeutralHLT", PhiNeutralLE, "[Eventbased] the azimuthal angle of the neutral cluster with the largest energy");
    REGISTER_VARIABLE("ENeutralHLT", ENeutralLE, "[Eventbased] the energy of the neutral cluster with the largest energy");

    REGISTER_VARIABLE("AngGTHLT", AngleGTLE, "[Eventbased] the max angle between C1 (C2) and T1 (T2)");

  }
}

