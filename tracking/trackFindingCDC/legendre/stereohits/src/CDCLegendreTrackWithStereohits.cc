/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/stereohits/CDCLegendreTrackWithStereohits.h>
#include <genfit/TrackCand.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

using namespace Belle2;
using namespace std;
using namespace TrackFindingCDC;

TrackCandidateWithStereoHits::TrackCandidateWithStereoHits(CDCTrack* oldCDCTrack) :
  TrackCandidate(0, 0, 0, std::vector<TrackHit * >()), m_trackCand(nullptr), m_cdcTrack(oldCDCTrack), m_polarAngle(0)
{

  std::vector<TrackHit*>& trackHits = getTrackHits();
  for (const CDCRecoHit3D& recoHit : oldCDCTrack->items()) {
    if (recoHit.getStereoType() == AXIAL)
      trackHits.push_back(new TrackHit(recoHit.getWireHit()));
  }

  double r = 1.5 * 0.00299792458 / oldCDCTrack->getStartTrajectory3D().getTrajectory2D().getAbsMom2D();
  double theta = oldCDCTrack->getStartFitMom3D().phi() - oldCDCTrack->getStartChargeSign() * TMath::Pi() / 2.;
  if (theta < 0.) theta += TMath::Pi() * 2.;

  int charge = oldCDCTrack->getStartChargeSign();

  setCharge(charge);
  setTheta(theta);
  setR(r);
  setReferencePoint(oldCDCTrack->getStartFitPos3D().x(), oldCDCTrack->getStartFitPos3D().y());

  B2DEBUG(100, "R value: " << getR() << "; theta: " << getTheta() << "; radius: " <<
          getRadius() << "; phi: " << getPhi() << "; charge: " << getChargeSign() << "; Xc = "
          << getXc() << "; Yc = " << getYc());
}
