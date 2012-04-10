#ifndef PHYSICS_H
#define PHYSICS_H

// ROOT related
#include <TLorentzVector.h>
#include <TVector3.h>

// returns the CMS (Y(4S)) boost vector
TVector3 getCMSBoostVector();

// returns the CMS Energy (=sqrt(s))
double getCMSEnergy();

// returns the cosine of the helicity angle
// imputs are the 4-momenta of the particle, it's mother and grandmother
double cosHelicityAngle(TLorentzVector particle, TLorentzVector parent, TLorentzVector grandparent);

#endif // PHYSICS_H   
