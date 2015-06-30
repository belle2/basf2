
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
#include <analysis/VariableManager/L1EmulatorVariables.h>
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
//#include <skim/L1Emulator/modules/L1Emulation/L1EmulationModule.h>
#include <skim/L1Emulator/dataobjects/L1EmulationInformation.h>
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
      StoreObjPtr<ParticleList> pionshlt("pi+:LE");
      return pionshlt->getListSize();
    }



    double nECLMatchTracksLE(const Particle*)
    {
      int result = 0;
      StoreObjPtr<ParticleList> pionshlt("pi+:LE");
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
      StoreObjPtr<ParticleList> pionshlt("pi+:LE");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        if (p->getKLMCluster()) result++;
      }
      return result;
    }

    const Particle*  T1(const Particle*)
    {
      const  Particle* t = NULL;
      StoreObjPtr<ParticleList> pionshlt("pi+:LE");
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
      StoreObjPtr<ParticleList> pionshlt("pi+:LE");
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

    double P1BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T1(p))
        result = (T1(p))->getMomentumMagnitude();
      return result;
    }

    double Theta1BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T1(p))
        result = ((T1(p))->getMomentum()).Theta();
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

    double P2BhabhaLE(const Particle* p)
    {
      double result = -1.;
      if (T2(p))
        result = (T2(p))->getMomentumMagnitude();
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


    double nClustersLE(const Particle*)
    {
      double nClus = 0;
      StoreObjPtr<ParticleList> gammahlt("gamma:LE");
      nClus += gammahlt->getListSize();

      StoreObjPtr<ParticleList> pionshlt("pi+:LE");
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

    double EtotLE(const Particle*)
    {
      double result = 0.;
      StoreObjPtr<ParticleList> pionshlt("pi+:LE");
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

      StoreObjPtr<ParticleList> gammahlt("gamma:LE");
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
      StoreObjPtr<ParticleList> pionshlt("pi+:LE");
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

    const ECLCluster*  ECLClusterC1LE(const Particle*)
    {
      const ECLCluster*  ecle = NULL;
      double E1 = -1.;
      StoreObjPtr<ParticleList> pionshlt("pi+:LE");
      StoreObjPtr<ParticleList> gammahlt("gamma:LE");
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
      StoreObjPtr<ParticleList> pionshlt("pi+:LE");
      StoreObjPtr<ParticleList> gammahlt("gamma:LE");
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

    double eclBhabha(const Particle*)
    {
      double result = 0;
      StoreArray<L1EmulationInformation> LEins;
      result = LEins[0]->getECLBhabha();
      return result;
    }

    double BhabhaVeto(const Particle*)
    {
      double result = 0;
      StoreArray<L1EmulationInformation> LEinsb;
      result = LEinsb[0]->getBhabhaVeto();
      return result;
    }

    double ggVeto(const Particle*)
    {
      double result = 0;
      StoreArray<L1EmulationInformation> LEinsb;
      result = LEinsb[0]->getggVeto();
      return result;
    }


    //Variables for LE
    VARIABLE_GROUP("L1Emulator");
    REGISTER_VARIABLE("NtLE",  nTracksLE,  "[Eventbased] number of tracks in the event after LE good tracks requirment");
    REGISTER_VARIABLE("NtcLE",  nECLMatchTracksLE,
                      "[Eventbased] number of tracks matched to ECL clusters in the event after LE good tracks requirment");
    REGISTER_VARIABLE("NtkLE",  nKLMMatchTracksLE,
                      "[Eventbased] number of tracks matched to KLM clusters in the event after LE good tracks requirment");
    REGISTER_VARIABLE("PT1LE",  P1BhabhaLE,
                      "[Eventbased] the transverse momentum of trk1, where trk1 is the track with the largest momentum");
    REGISTER_VARIABLE("PT2LE",  P2BhabhaLE,
                      "[Eventbased] the transverse momentum of trk2, where trk2 is the track with the largest momentum");

    REGISTER_VARIABLE("ThetaT1LE",  Theta1BhabhaLE,  "[Eventbased] the polar angle of trk1");
    REGISTER_VARIABLE("ThetaT2LE",  Theta2BhabhaLE,  "[Eventbased] the polar angle of trk2");

    REGISTER_VARIABLE("PhiT1LE",  Phi1BhabhaLE,  "[Eventbased] the azimuthal angle of trk1");
    REGISTER_VARIABLE("PhiT2LE",  Phi2BhabhaLE,  "[Eventbased] the azimuthal angle of trk2");
    REGISTER_VARIABLE("ChargeT1LE",  Charge1BhabhaLE,  "[Eventbased] the charge of trk1");
    REGISTER_VARIABLE("ChargeT2LE",  Charge2BhabhaLE,  "[Eventbased] the charge of trk2");

    REGISTER_VARIABLE("ET1LE",  E1BhabhaLE,  "[Eventbased] the energy of ECL cluster matched to trk1");
    REGISTER_VARIABLE("ET2LE",  E2BhabhaLE,  "[Eventbased] the energy of ECL cluster matched to trk2");

    REGISTER_VARIABLE("NcLE",  nClustersLE,  "[Eventbased] number of ECL clusters");
    REGISTER_VARIABLE("EtotLE", EtotLE, "[Eventbased] the total ECL energy");
    REGISTER_VARIABLE("maxAngTTLE", maxAngleTTLE,
                      "[Eventbased] the maximum angle between two charged tracks in e+e- center-of-mass frame");
    REGISTER_VARIABLE("NkLE", nKLMClustersLE, "[Eventbased] the number of KLM clusters");
    REGISTER_VARIABLE("EC1LE", EC1LE, "[Eventbased] the most energetic ECL cluster in the rest frame, the cluster is denoted as C1");
    REGISTER_VARIABLE("ThetaC1LE", ThetaC1LE, "[Eventbased] the pular angle of C1");
    REGISTER_VARIABLE("PhiC1LE", PhiC1LE, "[Eventbased] the azimuthal angle of C1");
    REGISTER_VARIABLE("RC1LE", RC1LE, "[Eventbased] the distance between C1 and IP");
    REGISTER_VARIABLE("EC2LE", EC2LE, "[Eventbased] the second most energetic ECL cluster, the cluster is denoted as C2");
    REGISTER_VARIABLE("ThetaC2LE", ThetaC2LE, "[Eventbased] the polar of C2");
    REGISTER_VARIABLE("PhiC2LE", PhiC2LE, "[Eventbased] the azimuthal angle of C2");
    REGISTER_VARIABLE("RC2LE", RC2LE, "[Eventbased] the distance between C2 and IP");
    REGISTER_VARIABLE("AngGGLE", AngleGGLE, "[Eventbased] the angle between C1 and C2");
    REGISTER_VARIABLE("eclBhabhaVetoLE", eclBhabha, "[Eventbased] the eclBhabha veto");
    REGISTER_VARIABLE("BhabhaVetoLE", BhabhaVeto, "[Eventbased] the Bhabha veto");
    REGISTER_VARIABLE("ggVetoLE", ggVeto, "[Eventbased] the gg Veto");

  }
}

