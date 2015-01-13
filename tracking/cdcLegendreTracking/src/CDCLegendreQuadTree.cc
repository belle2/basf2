/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreQuadTree.h>


using namespace std;
using namespace Belle2;
using namespace TrackFinderCDCLegendre;

float* QuadTree::s_sin_theta;
float* QuadTree::s_cos_theta;
bool QuadTree::s_sin_lookup_created;
int QuadTree::s_nbinsTheta;
unsigned int QuadTree::s_hitsThreshold;
float QuadTree::s_rThreshold;
int QuadTree::s_lastLevel;

/*
 * TODO: How to organise it? we can fill tree, then check, if hit was used twice or more times; in that case mark it as "multipleused" and don't include it to any track.
 * then create track cands (using criteria on number of hits) and mark (?) that hits again as good ones (might be appended to the good candidate)
 *
 * Lits of nodes with candidates should be provided as output of whole algorithm; then another class can create candidates using pointer to the node
 */
QuadTree::QuadTree():
  m_rMin(0), m_rMax(0), m_thetaMin(0), m_thetaMax(0), m_level(0)
{

}


QuadTree::QuadTree(float rMin, float rMax, int thetaMin, int thetaMax, int level, QuadTree* parent) :
  m_rMin(rMin), m_rMax(rMax), m_thetaMin(thetaMin), m_thetaMax(thetaMax), m_level(level)
{
  m_filled = false;
  m_neighborsDefined = false;

  if (m_level > 0) {
    m_parent = parent;
  } else {
    m_parent = NULL;
    s_sin_lookup_created = false;
    s_rThreshold = 0.15; //whole phase-space;
  }

//  Rcell = 2. * m_PI * 60. / 384.;

//  m_deltaR = 4. * Rcell / (4. / ((m_rMax + m_rMin) * (m_rMax + m_rMin)) - 4.*Rcell * Rcell);

  m_isMaxLevel = checkLimitsR();

  m_children = nullptr;

//  if (not m_isMaxLevel) initialize();
  initialize();

//  B2INFO("CREATED node:" << m_thetaMin<< "x" << m_thetaMax << "x" << m_rMin << "x" << m_rMax << " dr=" << m_deltaR << " level:" << m_level);
}


QuadTree::~QuadTree()
{
  if (not m_isMaxLevel) {

    if (m_children != nullptr) {
      for (int t_index = 0; t_index < m_nbins_theta; ++t_index) {
        for (int r_index = 0; r_index < m_nbins_r; ++r_index) {
          delete m_children[t_index][r_index];
        }
        delete[] m_children[t_index];
      }
      delete[] m_children;
    }

    delete[] m_r;
    delete[] m_thetaBin;

  }

  if ((m_level == 0) && (s_sin_lookup_created)) {
    delete[] s_sin_theta;
    delete[] s_cos_theta;
  }

}

bool QuadTree::checkLimitsR()
{

  if (m_level == 13)
    return true;

//  if (fabs(m_rMax - m_rMin) < m_deltaR) {
//    return true;
//  }

  return false;
}


void QuadTree::initialize()
{
  if (not s_sin_lookup_created) {
    //B2INFO("Create lookup table");
    s_nbinsTheta = 8192;
    float bin_width = m_PI / s_nbinsTheta;
    s_sin_theta = new float[s_nbinsTheta + 1];
    s_cos_theta = new float[s_nbinsTheta + 1];

    for (int i = 0; i <= s_nbinsTheta; ++i) {
      s_sin_theta[i] = sin(i * bin_width);
      s_cos_theta[i] = cos(i * bin_width);
    }

    s_sin_lookup_created = true;
  }

  //B2DEBUG(100, "DELTA r: " << delta_r << "; r_max-r_min: " << fabs(r_max - r_min));

  //calculate bin borders of 2x2 bin "histogram"
  /*
  if (not m_isMaxLevel) {
    //B2DEBUG(100, "NORMAL binning");
    m_nbins_r = 2;
    m_nbins_theta = 2;

    m_r = new float[3];
    m_r[0] = m_rMin;
    m_r[1] = m_rMin + 0.5 * (m_rMax - m_rMin);
    m_r[2] = m_rMax;

    m_thetaBin = new int[3];
    m_thetaBin[0] = m_thetaMin;
    m_thetaBin[1] = m_thetaMin + (m_thetaMax - m_thetaMin) / 2;
    m_thetaBin[2] = m_thetaMax;
  } else {
    //B2DEBUG(100, "1xN binning");
    m_nbins_r = 1;

    m_r = new float[2];
    m_r[0] = m_rMin;
    m_r[1] = m_rMax;

    m_nbins_theta = (m_thetaMax - m_thetaMin) / 8;
    m_thetaBin = new int[m_nbins_theta + 1];
    for (int t_index = 0; t_index < m_nbins_theta + 1; t_index++) {
      m_thetaBin[t_index] = m_thetaMin + t_index;
    }
  }
  */
  //B2DEBUG(100, "NORMAL binning");
  m_nbins_r = 2;
  m_nbins_theta = 2;

  m_r = new float[3];
  m_r[0] = m_rMin;
  m_r[1] = m_rMin + 0.5 * (m_rMax - m_rMin);
  m_r[2] = m_rMax;

  m_thetaBin = new int[3];
  m_thetaBin[0] = m_thetaMin;
  m_thetaBin[1] = m_thetaMin + (m_thetaMax - m_thetaMin) / 2;
  m_thetaBin[2] = m_thetaMax;

  //B2DEBUG(100, "Number of bins: " << nbins_r << "x" << nbins_theta);
  //B2DEBUG(100, "CREATING voting plane");


}

void QuadTree::initializeChildren()
{
  m_children = new QuadTree** [m_nbins_theta];
  for (int i = 0; i < m_nbins_theta; ++i) {
    m_children[i] = new QuadTree*[m_nbins_r];
    for (int j = 0; j < m_nbins_r; ++j) {
      if (m_level < (s_lastLevel - 5))m_children[i][j] = new QuadTree(m_r[j], m_r[j + 1], m_thetaBin[i], m_thetaBin[i + 1], m_level + 1, this);
      else {
        double r1 = m_r[j] - fabs(m_r[j + 1] - m_r[j]) / 4.;
        double r2 = m_r[j + 1] + fabs(m_r[j + 1] - m_r[j]) / 4.;
//        double r1 = m_r[j];
//        double r2 = m_r[j+1];
        //            theta_1_overlap = thetaBin[t_index]/* - fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.*/;
        //            theta_2_overlap = thetaBin[t_index + 1]/* + fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.*/;
        //            theta_1_overlap = thetaBin[t_index] - fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.;
        //            theta_2_overlap = thetaBin[t_index + 1] + fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.;
        int theta1 = m_thetaBin[i] - abs(pow(2, s_lastLevel + 0 - m_level) / 4);
        int theta2 = m_thetaBin[i + 1] + abs(pow(2, s_lastLevel + 0 - m_level) / 4);
//        int theta1 = m_thetaBin[i];
//        int theta2 = m_thetaBin[i + 1];

        if (theta1 < 0)theta1 = m_thetaBin[i];
        if (theta2 >= 8192)theta2 = m_thetaBin[i + 1];


        m_children[i][j] = new QuadTree(r1, r2, theta1, theta2, m_level + 1, this);
      }
    }
  }

}

void QuadTree::buildNeighborhood(int levelNeighborhood)
{
  /*  if (m_level < levelNeighborhood) {
      for (int i = 0; i < m_nbins_theta; ++i) {
        for (int j = 0; j < m_nbins_r; ++j) {
          m_children[i][j]->buildNeighborhood(levelNeighborhood);
        }
      }
    } else if (m_level == s_lastLevel){
      this->findNeighbors();
    }*/
  if (m_level < 13) {
    for (int i = 0; i < m_nbins_theta; ++i) {
      for (int j = 0; j < m_nbins_r; ++j) {
        m_children[i][j]->buildNeighborhood(levelNeighborhood);
      }
    }
  }

  if (m_level > 3)
    this->findNeighbors();

}

void QuadTree::provideHitSet(const std::set<TrackHit*>& hits_set)
{
  m_hits.clear();
  std::copy_if(hits_set.begin(), hits_set.end(), std::back_inserter(m_hits), [&](TrackHit * hit) {return (hit->getHitUsage() == TrackHit::not_used);});
}

void QuadTree::cleanHitsInNode()
{
  m_hits.erase(std::remove_if(m_hits.begin(), m_hits.end(), [&](TrackHit * hit) {return hit->getHitUsage() != TrackHit::not_used;}), m_hits.end());
}

void QuadTree::clearTree()
{
  if (!m_filled)return;

  clearNode();
  if (checkFilled()) {
    for (int t_index = 0; t_index < m_nbins_theta; ++t_index) {
      for (int r_index = 0; r_index < m_nbins_r; ++r_index) {
        if (m_children[t_index][r_index]) m_children[t_index][r_index]->clearTree();
      }
    }
  }
  m_filled = false;

  if (m_level == 0) {
    for (int t_index = 0; t_index < m_nbins_theta; ++t_index) {
      for (int r_index = 0; r_index < m_nbins_r; ++r_index) {
        delete m_children[t_index][r_index];
      }
      delete[] m_children[t_index];
    }
    delete[] m_children;
    m_children = nullptr;
  }

}

void QuadTree::startFillingTree(bool returnCandidate, TrackCandidate* cand)
{
  if (m_hits.size() < s_hitsThreshold) return;
  if (m_rMin * m_rMax >= 0 && fabs(m_rMin) > s_rThreshold && fabs(m_rMax) > s_rThreshold) return;
  if (m_level == s_lastLevel) {
//    B2INFO("Last level! Not filling children. Level = " << m_level << "; bin " << m_rMin << "-" << m_rMax << "x" << m_thetaMin << "-" << m_thetaMax);
//    B2INFO("Candidate: " << m_rMin << "-" << m_rMax << "x" << m_thetaMin << "-" << m_thetaMax << "; nhits=" << m_hits.size());

    QuadTreeCandidateCreator::Instance().createCandidateDirect(this);

    /*
    if(!returnCandidate) QuadTreeCandidateCreator::Instance().createCandidateDirect(this);
    else {
      if(cand == nullptr){
        cleanHitsInNode();
        std::vector<QuadTree*> nodeList;
        nodeList.push_back(this);
        cand = new TrackCandidate(nodeList);
      } else if(cand->getTrackHits().size() < getHits().size()){
        delete cand;

        cleanHitsInNode();
        std::vector<QuadTree*> nodeList;
        nodeList.push_back(this);
        cand = new TrackCandidate(nodeList);
      }
    }*/

    return;
  }

  if (m_children == nullptr) initializeChildren();


  if (!checkFilled()) fillChildren();



  bool binUsed[m_nbins_theta][m_nbins_r];
  for (int ii = 0; ii < m_nbins_theta; ii++)
    for (int jj = 0; jj < m_nbins_r; jj++)
      binUsed[ii][jj] = false;

//Processing, which bins are further investigated
  for (int bin_loop = 0; bin_loop < m_nbins_theta * m_nbins_r; bin_loop++) {
    int t_index = 0;
    int r_index = 0;
    float max_value_temp = 0;
    for (int t_index_temp = 0; t_index_temp < m_nbins_theta; ++t_index_temp) {
      for (int r_index_temp = 0; r_index_temp < m_nbins_r; ++r_index_temp) {
        if ((max_value_temp  < m_children[t_index_temp][r_index_temp]->getNHits()) && (!binUsed[t_index_temp][r_index_temp])) {
          max_value_temp = m_children[t_index_temp][r_index_temp]->getNHits();
          t_index = t_index_temp;
          r_index = r_index_temp;
        }
      }
    }

    binUsed[t_index][r_index] = true;

    m_children[t_index][r_index]->cleanHitsInNode();
    m_children[t_index][r_index]->startFillingTree();

  }
}

void QuadTree::fillChildren(/*const std::vector<CDCLegendreTrackHit*>& hits*/)
{

  float r_temp, r_1, r_2;

  float dist_1[m_nbins_theta + 1][m_nbins_r + 1];
  float dist_2[m_nbins_theta + 1][m_nbins_r + 1];

  //B2DEBUG(100, "VOTING");

  for (int t_index = 0; t_index < m_nbins_theta; ++t_index) {
    for (int r_index = 0; r_index < m_nbins_r; ++r_index) {
      m_children[t_index][r_index]->reserveHitsVector(2 * m_hits.size());
    }
  }

  //Voting within the four bins
  for (TrackHit * hit : m_hits) {
    //B2DEBUG(100, "PROCCESSING hit " << hit_counter << " of " << nhitsToReserve);
    if (hit->getHitUsage() != TrackHit::not_used) continue;
    for (int t_index = 0; t_index < m_nbins_theta + 1; ++t_index) {

      /*if (hit->checkRValue(m_thetaBin[t_index])) r_temp = hit->getRValue(m_thetaBin[t_index]);
      else {
        r_temp = hit->getConformalX() * s_cos_theta[m_thetaBin[t_index]] +
                 hit->getConformalY() * s_sin_theta[m_thetaBin[t_index]];
        hit->setRValue(m_thetaBin[t_index], r_temp);
      }*/

      /*
      r_temp = hit->getConformalX() * s_cos_theta[m_thetaBin[t_index]] +
               hit->getConformalY() * s_sin_theta[m_thetaBin[t_index]];
      */

      // for bin overlapping!
      if (t_index < m_nbins_theta)
        r_temp = hit->getConformalX() * s_cos_theta[m_children[t_index][0]->getThetaMin()] +
                 hit->getConformalY() * s_sin_theta[m_children[t_index][0]->getThetaMin()];
      else
        r_temp = hit->getConformalX() * s_cos_theta[m_children[t_index - 1][0]->getThetaMax()] +
                 hit->getConformalY() * s_sin_theta[m_children[t_index - 1][0]->getThetaMax()];


//      r_temp = CDCLegendreConformalPosition::InstanceTrusted().getConformalR(hit->getLayerId(), hit->getWireId(), m_thetaBin[t_index]);
      r_1 = r_temp + hit->getConformalDriftLength();
      r_2 = r_temp - hit->getConformalDriftLength();

      //calculate distances of lines to horizontal bin border
      for (int r_index = 0; r_index < m_nbins_r + 1; ++r_index) {
        dist_1[t_index][r_index] = m_r[r_index] - r_1;
        dist_2[t_index][r_index] = m_r[r_index] - r_2;
      }
    }

    //B2DEBUG(100, "VOTING for hit " << hit_counter << " of " << nhitsToReserve);
    //actual voting, based on the distances (test, if line is passing though the bin)
    for (int t_index = 0; t_index < m_nbins_theta; ++t_index) {
      for (int r_index = 0; r_index < m_nbins_r; ++r_index) {
        //curves are assumed to be straight lines, might be a reasonable assumption locally
        if (!FastHough::sameSign(dist_1[t_index][r_index], dist_1[t_index][r_index + 1], dist_1[t_index + 1][r_index], dist_1[t_index + 1][r_index + 1])) {
          //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
          m_children[t_index][r_index]->insertHit(hit);
        } else if (!FastHough::sameSign(dist_2[t_index][r_index], dist_2[t_index][r_index + 1], dist_2[t_index + 1][r_index], dist_2[t_index + 1][r_index + 1])) {
          //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
          m_children[t_index][r_index]->insertHit(hit);
        }
      }
    }
    //B2DEBUG(100, "MOVING to next hit");
  }

  setFilled();

}


void QuadTree::fillChildrenForced()
{
  if (!m_parent->checkFilled()) {
    m_parent->fillChildrenForced();
    m_parent->fillChildren();
  }
}


void QuadTree::findNeighbors()
{
  if (not m_neighborsDefined) {
    QuadTreeNeighborFinder::Instance().controller(this, this, m_parent);
    m_neighborsDefined = true;
  }
//  B2INFO("Number of neighbors: " << m_neighbors.size());

//  B2INFO("Number of hits in node: " << m_hits.size());
  /*  for(CDCLegendreQuadTree* node: m_neighbors){
      if(node->getNHits() == 0) node->fillChildrenForced();
      node->cleanHitsInNode();
  */
  //    B2INFO("Number of hits in neighbor: " << node->getNHits());
  /*    if(node->checkNode()){
        for(CDCLegendreTrackHit *hit: node->getHits()){
          m_hits.push_back(hit);
        }
  */
//  }

}

std::vector<QuadTree*>& QuadTree::getNeighborsVector()
{
  if (not m_neighborsDefined) {
    B2WARNING("Trying to get neighbors of the node which are not defined. Starting neighbors finding");
    findNeighbors();
  }
  return m_neighbors;
}

QuadTree* QuadTree::getChildren(int t_index, int r_index) const
{
  if ((t_index > m_nbins_theta) || (r_index > m_nbins_r))
    B2FATAL("BAD THETA OR R INDEX OF CHILDREN NODE! m_nbins_theta=" << m_nbins_theta << ", m_nbins_r" << m_nbins_r << "; t_index=" << t_index << ", r_index" << r_index);
  return m_children[t_index][r_index];
};

