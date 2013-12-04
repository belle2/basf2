
#include <analysis/utility/tracking.h>
#include <framework/logging/Logger.h>

int getTrackParametersAtGivenPoint(genfit::Track* thisTrack, TVector3 point, TVector3& poca, TVector3& momentum)
{

  if (thisTrack->getFitStatus()->isFitted()) {
    //direction of the track at the point of closest approach
    genfit::MeasuredStateOnPlane state = thisTrack->getFittedState();
    try {
      state.extrapolateToPoint(point);
    } catch (...) {
      B2WARNING("Track extrapolation failed!");
      return 0;
    }

    // get position and momentum
    state.getPosMom(poca, momentum);
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
