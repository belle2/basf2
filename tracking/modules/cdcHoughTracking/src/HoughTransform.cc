#include <tracking/modules/cdcHoughTracking/HoughTransform.h>
#include <math.h>
#include <TMath.h>
#include <boost/foreach.hpp>
#include <fstream>

using namespace Belle2;
using namespace Belle2::Tracking;
HoughTransform::HoughTransform(): m_precision(360), m_pBins(360), m_cBins(250), m_hits(0), m_pScale(m_pBins / 360.0f), m_cScale(m_cBins / 2500.0f)
{
}

HoughTransform::HoughTransform(const unsigned int precision, const unsigned int pBins, const int unsigned cBins, const std::vector<CDCHoughHit>* hits):
  m_precision(precision), m_pBins(pBins),  m_cBins(cBins), m_hits(hits),
  m_binsN(m_pBins, std::vector<int>(m_cBins, 0)), m_binsP(m_binsN),
  m_pScale(m_pBins / 360.0f), m_cScale(m_cBins / 2500.0f)


{
  m_candidatesN.reserve(20);
  m_candidatesP.reserve(20);
  m_tracksN.reserve(20);
  m_tracksP.reserve(20);
  m_gfTracks.reserve(20);
}

HoughTransform::~HoughTransform()
{
}


void HoughTransform::vote()
{
//
//  const float prec = m_precision;
//  const unsigned int iprec = m_precision;
//  const float phiScale = m_pScale;
//  const float curScale = m_cScale;
//
//  float angDegN[iprec];
//  float angDegP[iprec];
//  int sincosN[iprec];
//  int sincosP[iprec];
//
//
//  float phiN_vals[iprec];
//  float phiP_vals[iprec];
//  float curN_vals[iprec];
//  float curP_vals[iprec];
//
//  int binsCurN[iprec];
//  int binsCurP[iprec];
//  int binsPhiP[iprec];
//  int binsPhiN[iprec];
//
//  for (int i = 0; i < iprec; ++i) {
//    angDegN[i] = (i + 1) * 90.0f / prec;
//    angDegP[i] = -angDegN[i] + 360;
//    sincosN[i] = (int) (angDegN[i] * 8);
//    sincosP[i] = (int) (angDegP[i] * 8);
//
//  }
//
//
//
//  const std::vector<CDCHoughHit>& hits = *m_hits;
//
//  const unsigned int numberOfHits = hits.size();
//
//  float phi_vals[numberOfHits];
//  float r_vals[numberOfHits];
//
//  unsigned int maxPhi[numberOfHits];
//
//
//  for( unsigned int i = 0; i < numberOfHits; ++i ){
//    int sLayer = hits[i].getSuperlayerId(); //max_phi depends on super Layer
//////    B2INFO("superlayer " << sLayer);
//    switch (sLayer) {
//    case 0:
//      maxPhi[i] = 19; //check values again
//      break;
//    case 2:
//      maxPhi[i] = 36;
//      break;
//    case 4:
//      maxPhi[i] = 59;
//      break;
//    default:
//      maxPhi[i] = 90;
//      break;
//    }
//    maxPhi[i] *= prec/90.0f;
//  }
//
//////  B2INFO("1");
//
//  for( unsigned int i = 0; i < numberOfHits; ++i ){
//    if (!hits[i].getIsAxial())
//      continue;
//    phi_vals[i] = hits[i].getPhi().getDeg(); //polar angle in deg
//    r_vals[i]   = hits[i].getR() * 10;
//  }
//////  B2INFO("2");
////
////  //used to check if calculated bin is the same as last time
//  int oldCurN = -1;
//  int oldPhiN = -1;
//  int oldCurP = -1;
//  int oldPhiP = -1;
//
////  //helpers
////  int votes = 0;
////  int total = 0;
////  int samebin = 0;
////
//  for (int i_hit = 0; i_hit < numberOfHits; ++i_hit) {
//    if ( !hits[i_hit].getIsAxial() )
//      continue;
//    const unsigned int phi_max = maxPhi[i_hit];
//
//    for (int i = 0; i < phi_max; ++i) {
//      phiN_vals[i] = -angDegN[i] + phi_vals[i_hit] + 90;
//      phiP_vals[i] = -angDegP[i] + phi_vals[i_hit] + 270;
//    }
//
//    for (int i = 0; i < phi_max; ++i) {
//      while (phiN_vals[i] >= 360)
//        phiN_vals[i] -= 360;
//
//      while (phiN_vals[i] < 0)
//        phiN_vals[i] += 360;
//
//      while (phiP_vals[i] >= 360)
//        phiP_vals[i] -= 360;
//
//      while (phiP_vals[i] < 0)
//        phiP_vals[i] += 360;
//    }
//
//    for (int i = 0; i < phi_max; ++i) {
//      float xHitN = r_vals[i_hit] * cosine[sincosN[i]];
//      float yHitN = r_vals[i_hit] * sine[sincosN[i]];
//      curN_vals[i] = fabs(2 * yHitN * 1E6)
//          / (yHitN * yHitN + xHitN * xHitN);
//
//      const float xHitP = r_vals[i_hit] * cosine[sincosP[i]];
//      const float yHitP = r_vals[i_hit] * sine[sincosP[i]];
//      curP_vals[i] = fabs(2 * yHitP * 1E6)
//          / (yHitP * yHitP + xHitP * xHitP);
//
//
//    }
//
//
//    for (int i = 0; i < phi_max; ++i) {
////      B2INFO(i << " " << phiP_vals[i]);
//      int binCurN = (int) (curN_vals[i] * curScale);
//      int binCurP = (int) (curP_vals[i] * curScale);
//      int binPhiP = (int) (phiP_vals[i] * phiScale);
//      int binPhiN = (int) (phiN_vals[i] * phiScale);
//      if ((binPhiN == oldPhiN and binCurN == oldCurN) or curN_vals[i] > 2500) { //same bin again?
//////        ++samebin;
//      } else {
//        oldPhiN = binPhiN;
//        oldCurN = binCurN;
////        B2INFO(binPhiN << " N " << binCurN);
//        ++(m_binsN[binPhiN][binCurN]); //increment bin
//////        ++votes; //count used votes
//      }
//      if ((binPhiP == oldPhiP and binCurP == oldCurP) or curP_vals[i] > 2500) { //same bin again?
//////        ++samebin;
//      } else {
//        oldPhiP = binPhiP;
//        oldCurP = binCurP;
////        B2INFO(binPhiP << " P " << binCurP);
//        ++(m_binsP[binPhiP][binCurP]); //increment bin
//////        ++votes; //count used votes
//      }
//
//    }
//  }
//////  B2INFO("Voting Done!");



  float angDegN[m_precision];
  float angDegP[m_precision];
  int sincosN[m_precision];
  int sincosP[m_precision];

  for (unsigned int i = 0; i < m_precision; ++i) {
    angDegN[i] = (i + 1) * 90.0f / m_precision;
    angDegP[i] = -angDegN[i] + 360;
  }

  for (unsigned int i = 0; i < m_precision; ++i) {
    sincosN[i] = (int)(angDegN[i] * 8);
    sincosP[i] = (int)(angDegP[i] * 8);
  }

//used to check if calculated bin is the same as last time
  int oldCurN = -1;
  int oldPhiN = -1;
  int oldCurP = -1;
  int oldPhiP = -1;

//helpers
  int votes = 0;
  int total = 0;
  int samebin = 0;
  const std::vector<CDCHoughHit>& hits = *m_hits;
  for (int i_hit = 0, size = hits.size(); i_hit < size; ++i_hit) {
    if (!hits[i_hit].getIsAxial())
      continue;

    total += m_precision * 2;

    Coordinates tmp = Coordinates(hits[i_hit].getX() * 10,
                                  hits[i_hit].getY() * 10, true); //create Coordinates from Hit
    float c_phi = tmp.getPhi().getDeg(); //polar angle in deg
    float c_r = tmp.getR();

    int max_phi = 90; //maximum value for voting, to stay in sane region (curvature < 2500)

    int sLayer = hits[i_hit].getSuperlayerId(); //max_phi depends on super Layer

    switch (sLayer) {
      case 0:
        max_phi = 19; //check values again
        break;
      case 2:
        max_phi = 36;
        break;
      case 4:
        max_phi = 59;
        break;
      default:
        break;
    }

    for (unsigned int i = 0; i < m_precision; ++i) {
      const float xHitN = c_r * cosine[sincosN[i]];
      const float yHitN = c_r * sine[sincosN[i]];
      float curvatureN = fabs(2 * yHitN * 1E6)
                         / (pow(yHitN, 2) + pow(xHitN, 2));

      int phiN = -angDegN[i] + c_phi + 90;
      const float xHitP = c_r * cosine[sincosP[i]];
      const float yHitP = c_r * sine[sincosP[i]];
      float curvatureP = fabs(2 * yHitP * 1E6)
                         / (pow(yHitP, 2) + pow(xHitP, 2));

      int phiP = -angDegP[i] + c_phi + 270; //for width of 1

      while (phiN >= 360)
        phiN -= 360;

      while (phiN < 0)
        phiN += 360;

      while (phiP >= 360)
        phiP -= 360;

      while (phiP < 0)
        phiP += 360;

//      if (curvatureN > 2500){  //wouldn't make it into CDC anyway
//        ++curvecount;
//        continue;
//      }
      float phiScale = m_pBins / 360.0f;
      float curScale = m_cBins / 2500.0f;

      int binCurN = (int)(curvatureN * curScale);
      int binCurP = (int)(curvatureP * curScale);
      int binPhiP = (int)(phiP * phiScale);
      int binPhiN = (int)(phiN * phiScale);

      if ((binPhiN == oldPhiN and binCurN == oldCurN) or curvatureN > 2500) { //same bin again?
        ++samebin;
      } else {
        oldPhiN = binPhiN;
        oldCurN = binCurN;
        ++(m_binsN[binPhiN][binCurN]); //increment bin
        ++votes; //count used votes
      }
      if ((binPhiP == oldPhiP and binCurP == oldCurP) or curvatureP > 2500) { //same bin again?
        ++samebin;
      } else {
        oldPhiP = binPhiP;
        oldCurP = binCurP;
        ++(m_binsP[binPhiP][binCurP]); //increment bin
        ++votes; //count used votes
      }

    }
  }
//  B2INFO("Votes: " << votes << " Same: " << samebin << " Max Phi: " << countmaxphi << " Total: " << total);
//  B2INFO("Curve: " << curvecount);

}

void HoughTransform::peakFinder(const int charge)
{
  std::vector<std::vector<int> >* bins;
  if (charge < 0) // choose correct bins
    bins = &m_binsN;
  else
    bins = &m_binsP;

  const float phiscale = m_pScale;
  const float curscale = m_cScale;

  const int maxphi = 6 * phiscale; //6 //tolerances for region in hough space
  const int maxcur = 200 * curscale; //20
  const float threshold = 0.7f;

  int max = 0;

  std::vector<Candidate> candidates; //put bins together to find the real center
  std::vector<std::pair<int, int> > region; //put bins together to find the real center
  candidates.reserve(500);

  //find bin with most votes
  BOOST_FOREACH(std::vector<int>& vi, *bins) {
    BOOST_FOREACH(int i, vi) {
      if (i > max)
        max = i;
    }
  }
//  B2INFO("AvgBin: " << static_cast<float>(sum)/(bins->size()*bins[0].size()));
//  B2INFO("Old MaxBin: " << max);

  //put coordinates of bins bigger than threshold into candidates
  for (unsigned int i = 0; i < m_pBins; ++i) {
    for (unsigned int j = 0; j < m_cBins; ++j) {
      int bin = (*bins)[i][j];
      if (bin > max * threshold) {
        candidates.push_back(Candidate(bin, i, j));
      }

    }
  }

  std::vector<Coordinates>* centerCandidates;

  if (charge < 0) // choose correct histogram
    centerCandidates = &m_candidatesN;
  else
    centerCandidates = &m_candidatesP;

  //always start with biggest bin

  while (!candidates.empty()) { //sort until all candidates are allocated
    region.clear();
    std::sort(candidates.begin(), candidates.end(), SortCandidatesByVotes());

    float avPhi = candidates[0].m_binPhi;
    float avCur = candidates[0].m_binCur;
    int numHits = 1;
    //iterate over candidates but ignore first one, which is already used above
    for (std::vector<Candidate>::iterator i =
           candidates.begin() + 1; i != candidates.end();) {
      int phi = i->m_binPhi;
      int cur = i->m_binCur;
//      B2INFO("ThisCurve: " << cur << " " << phi);
//      B2INFO("ThisCand: " << i->first);
//      B2INFO("<maxcur: " << avCur - cur);
      if (fabs(avPhi - phi) < maxphi && fabs(avCur - cur) < maxcur) { //is candidate in region?
        ++numHits;
        avPhi = (avPhi * (numHits - 1) + phi) / numHits; //only averaging phi seems better
        avCur = (avCur * (numHits - 1) + cur) / numHits; //only averaging phi seems better
        region.push_back(std::make_pair(phi, cur)); //add candidate to region
        i = candidates.erase(i); //erase added candidate and keep iterator valid
      } else {
        ++i;
      }
    }
//    B2INFO("Curvature: " << avCur << ", " << avPhi);
    Coordinates possibleCandidate = getCenterFromBin(avPhi, avCur, *this);

    //    exclude Circles with Radius below 150mm
    if (possibleCandidate.getR() > 150) {
      centerCandidates->push_back(possibleCandidate);
//      B2INFO("Angle: " << avPhi);
    }
    candidates.erase(candidates.begin()); // erase seed of region
  }
}

void HoughTransform::trackBuilder(const int charge)
{
  std::vector<Track>* tracks;
  std::vector<Coordinates>* candidates;
  const std::vector<CDCHoughHit>& hits = *m_hits;
  const int trackThreshold = 15;
  const unsigned int minHits = 18;

  if (charge < 0) { // choose correct histogram
    tracks = &m_tracksN;
    candidates = &m_candidatesN;
  } else {
    tracks = &m_tracksP;
    candidates = &m_candidatesP;
  }

  const unsigned int numOfCandidates = candidates->size(); // number of possible tracks
  Track tmpTrack;

  for (unsigned int i = 0; i < numOfCandidates; ++i) {
    tmpTrack.m_indices.clear();
    tmpTrack.m_indices.reserve(100);
    Coordinates& cand = (*candidates)[i];
//    B2INFO("Cand in trackb: " << cand.getR() << " " << cand.getPhi().getDeg() << " " << cand.getX() << " " << cand.getY());
//    B2INFO("Cand in trackb, curv: " << 1/cand.getR()*1000);
    float radius = cand.getR();
    tmpTrack.m_center = cand;
    float x = cand.getX();
    float y = cand.getY();


    for (int i_hit = 0, size = hits.size(); i_hit < size; ++i_hit) {
      if (!hits[i_hit].getIsAxial()) {
        continue;
      }

      float cx = hits[i_hit].getX() * 10;
      float cy = hits[i_hit].getY() * 10;

      float diffx = cx - x;
      float diffy = cy - y;
      Coordinates hitRelToCenter(diffx, diffy); // set polar origin of hit to center
      Coordinates origin(-x, -y); // (0,0) in the new polar system
      float distance = hitRelToCenter.getR();
      Angle relToC = hitRelToCenter.getPhi();
      Angle ori = origin.getPhi();
      Angle diffAngle = relToC - ori; // hits phi in relation to origin at phi=0 for easy comparison

      // prevent tracks from beyond the origin from being added - depending on charge
      // maybe use variable parameter instead of middle value
      int circleModifier = 0;

      if (radius < 555) // full circle possible
        circleModifier = 1;

      if (charge < 0 and diffAngle.getDeg() > (90 + circleModifier * 180)
          and diffAngle.getDeg() < 360) {
        continue;
      }
      if (charge > 0 and diffAngle.getDeg() > 0
          and diffAngle.getDeg() < (270 - circleModifier * 180)) {
        continue;
      }

      //if hit is in innermost (denser) layer use smaller threshold
      float t = 0;
      if (hits[i_hit].getR() < 25) {
        t = trackThreshold * 0.4; // threshold for each layer?

      } else {
        t = trackThreshold;
      }

      if (radius < distance and distance < radius + t) {
        ++radius; //correct for dE/dx
        tmpTrack.m_indices.push_back(hits[i_hit].getStoreIndex());
//        B2INFO("Hit found: " << hits[i_hit].getX() * 10 << " " << hits[i_hit].getY() * 10);
//        B2INFO("Hit found: " << hits[i_hit].getForwardPosition()[0] * 10 << " " << hits[i_hit].getY() * 10);
//        B2INFO("distance and radius: " << distance << " " << radius);
      } else if (radius - t < distance and distance < radius) {
        --radius; //correct for dE/dx
        tmpTrack.m_indices.push_back(hits[i_hit].getStoreIndex());
//        B2INFO("Hit found: " << hits[i_hit].getX() * 10 << " " << hits[i_hit].getY() * 10);
//        B2INFO("distance and radius: " << distance << " " << radius);
      } else {
//        B2INFO("Hit bad: " << hits[i_hit].getX() * 10 << " " << hits[i_hit].getY() * 10);
//        B2INFO("distance and radius: " << distance << " " << radius);
//        B2INFO("+-: " << radius + t << " " << radius - t);
      }
    }
    if (tmpTrack.m_indices.size() > minHits) {
      tracks->push_back(tmpTrack);
      B2INFO("Track in TB: " << tmpTrack.m_indices.size() << " -- " << tmpTrack.m_center.getR() << " " << tmpTrack.m_center.getPhi().getDeg());
    }
  }
}

void HoughTransform::zFinder(const int charge)
{

  std::vector<Track>* ptracks;

  if (charge < 0) { // choose correct histogram
    ptracks = &m_tracksN;
  } else {
    ptracks = &m_tracksP;
  }

  std::vector<Track>& tracks = *ptracks;
  const std::vector<CDCHoughHit>& hits = *m_hits;

  const unsigned int numOfTracks = tracks.size();
  const unsigned int numOfHits   = hits.size();


  for (unsigned int i = 0; i < numOfTracks ; ++i) {

    int countF = 0;
    int countB = 0;

    const Coordinates& cand = tracks[i].m_center;
    float x = cand.getX();
    float y = cand.getY();
    float radius = cand.getR();
//    B2INFO("Cand in stereo: " << radius << " " << track->second.getPhi().getDeg());
    B2INFO("Cand in zb: " << cand.getR() << " " << cand.getPhi().getDeg() << " " << cand.getX() << " " << cand.getY());
//    B2INFO("zb curv in 1/m: " << 1/cand.getR()*1000);

    std::vector<std::pair<int, float> > forwardZ; //index,cotanges Theta
    std::vector<std::pair<int, float> > backwardZ;
//    B2INFO("cand: " << x << " " << y);
    std::vector<float> findMedian;
    for (unsigned int i_hit = 0; i_hit < numOfHits; ++i_hit) {
//      B2INFO("31");
      if (hits[i_hit].getIsAxial()) {
        continue;
      }
      const TVector3& forward = hits[i_hit].getForwardPosition();
      const TVector3& backward = hits[i_hit].getBackwardPosition();
      const TVector3 direction = backward - forward;

      Coordinates c1Rel = Coordinates(forward[0] * 10 - x, forward[1] * 10 - y);
      Coordinates c2Rel = Coordinates(backward[0] * 10 - x, backward[1] * 10 - y);
//      B2INFO("c1rel: " << c1Rel.getX() << " " << c1Rel.getY());
//      B2INFO("c2rel: " << c2Rel.getX() << " " << c2Rel.getY());
      Coordinates origin(-x, -y); // (0,0) in the new polar system

      float distance1 = c1Rel.getR();
      float distance2 = c2Rel.getR();
      Angle relToC1 = c1Rel.getPhi();
      Angle relToC2 = c2Rel.getPhi();
      Angle ori = origin.getPhi();
      Angle diffAngle1 = relToC1 - ori; // hits phi in relation to origin at phi=0 for easy comparison
      Angle diffAngle2 = relToC2 - ori; // hits phi in relation to origin at phi=0 for easy comparison
      int circleModifier = 0;

//      B2INFO("1");
      if (radius < 555) {// full circle possible
        circleModifier = 1;
      }
//      B2INFO("2");
      if (charge < 0 and diffAngle1.getDeg() > (90 + circleModifier * 180)
          and diffAngle1.getDeg() < 360) {
        continue;
      }
//      B2INFO("3: " << diffAngle1.getDeg());
      if (charge > 0 and diffAngle1.getDeg() > 0
          and diffAngle1.getDeg() < (270 - circleModifier * 180)) {
        continue;
      }
//      B2INFO("4");
      double dis1 = radius - distance1;
      double dis2 = radius - distance2;


//      B2INFO("radius,dis: " << radius<< " " << distance1 << " " << distance2);
      if (dis1* dis2 >= 0)
        continue;

//      B2INFO("f: " << forward.X()*10 << " " << forward.Y() *10<< " " << forward.Z()*10);
//      B2INFO("b: " << backward.X() *10<< " " << backward.Y() *10<< " " << backward.Z()*10);
//      B2INFO("track: " << x << " " << y);
//      B2INFO("drift: " << hits[i_hit].getDriftTime()*10);

      //          //new stuff here
      //
      //          //***************************
      float bx = c2Rel.getX();
      float by = c2Rel.getY();
      float fx = c1Rel.getX();
      float fy = c1Rel.getY();

      float dx = bx - fx;
      float dy = by - fy;

      float a = dx * dx + dy * dy;
      float b = 2 * (fx * dx + fy * dy);
      float c = fx * fx + fy * fy - radius * radius;
      ////          B2INFO("dir " << direction.X() << " " << direction.Y() << " " << direction.Z());
      ////          B2INFO("in " << bx << " " << by << ", " << fx << " " << fy);
      ////          B2INFO("zin " << backward.z() * 10 << " " << forward.z() * 10);
      ////          B2INFO("dxdy " << dx << " " << dy);
      ////          B2INFO("r " << radius);
      ////          B2INFO("qu " << a << " " << b << " " << c);

      float discrim = b * b - 4 * a * c;

      if (discrim < 0) {
//        B2INFO("Discrim < 0");
        continue;
      }

      float lambda = (-b + sqrt(discrim)) / (2 * a);

//      if (lambda > 1 or lambda < 0) {
      if (fabs(lambda) > 1) {
        lambda = (-b - sqrt(discrim)) / (2 * a);
      }
//      B2INFO("lambda: " << lambda);
      if (lambda > 1 or lambda < 0) {
//        B2INFO("this should not happen");
        continue;
      }

//      B2INFO("Lambda " << lambda);
//            TVector3 inter = TVector3(fx / 10.0 + lambda * direction.x(), fy / 10.0 + lambda * direction.y(), forward.z() + lambda * direction.z()); //doesnt make sense to use fx,fy
//      B2INFO(
//          "New results: " << inter.x() * 10 << ", " << inter.y() * 10 << ", " << inter.z()*10);
      float newX = (forward.x() + lambda * direction.x()) * 10;
      float newY = (forward.y() + lambda * direction.y()) * 10;
      float zval = (forward.z() + lambda * direction.z()) * 10;
      float rval = sqrt(newX * newX + newY * newY);
//      B2INFO("xy: " << newX << " " << newY);
//      file3 << newX << " " << newY << std::endl;
//      B2INFO("DIR: " << direction.Z());
      float cotTheta = zval / rval;
//      B2INFO("r: " << rval);
//      B2INFO("z: " << zval);
      //          //new stuff here end
      float thistheta = (TMath::Pi() / 2 - atan(cotTheta));
      findMedian.push_back(thistheta);
      if (zval > 0) {
        ++countF;
        forwardZ.push_back(std::make_pair(hits[i_hit].getStoreIndex(), cotTheta)); //change
      } else {
        ++countB;
        backwardZ.push_back(std::make_pair(hits[i_hit].getStoreIndex(), cotTheta)); //change
      }

    }
    if (findMedian.empty()) {
      B2INFO("No stereo hit found!");
      continue;
    }
    std::nth_element(findMedian.begin(), findMedian.begin() + findMedian.size() / 2, findMedian.end());
    typedef std::pair<int, float> pif;
    float avgCotTheta = 1 / tan(findMedian[findMedian.size() / 2]);

    if (countF > countB) {
      float sigma_cot = 0.0f;
      BOOST_FOREACH(pif & p, forwardZ) {
        sigma_cot += pow(p.second - avgCotTheta, 2);
      }
      sigma_cot = sqrt(sigma_cot / forwardZ.size());
      BOOST_FOREACH(pif & p, forwardZ) {
        float condition = fabs((p.second - avgCotTheta) / sigma_cot);
        if (condition < 2) {
          tracks[i].m_indices.push_back(p.first);
        }
      }

    } else {
      float sigma_cot = 0.0f;
      BOOST_FOREACH(pif & p, backwardZ) {
        sigma_cot += pow(p.second - avgCotTheta, 2);
      }
      sigma_cot = sqrt(sigma_cot / backwardZ.size());
      BOOST_FOREACH(pif & p, backwardZ) {
        float condition = fabs((p.second - avgCotTheta) / sigma_cot);
        if (condition < 2) {
          tracks[i].m_indices.push_back(p.first);
        }
      }
    }
    TVector3 position = TVector3(0, 0, 0);

    //calculate starting angle from candidate (improve later on to do it somewhere else)
    Angle starting = Angle(tracks[i].m_center.getPhi().getDeg() + 90 * charge, false);

    TVector3 direction = TVector3(cos(starting.getRad()),
                                  sin(starting.getRad()), 0);
    float p_t = 1.5f * (radius / 10) * 0.00299f;
    float qop = charge / p_t;

    direction = direction.Unit();
    direction = direction * p_t;
    direction.SetZ(p_t * avgCotTheta);

    std::sort(tracks[i].m_indices.begin(), tracks[i].m_indices.end());

    GFTrackCand gfc = GFTrackCand();
    gfc.setTrackSeed(position, direction, qop);
    BOOST_FOREACH(unsigned int i, tracks[i].m_indices) {
      gfc.addHit(2, i, 0, i);
    }
    m_gfTracks.push_back(gfc);
  }
}

void HoughTransform::printDebugInfo()
{
  std::ofstream file1("binsP.txt");
  std::ofstream file2("binsN.txt");
  std::ofstream file3("binsAll.txt");

  const std::vector< std::vector<int> >& binsP = this->m_binsP;
  const std::vector< std::vector<int> >& binsN = this->m_binsN;

  int size = binsP.size();
  int size2 = binsP.at(1).size();

  for (int i = 0 ; i < size; ++i) {
    for (int j = 0 ; j < size2; ++j) {
      file1 << binsP[i][j] << " ";
      file2 << binsN[i][j] << " ";
      file3 << binsP[i][j] + binsN[i][j] << " ";
    }
    file1 << std::endl;
    file2 << std::endl;
    file3 << std::endl;
  }

  B2INFO("Number of Candidates: " << this->m_candidatesN.size() + this->m_candidatesP.size());
  BOOST_FOREACH(Coordinates & c, this->m_candidatesN) {
    B2INFO("Candidate- : " <<  c.getX() << " " <<  c.getY() << " " <<  c.getR() << " " <<  c.getPhi().getDeg());
  }

  BOOST_FOREACH(Coordinates & c, this->m_candidatesP) {
    B2INFO("Candidate+ : " <<  c.getX() << " " <<  c.getY() << " " <<  c.getR() << " " <<  c.getPhi().getDeg());
  }

  B2INFO("Number of Tracks: " << this->m_tracksN.size() + this->m_tracksP.size());
  BOOST_FOREACH(Track & t, this->m_tracksN) {
    B2INFO("Track- : " << t.m_indices.size());
  }
  BOOST_FOREACH(Track & t, this->m_tracksP) {
    B2INFO("Track+ : " << t.m_indices.size());
  }

  B2INFO("Number of GFTracks: " << this->getGfTracks().size());
  BOOST_FOREACH(GFTrackCand & g, this->m_gfTracks) {
    B2INFO("GFTrack: " << g.getNHits());
  }


}

Coordinates Belle2::Tracking::getCenterFromBin(const float phi, const float curve, const HoughTransform& hough)
{
  Angle a(phi / hough.getPScale(), false);
  float r = 1000000.0f * hough.getCScale() / curve;
  return Coordinates(r, a.getRad(), false);
}


