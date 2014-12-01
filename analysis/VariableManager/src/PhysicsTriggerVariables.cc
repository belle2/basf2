
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
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

class getRelatedTo;
using namespace std;

namespace Belle2 {
  namespace Variable {


//HLT variables
    double nTracksHLT(const Particle*)
    {
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      return pionshlt->getListSize();

    }

    double nECLClustersHLT(const Particle*)
    {
      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      return gammahlt->getListSize();
    }


    double ECLEnergyHLT(const Particle*)
    {
      double result = 0.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        const Track* trk = p->getTrack();
        const ECLCluster* eclTrack = trk->getRelated<ECLCluster>();
        if (!eclTrack) continue;
        result += eclTrack->getEnergy();

      }

      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      for (unsigned int i = 0; i < gammahlt->getListSize(); i++) {
        Particle* p = gammahlt->getParticle(i);
        result += p->getEnergy();
      }
      return result;
    }



    double VisibleEnergyHLT(const Particle*)
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


    double VisiblePzHLT(const Particle*)
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


    double TrackP1HLT(const Particle*)
    {
      double result = -1.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        if (result < p->getMomentumMagnitude())result = p->getMomentumMagnitude();
      }
      return result;
    }


    double TrackP2HLT(const Particle*)
    {
      double result = -1.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      if (pionshlt->getListSize() < 2) return result;
      Particle* tmp = NULL;
      double Phigh = TrackP1HLT(tmp);
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        double mom = p->getMomentumMagnitude();
        if (mom == Phigh)continue;
        if (result < mom)
          result = mom;
      }

      return result;
    }


    double AngleTTHLT(const Particle*)
    {
      double result = -10.;
      int ind1 = -1;
      int ind2 = -1;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      if (pionshlt->getListSize() < 2) return result;
      Particle* tmp = NULL;
      double P1high = TrackP1HLT(tmp);
      double P2high = TrackP2HLT(tmp);
      for (int i = 0; i < (int)pionshlt->getListSize(); i++) {
        Particle* p1 = pionshlt->getParticle(i);
        double mom = p1->getMomentumMagnitude();
        if (mom == P1high)
          ind1 = i;
        else if (mom == P2high)
          ind2 = i;
      }
      if (ind1 == -1 || ind2 == -1) return result;

      Particle* par1 = pionshlt->getParticle(ind1);
      Particle* par2 = pionshlt->getParticle(ind2);
      TLorentzVector V4p1 = par1->get4Vector();
      TLorentzVector V4p2 = par2->get4Vector();
      const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
      const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
      result = V3p1.Angle(V3p2);
      return result;
    }


    double ECLClusterE1HLT(const Particle*)
    {
      double result = -1.;
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      for (unsigned int i = 0; i < pionshlt->getListSize(); i++) {
        Particle* p = pionshlt->getParticle(i);
        const Track* trk = p->getTrack();
        const ECLCluster* ecle = trk->getRelated<ECLCluster>();
        if (!ecle) { continue;}
        if (result < ecle->getEnergy())result = ecle->getEnergy();
      }

      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      for (unsigned int i = 0; i < gammahlt->getListSize(); i++) {
        Particle* p = gammahlt->getParticle(i);
        if (result < p->getEnergy())result = p->getEnergy();
      }

      return result;
    }


    double ECLClusterE2HLT(const Particle*)
    {
      double result = -1.;
      Particle* tmp = NULL;
      double Ehigh = ECLClusterE1HLT(tmp);
      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      for (unsigned  int i = 0; i < (pionshlt->getListSize() + gammahlt->getListSize()); i++) {
        Particle* p;
        const ECLCluster* ecle;
        if (i < pionshlt->getListSize()) {
          p = pionshlt->getParticle(i);
          const Track* trk = p->getTrack();
          ecle = trk->getRelated<ECLCluster>();
          if (!ecle) { continue;}

        } else {
          p = gammahlt->getParticle(i - pionshlt->getListSize());
          ecle = p->getECLCluster();
        }
        double e = ecle->getEnergy();
        if (Ehigh == e)continue;
        else if (result < e) result = e;
      }

      return result;
    }


    double AngleGGHLT(const Particle*)
    {
      double result = -10.;
      int index_1 = -1;
      int index_2 = -1;
      Particle* tmp = NULL;
      double E1 = ECLClusterE1HLT(tmp);
      double E2 = ECLClusterE2HLT(tmp);

      StoreObjPtr<ParticleList> pionshlt("pi+:HLT");
      StoreObjPtr<ParticleList> gammahlt("gamma:HLT");
      if ((pionshlt->getListSize() + gammahlt->getListSize()) < 2)return result;
      for (int i = 0; i < (int)(pionshlt->getListSize() + gammahlt->getListSize()); i++) {
        Particle* p;
        const ECLCluster* ecle;
        if (i < (int)pionshlt->getListSize()) {
          p = pionshlt->getParticle(i);
          const Track* trk = p->getTrack();
          ecle = trk->getRelated<ECLCluster>();
          if (!ecle) { continue;}
        } else {
          p = gammahlt->getParticle(i - pionshlt->getListSize());
          ecle = p->getECLCluster();
        }
        double e = ecle->getEnergy();
        if (e == E1)index_1 = i;
        else if (e == E2)index_2 = i;
      }
      if (index_1 == -1 || index_2 == -1) return result;

      Particle* par1;
      if (index_1 < (int)pionshlt->getListSize()) par1 = pionshlt->getParticle(index_1);
      else par1 = gammahlt->getParticle(index_1 - (pionshlt->getListSize()));

      Particle* par2;
      if (index_2 < (int)pionshlt->getListSize()) par2 = pionshlt->getParticle(index_2);
      else par2 = gammahlt->getParticle(index_2 - (pionshlt->getListSize()));

      TLorentzVector V4p1 = par1->get4Vector();
      TLorentzVector V4p2 = par2->get4Vector();
      const TVector3 V3p1 = (PCmsLabTransform::labToCms(V4p1)).Vect();
      const TVector3 V3p2 = (PCmsLabTransform::labToCms(V4p2)).Vect();
      result = V3p1.Angle(V3p2);
      return result;
    }

    double ECLClusterTimingHLT(const Particle* particle)
    {
      double result = -1. ;
      if (particle->getParticleType() == Particle::c_Track) {
        const Track* trk = particle->getTrack();
        const ECLCluster* cluster = trk->getRelated<ECLCluster>();
        if (!cluster) { return result;}
        result = cluster->getTiming();
      } else if (particle->getParticleType() == Particle::c_ECLCluster) {
        const ECLCluster* cluster = particle->getECLCluster();
        result = cluster->getTiming();
      }

      return result;
    }

    double ECLClusterEHLT(const Particle* particle)
    {
      double result = -1.0;
      if (particle->getParticleType() == Particle::c_Track) {
        const Track* trk = particle->getTrack();
        const ECLCluster* cluster = trk->getRelated<ECLCluster>();
        if (!cluster) { return result;}
        result = cluster->getEnergy();
      } else if (particle->getParticleType() == Particle::c_ECLCluster) {
        const ECLCluster* cluster = particle->getECLCluster();
        result = cluster->getEnergy();
      }

      return result;
    }


    double ECLClusterThetaHLT(const Particle* particle)
    {
      double result = -10.0;
      if (particle->getParticleType() == Particle::c_Track) {
        const Track* trk = particle->getTrack();
        const ECLCluster* cluster = trk->getRelated<ECLCluster>();
        if (!cluster) { return result;}
        result = cluster->getTheta();
      } else if (particle->getParticleType() == Particle::c_ECLCluster) {
        const ECLCluster* cluster = particle->getECLCluster();
        result = cluster->getTheta();
      }

      return result;
    }


    //Variables for HLT
    VARIABLE_GROUP("PhysicsTrigger");
    REGISTER_VARIABLE("nTracksHLT",  nTracksHLT,  "[Eventbased] number of tracks in the event after HLT good tracks requirment");
    REGISTER_VARIABLE("nECLClustersHLT", nECLClustersHLT, "[Eventbased] number of ECL in the event after HLT good cluster requirment");
    REGISTER_VARIABLE("ECLEnergyHLT", ECLEnergyHLT, "[Eventbased] total deposited energy in ECL in the event");
    REGISTER_VARIABLE("VisibleEnergyHLT", VisibleEnergyHLT, "[Eventbased] total visible energy (the sum of the charged track's momenta and the ECL cluster's energies)");
    REGISTER_VARIABLE("VisiblePzHLT", VisiblePzHLT, "[Eventbased] sum of the absolute momentum in z-coordinate in the event");
    REGISTER_VARIABLE("AngleTTHLT", AngleTTHLT, "[Eventbased] the angle between two charged tracks with the largest momentum in e+e- center-of-mass frame");
    REGISTER_VARIABLE("TrackP1HLT", TrackP1HLT, "[Eventbased] the largest momentum of the charged tracks in the event in the rest frame");
    REGISTER_VARIABLE("TrackP2HLT", TrackP2HLT, "[Eventbased] the second largest momentum of the charged tracks in the event in the rest frame");
    REGISTER_VARIABLE("AngleGGHLT", AngleGGHLT, "[Eventbased] the angle between the two most energetic gamma in ECL in the rest frame");
    REGISTER_VARIABLE("ECLClusterE1HLT", ECLClusterE1HLT, "[Eventbased] the most energetic gamma in the ECL in the rest frame");
    REGISTER_VARIABLE("ECLClusterE2HLT", ECLClusterE2HLT, "[Eventbased] the second most energetic gamma in the ECL in the rest framw");
    REGISTER_VARIABLE("ECLClusterEHLT",     ECLClusterEHLT, "Energy of the ECL clusters");
    REGISTER_VARIABLE("ECLClusterThetaHLT", ECLClusterThetaHLT, " Theta of the ECL clusters");
    REGISTER_VARIABLE("ECLClusterTimingHLT", ECLClusterTimingHLT, "Timing of the ELC clusters ");




  }
}

