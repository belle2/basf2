#include <analysis/utility/physics.h>

// generator related
#include <mdst/dataobjects/MCParticle.h>

// framework related
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

TVector3 getCMSBoostVector()
{
  StoreArray<MCParticle> mcParticles;

  for (int i = 0; i < mcParticles.getEntries(); i++) {
    MCParticle* mc = mcParticles[i];

    if (mc->getPDG() == 300553) {
      // get the CMS boost vector
      TLorentzVector cmsLV = mc->get4Vector();
      return -(cmsLV.BoostVector());
    }
  }
  return TVector3(0.0, 0.0, 0.0);
}

double getCMSEnergy()
{
  StoreArray<MCParticle> mcParticles;

  for (int i = 0; i < mcParticles.getEntries(); i++) {
    MCParticle* mc = mcParticles[i];

    if (mc->getPDG() == 300553) {
      TLorentzVector cmsLV = mc->get4Vector();
      TVector3 boostToCMS = -(cmsLV.BoostVector());
      cmsLV.Boost(boostToCMS);
      return cmsLV.E();
    }
  }
  return 0.0;
}


double cosHelicityAngle(TLorentzVector particle, TLorentzVector parent, TLorentzVector grandparent)
{

  TVector3 boosttoparent = -(parent.BoostVector());

  particle.Boost(boosttoparent);
  grandparent.Boost(boosttoparent);

  TVector3 particle3 = particle.Vect();
  TVector3 grandparent3 = grandparent.Vect();

  double numerator = particle3.Dot(grandparent3);
  double denominator = (particle3.Mag()) * (grandparent3.Mag());
  double temp = numerator / denominator;

  return temp;
}

