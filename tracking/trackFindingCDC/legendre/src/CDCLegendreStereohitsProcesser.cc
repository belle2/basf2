#include <tracking/trackFindingCDC/legendre/CDCLegendreStereohitsProcesser.h>

#define SQR(x) ((x)*(x)) //we will use it in least squares fit

#include <framework/logging/Logger.h>
#include "TH1F.h"
#include "TCanvas.h"

using namespace std;
using namespace Belle2;
using namespace TrackFinderCDCLegendre;

StereohitsProcesser::StereohitsProcesser()
{
//  m_cdcLegendreQuadTree = new QuadTree(-0.15, 0.15,  /*-1.*m_rc, m_rc,*/ 0, 8192, 0, NULL);
//  m_cdcLegendreQuadTree->setLastLevel(12);

}

StereohitsProcesser::~StereohitsProcesser()
{

//  m_cdcLegendreQuadTree->clearTree();
//  QuadTreeCandidateCreator::Instance().clearNodes();
//  QuadTreeCandidateCreator::Instance().clearCandidates();
//  delete m_cdcLegendreQuadTree;

}

void StereohitsProcesser::makeHistogramming(TrackCandidate* cand, std::vector<TrackHit*>& stereohits)
{

  B2INFO("Processing new track; assigning stereohits.");

  double Rcand = cand->getRadius();

  std::vector<std::pair<StereoHit, StereoHit>> hits_to_add;

  double trackCharge;
  int vote_pos(0), vote_neg(0);

  for (TrackHit * Hit : cand->getTrackHits()) {
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



  for (TrackHit * hit : stereohits) {

    if (hit->getHitUsage() == TrackHit::used_in_track) continue;

    std::pair<StereoHit, StereoHit> stereoHitPair;

    try {
      stereoHitPair = getDisplacements(cand, hit, trackCharge);
    } catch (const char* msg) {
      B2DEBUG(100, msg);
      continue;
    }

    double lWire = fabs(hit->getBackwardWirePosition().Z() - hit->getForwardWirePosition().Z());
    double rWire = sqrt(SQR(hit->getBackwardWirePosition().x() - hit->getForwardWirePosition().x()) + SQR(hit->getBackwardWirePosition().y() - hit->getForwardWirePosition().y()));

    int sign = 0;
    int sLayer = hit->getSuperlayerId();
    if (sLayer == 1 || sLayer == 5) sign = -1;
    else sign = 1;

    int sign_phi = 0;

    /*
    if (thetaTrack <= 3.1415 / 2.)
      sign_phi = -1. * cand->getChargeSign();
    else
      sign_phi = 1 * cand->getChargeSign();
    */

    double delta_phi = hit->getPhi() - (cand->getPhi() - m_PI * trackCharge / 2.) /*cand->getTheta()*/;
    if (delta_phi > 3.1415) delta_phi -= 2.*3.1415;
    if (delta_phi < -3.1415) delta_phi += 2.*3.1415;

    sign_phi = delta_phi / fabs(delta_phi) /* cand->getChargeSign()*/;


    double sign_final;
    sign_final = sign_phi * sign;

    if (std::isnan(sign_final)) continue;
    if (std::isnan(delta_phi)) continue;


//      if(alpha > m_PI) sign_final *= -1.;

    double theta1 = atan2(lWire * sign_final * stereoHitPair.first.getDisplacement(), stereoHitPair.first.getAlpha() * Rcand * rWire) + m_PI / 2.;
    double theta2 = atan2(lWire * sign_final * stereoHitPair.second.getDisplacement(), stereoHitPair.second.getAlpha() * Rcand * rWire) + m_PI / 2.;

    stereoHitPair.first.setPolarAngle(theta1);
    stereoHitPair.second.setPolarAngle(theta2);

    B2DEBUG(100, "SLayerID: " << sLayer <<  "; delta_phi: " << delta_phi << "; alpha: " <<  stereoHitPair.first.getAlpha() << "; sign_final: " << sign_final << "; lWire: " << lWire  << "; rWire: " << rWire << ";   Dist_1: " <<  stereoHitPair.first.getDisplacement() << "; Dist_2: " << stereoHitPair.second.getDisplacement() << "; theta1: " << theta1 * 180. / 3.1415 << "; theta2: " << theta2 * 180. / 3.1415);

    hits_to_add.push_back(stereoHitPair);

  }


  int nbins = 60;
  double thetaMin = 15.;
  double thetaMax = 170.;
  double binWidth = m_PI / static_cast<double>(nbins);

//    TCanvas canv1("canv1", "canv", 0, 0, 1400, 600);
//    canv1.Divide(2, 3);

  TH1F hist("hist", "hist", nbins, thetaMin, thetaMax);
//      hist.GetYaxis()->SetRangeUser(0,20);
//      hist.SetLineColor(ii);    //1 - black; 3 - green; 5 - yellow; 7 - blue
//      if(ii==1) hist.DrawClone("E1");
//      else hist.DrawClone("E1 same");
//      hist.Clear();
//    }



//    canv1.cd(1);

  /*

  TH1F hist1("hist1", "hist", nbins, thetaMin, thetaMax);
  for (auto entry : hits_to_add) {
    if (entry.first->getSuperlayerId() != 1) continue;
    double theta1 = entry.second.first;
    double theta2 = entry.second.second;
    hist1.Fill(theta1 * 180. / 3.1415);
    hist1.Fill(theta2 * 180. / 3.1415);

  }

  //    hist1.Draw("E1");

  //    canv1.cd(2);

  TH1F hist2("hist2", "hist", nbins, thetaMin, thetaMax);
  for (auto entry : hits_to_add) {
    if (entry.first->getSuperlayerId() != 3) continue;
    double theta1 = entry.second.first;
    double theta2 = entry.second.second;
    hist2.Fill(theta1 * 180. / 3.1415);
    hist2.Fill(theta2 * 180. / 3.1415);

  }

  //    hist2.Draw("E1");

  //    canv1.cd(3);

  TH1F hist3("hist3", "hist", nbins, thetaMin, thetaMax);
  for (auto entry : hits_to_add) {
    if (entry.first->getSuperlayerId() != 5) continue;
    double theta1 = entry.second.first;
    double theta2 = entry.second.second;
    hist3.Fill(theta1 * 180. / 3.1415);
    hist3.Fill(theta2 * 180. / 3.1415);

  }

  //    hist3.Draw("E1");

  //    canv1.cd(4);

  TH1F hist4("hist4", "hist", nbins, thetaMin, thetaMax);
  for (auto entry : hits_to_add) {
    if (entry.first->getSuperlayerId() != 7) continue;
    double theta1 = entry.second.first;
    double theta2 = entry.second.second;
    hist4.Fill(theta1 * 180. / 3.1415);
    hist4.Fill(theta2 * 180. / 3.1415);

  }

  //    hist4.Draw("E1");
  */
//    canv1.cd(5);

  for (auto stereoHitPair : hits_to_add) {
    double theta1 = stereoHitPair.first.getPolarAngle();
    double theta2 = stereoHitPair.first.getPolarAngle();

    if ((theta1 * 180. / 3.1415 > thetaMin) && (theta1 * 180. / 3.1415 < thetaMax))
      hist.Fill(theta1 * 180. / 3.1415);
    if ((theta2 * 180. / 3.1415 > thetaMin) && (theta2 * 180. / 3.1415 < thetaMax))
      hist.Fill(theta2 * 180. / 3.1415);

  }

//    TCanvas canv1("canv1", "canv", 0, 0, 1400, 600);
//    hist.Draw("E1");
//    canv1.SaveAs(Form("tmp/stereohits/efficiency/ev_%i_tr_%i_2d_hipt.png", eventnr, tracknr));
//    canv1.SaveAs(Form("tmp/stereohits/efficiency/ev_%i_tr_%i_2d_hipt.root", eventnr, tracknr));

  int maxBin = hist.GetMaximumBin();
  double meanTheta = hist.GetMean() * 3.1415 / 180.;
//    double range1 = binWidth*(maxBin-2);
//    double range2 = binWidth*(maxBin+2);
//    double range1 = meanTheta - binWidth * 2;
//    double range2 = meanTheta + binWidth * 2;
  double range1 = hist.GetBinCenter(maxBin - 3) * 3.1415 / 180.;
  double range2 = hist.GetBinCenter(maxBin + 3) * 3.1415 / 180.;

  meanTheta = (range1 + range2) / 2.;

  B2INFO("THETA WINDOW: range1: " << range1 * 180. / 3.1415 << "; range2: " << range2 * 180. / 3.1415);

  TH1F hist5("hist5", "hist", 30, 0, 15);



  for (auto stereoHitPair : hits_to_add) {

    TrackHit* hit = stereoHitPair.first.getTrackHit();

    //setting Z coordinate according to provided polar angle and comparing updated position of stereohit on consistency with track
    hit->setZReference(stereoHitPair.first.getAlpha() * Rcand / tan(meanTheta));
    double dist1 = Rcand - sqrt(SQR(hit->getWirePosition().X() - cand->getXc()) + SQR(hit->getWirePosition().Y() - cand->getYc())) + stereoHitPair.first.getInnerOuter() * hit->getDriftLength();

    hit->setZReference(stereoHitPair.second.getAlpha() * Rcand / tan(meanTheta));
    double dist2 = Rcand - sqrt(SQR(hit->getWirePosition().X() - cand->getXc()) + SQR(hit->getWirePosition().Y() - cand->getYc())) + stereoHitPair.second.getInnerOuter() * hit->getDriftLength();


    if (((fabs(dist1) < 1.5) || (fabs(dist2) < 1.5)) && (hit->getZReference() > hit->getBackwardWirePosition().Z() && hit->getZReference() < hit->getForwardWirePosition().Z())) {
      cand->addHit(hit);
      hit->setHitUsage(TrackHit::used_in_track);

      //Compare to original CDCHit
      const CDCSimHit* cdcSimHit = DataStore::getRelatedToObj<CDCSimHit>(hit->getOriginalCDCHit());
      double zHitOriginal = cdcSimHit->getPosWire().Z();

      cdcSimHit = nullptr;

      //        B2INFO("Delta Z position: " << zHitOriginal - hit->getZReference() << "; Z original: " << zHitOriginal);


    } else {
      //restore default hit coordinates
      hit->setZReference(0);
    }

  }

}

/*
void StereohitsProcesser::assignStereohitsByAngleWithQuadtree(TrackCandidate* cand, double theta, std::vector<TrackHit*>& stereohits, double Z0)
{
  B2INFO("Tree stereo tracklet finding");
  std::sort(stereohits.begin(), stereohits.end());

//  m_cdcLegendreQuadTree->setLastLevel(10);

  int trackCharge;
  int vote_pos(0), vote_neg(0);

  //estimation of charge of the track candidate
  for (TrackHit * Hit : cand->getTrackHits()) {
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


  std::set<TrackHit*> hits_set;
  std::set<TrackHit*>::iterator it = hits_set.begin();
  for (TrackHit * trackHit : stereohits) {
    if (trackHit->getHitUsage() == TrackHit::used_in_track) continue;



    std::pair<StereoHit, StereoHit> stereoHitPair;

    try{
      stereoHitPair = getDisplacements(cand, trackHit, trackCharge);
    } catch (...) {
      continue;
    }


    //setting Z coordinate according to provided polar angle and comparing updated position of stereohit on consistency with track
    trackHit->setZReference(stereoHitPair.first.getAlpha() * Rcand / tan(theta) + Z0);
    double dist1 = Rcand - sqrt(SQR(trackHit->getWirePosition().X() - cand->getXc()) + SQR(trackHit->getWirePosition().Y() - cand->getYc())) + stereoHitPair.first.getInnerOuter() * trackHit->getDriftLength();

    trackHit->setZReference(stereoHitPair.second.getAlpha() * Rcand / tan(theta) + Z0);
    double dist2 = Rcand - sqrt(SQR(trackHit->getWirePosition().X() - cand->getXc()) + SQR(trackHit->getWirePosition().Y() - cand->getYc())) + stereoHitPair.second.getInnerOuter() * trackHit->getDriftLength();


    double lWire = fabs(trackHit->getBackwardWirePosition().Z() - trackHit->getForwardWirePosition().Z());
    double rWire = sqrt(SQR(trackHit->getBackwardWirePosition().x() - trackHit->getForwardWirePosition().x()) + SQR(trackHit->getBackwardWirePosition().y() - trackHit->getForwardWirePosition().y()));

    if ((fabs(dist1) > rWire/1.5) || (fabs(dist2) > rWire/1.5)) continue;

    if(dist1<dist2){
      trackHit->setZReference(stereoHitPair.first.getAlpha() * Rcand / tan(theta) + Z0);
    } else {
      trackHit->setZReference(stereoHitPair.second.getAlpha() * Rcand / tan(theta) + Z0);
    }

    it = hits_set.insert(it, trackHit);
  }


  TrackCandidate* stereoPart;

  m_cdcLegendreQuadTree->clearTree();
  QuadTreeCandidateCreator::Instance().clearNodes();
  QuadTreeCandidateCreator::Instance().clearCandidates();

  double limit = 6;
  double rThreshold = 0.15;
  m_cdcLegendreQuadTree->provideHitSet(hits_set);
//  m_cdcLegendreQuadTree->setRThreshold(rThreshold);
//  m_cdcLegendreQuadTree->setHitsThreshold(limit);
  m_cdcLegendreQuadTree->startFillingTree(true, stereoPart);

  if(stereoPart != nullptr){
    for(TrackHit* hit: stereoPart->getTrackHits()){
      cand->addHit(hit);
    }

    delete stereoPart;
  }

  m_cdcLegendreQuadTree->clearTree();
  QuadTreeCandidateCreator::Instance().clearNodes();
  QuadTreeCandidateCreator::Instance().clearCandidates();


}

*/



void StereohitsProcesser::assignStereohitsByAngle(TrackCandidate* cand, double theta, std::vector<TrackHit*>& stereohits, double Z0)
{


//  assignStereohitsByAngleWithQuadtree(cand, theta, stereohits, Z0);

//  return;


  TVector3 momentum = cand->getMomentumEstimation(true);

  int trackCharge;
  int vote_pos(0), vote_neg(0);

  //estimation of charge of the track candidate
  for (TrackHit * Hit : cand->getTrackHits()) {
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

  for (TrackHit * hit : stereohits) {


    if (hit->getHitUsage() == TrackHit::used_in_track) continue;

    std::pair<StereoHit, StereoHit> stereoHitPair;

    try {
      stereoHitPair = getDisplacements(cand, hit, trackCharge);
    } catch (...) {
      continue;
    }


    //setting Z coordinate according to provided polar angle and comparing updated position of stereohit on consistency with track
    hit->setZReference(stereoHitPair.first.getAlpha() * Rcand / tan(theta) + Z0);
    double dist1 = Rcand - sqrt(SQR(hit->getWirePosition().X() - cand->getXc()) + SQR(hit->getWirePosition().Y() - cand->getYc())) + stereoHitPair.first.getInnerOuter() * hit->getDriftLength();

    hit->setZReference(stereoHitPair.second.getAlpha() * Rcand / tan(theta) + Z0);
    double dist2 = Rcand - sqrt(SQR(hit->getWirePosition().X() - cand->getXc()) + SQR(hit->getWirePosition().Y() - cand->getYc())) + stereoHitPair.second.getInnerOuter() * hit->getDriftLength();


    //TODO: set reasonable values instead of "1.5"
    if (((fabs(dist1) < 1.5) || (fabs(dist2) < 1.5)) && (hit->getZReference() > hit->getBackwardWirePosition().Z() && hit->getZReference() < hit->getForwardWirePosition().Z())) {
      cand->addHit(hit);
      hit->setHitUsage(TrackHit::used_in_track);

      //Compare to original CDCHit
      const CDCSimHit* cdcSimHit = DataStore::getRelatedToObj<CDCSimHit>(hit->getOriginalCDCHit());
      double zHitOriginal = cdcSimHit->getPosWire().Z();

      cdcSimHit = nullptr;

//        B2INFO("Delta Z position: " << zHitOriginal - hit->getZReference() << "; Z original: " << zHitOriginal);


    } else {
      //restore default hit coordinates
      hit->setZReference(0);
    }

  }



}




std::pair<StereoHit, StereoHit> StereohitsProcesser::getDisplacements(TrackCandidate* cand, TrackHit* hit,  int trackCharge)
{

  double Rcand = cand->getRadius();

  if (trackCharge == 0) {
    int vote_pos(0), vote_neg(0);

    //estimation of charge of the track candidate
    for (TrackHit * Hit : cand->getTrackHits()) {
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

  double lWire = fabs(hit->getBackwardWirePosition().Z() - hit->getForwardWirePosition().Z());
  double rWire = sqrt(SQR(hit->getBackwardWirePosition().x() - hit->getForwardWirePosition().x()) + SQR(hit->getBackwardWirePosition().y() - hit->getForwardWirePosition().y()));

  if ((fabs(stereoHitInner.getDisplacement()) > rWire) && (fabs(stereoHitOuter.getDisplacement()) > rWire))
    throw "Displacement exceeds wire lenght of wire projection!" ;

  return std::make_pair(stereoHitOuter, stereoHitInner);

}


StereoHit StereohitsProcesser::getDisplacement(TrackCandidate* cand, TrackHit* hit, int InnerOuter)
{
  double Rcand = cand->getRadius();

  //-------- drift time correction factor! should be discussed --------
  double driftLenghtCorr = fabs(hit->getBackwardWirePosition().Z() - hit->getForwardWirePosition().Z()) / sqrt(SQR(hit->getBackwardWirePosition().Z() - hit->getForwardWirePosition().Z()) + SQR(hit->getBackwardWirePosition().x() - hit->getForwardWirePosition().x()) + SQR(hit->getBackwardWirePosition().y() - hit->getForwardWirePosition().y()));

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
  double C = hit->getForwardWirePosition().X() * hit->getBackwardWirePosition().Y() - hit->getBackwardWirePosition().X() * hit->getForwardWirePosition().Y();

  double x0circle = cand->getXc();
  double y0circle = cand->getYc();

  double R_outer = Rcand + InnerOuter * hit->getDriftLength() * driftLenghtCorr;

  double x1 = 1. / (A * A + B * B) * (-1.*pow(-1.*B * B * (A * A * x0circle * x0circle - A * A * R_outer * R_outer + 2.*A * B * x0circle * y0circle + 2.*A * C * x0circle + B * B * y0circle * y0circle - B * B * R_outer * R_outer + 2.*B * C * y0circle + C * C) , 0.5) - A * B * y0circle - A * C + B * B * x0circle);
  double y1 = 1. / (B * (A * A + B * B)) * (A * pow(-1.*B * B * (A * A * x0circle * x0circle - A * A * R_outer * R_outer + 2.*A * B * x0circle * y0circle + 2.*A * C * x0circle + B * B * y0circle * y0circle - B * B * R_outer * R_outer + 2.*B * C * y0circle + C * C) , 0.5) + A * A * B * y0circle - A * B * B * x0circle - B * B * C);

  double x2 = 1. / (A * A + B * B) * (pow(-1.*B * B * (A * A * x0circle * x0circle - A * A * R_outer * R_outer + 2.*A * B * x0circle * y0circle + 2.*A * C * x0circle + B * B * y0circle * y0circle - B * B * R_outer * R_outer + 2.*B * C * y0circle + C * C) , 0.5) - A * B * y0circle - A * C + B * B * x0circle);
  double y2 = 1. / (B * (A * A + B * B)) * (-1.*A * pow(-1.*B * B * (A * A * x0circle * x0circle - A * A * R_outer * R_outer + 2.*A * B * x0circle * y0circle + 2.*A * C * x0circle + B * B * y0circle * y0circle - B * B * R_outer * R_outer + 2.*B * C * y0circle + C * C) , 0.5) + A * A * B * y0circle - A * B * B * x0circle - B * B * C);


  double dist_to_hit1 = sqrt((x1 - hit->getOriginalWirePosition().X()) * (x1 - hit->getOriginalWirePosition().X()) + (y1 - hit->getOriginalWirePosition().Y()) * (y1 - hit->getOriginalWirePosition().Y()));
  double dist_to_hit2 = sqrt((x2 - hit->getOriginalWirePosition().X()) * (x2 - hit->getOriginalWirePosition().X()) + (y2 - hit->getOriginalWirePosition().Y()) * (y2 - hit->getOriginalWirePosition().Y()));

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

  double dist = Rcand - sqrt(SQR(hit->getOriginalWirePosition().X() - cand->getXc()) + SQR(hit->getOriginalWirePosition().Y() - cand->getYc())) + InnerOuter * hit->getDriftLength() * driftLenghtCorr;
  dist = dist / fabs(dist) * fabs(dist_to_hit);

  B2DEBUG(100, "A = " << A << "; B = " << B << "; C = " << C << "; x0circle = " << x0circle << "; y0circle = " << y0circle << "; R_outer = " << R_outer);

  return StereoHit(alpha, hitProjectionX, hitProjectionY, InnerOuter, hit, dist);

}


double StereohitsProcesser::getAlpha(TrackCandidate* cand, std::pair<double, double> pos)
{

  double alpha = acos(1. - (SQR(pos.first - cand->getReferencePoint().X()) + SQR(pos.second - cand->getReferencePoint().Y())) / (2.*SQR(cand->getRadius())));

  double hitPhi = 0.;
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

  int charge = cand->getChargeSign();

  double delta_phi_outer = hitPhi - (cand->getPhi() - m_PI * cand->getChargeSign() / 2.) /*cand->getTheta()*/;
  if (delta_phi_outer > 3.1415) delta_phi_outer -= 2.*3.1415;
  if (delta_phi_outer < -3.1415) delta_phi_outer += 2.*3.1415;

  if (delta_phi_outer * cand->getChargeSign() < 0) alpha = m_PI * 2. - alpha;

  return alpha;

}
