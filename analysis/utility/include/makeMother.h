#ifndef MAKEMOTHER_H
#define MAKEMOTHER_H

#include "analysis/particle/Particle.h"

#include <analysis/KFit/MassFitKFit.h>
#include <analysis/KFit/MassVertexFitKFit.h>
#include <analysis/KFit/VertexFitKFit.h>

using namespace Belle2;
using namespace analysis;

/* Updates the momentum of mother particle after vertex fit. */
unsigned makeMother(VertexFitKFit& kv, Particle& mother);

/* Updates the momentum of mother particle after mass fit. */
unsigned makeMother(MassFitKFit& km, Particle& mother);

/* Updates the momentum of mother particle after mass constrained vertex fit. */
unsigned makeMother(MassVertexFitKFit& km, Particle& mother);

#endif // MAKEMOTHER_H 
