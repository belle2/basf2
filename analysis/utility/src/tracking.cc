
#include <analysis/utility/tracking.h>
#include <framework/logging/Logger.h>

int getTrackParametersAtGivenPoint(GFTrack* thisTrack, TVector3 point, TVector3& poca, TVector3& momentum)
{

  if (thisTrack->getCardinalRep()->getStatusFlag() == 0) {
    //direction of the track at the point of closest approach
    TVector3 dirInPoca(0., 0., 0.);
    try {
      thisTrack->getCardinalRep()->extrapolateToPoint(point, poca, dirInPoca);
    } catch (...) {
      B2WARNING("Track extrapolation failed!");
      return 0;
    }

    // get track position and momentum in a certain plane
    // define a plane, here just the vectors from above are used
    // if the plane is defined to be parallel to xy plane the result is the same,
    // but it's still not clear which direction should be taken (or if it is important at all)
    GFDetPlane plane(poca, dirInPoca);
    TVector3 resultPosition;
    TMatrixT<double> resultCovariance;

    thisTrack->getPosMomCov(plane, resultPosition, momentum, resultCovariance);
  }
  return 1;
}

int getNumberOfHits(MCParticle* part, RelationArray& relation)
{
  int nMcPartToHits = relation.getEntries();

  for (int i = 0; i < nMcPartToHits; ++i) {
    if (relation[i].getFromIndex() == unsigned(part->getArrayIndex())) {
      return relation[i].getToIndices().size();
    }
  }
  return 0;
}
