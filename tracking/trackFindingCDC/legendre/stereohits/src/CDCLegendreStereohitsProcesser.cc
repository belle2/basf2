#include <tracking/trackFindingCDC/legendre/stereohits/CDCLegendreStereohitsProcesser.h>

#define SQR(x) ((x)*(x)) //we will use it in least squares fit

#include <framework/logging/Logger.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

StereohitsProcesser::StereohitsProcesser() : m_cdcLegendreQuadTree(-0.15, 0.15,  0, 8192, 0, NULL)
{
}

StereohitsProcesser::~StereohitsProcesser()
{

  m_cdcLegendreQuadTree.clearTree();
}

void StereohitsProcesser::makeHistogramming(TrackCandidate* cand, std::vector<TrackHit*>& stereohits)
{

  B2DEBUG(100, "Processing new track; assigning stereohits.");

  double Rcand = cand->getRadius();

  std::vector<std::pair<StereoHit, StereoHit>> hits_to_add;

  double trackCharge;
  int vote_pos(0), vote_neg(0);

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



  for (TrackHit* hit : stereohits) {

//    if (hit->getHitUsage() == TrackHit::used_in_track) continue;

    std::pair<StereoHit, StereoHit> stereoHitPair;

    try {
      stereoHitPair = getDisplacements(cand, hit, trackCharge);
    } catch (const char* msg) {
      B2DEBUG(100, msg);
      continue;
    }

    double lWire = fabs(hit->getBackwardWirePosition().Z() - hit->getForwardWirePosition().Z());
    double rWire = sqrt(SQR(hit->getBackwardWirePosition().x() - hit->getForwardWirePosition().x()) + SQR(
                          hit->getBackwardWirePosition().y() - hit->getForwardWirePosition().y()));

    stereoHitPair.first.setRWire(rWire);
    stereoHitPair.first.setLWire(lWire);
    stereoHitPair.second.setRWire(rWire);
    stereoHitPair.second.setLWire(lWire);

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

    double delta_phi = hit->getPhi() - (cand->getPhi() - TMath::Pi() * trackCharge / 2.) /*cand->getTheta()*/;
    if (delta_phi > 3.1415) delta_phi -= 2.*3.1415;
    if (delta_phi < -3.1415) delta_phi += 2.*3.1415;

    sign_phi = delta_phi / fabs(delta_phi) /* cand->getChargeSign()*/;


    double sign_final;
    sign_final = sign_phi * sign;


    if (std::isnan(sign_final)) continue;
    if (std::isnan(delta_phi)) continue;

    stereoHitPair.first.setSign(sign_final);
    stereoHitPair.second.setSign(sign_final);

    stereoHitPair.first.setRcand(Rcand);
    stereoHitPair.second.setRcand(Rcand);
    stereoHitPair.first.setZ0(0);
    stereoHitPair.second.setZ0(0);

//      if(alpha > m_PI) sign_final *= -1.;

//    double theta1 = atan2(lWire * sign_final * stereoHitPair.first.getDisplacement(), stereoHitPair.first.getAlpha() * Rcand * rWire) + m_PI / 2.;
//    double theta2 = atan2(lWire * sign_final * stereoHitPair.second.getDisplacement(), stereoHitPair.second.getAlpha() * Rcand * rWire) + m_PI / 2.;

//    stereoHitPair.first.setPolarAngle(theta1);
//    stereoHitPair.second.setPolarAngle(theta2);

    stereoHitPair.first.computePolarAngle();
    stereoHitPair.second.computePolarAngle();

    B2DEBUG(100, "SLayerID: " << sLayer <<  "; delta_phi: " << delta_phi << "; alpha: " <<  stereoHitPair.first.getAlpha() <<
            "; sign_final: " << sign_final
            << "; lWire: " << lWire  << "; rWire: " << rWire << ";   Dist_1: " <<  stereoHitPair.first.getDisplacement() << "; Dist_2: " <<
            stereoHitPair.second.getDisplacement()
            << "; theta1: " << stereoHitPair.first.getPolarAngle() * 180. / 3.1415 << "; theta2: " << stereoHitPair.first.getPolarAngle() *
            180. / 3.1415);

    hits_to_add.push_back(stereoHitPair);

  }

  std::vector<TrackHit*> stereohitsForCandidate;

  MaxFastHoughStereofinding(stereohitsForCandidate, hits_to_add, 0, tan(-75.*3.1415 / 180.), tan(75.*3.1415 / 180.), -20., 20.);

  for (TrackHit* hit : stereohitsForCandidate) {
    cand->addHit(hit);
    hit->setHitUsage(TrackHit::used_in_track);
  }

  return;

  int nbins = 60;
  double thetaMin = 15.;
  double thetaMax = 170.;
//  double binWidth = m_PI / static_cast<double>(nbins);

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

  B2DEBUG(100, "THETA WINDOW: range1: " << range1 * 180. / 3.1415 << "; range2: " << range2 * 180. / 3.1415);

  TH1F hist5("hist5", "hist", 30, 0, 15);



  for (auto stereoHitPair : hits_to_add) {

    TrackHit* hit = stereoHitPair.first.getTrackHit();

    //setting Z coordinate according to provided polar angle and comparing updated position of stereohit on consistency with track
    hit->setZReference(stereoHitPair.first.getAlpha() * Rcand / tan(meanTheta));
    double dist1 = Rcand - sqrt(SQR(hit->getWirePosition().X() - cand->getXc()) + SQR(hit->getWirePosition().Y() - cand->getYc())) +
                   stereoHitPair.first.getInnerOuter() * hit->getDriftLength();

    hit->setZReference(stereoHitPair.second.getAlpha() * Rcand / tan(meanTheta));
    double dist2 = Rcand - sqrt(SQR(hit->getWirePosition().X() - cand->getXc()) + SQR(hit->getWirePosition().Y() - cand->getYc())) +
                   stereoHitPair.second.getInnerOuter() * hit->getDriftLength();


    if (((fabs(dist1) < 1.5) || (fabs(dist2) < 1.5)) && (hit->getZReference() > hit->getBackwardWirePosition().Z()
                                                         && hit->getZReference() < hit->getForwardWirePosition().Z())) {
      cand->addHit(hit);
      hit->setHitUsage(TrackHit::used_in_track);

      //Compare to original CDCHit
//      const CDCSimHit* cdcSimHit = DataStore::getRelatedToObj<CDCSimHit>(hit->getOriginalCDCHit());
//      double zHitOriginal = cdcSimHit->getPosWire().Z();

//      cdcSimHit = nullptr;

      //        B2INFO("Delta Z position: " << zHitOriginal - hit->getZReference() << "; Z original: " << zHitOriginal);


    } else {
      //restore default hit coordinates
      hit->setZReference(0);
    }
    /*
        std::vector<TrackHit*> stereoHitsToAdd;
        for (auto stereoHitPair : hits_to_add) {
          if(stereoHitPair.first.getTrackHit()->getHitUsage() != TrackHit::not_used) continue;
          stereoHitsToAdd.push_back(stereoHitPair.first.getTrackHit());
        }
    */
//    assignStereohitsByAngle(cand, meanTheta, stereoHitsToAdd);

  }

}


void StereohitsProcesser::assignStereohitsByAngleWithQuadtree(TrackCandidate* cand, double theta,
    std::vector<TrackHit*>& stereohits, double Z0)
{
  B2DEBUG(100, "Tree stereo tracklet finding");
  std::sort(stereohits.begin(), stereohits.end());

//  m_cdcLegendreQuadTree->setLastLevel(10);

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


  std::set<TrackHit*> hits_set;
  std::set<TrackHit*>::iterator it = hits_set.begin();
  for (TrackHit* trackHit : stereohits) {
    if (trackHit->getHitUsage() == TrackHit::used_in_track) continue;



    std::pair<StereoHit, StereoHit> stereoHitPair;

    try {
      stereoHitPair = getDisplacements(cand, trackHit, trackCharge);
    } catch (...) {
      continue;
    }


    //setting Z coordinate according to provided polar angle and comparing updated position of stereohit on consistency with track
    trackHit->setZReference(stereoHitPair.first.getAlpha() * Rcand / tan(theta) + Z0);
    double dist1 = Rcand - sqrt(SQR(trackHit->getWirePosition().X() - cand->getXc()) + SQR(trackHit->getWirePosition().Y() -
                                cand->getYc())) + stereoHitPair.first.getInnerOuter() * trackHit->getDriftLength();

    trackHit->setZReference(stereoHitPair.second.getAlpha() * Rcand / tan(theta) + Z0);
    double dist2 = Rcand - sqrt(SQR(trackHit->getWirePosition().X() - cand->getXc()) + SQR(trackHit->getWirePosition().Y() -
                                cand->getYc())) + stereoHitPair.second.getInnerOuter() * trackHit->getDriftLength();


//    double lWire = fabs(trackHit->getBackwardWirePosition().Z() - trackHit->getForwardWirePosition().Z());
    double rWire = sqrt(SQR(trackHit->getBackwardWirePosition().x() - trackHit->getForwardWirePosition().x()) + SQR(
                          trackHit->getBackwardWirePosition().y() - trackHit->getForwardWirePosition().y()));

    if ((fabs(dist1) > rWire / 0.9) || (fabs(dist2) > rWire / 0.9)) continue;

    if (dist1 < dist2) {
      trackHit->setZReference(stereoHitPair.first.getAlpha() * Rcand / tan(theta) + Z0);
    } else {
      trackHit->setZReference(stereoHitPair.second.getAlpha() * Rcand / tan(theta) + Z0);
    }

    it = hits_set.insert(it, trackHit);
  }


  // used only by commented code
  //TrackCandidate* stereoPart = nullptr;

  m_cdcLegendreQuadTree.clearTree();

  unsigned int __attribute__((unused)) limit = 6;
  double __attribute__((unused)) rThreshold = 0.15;

  std::vector<QuadTreeLegendre*> nodeList;

  // used only by commented code
  //int lastlevel(0);
  //lastlevel = m_cdcLegendreQuadTree.getLastLevel();
  //m_cdcLegendreQuadTree.setLastLevel(9);

  QuadTreeProcessor qtProcessor(9);
  m_cdcLegendreQuadTree.provideItemsSet(qtProcessor, hits_set);
  B2DEBUG(100, "Number of stereohits = " << hits_set.size());
//  m_cdcLegendreQuadTree->setRThreshold(rThreshold);
  //m_cdcLegendreQuadTree.setNItemsThreshold(limit);

  // store all candidates in a simple vector
  QuadTreeLegendre::CandidateProcessorLambda lmdCandidateProcessing = [&nodeList](QuadTreeLegendre * qt) -> void {
    nodeList.push_back(qt);
  };

  // todo: move usage to fillGivenTree
  //m_cdcLegendreQuadTree.startFillingTree(qtProcessor,lmdCandidateProcessing);
  qtProcessor.fillGivenTree(&m_cdcLegendreQuadTree, lmdCandidateProcessing,
                            limit);

  //m_cdcLegendreQuadTree.setLastLevel(lastlevel);


  if (nodeList.size() != 0) {
//    B2INFO(nodeList.size() << " NODES WERE ADDED!");

    QuadTreeLegendre* nodeWithMostHits = nullptr;

    for (QuadTreeLegendre* node : nodeList) {
      if (nodeWithMostHits == nullptr) {
        nodeWithMostHits = node;
      } else {
        if (nodeWithMostHits->getItemsVector().size() < node->getItemsVector().size())
          nodeWithMostHits = node;
      }
    }

    for (TrackHit* hit : nodeWithMostHits->getItemsVector()) {
      cand->addHit(hit);
      hit->setHitUsage(TrackHit::used_in_track);
    }

    nodeWithMostHits = nullptr;

    /*
    B2INFO("NUMBER OF HITS TO ADD = " << stereoPart->getTrackHits().size());
    for(TrackHit* hit: stereoPart->getTrackHits()){
      cand->addHit(hit);
    }

    delete stereoPart;
    */
  } else {
//    B2INFO("NO NODES WERE ADDED!");
  }


  m_cdcLegendreQuadTree.clearTree();


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


    if (hit->getHitUsage() == TrackHit::used_in_track) continue;

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
      hit->setHitUsage(TrackHit::used_in_track);

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

//  int charge = cand->getChargeSign();

  double delta_phi_outer = hitPhi - (cand->getPhi() - TMath::Pi() * cand->getChargeSign() / 2.) /*cand->getTheta()*/;
  if (delta_phi_outer > 3.1415) delta_phi_outer -= 2.*3.1415;
  if (delta_phi_outer < -3.1415) delta_phi_outer += 2.*3.1415;

  if (delta_phi_outer * cand->getChargeSign() < 0) alpha = TMath::Pi() * 2. - alpha;

  return alpha;

}



void StereohitsProcesser::MaxFastHoughStereofinding(
  std::vector<TrackHit*>& hitsToAdd,
  std::vector<std::pair<StereoHit, StereoHit>>& hits,
  const int level,
  const double lambda_min,
  const double lambda_max,
  const double z0_min,
  const double z0_max)
{

  //calculate bin borders of 2x2 bin "histogram"
  double lambdaBin[3];
  lambdaBin[0] = lambda_min;
  lambdaBin[1] = lambda_min + (lambda_max - lambda_min) / 2;
  lambdaBin[2] = lambda_max;

  double z0[3];
  z0[0] = z0_min;
  z0[1] = z0_min + 0.5 * (z0_max - z0_min);
  z0[2] = z0_max;

  //2 x 2 voting plane
  std::vector<std::pair<StereoHit, StereoHit>> voted_hits[2][2];
  for (unsigned int i = 0; i < 2; ++i)
    for (unsigned int j = 0; j < 2; ++j)
      voted_hits[i][j].reserve(1024);

  double lambda_1, lambda_2;
  double dist_1[3][3];
  double dist_2[3][3];

  //Voting within the four bins
  for (std::pair<StereoHit, StereoHit>& hit : hits) {
//    if (hit.first.getTrackHit()->getHitUsage() != TrackHit::not_used) continue;
    for (int z0_index = 0; z0_index < 3; ++z0_index) {

      hit.first.setZ0(z0[z0_index]);
      lambda_1 = hit.first.computePolarAngle();
      hit.second.setZ0(z0[z0_index]);
      lambda_2 = hit.second.computePolarAngle();

      //calculate distances of lines to horizontal bin border
      for (int lambda_index = 0; lambda_index < 3; ++lambda_index) {
        dist_1[z0_index][lambda_index] = lambdaBin[lambda_index] - lambda_1;
        dist_2[z0_index][lambda_index] = lambdaBin[lambda_index] - lambda_2;
      }
    }

    //actual voting, based on the distances (test, if line is passing though the bin)
    for (int z0_index = 0; z0_index < 2; ++z0_index) {
      for (int lambda_index = 0; lambda_index < 2; ++lambda_index) {
        //curves are assumed to be straight lines, might be a reasonable assumption locally
        if (!sameSign(dist_1[z0_index][lambda_index], dist_1[z0_index][lambda_index + 1], dist_1[z0_index + 1][lambda_index],
                      dist_1[z0_index + 1][lambda_index + 1]))
          voted_hits[z0_index][lambda_index].push_back(hit);
        else if (!sameSign(dist_2[z0_index][lambda_index], dist_2[z0_index][lambda_index + 1], dist_2[z0_index + 1][lambda_index],
                           dist_2[z0_index + 1][lambda_index + 1]))
          voted_hits[z0_index][lambda_index].push_back(hit);
      }
    }

  }

  bool binUsed[2][2];
  for (int ii = 0; ii < 2; ii++)
    for (int jj = 0; jj < 2; jj++)
      binUsed[ii][jj] = false;

//Processing, which bins are further investigated
  for (int bin_loop = 0; bin_loop < 4; bin_loop++) {
    int z0_index = 0;
    int lambda_index = 0;
    double max_value_temp = 0;
    for (int z0_index_temp = 0; z0_index_temp < 2; ++z0_index_temp) {
      for (int lambda_index_temp = 0; lambda_index_temp < 2; ++lambda_index_temp) {
        if ((max_value_temp  < voted_hits[z0_index_temp][lambda_index_temp].size()) && (!binUsed[z0_index_temp][lambda_index_temp])) {
          max_value_temp = voted_hits[z0_index_temp][lambda_index_temp].size();
          z0_index = z0_index_temp;
          lambda_index = lambda_index_temp;
        }
      }
    }

    binUsed[z0_index][lambda_index] = true;

    //bin must contain more hits than the limit and maximal found track candidate


    //bin must contain more hits than the limit and maximal found track candidate
    if (voted_hits[z0_index][lambda_index].size() >= 5) {

      //if max level of fast Hough is reached, mark candidate and return
      //        if (((!allow_overlap)&&(level == (m_maxLevel - level_diff))) || ((allow_overlap)&&(level == (m_maxLevel - level_diff) + 2))) {
      if (level >= 7) {

        if (hitsToAdd.size() >= voted_hits[z0_index][lambda_index].size()) continue;

        hitsToAdd.clear();

        for (std::pair<StereoHit, StereoHit>& hit : voted_hits[z0_index][lambda_index]) {
          hitsToAdd.push_back(hit.first.getTrackHit());
        }

        B2DEBUG(100, "Theta: " << lambdaBin[lambda_index] << "; Z0: " << z0[z0_index] << "; nhits: " << hitsToAdd.size());

      } else {
        //Recursive calling of the function with higher level and smaller box
        MaxFastHoughStereofinding(hitsToAdd, voted_hits[z0_index][lambda_index], level + 1, lambdaBin[lambda_index],
                                  lambdaBin[lambda_index + 1],
                                  z0[z0_index], z0[z0_index + 1]);

      }
    }
  }

}


