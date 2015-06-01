#include <tracking/trackFindingCDC/legendre/stereohits/CDCLegendreStereohitsProcesser.h>

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorImplementation.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <TFile.h>
#include <TH2F.h>

#define SQR(x) ((x)*(x)) //we will use it in least squares fit

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool StereohitsProcesser::rlWireHitMatchesTrack(const CDCRLWireHit& rlWireHit, const CDCTrajectory2D& trajectory2D)
{
  if (rlWireHit.getStereoType() == AXIAL or rlWireHit.getWireHit().getAutomatonCell().hasTakenFlag())
    return false;

  const Vector2D& center = trajectory2D.getGlobalCircle().center();
  double trackPhi = center.phi();
  double hitPhi = rlWireHit.getRefPos2D().phi();


  double phi_diff = trackPhi - hitPhi;
  if (not std::isnan(phi_diff)) {
    phi_diff = TVector2::Phi_0_2pi(phi_diff);
  }

  int charge = 1;
  if (phi_diff <= TMath::Pi())
    charge = -1;

  if (trajectory2D.getChargeSign() != charge)
    return false;

  return true;
}

void StereohitsProcesser::makeHistogramming(CDCTrack& track)
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  typedef TrackFindingCDC::StereoHitQuadTreeProcessor Processor;
  std::vector<Processor::ReturnList> foundTracks;

  // Ranges: slope, z0
  Processor::ChildRanges ranges(Processor::rangeX(tan(-75.* TMath::Pi() / 180.), tan(75.* TMath::Pi() / 180.)),
                                Processor::rangeY(-20, 20));

  Processor::ReturnList hits_set;
  typedef std::pair<Processor::QuadTree*, Processor::ReturnList> Result;
  std::vector<Result> possibleStereoSegments;

  // Fill in every unused stereo hit. Calculate the correct z-information before.
  for (const CDCRLWireHit& rlWireHit : wireHitTopology.getRLWireHits()) {
    if (rlWireHitMatchesTrack(rlWireHit, trajectory2D)) {
      const CDCWire& wire = rlWireHit.getWire();
      const double forwardZ = wire.getSkewLine().forwardZ();
      const double backwardZ = wire.getSkewLine().backwardZ();

      Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
      if (backwardZ < recoPos3D.z() and recoPos3D.z() < forwardZ) {
        FloatType perpS = trajectory2D.calcPerpS(recoPos3D.xy());
        hits_set.push_back(new CDCRecoHit3D(&(rlWireHit), recoPos3D, perpS));
      }
    }
  }

  Processor::CandidateProcessorLambda lmdCandidateProcessing = [&](const Processor::ReturnList & items,
  Processor::QuadTree * node) -> void {
    possibleStereoSegments.push_back(std::make_pair(node, std::move(items)));
    B2DEBUG(100, "Lambda: " << node->getXMean() << "; Z0: " << node->getYMean() << "; nhits: " << items.size());
  };

  unsigned int level = 6;
  Processor qtProcessor(level, ranges, m_param_debugOutput);
  qtProcessor.provideItemsSet(hits_set);
  qtProcessor.fillGivenTree(lmdCandidateProcessing, 5);

  /* DEBUG */
  if (m_param_debugOutput) {
    // Debug output
    const auto& debugMap = qtProcessor.getDebugInformation();

    TFile file("quadtree_content.root", "RECREATE");
    TH2F hist("hist", "QuadTreeContent", std::pow(2, level), ranges.first.first, ranges.first.second, std::pow(2, level),
              ranges.second.first, ranges.second.second);

    for (const auto& debug : debugMap) {
      const auto& positionInformation = debug.first;
      const auto& quadItemsVector = debug.second;
      hist.Fill(positionInformation.first, positionInformation.second, quadItemsVector.size());
    }

    hist.Write();
    file.Clone();
  }
  /* DEBUG */

  if (possibleStereoSegments.size() == 0)
    return;

  auto maxList = std::max_element(possibleStereoSegments.begin(), possibleStereoSegments.end(), [](const Result & a,
  const Result & b) {
    return a.second.size() < b.second.size();
  });

  // There is the possibility that we have added one cdc hits twice (as left and right one). We search for those cases here:
  Processor::ReturnList& foundStereoHits = maxList->second;
  Processor::QuadTree* node = maxList->first;

  std::vector<CDCRecoHit3D*> doubledRecoHits;
  doubledRecoHits.reserve(foundStereoHits.size() / 2);

  for (Processor::ReturnList::iterator outerIterator = foundStereoHits.begin(); outerIterator != foundStereoHits.end();
       ++outerIterator) {
    const CDCHit* currentHit = (*outerIterator)->getWireHit().getHit();
    for (Processor::ReturnList::iterator innerIterator = foundStereoHits.begin(); innerIterator != outerIterator; ++innerIterator) {
      if (currentHit == (*innerIterator)->getWireHit().getHit()) {
        FloatType lambda11 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node->getYMin());
        FloatType lambda12 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node->getYMax());
        FloatType lambda21 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node->getYMin());
        FloatType lambda22 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node->getYMax());

        if (fabs((lambda11 + lambda12) / 2 - node->getXMean()) < fabs((lambda21 + lambda22) / 2 - node->getXMean())) {
          doubledRecoHits.push_back(*outerIterator);
        } else {
          doubledRecoHits.push_back(*innerIterator);
        }
      }
    }
  }

  foundStereoHits.erase(std::remove_if(foundStereoHits.begin(),
  foundStereoHits.end(), [&doubledRecoHits](CDCRecoHit3D * recoHit3D) -> bool {
    return std::find(doubledRecoHits.begin(), doubledRecoHits.end(), recoHit3D) != doubledRecoHits.end();
  }), foundStereoHits.end());

  for (CDCRecoHit3D* hit : foundStereoHits) {
    if (hit->getWireHit().getAutomatonCell().hasTakenFlag()) B2FATAL("Adding already found hit")
      track.push_back(*hit);
    hit->getWireHit().getAutomatonCell().setTakenFlag();
  }

  for (CDCRecoHit3D* recoHit : hits_set) {
    delete recoHit;
  }
}


void StereohitsProcesser::assignStereohitsByAngle(TrackCandidate* cand, double theta, std::vector<TrackHit*>& stereohits, double Z0)
{


//  return;


  TVector3 momentum = cand->getMomentumEstimation(true);

  int trackCharge;
  int vote_pos(0), vote_neg(0);

  //estimation of charge of the track candidate
  for (TrackHit* Hit : cand->getTrackHits()) {
    int curve_sign = Hit->getCurvatureSignWrt(cand->getXc(), cand->getYc());

    if (curve_sign == TrackCandidate::charge_positive)
      ++vote_pos;
    else if (curve_sign == TrackCandidate::charge_negative)
      ++vote_neg;
    else {
      B2ERROR(
        "Strange behaviour of TrackHit::getCurvatureSignWrt");
      exit(EXIT_FAILURE);
    }
  }

  if (vote_pos > vote_neg)
    trackCharge = TrackCandidate::charge_positive;

  else
    trackCharge = TrackCandidate::charge_negative;

  double Rcand = cand->getRadius();

  for (TrackHit* hit : stereohits) {


    if (hit->getHitUsage() == TrackHit::c_usedInTrack) continue;

    std::pair<StereoHit, StereoHit> stereoHitPair;

    try {
      stereoHitPair = getDisplacements(cand, hit, trackCharge);
    } catch (...) {
      continue;
    }


    //setting Z coordinate according to provided polar angle and comparing updated position of stereohit on consistency with track
    hit->setZReference(stereoHitPair.first.getAlpha() * Rcand / tan(theta) + Z0);
    double dist1 = Rcand - sqrt(SQR(hit->getWirePosition().X() - cand->getXc()) + SQR(hit->getWirePosition().Y() - cand->getYc())) +
                   stereoHitPair.first.getInnerOuter() * hit->getDriftLength();

    hit->setZReference(stereoHitPair.second.getAlpha() * Rcand / tan(theta) + Z0);
    double dist2 = Rcand - sqrt(SQR(hit->getWirePosition().X() - cand->getXc()) + SQR(hit->getWirePosition().Y() - cand->getYc())) +
                   stereoHitPair.second.getInnerOuter() * hit->getDriftLength();


    //TODO: set reasonable values instead of "1.5"
    if (((fabs(dist1) < 1.5) || (fabs(dist2) < 1.5)) && (hit->getZReference() > hit->getBackwardWirePosition().Z()
                                                         && hit->getZReference() < hit->getForwardWirePosition().Z())) {
      cand->addHit(hit);
      hit->setHitUsage(TrackHit::c_usedInTrack);

      //Compare to original CDCHit
//      const CDCSimHit* cdcSimHit = DataStore::getRelatedToObj<CDCSimHit>(hit->getOriginalCDCHit());
//      double zHitOriginal = cdcSimHit->getPosWire().Z();

//      cdcSimHit = nullptr;

//        B2INFO("Delta Z position: " << zHitOriginal - hit->getZReference() << "; Z original: " << zHitOriginal);


    } else {
      //restore default hit coordinates
      hit->setZReference(0);
    }

  }


//  assignStereohitsByAngleWithQuadtree(cand, theta, stereohits, Z0);


}


std::pair<StereoHit, StereoHit> StereohitsProcesser::getDisplacements(TrackCandidate* cand, TrackHit* hit,  int trackCharge)
{

//  double Rcand = cand->getRadius();

  if (trackCharge == 0) {
    int vote_pos(0), vote_neg(0);

    //estimation of charge of the track candidate
    for (TrackHit* Hit : cand->getTrackHits()) {
      int curve_sign = Hit->getCurvatureSignWrt(cand->getXc(), cand->getYc());

      if (curve_sign == TrackCandidate::charge_positive)
        ++vote_pos;
      else if (curve_sign == TrackCandidate::charge_negative)
        ++vote_neg;
      else {
        B2ERROR(
          "Strange behaviour of TrackHit::getCurvatureSignWrt");
        exit(EXIT_FAILURE);
      }
    }

    if (vote_pos > vote_neg)
      trackCharge = TrackCandidate::charge_positive;

    else
      trackCharge = TrackCandidate::charge_negative;
  }


  StereoHit stereoHitOuter = getDisplacement(cand, hit, 1);  // outer: InnerOuter = +1
  StereoHit stereoHitInner = getDisplacement(cand, hit, -1); // outer: InnerOuter = -1


  if (std::isnan(stereoHitInner.getDisplacement()) || std::isnan(stereoHitOuter.getDisplacement()))
    throw "Displacement is NAN!" ;

  if (std::isnan(stereoHitInner.getAlpha()) || std::isnan(stereoHitOuter.getAlpha()))
    throw "Alpha is NAN!" ;

//  double lWire = fabs(hit->getBackwardWirePosition().Z() - hit->getForwardWirePosition().Z());
  double rWire = sqrt(SQR(hit->getBackwardWirePosition().x() - hit->getForwardWirePosition().x()) + SQR(
                        hit->getBackwardWirePosition().y() - hit->getForwardWirePosition().y()));

  if ((fabs(stereoHitInner.getDisplacement()) > rWire) && (fabs(stereoHitOuter.getDisplacement()) > rWire))
    throw "Displacement exceeds wire lenght of wire projection!" ;

  return std::make_pair(stereoHitOuter, stereoHitInner);

}


StereoHit StereohitsProcesser::getDisplacement(TrackCandidate* cand, TrackHit* hit, int InnerOuter)
{
  double Rcand = cand->getRadius();

  //-------- drift time correction factor! should be discussed --------
  double driftLenghtCorr = fabs(hit->getBackwardWirePosition().Z() - hit->getForwardWirePosition().Z()) / sqrt(SQR(
                             hit->getBackwardWirePosition().Z() - hit->getForwardWirePosition().Z()) + SQR(hit->getBackwardWirePosition().x() -
                                 hit->getForwardWirePosition().x()) + SQR(hit->getBackwardWirePosition().y() - hit->getForwardWirePosition().y()));

  //------------------------------
  // here we are solving system of equations:
  // 1. (x-x0)^2 + (y-y0)^2 = R^2 - equation of the circle (track trajectory)
  // 2. ax + by + c = 0 - equation of stereowire projection on xy plane
  // as result we are getting position on track where this stereohit was created (projection of stereohit on track)

  //another approach: instead of wire position take into account driftLength. as result its possible to estimate projection in correct way.
  //but still, we have to estimate angle on track (alpha) where hit was created.
  //if we know alpha we can reestimate projection with driftLength. for this we are taking 2 points on _drift circe_ and repeating procedure of solving our system for that 2 points

  //------------------------------
  // estimating alpha and solving our system, taking into account drift circle

//      double alpha = acos(1. - (SQR(hitProjectionX - cand->getReferencePoint().X()) + SQR(hitProjectionY - cand->getReferencePoint().Y())) / (2.*SQR(Rcand)));

//      tan = cand->getXc() - hitProjectionX /cand->getYc() - hitProjectionY

  // OUTER\INNER - outside or inside of trajectory circle (in some sense - L/R info)

  double A = hit->getForwardWirePosition().Y() - hit->getBackwardWirePosition().Y();
  double B = hit->getBackwardWirePosition().X() - hit->getForwardWirePosition().X();
  double C = hit->getForwardWirePosition().X() * hit->getBackwardWirePosition().Y() - hit->getBackwardWirePosition().X() *
             hit->getForwardWirePosition().Y();

  double x0circle = cand->getXc();
  double y0circle = cand->getYc();

  double R_outer = Rcand + InnerOuter * hit->getDriftLength() * driftLenghtCorr;

  double x1 = 1. / (A * A + B * B) * (-1.*pow(-1.*B * B * (A * A * x0circle * x0circle - A * A * R_outer * R_outer + 2.*A * B *
                                                           x0circle * y0circle + 2.*A * C * x0circle + B * B * y0circle * y0circle - B * B * R_outer * R_outer + 2.*B * C * y0circle + C * C) ,
                                              0.5) - A * B * y0circle - A * C + B * B * x0circle);
  double y1 = 1. / (B * (A * A + B * B)) * (A * pow(-1.*B * B * (A * A * x0circle * x0circle - A * A * R_outer * R_outer + 2.*A * B *
                                                    x0circle * y0circle + 2.*A * C * x0circle + B * B * y0circle * y0circle - B * B * R_outer * R_outer + 2.*B * C * y0circle + C * C) ,
                                                    0.5) + A * A * B * y0circle - A * B * B * x0circle - B * B * C);

  double x2 = 1. / (A * A + B * B) * (pow(-1.*B * B * (A * A * x0circle * x0circle - A * A * R_outer * R_outer + 2.*A * B * x0circle *
                                                       y0circle + 2.*A * C * x0circle + B * B * y0circle * y0circle - B * B * R_outer * R_outer + 2.*B * C * y0circle + C * C) ,
                                          0.5) - A * B * y0circle - A * C + B * B * x0circle);
  double y2 = 1. / (B * (A * A + B * B)) * (-1.*A * pow(-1.*B * B * (A * A * x0circle * x0circle - A * A * R_outer * R_outer + 2.*A *
                                                        B * x0circle * y0circle + 2.*A * C * x0circle + B * B * y0circle * y0circle - B * B * R_outer * R_outer + 2.*B * C * y0circle + C *
                                                        C) , 0.5) + A * A * B * y0circle - A * B * B * x0circle - B * B * C);


  double dist_to_hit1 = sqrt((x1 - hit->getOriginalWirePosition().X()) * (x1 - hit->getOriginalWirePosition().X()) +
                             (y1 - hit->getOriginalWirePosition().Y()) * (y1 - hit->getOriginalWirePosition().Y()));
  double dist_to_hit2 = sqrt((x2 - hit->getOriginalWirePosition().X()) * (x2 - hit->getOriginalWirePosition().X()) +
                             (y2 - hit->getOriginalWirePosition().Y()) * (y2 - hit->getOriginalWirePosition().Y()));

  B2DEBUG(100, "OUTER distances from track to hit: " << dist_to_hit1 << "; " << dist_to_hit2);

  double hitProjectionX(0), hitProjectionY(0), dist_to_hit(0);

  if (dist_to_hit1 < dist_to_hit2) {
    hitProjectionX = x1;
    hitProjectionY = y1;
    dist_to_hit = dist_to_hit1;
  } else {
    hitProjectionX = x2;
    hitProjectionY = y2;
    dist_to_hit = dist_to_hit2;
  }

  double alpha = getAlpha(cand, std::make_pair(hitProjectionX, hitProjectionY));

  double dist = Rcand - sqrt(SQR(hit->getOriginalWirePosition().X() - cand->getXc()) + SQR(hit->getOriginalWirePosition().Y() -
                             cand->getYc())) + InnerOuter * hit->getDriftLength() * driftLenghtCorr;
  dist = dist / fabs(dist) * fabs(dist_to_hit);

  B2DEBUG(100, "A = " << A << "; B = " << B << "; C = " << C << "; x0circle = " << x0circle << "; y0circle = " << y0circle <<
          "; R_outer = " << R_outer);

  return StereoHit(alpha, hitProjectionX, hitProjectionY, InnerOuter, hit, dist);

}


double StereohitsProcesser::getAlpha(TrackCandidate* cand, std::pair<double, double> pos)
{

  double alpha = acos(1. - (SQR(pos.first - cand->getReferencePoint().X()) + SQR(pos.second - cand->getReferencePoint().Y())) /
                      (2.*SQR(cand->getRadius())));

  double hitPhi(0.);
  hitPhi = atan(pos.second / pos.first);

  //distribute the phi values from 0 to 2pi
  if (pos.first >= 0 && pos.second >= 0) {
    hitPhi = atan(pos.second / pos.first);
  }
  if (pos.first < 0) {
    hitPhi = TMath::Pi() + atan(pos.second / pos.first);
  }
  if (pos.first >= 0 && pos.second < 0) {
    hitPhi = 2 * TMath::Pi() + atan(pos.second / pos.first);
  }

//  int charge = cand->getChargeSign();

  double delta_phi_outer = hitPhi - (cand->getPhi() - TMath::Pi() * cand->getChargeSign() / 2.) /*cand->getTheta()*/;
  if (delta_phi_outer > 3.1415) delta_phi_outer -= 2.*3.1415;
  if (delta_phi_outer < -3.1415) delta_phi_outer += 2.*3.1415;

  if (delta_phi_outer * cand->getChargeSign() < 0) alpha = TMath::Pi() * 2. - alpha;

  return alpha;

}
