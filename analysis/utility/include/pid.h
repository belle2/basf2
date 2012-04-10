#ifndef PID_H
#define PID_H

// generator related
#include <generators/dataobjects/MCParticle.h>

// PID related
#include <top/dataobjects/TOPLikelihoods.h>
#include <arich/dataobjects/ARICHLikelihoods.h>

using namespace Belle2;

// returns the pointer to TOPLikelihood fro given MCParticle
const TOPLikelihoods* getTOPLikelihoods(const MCParticle* particle);
// returns the pointer to ARICHLikelihood fro given MCParticle
const ARICHLikelihoods* getARICHLikelihoods(const MCParticle* particle);
// returns the difference between Log likelihoods of 2 hypotheses for given MCParticle based on TOP response
double getTOPPID(int hyp1, int hyp2, const MCParticle* particle);
// returns the difference between Log likelihoods of 2 hypotheses for given MCParticle based on ARICH response
double getARICHPID(int hyp1, int hyp2, const MCParticle* particle);

#endif // PID_H                                                                                                                                         

