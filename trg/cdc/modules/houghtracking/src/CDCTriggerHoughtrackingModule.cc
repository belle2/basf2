/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Sara Neuhaus                            *
 *                                                                        *
 **************************************************************************/

#include <trg/cdc/modules/houghtracking/CDCTriggerHoughtrackingModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <cmath>

using namespace std;
using namespace Belle2;
using namespace Belle2::CDC;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCTriggerHoughtracking)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCTriggerHoughtrackingModule::CDCTriggerHoughtrackingModule() : Module()
{
  //Set module properties
  setDescription("Hough tracking algorithm for SVD data");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Define module parameters
  addParam("nCellsPhi", m_nCellsPhi,
           "Number of Hough cells in phi (limits: [-180, 180]). Must be an even number.",
           (unsigned)(160));
  addParam("nCellsR", m_nCellsR,
           "Number of Hough cells in 1/r. Must be an even number.",
           (unsigned)(32));
  addParam("minPt", m_minPt,
           "Minimum Pt [GeV]. "
           "Hough plane limits in 1/r are [-1/r(minPt), 1/r(minPt)]", (double)(0.3));

  addParam("minHits", m_minHits,
           "Minimum hits from different super layers required in a peak cell.",
           (unsigned)(4));
  addParam("minCells", m_minCells,
           "Peaks with less than minCells connected cells are ignored.",
           (unsigned)(1));
  addParam("onlyLocalMax", m_onlyLocalMax,
           "Switch to remove cells connected to a cell with higher super layer count.",
           false);
  addParam("connect", m_connect,
           "Definition for connected cells. 4: direct (left/right/top/bottom), "
           "6: direct + 2 diagonal (top right/bottom left), "
           "8: direct + all diagonal (top right/top left/bottom right/bottom left)",
           (unsigned)(6));
  addParam("ignore2ndPriority", m_ignore2nd,
           "Switch to skip second priority hits.", false);
  addParam("storeHoughPlane", m_storePlane,
           "Switch for saving Hough plane as TMatrix in DataStore. "
           "0: don't store anything, 1: store only peaks, 2: store full plane "
           "(will increase runtime).", (unsigned)(0));
}

void
CDCTriggerHoughtrackingModule::initialize()
{
  StoreArray<CDCTriggerSegmentHit>::required();
  StoreArray<CDCTriggerTrack>::registerPersistent("CDCTriggerTracks");

  StoreArray<CDCTriggerSegmentHit> segmentHits;
  StoreArray<CDCTriggerTrack> tracks("CDCTriggerTracks");
  StoreArray<MCParticle> mcparticles;

  tracks.registerRelationTo(segmentHits);
  tracks.registerRelationTo(mcparticles);

  if (m_storePlane > 0) StoreObjPtr<TMatrix>::registerPersistent("HoughPlane");

  CDCGeometryPar& cdc = CDCGeometryPar::Instance();
  int layerId = 3;
  int nTS = 0;
  for (int iSL = 0; iSL < 9; ++iSL) {
    TSoffset[iSL] = nTS;
    nTS += cdc.nWiresInLayer(layerId);
    TSoffset[iSL + 1] = nTS;
    for (int priority = 0; priority < 2; ++ priority) {
      radius[iSL][priority] = cdc.senseWireR(layerId + priority);
    }
    layerId += (iSL > 0 ? 6 : 7);
  }
}

void
CDCTriggerHoughtrackingModule::event()
{
  StoreArray<CDCTriggerSegmentHit> tsHits("CDCTriggerSegmentHits");
  StoreArray<CDCTriggerTrack> storeTracks("CDCTriggerTracks");
  StoreArray<MCParticle> particles("MCParticles");

  /* Clean hits */
  hitMap.clear();
  houghCand.clear();
  houghTrack.clear();

  if (tsHits.getEntries() == 0) {
    B2WARNING("CDCTracking: tsHitsCollection is empty!");
    return;
  }

  /* get CDCHits coordinates in conformal space */
  for (int iHit = 0; iHit < tsHits.getEntries(); iHit++) {
    unsigned short iSL = tsHits[iHit]->getISuperLayer();
    if (iSL % 2) continue;
    if (m_ignore2nd && tsHits[iHit]->getPriorityPosition() < 3) continue;
    double phi = tsHits[iHit]->getSegmentID() - TSoffset[iSL]
                 + 0.5 * (((tsHits[iHit]->getPriorityPosition() >> 1) & 1)
                          - (tsHits[iHit]->getPriorityPosition() & 1));
    phi = phi * 2. * M_PI / (TSoffset[iSL + 1] - TSoffset[iSL]);
    double r = radius[iSL][int(tsHits[iHit]->getPriorityPosition() < 3)];
    TVector2 pos(cos(phi) / r, sin(phi) / r);
    hitMap.insert(std::make_pair(iHit, std::make_pair(iSL, pos)));
  }

  /* Extent the Hough plane such that the cell number is a power of 2 (same for x and y).
   * This is for the fast peak finder, which divides the Hough plane in half in each step.
   * Peaks found outside of the actual limits are ignored. */
  maxIterations = ceil(log2(max(m_nCellsPhi, m_nCellsR))) - 1;
  nCells = pow(2, maxIterations + 1);
  /* limits in phi: [-pi, pi] + extra cells */
  double rectX = M_PI * nCells / m_nCellsPhi;
  /* limits in R: [-R(minPt), R(minPt)] + extra cells */
  maxR = 0.5 * Const::speedOfLight * 1.5e-4 / m_minPt;
  double rectY = maxR * nCells / m_nCellsR;

  B2DEBUG(50, "extending Hough plane to " << maxIterations << " iterations, "
          << nCells << " cells: phi in ["
          << -rectX * 180. / M_PI << ", " << rectX * 180. / M_PI
          << "] deg, 1/r in [" << -rectY << ", " << rectY << "] cm");

  /* prepare matrix for storing the Hough plane */
  if (m_storePlane > 0) {
    StoreObjPtr<TMatrix> plane("HoughPlane");
    plane.create();
    plane->ResizeTo(m_nCellsPhi, m_nCellsR);
  }

  /* find track candidates in Hough plane */
  fastInterceptFinder(hitMap, -rectX, rectX, -rectY, rectY, 0, 0, 0);

  /* merge track candidates */
  connectedRegions();

  /* write tracks to datastore */
  vector<unsigned> idList;
  TVector2 coord;
  double pt, phi;
  int charge;

  if (!storeTracks.isValid()) {
    storeTracks.create();
  } else {
    storeTracks.getPtr()->Clear();
  }

  for (auto it = houghTrack.begin(); it != houghTrack.end(); ++it) {
    idList = it->getIdList();
    coord = it->getCoord();

    pt = 0.5 / abs(coord.Y()) * Const::speedOfLight * 1.5e-4;
    charge = coord.Y() / abs(coord.Y());
    phi = coord.X();

    CDCTriggerTrack* track = storeTracks.appendNew();
    /* Hough transformation was defined to give phi of momentum vector
     * current track definition takes phi of circle center as input */
    track->add2DHoughResult(charge, pt, remainder(phi - M_PI_2, 2. * M_PI));
    //track->add2DHoughResult(charge, pt, phi);

    // relations
    vector<unsigned> relParticleIds[5];
    for (unsigned i = 0; i < idList.size(); ++i) {
      unsigned its = idList[i];
      track->addRelationTo(tsHits[its]);
      unsigned iax = tsHits[its]->getISuperLayer() / 2;
      RelationVector<MCParticle> mcrel = tsHits[its]->getRelationsFrom<MCParticle>();
      for (unsigned imc = 0; imc < mcrel.size(); ++imc) {
        relParticleIds[iax].push_back(mcrel[imc]->getArrayIndex());
      }
    }
    for (unsigned iax = 0; iax < 5; ++iax) {
      double weight = 1. / (5 * relParticleIds[iax].size());
      for (unsigned imc = 0; imc < relParticleIds[iax].size(); ++imc) {
        track->addRelationTo(particles[relParticleIds[iax][imc]], weight);
      }
    }
    RelationArray tracksToParticles(storeTracks, particles);
    if (tracksToParticles.getEntries() > 0)
      tracksToParticles.consolidate();
  }
}
