/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cmath>
#include <cfloat>
#include "framework/datastore/StoreArray.h"
#include "cdc/geometry/CDCGeometryPar.h"
#include "cdc/dataobjects/CDCHit.h"
#include "hlt/hlt/modules/level3/FTFinder.h"

using namespace Belle2;
using namespace L3;

#include <TVector3.h>

//...Globals...
FTFinder*
FTFinder::s_tFinder = NULL;

FTFinder&
FTFinder::instance(void)
{
  if (!s_tFinder) s_tFinder = new FTFinder();
  return *s_tFinder;
}

FTFinder::FTFinder()
  : m_tOffSet(FLT_MAX),
    m_xtCoEff(FLT_MAX),
    m_xtCoEff2(FLT_MAX),
    m_tWindow(400.),
    m_tZero(0.),
    m_wire(NULL),
    m_layer(NULL),
    m_superLayer(NULL),
    m_Nwire(0),
    m_Nlayer(0),
    m_NsuperLayer(0),
    m_tracks(*(new FTList<FTTrack * >(20))),
    m_linkedSegments(new FTList<FTSegment * >(6)),
    m_tWindowLow(0.),
    m_tWindowHigh(0.),
    m_vx(0.),
    m_vy(0.),
    m_vz(0.),
    m_EvtByEvtBadWires(NULL)
{
}

FTFinder::FTFinder(const FTFinder& o)
  : m_tOffSet(o.m_tOffSet),
    m_xtCoEff(o.m_xtCoEff),
    m_xtCoEff2(o.m_xtCoEff2),
    m_tWindow(o.m_tWindow),
    m_tZero(o.m_tZero),
    m_wire(o.m_wire),
    m_layer(o.m_layer),
    m_superLayer(o.m_superLayer),
    m_Nwire(o.m_Nwire),
    m_Nlayer(o.m_Nlayer),
    m_NsuperLayer(o.m_NsuperLayer),
    m_tracks(*(new FTList<FTTrack * >(20))),
    m_linkedSegments(new FTList<FTSegment * >(6)),
    m_tWindowLow(0.),
    m_tWindowHigh(0.),
    m_vx(0.),
    m_vy(0.),
    m_vz(0.),
    m_EvtByEvtBadWires(NULL)
{
}

void
FTFinder::init()
{
  clearGeometry();

  // local arrays
  CDC::CDCGeometryPar& cdc2 =
    CDC::CDCGeometryPar::Instance();

  m_Nlayer = cdc2.nWireLayers();

  int sup_Nlayer[MAX_N_SLAYERS];
  for (unsigned i = 0; i < MAX_N_SLAYERS; i++) sup_Nlayer[i] = 0;

  int iWir(0);
  int lLyr(0);
  int iSup(-1);

  int nShiftsBack = -99999;
  for (lLyr = 0; lLyr < m_Nlayer; lLyr++) {
    const int nWiresInLayer = cdc2.nWiresInLayer(lLyr);
    const int nShifts = cdc2.nShifts(lLyr);

    if (nShifts != nShiftsBack) {
      // superlayer
      sup_Nlayer[++iSup] = 0;
      nShiftsBack = nShifts;
    }
    // layer
    sup_Nlayer[iSup]++;
    iWir += nWiresInLayer;
  }

  // allocate memory for geometry
  m_Nwire = iWir;
  m_NsuperLayer = iSup + 1;
  m_wire = static_cast<FTWire*>(malloc((m_Nwire + 1) * sizeof(FTWire)));
  m_layer = static_cast<FTLayer*>(malloc(m_Nlayer * sizeof(FTLayer)));
  m_superLayer =
    static_cast<FTSuperLayer*>(malloc(m_NsuperLayer * sizeof(FTSuperLayer)));

  if (!m_wire) B2FATAL("Realloc failed for wires");
  if (!m_layer) B2FATAL("Realloc failed for layers");
  if (!m_superLayer) B2FATAL("Realloc failed for super layers");

  nShiftsBack = -99999;
  iSup = -1;
  iWir = 0;
  int lyr_localID = 0;
  for (lLyr = 0; lLyr < m_Nlayer; lLyr++) {
    const int nWiresInLayer = cdc2.nWiresInLayer(lLyr);
    const int nShifts = cdc2.nShifts(lLyr);
    if (nShifts != nShiftsBack) {
      // superlayer
      iSup++;
      new(m_superLayer + iSup) FTSuperLayer(sup_Nlayer[iSup], iSup,
                                            (nShifts == 0) ? 1 : 0,
                                            0 == iSup, m_layer + lLyr);
      lyr_localID = 0;
      nShiftsBack = nShifts;
    }
    // layer
    double slant = 0.;
    //double slant2 = 0.;
    if (nShifts) {
      double delfi = (double)nShifts * M_PI / (double)cdc2.nWiresInLayer(lLyr);
      double sinhdel = std::sin(delfi / 2.0);
      slant = std::atan2(2.0 * (cdc2.senseWireR(lLyr)) * sinhdel,
                         cdc2.senseWireFZ(lLyr) - cdc2.senseWireBZ(lLyr));
      //TVector3 fb(cdc2.wireForwardPosition(lLyr, 0) -
      //    cdc2.wireBackwardPosition(lLyr, 0));
      //slant2 = nShifts/abs(nShifts) * std::atan2(fb.Perp(), fb.z());
    }
    //B2INFO("layer:" << lLyr << " nwires=" << nWiresInLayer << " nshifts=" << nShifts << " slant=" << slant << " slant2=" << slant2 << " nLocalLayer=" << sup_Nlayer[iSup] << " iSup=" << iSup << " lyr_localID=" << lyr_localID << " nWires" << cdc2.nWiresInLayer(lLyr) << " offset=" << cdc2.offset(lLyr));
    new(m_layer + lLyr) FTLayer(cdc2.senseWireR(lLyr), slant,
                                cdc2.senseWireFZ(lLyr), cdc2.senseWireBZ(lLyr),
                                cdc2.offset(lLyr), lLyr, lyr_localID++,
                                cdc2.nWiresInLayer(lLyr), m_superLayer[iSup],
                                iSup ? 0 : 1, m_wire + iWir);
    for (int j = 0; j < nWiresInLayer; j++) {
      // wire
      const TVector3 wf(cdc2.wireForwardPosition(lLyr, j));
      const TVector3 wb(cdc2.wireBackwardPosition(lLyr, j));
      if (0 == nShifts) { // axial wire
        new(m_wire + iWir) FTWire(0.5 * (wb.x() + wf.x()),
                                  0.5 * (wb.y() + wf.y()),
                                  0., 0., m_layer[lLyr], j);
      } else {           // stereo wire
        new(m_wire + iWir) FTWire(wb.x(), wb.y(), wf.x() - wb.x(),
                                  wf.y() - wb.y(), m_layer[lLyr], j);
      }
      iWir++;
    }
  }

  new(m_wire + m_Nwire) FTWire(); // virtual wire for boundary cell's neighbor

  for (int i = 0; i < m_Nwire; i++) {
    m_wire[i].set_neighbors(m_wire + m_Nwire);
  }
  /*
  for (int j = 1; j < 4; j++){
    FTLayer& ll = *(m_superLayer[0].firstLayer() + j);
    for (int i = 0; i < ll.nWire(); i++){
      FTWire& w = *(ll.firstWire() + i);
      B2INFO("layer=" << j << " this_wire=" << getWireId(&w) <<
       " inL=" << getWireId(*(w.neighborPtr())) <<
       " inR=" << getWireId(*(w.neighborPtr()+1)) <<
       " L=" << getWireId(*(w.neighborPtr()+2)) <<
       " R=" << getWireId(*(w.neighborPtr()+3)) <<
       " outL=" << getWireId(*(w.neighborPtr()+4)) <<
       " outR=" << getWireId(*(w.neighborPtr()+5)));
    }
  }
  */
}

void
FTFinder::term()
{
  clear();
  if (m_EvtByEvtBadWires) delete m_EvtByEvtBadWires;
  delete &m_tracks;
  delete m_linkedSegments;
  clearGeometry();
}

void
FTFinder::beginRun()
{

  // set default x-t parameters
  m_tOffSet = -15.;
  m_xtCoEff = 0.025;
  m_xtCoEff2 = 0.6;
  m_tWindowLow = -m_tWindow;
  m_tWindowHigh = m_tWindow + 300.;
  FTTrack::s_additionalTdcCuts = true;

  // read bad wires, T0 and pedestal
  m_tZero = 8340.;

}

void
FTFinder::event(const int findEventVertex)
{
  //--
  // clear old information
  //--
  clear();

  //--
  // set event by event bad wires
  //--
  if (m_EvtByEvtBadWires) {
    setBadWires(true, false);
  }

  //--
  // update wirehit information
  //--
  updateCdc3();

  //--
  // segment finding
  //--
  for (int i =  0; i ^ m_NsuperLayer; i++) {
    (*(m_superLayer + i)).mkSegmentList();
  }
  //--
  // axial segment liking
  //--
  mkTrackList();

  //--
  // 2D track fitting
  //--
  (*m_superLayer).reAppendSalvage();
  int n = m_tracks.length();
  for (int i = 0; i ^ n; i++) {
    if (!m_tracks[i]->rPhiFit()) {
      delete m_tracks[i];
      n = m_tracks.remove(i);
    }
  }
  if (!n) return;

  ///--
  /// vertex(r-phi) fit and event timing correction
  //--
  int vtxFlag = VertexFit(0);
  FTTrack::setEvtTiming(CorrectEvtTiming());

  ///--
  /// 2D track re-fitting
  //--
  for (int i = 0; i ^ n; i++) {
    m_tracks[i]->rPhiReFit(m_vx, m_vy, vtxFlag);
  }


  //--
  // stereo segment linking
  //--
  mkTrack3D();

  //--
  // final track fittng
  //--
  n = m_tracks.length();
  for (int i = 0; i ^ n; i++) {
    m_tracks[i]->szFit();
  }

  //--
  // find primary event vertex
  //--
  if (findEventVertex) {
    VertexFit(1);
    //findBestVertex();
  }

}

void
FTFinder::updateCdc3(void)
{

  StoreArray<CDCHit> CDCHits("CDCHits");
  if (!CDCHits) {
    B2WARNING("no CDCHits");
  }

  const int nHits = CDCHits.getEntries();

  for (int i = 0; i < nHits; i++) {

    const CDCHit& h = * CDCHits[i];

    double time = m_tZero - (double)h.getTDCCount();
    //double adc = (double)h.getADCCount();

    FTSuperLayer& sup = m_superLayer[h.getISuperLayer()];
    FTLayer& layer = *(sup.firstLayer() + h.getILayer());
    FTWire& wire = *(layer.firstWire() + h.getIWire());
    //B2INFO("sup=" << sup.superLayerId() << " layerId=" << layer.layerId() << " localId=" << h.getIWire() << " wid=" << getWireId(&wire) << " time=" << time);

    //... Remove bad wire
    if (wire.stateAND(FTWire::Dead)) continue;

    if (time < m_tWindowLow || time > m_tWindowHigh) continue;

    //... execute adc cuts
    //if (!adc_cut_(&m_ExpNo,&m_RunNo,&adc,&layer,&local)) continue;

    wire.distance(t2x(layer, time));
    wire.state(FTWire::Hit);
    sup.appendHit(&wire);

    wire.time(time);
  }


#ifdef FZISAN_DEBUG
  StoreArray<MCParticle> mcParticles;
  RelationArray relsMC(mcParticles, CDCHits);
  const unsigned nRelsMC = relsMC.getEntries();

  for (int i = 0; i < nHits; i++) {
    const CDCHit& h = * CDCHits[i];

    const int layerId = h.getILayer();
    //const int localId = h.getIWire();

    FTLayer& layer = m_layer[layerId];
    FTWire& wire = layer.wire(local);

    int iMCPart = -1;
    for (unsigned j = 0; j < nRelsMC; j++) {
      const unsigned k = relsMC[j].getToIndices().size();
      for (unsigned l = 0; l < k; l++) {
        if (relsMC[j].getToIndex(l) == i) {
          iMCPart = relsMC[j].getFromIndex();
          break;
        }
      }
    }
    if (iMCPart >= 0) {
      wire.mcPart(&mcParticles[iMCPart]);
    }
  }
#endif
}

void
FTFinder::clear(void)
{
#ifdef FZISAN_DEBUG
  FTWire* w = m_wire + m_Nwire;
  FTWire* const last = m_wire;
  while (w != last)(*(--w)).mcPart(NULL);
#endif
  FTTrack::setEvtTiming(0.);
  if (m_superLayer) {
    for (int i = 0; i ^ m_NsuperLayer; i++)(*(m_superLayer + i)).clear();
  }
  m_tracks.deleteAll();
  m_vx = -99999.;
  m_vy = -99999.;
  m_vz = -99999.;
}

void
FTFinder::clearGeometry(void)
{
  if (m_superLayer) {
    for (int i = 0; i < m_NsuperLayer; i++) m_superLayer[i].~FTSuperLayer();
    free(m_superLayer);
    m_superLayer = NULL;
  }
  if (m_layer) {
    for (int i = 0; i < m_Nlayer; i++) m_layer[i].~FTLayer();
    free(m_layer);
    m_layer = NULL;
  }
  if (m_wire) {
    for (int i = 0; i < m_Nwire + 1; i++) m_wire[i].~FTWire();
    free(m_wire);
    m_wire = NULL;
  }
}

void
FTFinder::setBadWires(const bool EvtByEvt, const bool inBeginRun)
{
  if (m_EvtByEvtBadWires) {
    // reset EvtByEvt bad wires
    const int n = m_EvtByEvtBadWires->length();
    for (int i = 0; i != n; i++) {
      (*m_EvtByEvtBadWires)[i]->stateORXOR(FTWire::StateMask);
    }
    m_EvtByEvtBadWires->clear();
    if (inBeginRun) {
      delete m_EvtByEvtBadWires;
      m_EvtByEvtBadWires = NULL;
    }
  }
  if (EvtByEvt && inBeginRun) {
    m_EvtByEvtBadWires = new FTList<FTWire*>(100);
  } else {
    // set bad wires
  }
}

void
FTFinder::mkTrackList(void)
{
  FTTrack** tracks = new  FTTrack* [6];
  int* Nsame = new int[6];
  for (int k = 8; k ^ 2; k -= 2) {
    FTList<FTSegment*>& initials = m_superLayer[k].segments();
    const int Ninitials = initials.length();
    for (int l = 0; l ^ Ninitials; l++) {
      if (initials[l]->track()) continue;
      FTTrack* trackCand = linkAxialSegments(initials[l]);
      if (!trackCand) continue;
      m_linkedSegments = new FTList<FTSegment*>(6);
      //compair this to appended track candidate
      FTList<FTSegment*> segments = trackCand->getAxialSegments();
      int n = segments.length();
      int m = 0;
      int Nmult = 0;
      for (int i = 0; i < n; i++) {
        FTTrack* trkTmp = segments[i]->track();
        if (!trkTmp) continue;
        Nmult++;
        int alreadyhas = 0;
        for (int j = 0; j < m; j++) {
          if (trkTmp != *(tracks + j)) continue;
          alreadyhas = 1;
          (*(Nsame + j))++;
        }
        if (!alreadyhas) {
          *(tracks + m) = trkTmp;
          *(Nsame + m) = 1;
          m++;
        }
      }
      int  max_Nsame = 0;
      FTTrack* track_of_max = NULL;
      for (int i = 0; i ^ m; i++) {
        if (*(Nsame + i) > max_Nsame) {
          max_Nsame = *(Nsame + i);
          track_of_max = *(tracks + i);
        }
      }

      if (2 * Nmult > n) {
        if (2 * max_Nsame > Nmult) { //compair which is better
          FTTrack* t = NULL;
          int cache_i = -1;
          int nTrks = m_tracks.length();
          for (int i = 0; i ^ nTrks; i++) {
            if (m_tracks[i] == track_of_max) {
              t = m_tracks[i];
              cache_i = i;
            }
          }
          FTList<FTSegment*>& segments2 = t->getAxialSegments();
          int n2 = segments2.length();
          if (n > n2 + 1 ||
              (n + 1 >= n2 && trackCand->chi2KappaTmp() < t->chi2KappaTmp())) {
            // swap this and appended one
            for (int i = 0; i < n2; i++) {
              segments2[i]->track(NULL);
            }
            for (int i = 0; i < n; i++) {
              segments[i]->track(trackCand);
            }
            m_tracks.replace(cache_i, trackCand);
            delete t;
          } else {
            delete trackCand;
          }
        } else {
          delete trackCand;
        }
      } else {
        // append this
        for (int i = 0; i < n; i++) {
          segments[i]->track(trackCand);
        }
        m_tracks.append(trackCand);
      }
    }
  }
  delete[] tracks;
  delete[] Nsame;
}


FTTrack*
FTFinder::linkAxialSegments(const FTSegment* initial)
{
  static const double alpha(222.37606);
  double chi2Kappa = 3000.;
  m_linkedSegments->clear();
  int n = m_linkedSegments->append(const_cast<FTSegment*>(initial));
  double SigmaK = initial->kappa();
  double SigmaRR = initial->r(); SigmaRR *= SigmaRR;
  double SigmaKRR = SigmaK * SigmaRR;
  double SigmaKKRR = SigmaK * SigmaKRR;
  int currentLayer = initial->superLayer().superLayerId();
  do {      // loop over linked segments
    FTSegment& s = * (*m_linkedSegments)[n - 1];
    FTSegment* innerSegment = NULL;
    double SigmaKCache(0.), SigmaRRCache(0.), SigmaKRRCache(0.), SigmaKKRRCache(0.);
    double minChi2 = 4000.;
    double inX = s.incomingX();
    double inY = s.incomingY();
    const FTWire& innerBoundHit = * s.getInnerBoundHits().first();
    double in_r = innerBoundHit.layer().r();
    double incomingPhi = innerBoundHit.phi();
    int j = currentLayer;
    int limit = (n == 1) ? 2 : 0;
    do {      // loop over superLayers to link
      j -= 2;
      FTList<FTSegment*>& NextLayerSegments = m_superLayer[j].segments();
      int m = NextLayerSegments.length();
      for (int k = 0; k ^ m; k++) { // loop over segments in a superLayer
        FTSegment& next = * NextLayerSegments[k];
        const FTWire& NextOuterBoundHit = * next.getOuterBoundHits().first();
        double deltaPhi = fabs(incomingPhi - NextOuterBoundHit.phi());
        if (deltaPhi > 0.075 * M_PI && deltaPhi < 1.925 * M_PI) continue;
        double SegK = next.kappa();
        double SegRR = next.r(); SegRR *= SegRR;
        double outX = next.outgoingX();
        double outY = next.outgoingY();
        const double out_r = NextOuterBoundHit.layer().r();
        // kappa = -2. * alpha * ((Vout X Vin)_z / |Vin|*|Vout|) / |Vin-Vout|
        double GapK = 2.*alpha * (inX * outY - inY * outX) /
                      (in_r * out_r * sqrt((inX - outX) * (inX - outX) +
                                           (inY - outY) * (inY - outY)));
        double GapRR =
          (currentLayer == j + 2) ? 0.5 * (in_r + out_r) : in_r + out_r;
        GapRR *= GapRR;
        double SigmaKTmp = (SigmaK + SegK + GapK);
        double SigmaRRTmp = SigmaRR + SegRR + GapRR;
        double SigmaKRRTmp = SigmaKRR + SegK * SegRR + GapK * GapRR;
        double SigmaKKRRTmp =
          SigmaKKRR + SegK * SegK * SegRR + GapK * GapK * GapRR;
        double MuKTmp = SigmaKTmp / (2 * n + 1);
        double chi2 = (MuKTmp * MuKTmp * SigmaRRTmp
                       - 2.*MuKTmp * SigmaKRRTmp + SigmaKKRRTmp) / (2 * n + 1);
        if ((chi2 - chi2Kappa) < minChi2) {
          minChi2 = chi2;
          innerSegment = &next;
          SigmaKCache = SigmaKTmp;
          SigmaRRCache = SigmaRRTmp;
          SigmaKRRCache = SigmaKRRTmp;
          SigmaKKRRCache = SigmaKKRRTmp;
        }
      }
    } while (!innerSegment && j ^ limit);
    if (innerSegment) {
      n = m_linkedSegments->append(innerSegment);
      currentLayer = j;
      SigmaK = SigmaKCache;
      SigmaRR = SigmaRRCache;
      SigmaKRR = SigmaKRRCache;
      SigmaKKRR = SigmaKKRRCache;
      chi2Kappa = minChi2;
    } else {
      break;
    }
  } while (currentLayer);

  if (n < 3) return NULL;
  //if (fabs(SigmaK/(2*n-1)) > 1.2/minPt) return NULL;
  return (new FTTrack(*m_linkedSegments, SigmaK / (2 * n - 1), chi2Kappa));
}


void
FTFinder::mkTrack3D(void)
{
  int n = m_tracks.length();
  // select segment candidate and cache sz
  FTList<FTSegment*> multiTrackCand(20);
  for (int i = 7; i != -1; i -= 2) {
    FTList<FTSegment*>& segments = m_superLayer[i].segments();
    int m = segments.length();
    for (int j = 0; j ^ m; j++) {
      FTSegment* s = segments[j];
      int nTrack = 0;
      FTTrack* t(NULL);
      for (int k = 0; k ^ n; k++) {
        if (s->update3D(m_tracks[k])) { // calcurate s and z
          t = m_tracks[k];
          nTrack++;
        }
      }
      switch (nTrack) {
        case 0:
          continue;
        case 1:
          t->appendStereoCache(s);
          break;
        default:
          multiTrackCand.append(s);
          break;
      }
    }
    // calcurate tanLambda & dz from the segments
    // whose relation between this is unique
    for (int j = 0; j ^ n; j++) m_tracks[j]->updateSZ();
  }
  // link segments by tanLambda & dz
  for (int i = 0; i ^ n; i++) m_tracks[i]->linkStereoSegments();
  n = multiTrackCand.length();
  for (int i = 0; i ^ n; i++) multiTrackCand[i]->linkStereoSegments();
}

int
FTFinder::VertexFit2D()
{
#if 0
  int n = m_tracks.length();
  if (n < 2) {
    m_vx = -99999.;
    m_vy = -99999.;
    m_vz = -99999.;
    return 0;
  }
  FTList<double> px(10);
  FTList<double> py(10);
  FTList<double> dx(10);
  FTList<double> dy(10);
  FTList<double> sigmaR2(10);

  for (int i = 0; i < n; i++) {
    const Lpav& la = m_tracks[i]->lpav();
    if (la.nc() <= 3) continue;
    TVectorD a = la.Hpar(FTTrack::pivot());

    const double dri = a(0);
    const double pxi = - std::sin(a(1));
    const double pyi = std::cos(a(1));

    px.append(pxi);
    py.append(pyi);
    dx.append(dri * pyi);
    dy.append(-dri * pxi);
    sigmaR2.append(la.chisq() / (la.nc() - 3));
  }

  n = dx.length();
  if (n < 2) {
    m_vx = -99999.;
    m_vy = -99999.;
    m_vz = -99999.;
    return 0;
  }

  FTList<double> vx(20);
  FTList<double> vy(20);
  FTList<double> w2(20);
  unsigned nVts(0);
  for (int i = dx.length() - 1; i; i--) {
    for (int j = 0; j < i; j++) {

      const double pijz = px[i] * py[j] - py[i] * px[j];

      if (pijz == 0.) continue;

      const double sr = sigmaR2[i] + sigmaR2[j];

      const double ddx = dx[i] - dx[j];
      const double ddy = dy[i] - dy[j];

      const double di = (px[j] * ddy - py[j] * ddx) / pijz;

      //const double dj = (px[i]*ddy-py[i]*ddx)/pijz;

      const double vxi = dx[i] + px[i] * di;
      const double vyi = dy[i] + py[i] * di;

      //const double vxj = dx[j] + px[j]*dj;
      //const double vyj = dy[j] + py[j]*dj;

      vx.append(vxi);
      vy.append(vyi);

      w2.append(1. / sr);
      nVts++;
    }
  }
  n = w2.length();
  double wSum(0.);
  for (int i = 0; i != n; i++) {
    m_vx += vx[i] * w2[i];
    m_vy += vy[i] * w2[i];
    wSum += w2[i];
  }
  int rtn_flag = 0;
  if (wSum <= 0.) {
    m_vx = -99999.;
    m_vy = -99999.;
  } else {
    m_vx /= wSum;
    m_vy /= wSum;
    rtn_flag = 1;
  }
  return rtn_flag;
#endif

#if 1
  int nTrks = m_tracks.length();
  if (nTrks < 2) {
    m_vx = -99999.;
    m_vy = -99999.;
    m_vz = -99999.;
    return 0;
  }
  FTList<double> px(10);
  FTList<double> py(10);
  FTList<double> dx(10);
  FTList<double> dy(10);
  FTList<double> w(10);
  for (int i = 0; i < nTrks; i++) {
    const Lpav& la = m_tracks[i]->lpav();
    TVectorD a = la.Hpar(FTTrack::pivot());

    const double dri = a(0);
    if (fabs(a(1)) > 1.5) continue;
    const double pxi = - sin(a(1));
    const double pyi = cos(a(1));

    //const double dxi = dri*pyi;
    //const double dyi = -dri*pxi;

    double wi = la.chisq() / (la.nc() * 0.02);

    px.append(pxi);
    py.append(pyi);
    dx.append(dri * pyi);
    dy.append(-dri * pxi);
    w.append(exp(-wi * wi));
  }
  if (dx.length() < 2) {
    m_vx = -99999.;
    m_vy = -99999.;
    m_vz = -99999.;
    return 0;
  }
  double wSum = 0.;
  for (int i = dx.length() - 1; i; i--) {
    const double pxi = px[i];
    const double pyi = py[i];
    const double dxi = dx[i];
    const double dyi = dy[i];
    const double wi = w[i];
    for (int j = 0; j < i; j++) {
      const double pxj = px[j];
      const double pyj = py[j];
      //const double wj = w[j];

      const double ddx = dx[j] - dxi;
      const double ddy = dx[j] - dyi;

      const double tmp_par = pxi * pyj - pxj * pyi;
      //      const double par = (pyj*ddx-pxj*ddy)/tmp_par;
      double par(0.);
      if (tmp_par != 0.) {
        par = (pyj * ddx - pxj * ddy) / tmp_par;
      }
      double wij = wi * w[j];
      wSum += wi * w[j];
      if (tmp_par == 0 ||
          par < -0.5 || (pyi * ddx - pxi * ddy) / tmp_par < -0.5 ||
          fabs((pxi * pxj + pyi * pyj) /
               sqrt((pxi * pxi + pyi * pyi) * (pxj * pxj + pyj * pyj))) > 0.86) {
        m_vx += (dxi + 0.5 * ddx) * wij;
        m_vy += (dyi + 0.5 * ddy) * wij;
      } else {
        m_vx += (dxi + par * pxi) * wij;
        m_vy += (dyi + par * pyi) * wij;
      }
    }
  }

  int rtn_flag = 0;
  if (wSum == 0.) {
    m_vx = -99999.;
    m_vy = -99999.;
    m_vz = -99999.;
  } else {
    m_vx /= wSum;
    m_vy /= wSum;
    m_vz = -99999.;
    rtn_flag = 1;
  }
  return rtn_flag;

#endif
}

int
FTFinder::VertexFit(int z_flag)
{
  m_vx = 0.;
  m_vy = 0.;
  m_vz = 0.;
  if (!z_flag) {
    return VertexFit2D();
  }
  int n = m_tracks.length();
  if (n < 2) {
    m_vx = -99999.;
    m_vy = -99999.;
    m_vz = -99999.;
    return 0;
  }
  FTList<double> px(10);
  FTList<double> py(10);
  FTList<double> pz(10);
  FTList<double> dx(10);
  FTList<double> dy(10);
  FTList<double> dz(10);
  FTList<double> w(10);
  FTList<double> wz(10);
  FTList<double> sigmaR2(10);
  FTList<double> sigmaZ2(10);

  for (int i = 0; i < n; i++) {
    const Lpav& la = m_tracks[i]->lpav();
    if (la.nc() <= 3) continue;
    const Zav& za = m_tracks[i]->zav();
    if (za.nc() > 2 && za.b() > -900) {
      pz.append(za.a());
      dz.append(za.b());
      sigmaZ2.append(za.chisq() / (za.nc() - 2));
      wz.append(exp(-(za.chisq() / (za.nc() - 2))));
    } else {
      continue;
    }

    const TVectorD a = la.Hpar(FTTrack::pivot());

    const double dri = a(0);
    const double pxi = - std::sin(a(1));
    const double pyi = std::cos(a(1));

    px.append(pxi);
    py.append(pyi);
    dx.append(dri * pyi);
    dy.append(-dri * pxi);
    sigmaR2.append(std::sqrt(la.chisq()) / (la.nc() - 3));
    w.append(exp(-(std::sqrt(la.chisq()) / (la.nc() - 3))));
  }

  n = dx.length();
  if (n < 2) {
    m_vx = -9999.;
    m_vy = -9999.;
    m_vz = -9999.;
    return 0;
  }

  FTList<double> vx(20);
  FTList<double> vy(20);
  FTList<double> vz(20);
  FTList<double> w2(20);
  FTList<double> wz2(20);
  FTList<double> vtx_chi2(20);
  unsigned nVts(0);
  for (int i = n - 1; i; i--) {
    for (int j = 0; j < i; j++) {
      // min. chi2 fit w/ line approximation
      const double pijX = py[i] * pz[j] - pz[i] * py[j];
      const double pijY = pz[i] * px[j] - px[i] * pz[j];
      const double pijZ = px[i] * py[j] - py[i] * px[j];

      if (pijX == 0. && pijY == 0. && pijZ == 0.) continue;

      const double sr = sigmaR2[i] + sigmaR2[j];
      const double sz = sigmaZ2[i] + sigmaZ2[j];

      const double ddx = dx[i] - dx[j];
      const double ddy = dy[i] - dy[j];
      const double ddz = dz[i] - dz[j];

      const double pij2 = pijX * pijX / (sr * sz) +
                          pijY * pijY / (sr * sz) + pijZ * pijZ / (sr * sr);

      const double di = (pijX * (py[j] * ddz - pz[j] * ddy) / (sr * sz) +
                         pijY * (pz[j] * ddx - px[j] * ddz) / (sr * sz) +
                         pijZ * (px[j] * ddy - py[j] * ddx) / (sr * sr)) / pij2;

      const double dj = (pijX * (py[i] * ddz - pz[i] * ddy) / (sr * sz) +
                         pijY * (pz[i] * ddx - px[i] * ddz) / (sr * sz) +
                         pijZ * (px[i] * ddy - py[i] * ddx) / (sr * sr)) / pij2;

      const double vxi = dx[i] + px[i] * di;
      const double vyi = dy[i] + py[i] * di;
      const double vzi = dz[i] + pz[i] * di;

      const double vxj = dx[j] + px[j] * dj;
      const double vyj = dy[j] + py[j] * dj;
      const double vzj = dz[j] + pz[j] * dj;

      const double wij = w[i] + w[j];
      vx.append((w[j]*vxi + w[i]*vxj) / wij);
      vy.append((w[j]*vyi + w[i]*vyj) / wij);
      vz.append((wz[j]*vzi + wz[i]*vzj) / (wz[i] + wz[j]));

      w2.append(exp(-sr));
      wz2.append(exp(-sz));
      vtx_chi2.append(((vxi - vxj) * (vxi - vxj) + (vyi - vyj) * (vyi - vyj))
                      / sr + (vzi - vzj) * (vzi - vzj) / sz);
      nVts++;
    }
  }
  n = vtx_chi2.length();
  double wSum(0.);
  double wzSum(0.);
  for (int i = 0; i != n; i++) {
    if (vtx_chi2[i] > 10.) continue;
    double wInner(std::exp(-vtx_chi2[i]));
    m_vx += vx[i] * w2[i] * wInner;
    m_vy += vy[i] * w2[i] * wInner;
    m_vz += vz[i] * wz2[i] * wInner;
    wSum += w2[i] * wInner;
    wzSum += wz2[i] * wInner;
  }
  int rtn_flag = 0;
  if (wSum <= 0.) {
    m_vx = -9999.;
    m_vy = -9999.;
  } else {
    m_vx /= wSum;
    m_vy /= wSum;
    rtn_flag = 1;
  }
  if (!z_flag) return rtn_flag;
  if (wzSum <= 0.) {
    m_vz = -9999.;
  } else {
    m_vz /= wzSum;
    rtn_flag++;
  }
  return rtn_flag;
}

/*
int
FTFinder::findBestVertex(void)
{
  int nTrks = m_tracks.length();
  if (nTrks < 2) {
    m_vx = -9999.;
    m_vy = -9999.;
    m_vz = -9999.;
    return 0;
  }
  double minDr = 9999.;
  double phi0 = 9999.;
  for (int i = 0; i < nTrks; i++) {
    TVectorD a = m_tracks[i]->lpav().Hpar(FTTrack::pivot());
    if (fabs(a(0)) < fabs(minDr)) {
      minDr = a(0);
      phi0 = a(1);
    }
  }
  m_vx = minDr * cos(phi0);
  m_vy = minDr * sin(phi0);
  return 1;
}
*/

TVector3
FTFinder::getVertex(void) const
{
  return TVector3(m_vx, m_vy, m_vz);
}

int
FTFinder::CorrectEvtTiming(void)
{
  int nTrks = m_tracks.length();
  double wSum = 0.;
  double dtSum2 = 0.;
  for (int i = 0; i ^ nTrks; i++) {
    double dtSum = 0.;
    double dttSum = 0.;
    int nHits = 0;
    const Lpav& la = m_tracks[i]->lpav();
    FTList<FTSegment*>& axialSgmnts = m_tracks[i]->getAxialSegments();
    int m = axialSgmnts.length();
    for (int j = 0; j ^ m; j++) {
      FTList<FTWire*>& hits = axialSgmnts[j]->getWireHits();
      int l = hits.length();
      for (int k = 0; k ^ l; k++) {
        FTWire& h = * hits[k];
        const double x = h.x();
        const double y = h.y();
        const double d0 = fabs(la.d(x, y));
        if (d0 >= 0.45 * h.layer().csize()) continue;
        nHits++;
        double dt = x2t(d0) - h.time();
        dtSum += dt;
        dttSum += (dt * dt);
      }
    }
    if (!nHits) continue;
    const double w = exp(-(dttSum - (dtSum * dtSum / nHits)) / (nHits * 1600.));
    wSum += (nHits * w);
    dtSum2 += (dtSum * w);
  }
  return int(dtSum2 / wSum);
}
