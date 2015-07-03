#include <tracking/dataobjects/RecoTrack.h>

#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;

ClassImp(RecoTrack);

bool RecoTrack::addCDCHit(CDCHit* cdcHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                          double travelS, const TVector3& reconstructedPosition)
{
  if (hasCDCHit(cdcHit)) {
    return false;
  }

  StoreArray<RecoHitInformation> recoHitInformations(m_storeArrayNameOfRecoHitInformation);
  RecoHitInformation* newRecoHitInformation = recoHitInformations.appendNew(cdcHit, rightLeftInformation,
                                              foundByTrackFinder, travelS, reconstructedPosition);

  addCDCHit(cdcHit, newRecoHitInformation);

  return true;
}

bool RecoTrack::addCDCHit(CDCHit* cdcHit, RecoHitInformation* recoHitInformation)
{
  cdcHit->addRelationTo(this);
  addRelationTo(recoHitInformation);

  return true;
}
