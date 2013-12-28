#ifndef TRACKING_H
#define TRACKING_H

// ROOT related
#include <TVector3.h>

// tracking related
#include <genfit/Track.h>

// generator related
#include <mdst/dataobjects/MCParticle.h>

// framework related
#include <framework/datastore/RelationArray.h>

using namespace Belle2;

//get the track parameters (point of closest aproach, poca, and the 3-momentum vector, momenutum) for given GFtrack at arbitrary point
int getTrackParametersAtGivenPoint(genfit::Track* thisTrack, TVector3 point, TVector3& poca, TVector3& momentum);

// returns the number of produced hits (PXD/SVD/CDC) for given MCParticle
int getNumberOfHits(MCParticle* part, RelationArray& relation);

#endif // TRACKING_H
