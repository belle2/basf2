#include <trg/cdc/NeuroTrigger.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>

#include <cmath>
#include <TFile.h>

using namespace Belle2;
using namespace CDC;
using namespace std;

void
NeuroTrigger::initialize(const Parameters& p)
{
  // check parameters
  bool okay = true;
  // ensure that length of lists matches number of sectors
  if (p.nInput.size() != 1 && p.nInput.size() != p.nMLP) {
    B2ERROR("Number of nInput values should be 1 or " << p.nMLP);
    okay = false;
  }
  if (p.nHidden.size() != 1 && p.nHidden.size() != p.nMLP) {
    B2ERROR("Number of nHidden lists should be 1 or " << p.nMLP);
    okay = false;
  }
  if (p.outputScale.size() != 1 && p.outputScale.size() != p.nMLP) {
    B2ERROR("Number of outputScale lists should be 1 or " << p.nMLP);
    okay = false;
  }
  bool rangeProduct = (p.phiRange.size() * p.invptRange.size() * p.thetaRange.size() * p.SLpattern.size() == p.nMLP);
  if (!rangeProduct) {
    if (p.phiRange.size() != 1 && p.phiRange.size() != p.nMLP) {
      B2ERROR("Number of phiRange lists should be 1 or " << p.nMLP);
      okay = false;
    }
    if (p.invptRange.size() != 1 && p.invptRange.size() != p.nMLP) {
      B2ERROR("Number of invptRange lists should be 1 or " << p.nMLP);
      okay = false;
    }
    if (p.thetaRange.size() != 1 && p.thetaRange.size() != p.nMLP) {
      B2ERROR("Number of thetaRange lists should be 1 or " << p.nMLP);
      okay = false;
    }
    if (p.SLpattern.size() != 1 && p.SLpattern.size() != p.nMLP) {
      B2ERROR("Number of SLpattern lists should be 1 or " << p.nMLP);
      okay = false;
    }
  }
  // ensure that number of input and target nodes are valid
  for (unsigned i = 0; i < p.nInput.size(); ++i) {
    if (p.nInput[i] % 3 != 0 || p.nInput[i] > 27) {
      B2ERROR("nInput should be a multiple of 3 <= 27");
      okay = false;
      continue;
    }
  }
  unsigned short nTarget = int(p.targetZ) + int(p.targetTheta);
  if (nTarget < 1) {
    B2ERROR("No outputs! Turn on either targetZ or targetTheta.");
    okay = false;
  }
  // ensure that sector ranges are valid
  for (unsigned iPhi = 0; iPhi < p.phiRange.size(); ++iPhi) {
    if (p.phiRange[iPhi].size() != 2) {
      B2ERROR("phiRange should be exactly 2 values");
      okay = false;
      continue;
    }
    if (p.phiRange[iPhi][0] >= p.phiRange[iPhi][1]) {
      B2ERROR("phiRange[0] should be smaller than phiRange[1]");
      okay = false;
    }
    if (p.phiRange[iPhi][0] < -360. || p.phiRange[iPhi][1] > 360. ||
        (p.phiRange[iPhi][1] - p.phiRange[iPhi][0]) > 360.) {
      B2ERROR("phiRange should be in [-360, 360], with maximal width of 360");
      okay = false;
    }
  }
  for (unsigned iPt = 0; iPt < p.invptRange.size(); ++iPt) {
    if (p.invptRange[iPt].size() != 2) {
      B2ERROR("invptRange should be exactly 2 values");
      okay = false;
    }
    if (p.invptRange[iPt][0] >= p.invptRange[iPt][1]) {
      B2ERROR("invptRange[0] should be smaller than invptRange[1]");
      okay = false;
    }
  }
  for (unsigned iTheta = 0; iTheta < p.thetaRange.size(); ++iTheta) {
    if (p.thetaRange[iTheta].size() != 2) {
      B2ERROR("thetaRange should be exactly 2 values");
      okay = false;
      continue;
    }
    if (p.thetaRange[iTheta][0] >= p.thetaRange[iTheta][1]) {
      B2ERROR("thetaRange[0] should be smaller than thetaRange[1]");
      okay = false;
    }
    if (p.thetaRange[iTheta][0] < 0. || p.thetaRange[iTheta][1] > 180.) {
      B2ERROR("thetaRange should be in [0, 180]");
      okay = false;
    }
  }
  for (unsigned iScale = 0; iScale < p.outputScale.size(); ++iScale) {
    if (p.outputScale[iScale].size() != 2 * nTarget) {
      B2ERROR("outputScale should be exactly " << 2 * nTarget << " values");
      okay = false;
    }
  }
  // ensure that train sectors are valid
  if (p.phiRange.size() != p.phiRangeTrain.size()) {
    B2ERROR("Number of phiRange lists and phiRangeTrain lists should be equal.");
    okay = false;
  } else {
    for (unsigned iPhi = 0; iPhi < p.phiRange.size(); ++iPhi) {
      if (p.phiRangeTrain[iPhi].size() != 2) {
        B2ERROR("phiRangeTrain should be exactly 2 values.");
        okay = false;
      } else if (p.phiRangeTrain[iPhi][0] > p.phiRange[iPhi][0] ||
                 p.phiRangeTrain[iPhi][1] < p.phiRange[iPhi][1]) {
        B2ERROR("phiRangeTrain should be wider than phiRange or equal.");
        okay = false;
      }
    }
  }
  if (p.invptRange.size() != p.invptRangeTrain.size()) {
    B2ERROR("Number of invptRange lists and invptRangeTrain lists should be equal.");
    okay = false;
  } else {
    for (unsigned iPt = 0; iPt < p.invptRange.size(); ++iPt) {
      if (p.invptRangeTrain[iPt].size() != 2) {
        B2ERROR("invptRangeTrain should be exactly 2 values.");
        okay = false;
      } else if (p.invptRangeTrain[iPt][0] > p.invptRange[iPt][0] ||
                 p.invptRangeTrain[iPt][1] < p.invptRange[iPt][1]) {
        B2ERROR("invptRangeTrain should be wider than invptRange or equal.");
        okay = false;
      }
    }
  }
  if (p.thetaRange.size() != p.thetaRangeTrain.size()) {
    B2ERROR("Number of thetaRange lists and thetaRangeTrain lists should be equal.");
    okay = false;
  } else {
    for (unsigned iTheta = 0; iTheta < p.thetaRange.size(); ++iTheta) {
      if (p.thetaRangeTrain[iTheta].size() != 2) {
        B2ERROR("thetaRangeTrain should be exactly 2 values.");
        okay = false;
      } else if (p.thetaRangeTrain[iTheta][0] > p.thetaRange[iTheta][0] ||
                 p.thetaRangeTrain[iTheta][1] < p.thetaRange[iTheta][1]) {
        B2ERROR("thetaRangeTrain should be wider than thetaRange or equal.");
        okay = false;
      }
    }
  }

  if (!okay) return;

  // initialize MLPs
  m_MLPs.clear();
  for (unsigned iMLP = 0; iMLP < p.nMLP; ++iMLP) {
    //get number of nodes for each layer
    unsigned short nInput = (p.nInput.size() == 1) ? p.nInput[0] : p.nInput[iMLP];
    vector<float> nHidden = (p.nHidden.size() == 1) ? p.nHidden[0] : p.nHidden[iMLP];
    vector<unsigned short> nNodes = {nInput};
    for (unsigned iHid = 0; iHid < nHidden.size(); ++iHid) {
      if (p.multiplyHidden) {
        nNodes.push_back(nHidden[iHid] * nNodes[0]);
      } else {
        nNodes.push_back(nHidden[iHid]);
      }
    }
    nNodes.push_back(nTarget);
    unsigned short targetVars = int(p.targetZ) + (int(p.targetTheta) << 1);
    //get sector ranges (initially train ranges)
    vector<unsigned> indices = getRangeIndices(p, iMLP);
    vector<float> phiRange = p.phiRangeTrain[indices[0]];
    vector<float> invptRange = p.invptRangeTrain[indices[1]];
    vector<float> thetaRange = p.thetaRangeTrain[indices[2]];
    unsigned short SLpattern = p.SLpattern[indices[3]];
    B2DEBUG(50, "Ranges for sector " << iMLP
            << ": phiRange [" << phiRange[0] << ", " << phiRange[1]
            << "], invptRange [" << invptRange[0] << ", " << invptRange[1]
            << "], thetaRange [" << thetaRange[0] << ", " << thetaRange[1]
            << "], SLpattern " << SLpattern);
    //get scaling values
    vector<float> outputScale = (p.outputScale.size() == 1) ? p.outputScale[0] : p.outputScale[iMLP];
    //convert phi and theta from degree to radian
    phiRange[0] *= Unit::deg;
    phiRange[1] *= Unit::deg;
    thetaRange[0] *= Unit::deg;
    thetaRange[1] *= Unit::deg;
    if (p.targetTheta) {
      outputScale[2 * int(p.targetZ)] *= Unit::deg;
      outputScale[2 * int(p.targetZ) + 1] *= Unit::deg;
    }
    //create new MLP
    m_MLPs.push_back(CDCTriggerMLP(nNodes, targetVars, outputScale,
                                   phiRange, invptRange, thetaRange, SLpattern, p.tMax));
  }
  // load some values from the geometry that will be needed for the input
  CDCGeometryPar& cdc = CDCGeometryPar::Instance();
  int layerId = 3;
  int nTS = 0;
  for (int iSL = 0; iSL < 9; ++iSL) {
    m_TSoffset[iSL] = nTS;
    nTS += cdc.nWiresInLayer(layerId);
    m_TSoffset[iSL + 1] = nTS;
    for (int priority = 0; priority < 2; ++ priority) {
      m_radius[iSL][priority] = cdc.senseWireR(layerId + priority);
    }
    layerId += (iSL > 0 ? 6 : 7);
  }
}

vector<unsigned>
NeuroTrigger::getRangeIndices(const Parameters& p, unsigned isector)
{
  std::vector<unsigned> indices = {0, 0, 0, 0};
  if (p.phiRange.size() * p.invptRange.size() * p.thetaRange.size() * p.SLpattern.size() == p.nMLP) {
    indices[0] = isector % p.phiRange.size();
    indices[1] = (isector / p.phiRange.size()) % p.invptRange.size();
    indices[2] = (isector / (p.phiRange.size() * p.invptRange.size())) % p.thetaRange.size();
    indices[3] = isector / (p.phiRange.size() * p.invptRange.size() * p.thetaRange.size());
  } else {
    indices[0] = (p.phiRange.size() == 1) ? 0 : isector;
    indices[1] = (p.invptRange.size() == 1) ? 0 : isector;
    indices[2] = (p.thetaRange.size() == 1) ? 0 : isector;
    indices[3] = (p.SLpattern.size() == 1) ? 0 : isector;
  }
  return indices;
}

int
NeuroTrigger::selectMLP(const CDCTriggerTrack& track)
{
  if (m_MLPs.size() == 0) {
    B2WARNING("Trying to select MLP before initializing MLPs.");
    return -1;
  }

  float phi0 = track.getPhi0();
  float pt = track.getTransverseMomentum() * track.getChargeSign();
  float theta = atan2(1., track.getCotTheta());

  // find sector
  // ranges should be unique
  // if several sectors match, first in the list is taken
  int bestIndex = -1;
  for (unsigned isector = 0; isector < m_MLPs.size(); ++isector) {
    if (m_MLPs[isector].inPhiRange(phi0) && m_MLPs[isector].inPtRange(pt)
        && m_MLPs[isector].inThetaRange(theta)) {
      unsigned short hitPattern = getInputPattern(isector);
      unsigned short sectorPattern = m_MLPs[isector].getSLpattern();
      B2DEBUG(250, "hitPattern " << hitPattern << " sectorPattern " << sectorPattern);
      // no hit pattern restriction -> keep looking for exact match
      if (sectorPattern == 0) {
        B2DEBUG(250, "found match for general sector");
        bestIndex = isector;
      }
      // exact match -> keep this sector
      if (hitPattern == sectorPattern) {
        B2DEBUG(250, "found match for hit pattern " << hitPattern);
        bestIndex = isector;
        break;
      }
    }
  }

  if (bestIndex < 0) {
    B2DEBUG(150, "Track does not match any sector.");
    B2DEBUG(150, "pt=" << pt << ", phi=" << phi0 * 180. / M_PI << ", theta=" << theta * 180. / M_PI);
  }

  return bestIndex;
}

vector<int>
NeuroTrigger::selectMLPs(const CDCTriggerTrack& track,
                         const MCParticle& mcparticle, bool selectByMC)
{
  vector<int> indices = {};

  if (m_MLPs.size() == 0) {
    B2WARNING("Trying to select MLP before initializing MLPs.");
    return indices;
  }

  float phi0 = track.getPhi0();
  float pt = track.getTransverseMomentum() * track.getChargeSign();
  float theta = atan2(1., track.getCotTheta());

  if (selectByMC) {
    phi0 = mcparticle.getMomentum().Phi();
    pt = mcparticle.getMomentum().Pt() * mcparticle.getCharge();
    theta = mcparticle.getMomentum().Theta();
  }

  // find all matching sectors
  for (unsigned isector = 0; isector < m_MLPs.size(); ++isector) {
    if (m_MLPs[isector].inPhiRange(phi0) && m_MLPs[isector].inPtRange(pt)
        && m_MLPs[isector].inThetaRange(theta)) {
      indices.push_back(isector);
    }
  }

  if (indices.size() == 0) {
    B2DEBUG(150, "Track does not match any sector.");
    B2DEBUG(150, "pt=" << pt << ", phi=" << phi0 * 180. / M_PI << ", theta=" << theta * 180. / M_PI);
  }

  return indices;
}

void
NeuroTrigger::updateTrack(const CDCTriggerTrack& track)
{
  double omega = track.getOmega(); // signed track curvature
  for (int iSL = 0; iSL < 9; ++iSL) {
    for (int priority = 0; priority < 2; ++priority) {
      double phiPt = M_PI_2;
      if (m_radius[iSL][priority] < 2. / abs(omega))
        phiPt = asin(m_radius[iSL][priority] * omega / 2.);
      m_idRef[iSL][priority] = remainder(((track.getPhi0() - phiPt) *
                                          (m_TSoffset[iSL + 1] - m_TSoffset[iSL]) / 2. / M_PI),
                                         (m_TSoffset[iSL + 1] - m_TSoffset[iSL]));
      m_arclength[iSL][priority] = 2. * phiPt / omega;
    }
  }
}

double
NeuroTrigger::getRelId(const CDCTriggerSegmentHit& hit)
{
  int iSL = hit.getISuperLayer();
  int priority = hit.getPriorityPosition();
  double relId = hit.getSegmentID() + 0.5 * (((priority >> 1) & 1) - (priority & 1))
                 - m_TSoffset[iSL] - m_idRef[iSL][int(priority < 3)];
  relId = remainder(relId, (m_TSoffset[iSL + 1] - m_TSoffset[iSL]));
  return relId;
}

unsigned short
NeuroTrigger::getInputPattern(unsigned isector)
{
  StoreArray<CDCTriggerSegmentHit> hits("CDCTriggerSegmentHits");
  CDCTriggerMLP& expert = m_MLPs[isector];
  unsigned short hitPattern = 0;
  // loop over hits
  for (int ihit = 0; ihit < hits.getEntries(); ++ ihit) {
    unsigned short iSL = hits[ihit]->getISuperLayer();
    // get priority time (TODO: get event time)
    int t = hits[ihit]->priorityTime();
    if (t < 0 || t > expert.getTMax()) continue;
    double relId = getRelId(*hits[ihit]);
    if (expert.isRelevant(relId, iSL)) {
      hitPattern |= 1 << iSL;
      B2DEBUG(250, "hit in SL " << iSL);
    }
  }
  B2DEBUG(250, "hitPattern " << hitPattern);
  return hitPattern;
}

vector<float>
NeuroTrigger::getInputVector(unsigned isector)
{
  StoreArray<CDCTriggerSegmentHit> hits("CDCTriggerSegmentHits");
  CDCTriggerMLP& expert = m_MLPs[isector];
  // prepare empty input vector and vectors to keep best drift times
  vector<float> inputVector;
  inputVector.assign(expert.nNodesLayer(0), 0.);
  vector<int> tMin;
  tMin.assign(9, expert.getTMax());
  vector<bool> LRknown;
  LRknown.assign(9, false);
  vector<int> hitIds;
  hitIds.assign(9, -1);
  // loop over hits, choosing only 1 per superlayer
  for (int ihit = 0; ihit < hits.getEntries(); ++ ihit) {
    unsigned short iSL = hits[ihit]->getISuperLayer();
    if (inputVector.size() <= 3 * iSL) continue;
    if (expert.getSLpattern() > 0 && !((expert.getSLpattern() >> iSL) & 1)) {
      B2DEBUG(250, "skipping hit in SL " << iSL);
      continue;
    }
    int priority = hits[ihit]->getPriorityPosition();
    // get priority time (TODO: get event time)
    int t = hits[ihit]->priorityTime();
    if (t < 0) continue;
    int LR = hits[ihit]->getLeftRight();
    double relId = getRelId(*hits[ihit]);
    if (expert.isRelevant(relId, iSL) && t <= expert.getTMax()) {
      // choose best hit (LR known before LR unknown, then shortest drift time)
      bool useHit = false;
      if (LRknown[iSL]) {
        useHit = (hits[ihit]->LRknown() && t <= tMin[iSL]);
      } else {
        useHit = (hits[ihit]->LRknown() || t <= tMin[iSL]);
      }
      if (useHit) {
        // keep drift time and LR
        LRknown[iSL] = hits[ihit]->LRknown();
        tMin[iSL] = t;
        hitIds[iSL] = ihit;
        // get scaled input values: (relId, t, 2D arclength)
        inputVector[3 * iSL] = expert.scaleId(relId, iSL);
        inputVector[3 * iSL + 1] = (((LR >> 1) & 1) - (LR & 1)) * t / float(expert.getTMax());
        inputVector[3 * iSL + 2] = 2. * (m_arclength[iSL][int(priority < 3)] - m_radius[iSL][0])
                                   / (M_PI_2 * m_radius[iSL][1] - m_radius[iSL][0]) - 1.;
      }
    }
  }
  // save selected hit Ids (for making relations to track)
  m_selectedHitIds.clear();
  for (unsigned iSL = 0; iSL < 9; ++iSL) {
    if (hitIds[iSL] >= 0) m_selectedHitIds.push_back(hitIds[iSL]);
  }
  return inputVector;
}

vector<float>
NeuroTrigger::runMLP(unsigned isector, vector<float> input)
{
  const CDCTriggerMLP& expert = m_MLPs[isector];
  vector<float> weights = expert.getWeights();
  vector<float> layerinput = input;
  vector<float> layeroutput = {};
  unsigned iw = 0;
  for (unsigned il = 1; il < expert.nLayers(); ++il) {
    //add bias input
    layerinput.push_back(1.);
    //prepare output
    layeroutput.clear();
    layeroutput.assign(expert.nNodesLayer(il), 0.);
    //loop over outputs
    for (unsigned io = 0; io < layeroutput.size(); ++io) {
      //loop over inputs
      for (unsigned ii = 0; ii < layerinput.size(); ++ii) {
        layeroutput[io] += layerinput[ii] * weights[iw++];
      }
      //apply activation function
      layeroutput[io] = tanh(layeroutput[io] / 2.);
    }
    //output is new input
    layerinput = layeroutput;
  }
  return expert.unscaleTarget(layeroutput);
}

void
NeuroTrigger::save(const string& filename, const string& arrayname)
{
  B2INFO("Saving networks to file " << filename << ", array " << arrayname);
  TFile datafile(filename.c_str(), "UPDATE");
  TObjArray* MLPs = new TObjArray(m_MLPs.size());
  for (unsigned isector = 0; isector < m_MLPs.size(); ++isector) {
    MLPs->Add(&m_MLPs[isector]);
  }
  MLPs->Write(arrayname.c_str(), TObject::kSingleKey | TObject::kOverwrite);
  datafile.Close();
  MLPs->Clear();
  delete MLPs;
}

bool
NeuroTrigger::load(const string& filename, const string& arrayname)
{
  TFile datafile(filename.c_str(), "READ");
  if (!datafile.IsOpen()) {
    B2WARNING("Could not open file " << filename);
    return false;
  }
  TObjArray* MLPs = (TObjArray*)datafile.Get(arrayname.c_str());
  if (!MLPs) {
    datafile.Close();
    B2WARNING("File " << filename << " does not contain key " << arrayname);
    return false;
  }
  m_MLPs.clear();
  for (int isector = 0; isector < MLPs->GetEntriesFast(); ++isector) {
    CDCTriggerMLP* expert = dynamic_cast<CDCTriggerMLP*>(MLPs->At(isector));
    if (expert) m_MLPs.push_back(*expert);
    else B2WARNING("Wrong type " << MLPs->At(isector)->ClassName() << ", ignoring this entry.");
  }
  MLPs->Clear();
  delete MLPs;
  datafile.Close();
  B2DEBUG(100, "loaded " << m_MLPs.size() << " networks");

  // load some values from the geometry that will be needed for the input
  CDCGeometryPar& cdc = CDCGeometryPar::Instance();
  int layerId = 3;
  int nTS = 0;
  for (int iSL = 0; iSL < 9; ++iSL) {
    m_TSoffset[iSL] = nTS;
    nTS += cdc.nWiresInLayer(layerId);
    m_TSoffset[iSL + 1] = nTS;
    for (int priority = 0; priority < 2; ++ priority) {
      m_radius[iSL][priority] = cdc.senseWireR(layerId + priority);
    }
    layerId += (iSL > 0 ? 6 : 7);
  }
  return true;
}
