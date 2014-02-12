//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Fitter3D.cc
// Section  : TRG CDC
// Owner    : KyungTae KIM (K.U.)
// Email    : ktkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to fit tracks in 3D
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "trg/trg/Debug.h"
#include "trg/cdc/Fitter3D.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Link.h"
#include <cstdlib>
#include <bitset>

#include "framework/datastore/StoreArray.h"
#include "framework/datastore/RelationArray.h"
#include "cdc/dataobjects/CDCHit.h"
#include "cdc/dataobjects/CDCSimHit.h"
#include "cdc/geometry/CDCGeometryPar.h"
#include "trg/trg/Time.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/TrackMC.h"
#include "trg/cdc/Relation.h"
#include "mdst/dataobjects/MCParticle.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/HoughFinder.h"
#include "trg/cdc/LUT.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/trg/Constants.h"
#include "trg/cdc/Helix.h"

#include "trg/cdc/Fitter3DUtility.h"

#include "trg/cdc/EventTime.h"


using namespace std;

namespace Belle2 {

  std::string
    TRGCDCFitter3D::version(void) const {
      return string("TRGCDCFitter3D 5.1");
    }

  TRGCDCFitter3D::TRGCDCFitter3D(const string & name, 
      const string & rootFitter3DFile,
      const TRGCDC & TRGCDC,
      const TRGCDCEventTime * eventTime,
      bool fLRLUT,
      bool fevtTime,
      bool fzierror,
      bool fmclr,
      bool makeRootFile)
    : _name(name),
    _cdc(TRGCDC),
    _eventTime(eventTime),
    m_rootFitter3DFilename(rootFitter3DFile),
    m_flagWireLRLUT(fLRLUT),
    m_flagEvtTime(fevtTime),
    m_flagzierror(fzierror),
    m_flagmclr(fmclr),
    m_makeRootFile(makeRootFile) {

      //...Initialization...
    }

  TRGCDCFitter3D::~TRGCDCFitter3D() {
  }

  void
    TRGCDCFitter3D::callLUT(){

      int lutindex=0;

      m_intnum2=1,m_numbit2=13;	//For signed bits for myphi0
      for(unsigned i=0;i<m_numbit2-1;i++){m_intnum2*=2;}
      m_intnum2-=1;
      m_intnum3=1,m_numbit3=12;	//For signed bits for rho
      for(unsigned i=0;i<m_numbit3;i++){m_intnum3*=2;}
      m_intnum3-=1;
      m_intnum5=1,m_numbit5=9;	//For signed bits for zz
      for(unsigned i=0;i<m_numbit5-1;i++){m_intnum5*=2;}
      m_intnum5-=1;
      //m_intnum6=1,m_numbit6=9;	//For signed bits for rr
      m_intnum6=1,m_numbit6=7;	//For signed bits for rr
      for(unsigned i=0;i<m_numbit6;i++){m_intnum6*=2;}
      m_intnum6-=1;
      m_intnum7=1,m_numbit7=4;	//For signed bits for iezz2
      for(unsigned i=0;i<m_numbit7;i++){m_intnum7*=2;}
      m_intnum7-=1;

      m_intnum8=1;m_numbit8=23;
      for(unsigned i=0;i<m_numbit8;i++){m_intnum8*=2;}
      m_intnum9=1,m_numbit9=9;
      for(unsigned i=0;i<m_numbit9;i++){m_intnum9*=2;}
      m_intnum10=1,m_numbit10=10;
      for(unsigned i=0;i<m_numbit10;i++){m_intnum10*=2;}
      m_intnum11=1,m_numbit11=8;
      for(unsigned i=0;i<m_numbit11;i++){m_intnum11*=2;}

      string first,second;
      ifstream f_lut00;
      f_lut00.open("trg/data/lut00.coe");
      while(1){
        if (lutindex == 4097) break;
        f_lut00 >> first >> second;
        istringstream isecond(second);
        if(lutindex != 0) {
          isecond >> m_lut00[lutindex-1];
        }
        lutindex += 1;
      }
      f_lut00.close();

      lutindex=0;
      ifstream f_lut01;
      f_lut01.open("trg/data/lut01.coe");
      while(1){
        if (lutindex == 4097) break;
        f_lut01 >> first >> second;
        istringstream isecond(second);
        if(lutindex != 0) {
          isecond >> m_lut01[lutindex-1];
        }
        lutindex += 1;
      }
      f_lut01.close();

      lutindex=0;
      ifstream f_lut02;
      f_lut02.open("trg/data/lut02.coe");
      while(1){
        if (lutindex == 4097) break;
        f_lut02 >> first >> second;
        istringstream isecond(second);
        if(lutindex != 0) {
          isecond >> m_lut02[lutindex-1];
        }
        lutindex += 1;
      }
      f_lut02.close();

      lutindex=0;
      ifstream f_lut03;
      f_lut03.open("trg/data/lut03.coe");
      while(1){
        if (lutindex == 4097) break;
        f_lut03 >> first >> second;
        istringstream isecond(second);
        if(lutindex != 0) {
          isecond >> m_lut03[lutindex-1];
        }
        lutindex += 1;
      }
      f_lut03.close();

      lutindex=0;
      ifstream f_zz_0_lut;
      f_zz_0_lut.open("trg/data/zz_0_lut.coe");
      while(1){
        if (lutindex == 1025) break;
        f_zz_0_lut >> first >> second;
        istringstream isecond(second);
        if(lutindex != 0) {
          isecond >> m_zz_0_lut[lutindex-1];
        }
        lutindex += 1;
      }
      f_zz_0_lut.close();

      lutindex=0;
      ifstream f_zz_1_lut;
      f_zz_1_lut.open("trg/data/zz_1_lut.coe");
      while(1){
        if (lutindex == 1025) break;
        f_zz_1_lut >> first >> second;
        istringstream isecond(second);
        if(lutindex != 0) {
          isecond >> m_zz_1_lut[lutindex-1];
        }
        lutindex += 1;
      }
      f_zz_1_lut.close();

      lutindex=0;
      ifstream f_zz_2_lut;
      f_zz_2_lut.open("trg/data/zz_2_lut.coe");
      while(1){
        if (lutindex == 1025) break;
        f_zz_2_lut >> first >> second;
        istringstream isecond(second);
        if(lutindex != 0) {
          isecond >> m_zz_2_lut[lutindex-1];
        }
        lutindex += 1;
      }
      f_zz_2_lut.close();

      lutindex=0;
      ifstream f_zz_3_lut;
      f_zz_3_lut.open("trg/data/zz_3_lut.coe");
      while(1){
        if (lutindex == 1025) break;
        f_zz_3_lut >> first >> second;
        istringstream isecond(second);
        if(lutindex != 0) {
          isecond >> m_zz_3_lut[lutindex-1];
        }
        lutindex += 1;
      }
      f_zz_3_lut.close();

      lutindex=0;
      ifstream f_iz0den_lut;
      f_iz0den_lut.open("trg/data/iz0den_lut.coe");
      while(1){
        if (lutindex == 8193) break;
        f_iz0den_lut >> first >> second;
        istringstream isecond(second);
        if(lutindex != 0) {
          isecond >> m_iz0den_lut[lutindex-1];
        }
        lutindex += 1;
      }
      f_iz0den_lut.close();
    }

  void TRGCDCFitter3D::initializeRoot(){
    if(m_makeRootFile) m_fileFitter3D = new TFile((char*)m_rootFitter3DFilename.c_str(),"RECREATE");
    m_treeTrackFitter3D = new TTree("m_treeTrackFitter3D","track");

    m_tSTrackFitter3D = new TClonesArray("TVectorD");
    m_tsPhiTrackFitter3D = new TClonesArray("TVectorD");
    m_tsDriftTrackFitter3D = new TClonesArray("TVectorD");
    m_tsLRTrackFitter3D = new TClonesArray("TVectorD");
    m_mcTSTrackFitter3D = new TClonesArray("TVectorD");
    m_mcTSLRTrackFitter3D = new TClonesArray("TVectorD");
    m_fitTrackFitter3D = new TClonesArray("TVectorD");
    m_wFit2DTrackFitter3D = new TClonesArray("TVectorD");
    m_fit2DTrackFitter3D = new TClonesArray("TVectorD");
    m_szTrackFitter3D = new TClonesArray("TVectorD");
    m_wStAxPhiTrackFitter3D = new TClonesArray("TVectorD");
    m_stAxPhiTrackFitter3D = new TClonesArray("TVectorD");
    m_mcTrackFitter3D = new TClonesArray("TVectorD");
    m_mcStatusTrackFitter3D = new TClonesArray("TVectorD");
    m_mcVertexTrackFitter3D = new TClonesArray("TVector3");
    m_mc4VectorTrackFitter3D = new TClonesArray("TLorentzVector");
    m_mcHelixCenterTrackFitter3D = new TClonesArray("TVector2");
    m_mcImpactPositionTrackFitter3D = new TClonesArray("TVector3");
    m_eventTrackIDTrackFitter3D = new TClonesArray("TVectorD");

    m_treeTrackFitter3D->Branch("tSTrackFitter3D", &m_tSTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("tsPhiTrackFitter3D", &m_tsPhiTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("tsDriftTrackFitter3D", &m_tsDriftTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("tsLRTrackFitter3D", &m_tsLRTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mcTSTrackFitter3D", &m_mcTSTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mcTSLRTrackFitter3D", &m_mcTSLRTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("fitTrackFitter3D", &m_fitTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("wFit2DTrackFitter3D", &m_wFit2DTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("fit2DTrackFitter3D", &m_fit2DTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("szTrackFitter3D", &m_szTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("wStAxPhiTrackFitter3D", &m_wStAxPhiTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("stAxPhiTrackFitter3D", &m_stAxPhiTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mcTrackFitter3D", &m_mcTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mcStatusTrackFitter3D", &m_mcStatusTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mcVertexTrackFitter3D", &m_mcVertexTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mc4VectorTrackFitter3D", &m_mc4VectorTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mcHelixCenterTrackFitter3D", &m_mcHelixCenterTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mcImpactPositionTrackFitter3D", &m_mcImpactPositionTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("eventTrackIDTrackFitter3D", &m_eventTrackIDTrackFitter3D,32000,0);

    m_treeConstantsFitter3D = new TTree("m_treeConstantsFitter3D","constants");
    m_geometryFitter3D = new TVectorD(17);
    m_nWiresFitter3D = new TVectorD(9);
    m_errorFitter3D = new TVectorD(9);
    m_treeConstantsFitter3D->Branch("geometryFitter3D", &m_geometryFitter3D,32000,0);
    m_treeConstantsFitter3D->Branch("nWiresFitter3D", &m_nWiresFitter3D,32000,0);
    m_treeConstantsFitter3D->Branch("errorFitter3D", &m_errorFitter3D,32000,0);

  }

  void TRGCDCFitter3D::saveInitializationValues(){
    // Save geometry to root file
    TVectorD &geometryFitter3D = *m_geometryFitter3D;
    TVectorD &nWiresFitter3D = *m_nWiresFitter3D;
    for(int i=0; i<9; i++){
      geometryFitter3D[i] = m_rr[i];
    }
    for(int i=0; i<9; i++){
      nWiresFitter3D[i] = m_ni[i];
    }
    for(int i=0; i<4; i++){
      geometryFitter3D[i+9] = m_anglest[i];
      geometryFitter3D[i+13] = m_ztostraw[i];
    }

    // Save error to root file
    TVectorD &errorFitter3D = *m_errorFitter3D;
    for(int i=0; i<5; i++){
      errorFitter3D[2*i] = m_phierror[i];
    }
    for(int i=0; i<4; i++){
      errorFitter3D[2*i+1] = m_zerror[i];
    }

    m_treeConstantsFitter3D->Fill();
  }

  void TRGCDCFitter3D::initialize(){

    // Flags
    m_flagRealInt = 0;
    m_flagNonTSStudy = 0;
    m_boolMc = 1;

    initializeRoot();

    callLUT();
    m_Trg_PI = 3.141592653589793; 
    m_zerror[0] = 0.0319263; m_zerror[1] = 0.028765; m_zerror[2] = 0.0290057; m_zerror[3]=0.0396206;
    m_zerror1[0]=0.0581; m_zerror1[1]=0.0785; m_zerror1[2]=0.0728; m_zerror1[3]=0.0767;
    m_zerror2[0]=0.00388; m_zerror2[1]=0.00538; m_zerror2[2]=0.00650; m_zerror2[3]=0.00842;
    //m_zerror1[0]=0.0428; m_zerror1[1]=0.0564; m_zerror1[2]=0.0573; m_zerror1[3]=0.0670;
    //m_zerror2[0]=0.0196; m_zerror2[1]=0.0167; m_zerror2[3]=0.0177; m_zerror2[3]=0.0168;
    m_phierror[0] = 0.0085106; m_phierror[1] = 0.0039841; m_phierror[2] = 0.0025806; m_phierror[3] = 0.0019084; m_phierror[4] = 0.001514;


    // Geometry
    CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
    //Initialize rr,ztostarw,anglest,ni
    m_rr[0]=cdcp.senseWireR(2)*0.01;
    m_ni[0]=cdcp.nWiresInLayer(2)*2;
    for(int axSuperLayer=1;axSuperLayer<5;axSuperLayer++){
      m_rr[2*axSuperLayer]=cdcp.senseWireR(12*axSuperLayer+4)*0.01;
      m_ni[2*axSuperLayer]=cdcp.nWiresInLayer(12*axSuperLayer+4)*2;
    }
    for(int stSuperLayer=0;stSuperLayer<4;stSuperLayer++){
      m_rr[2*stSuperLayer+1]=cdcp.senseWireR(12*stSuperLayer+10)*0.01;
      m_ni[2*stSuperLayer+1]=cdcp.nWiresInLayer(12*stSuperLayer+10)*2;
      m_ztostraw[stSuperLayer]=cdcp.senseWireBZ(12*stSuperLayer+10)*0.01;
      m_anglest[stSuperLayer]=2*m_rr[2*stSuperLayer+1]*sin(m_Trg_PI*cdcp.nShifts(12*stSuperLayer+10)/(2*cdcp.nWiresInLayer(12*stSuperLayer+10)))/(cdcp.senseWireFZ(12*stSuperLayer+10)-cdcp.senseWireBZ(12*stSuperLayer+10))/0.01;
    }

    saveInitializationValues();
  }

  void TRGCDCFitter3D::initializeEventRoot(){
    m_tSTrackFitter3D->Clear();
    m_tsPhiTrackFitter3D->Clear();
    m_tsDriftTrackFitter3D->Clear();
    m_tsLRTrackFitter3D->Clear();
    m_mcTSTrackFitter3D->Clear();
    m_mcTSLRTrackFitter3D->Clear();
    m_fitTrackFitter3D->Clear();
    m_wFit2DTrackFitter3D->Clear();
    m_fit2DTrackFitter3D->Clear();
    m_szTrackFitter3D->Clear();
    m_wStAxPhiTrackFitter3D->Clear();
    m_stAxPhiTrackFitter3D->Clear();
    m_mcTrackFitter3D->Clear();
    m_mcStatusTrackFitter3D->Clear();
    m_mcVertexTrackFitter3D->Clear();
    m_mc4VectorTrackFitter3D->Clear();
    m_mcHelixCenterTrackFitter3D->Clear();
    m_mcImpactPositionTrackFitter3D->Clear();
    m_eventTrackIDTrackFitter3D->Clear();
  }

  void TRGCDCFitter3D::saveTrackValues(double* phi, double* phi_w, double pt, 
            double myphi0, double z0, double cot, int mysign, double zchi2, 
            double* zz, int* lutv, double pt_w, double myphi0_w, TCTrack* aTrack, 
            int iFit, double* arcS, TVectorD* wStAxPhi, TVectorD* stAxPhi,
            double* drift, int* mclutv
            ) {

    //StoreArray<MCParticle> MCParticles("");
    //StoreArray<CDCSimHit> SimHits("CDCSimHits");

    if(m_boolMc){
      // Access to track's MC particle.
      const TCRelation & trackRelation = aTrack->relation();
      //unsigned nMCtracks = trackRelation.nContributors();
      // Biggest contibutor is 0. Next is 1 and so on.
      //float purity = trackRelation.purity(0);
      //const unsigned particleId = trackRelation.contributor(0);
      const MCParticle & trackMCParticle = trackRelation.mcParticle(0);
      // Access track's particle parameters
      double mcPt = trackMCParticle.getMomentum().Pt();
      double mcPhi0=0;
      if(trackMCParticle.getCharge()>0) mcPhi0=trackMCParticle.getMomentum().Phi()-m_Trg_PI/2;
      if(trackMCParticle.getCharge()<0) mcPhi0=trackMCParticle.getMomentum().Phi()+m_Trg_PI/2;
      // Change range to [0,2pi]
      if(mcPhi0<0) mcPhi0+=2*m_Trg_PI;
      double mcZ0 = trackMCParticle.getVertex().Z()/100;
      double mcCot=trackMCParticle.getMomentum().Pz()/trackMCParticle.getMomentum().Pt();
      double mcCharge = trackMCParticle.getCharge();
      // mcStatus[0]: statusbit, mcStatus[1]: pdg, mcStatus[2]: charge
      TVectorD mcStatus(3);
      mcStatus[0] = trackMCParticle.getStatus();
      mcStatus[1] = trackMCParticle.getPDG();
      mcStatus[2] = trackMCParticle.getCharge();
      TVector3 vertex;
      TLorentzVector vector4;
      vertex = trackMCParticle.getVertex();
      vector4 = trackMCParticle.get4Vector();
      TVector2 helixCenter;
      TVector3 impactPosition;
      findImpactPosition(&vertex, &vector4, int(mcStatus[2]), helixCenter, impactPosition);

      // [TODO]Find position of track for each super layer
      //...G4 trackID...
      unsigned id = trackRelation.contributor(0);
      vector<const TCSHit*> mcAllTSList[9];
      vector<const TCSHit*> mcTSList(9);
      //...Segment loop...
      const vector<const TCSHit*> hits = _cdc.segmentHits();
      for (unsigned i = 0; i < hits.size(); i++) {
        const TCSHit& ts = * hits[i];
        if (! ts.signal().active()) continue;
        const TCWHit* wh = ts.segment().center().hit();
        if (! wh) continue;
        const unsigned trackId = wh->iMCParticle();
        if (id == trackId)
          mcAllTSList[wh->wire().superLayerId()].push_back(& ts);
      }
      //...Select best one in each super layer...
      for (unsigned i = 0; i < 9; i++) {
        const TCSHit* best = 0;
        if (mcAllTSList[i].size() == 0) {
        } else if (mcAllTSList[i].size() == 1) {
          best = mcAllTSList[i][0];
        } else {
          int timeMin = 99999;
          for (unsigned k = 0; k < mcAllTSList[i].size(); k++) {
            const TRGSignal& timing = mcAllTSList[i][k]->signal();
            const TRGTime& t = * timing[0];
            if (t.time() < timeMin) {
              timeMin = t.time();
              best = mcAllTSList[i][k];
            }
          }
        }
        mcTSList[i] = best;
      }
      // Save mc track positions. Unit is cm.
      TVectorD tempVector(27);
      for(unsigned iSL=0; iSL<9; iSL++){
        if(mcTSList[iSL]==0) {
          tempVector[iSL*3] = 9999;
          tempVector[iSL*3+1] = 9999;
          tempVector[iSL*3+2] = 9999;
        } else {
          TVector3 posTrack = mcTSList[iSL]->simHit()->getPosTrack();
          //TVector3 posWire = mcTSList[iSL]->simHit()->getPosWire();
          tempVector[iSL*3] = posTrack.X();
          tempVector[iSL*3+1] = posTrack.Y();
          tempVector[iSL*3+2] = posTrack.Z();
        }
      }
      new((*m_mcTSTrackFitter3D)[iFit]) TVectorD(tempVector);

      TVectorD tempMC(5);
      tempMC[0] = mcPt;
      tempMC[1] = mcPhi0;
      tempMC[2] = mcZ0;
      tempMC[3] = mcCot;
      tempMC[4] = mcCharge;
      new((*m_mcTrackFitter3D)[iFit]) TVectorD(tempMC);

      // Save mc information
      new((*m_mcStatusTrackFitter3D)[iFit]) TVectorD(mcStatus);
      new((*m_mcVertexTrackFitter3D)[iFit]) TVector3(vertex);
      new((*m_mc4VectorTrackFitter3D)[iFit]) TLorentzVector(vector4);
      new((*m_mcHelixCenterTrackFitter3D)[iFit]) TVector2(helixCenter);
      new((*m_mcImpactPositionTrackFitter3D)[iFit]) TVector3(impactPosition);

      TVectorD t_mcTSLR(9);
      for(unsigned iSL=0; iSL<9; iSL++) t_mcTSLR[iSL] = mclutv[iSL];
      new((*m_mcTSLRTrackFitter3D)[iFit]) TVectorD(t_mcTSLR);

    }

    // Save track segment information
    TVectorD tempPhi(9,phi);
    new((*m_tSTrackFitter3D)[iFit]) TVectorD(tempPhi);
    TVectorD t_tsPhi_w(9,phi_w);
    new((*m_tsPhiTrackFitter3D)[iFit]) TVectorD(t_tsPhi_w);
    TVectorD t_tsDrift(9,drift);
    new((*m_tsDriftTrackFitter3D)[iFit]) TVectorD(t_tsDrift);
    TVectorD t_tsLR(9);
    for(unsigned iSL=0; iSL<9; iSL++) t_tsLR[iSL] = lutv[iSL];
    new((*m_tsLRTrackFitter3D)[iFit]) TVectorD(t_tsLR);

    // Save s and z hits
    TVectorD tempSz(8);
    for(unsigned i=0; i<4; i++){
      tempSz[i] = arcS[i];
      tempSz[i+4] = zz[i];
    }
    new((*m_szTrackFitter3D)[iFit]) TVectorD(tempSz);

    // Save stereo's axial phi
    new((*m_wStAxPhiTrackFitter3D)[iFit]) TVectorD(*wStAxPhi);
    new((*m_stAxPhiTrackFitter3D)[iFit]) TVectorD(*stAxPhi);


    // Save fit values
    TVectorD tempFit(6);
    tempFit[0]=pt;
    tempFit[1]=myphi0;
    tempFit[2]=z0;
    tempFit[3]=cot;
    tempFit[4]=mysign;
    tempFit[5]=zchi2;
    new((*m_fitTrackFitter3D)[iFit]) TVectorD(tempFit);

    // Save fit values
    TVectorD tempWFit(3);
    tempWFit[0]=pt_w;
    tempWFit[1]=myphi0_w;
    tempWFit[2]=mysign;
    new((*m_wFit2DTrackFitter3D)[iFit]) TVectorD(tempWFit);


    // Save event and track ID
    TVectorD eventTrackID(2);
    eventTrackID[0] = m_eventNum;
    eventTrackID[1] = aTrack->getTrackID();
    new((*m_eventTrackIDTrackFitter3D)[iFit]) TVectorD(eventTrackID);

     // Save 2D fit values from 2D fitter
     double pt2D;
     double phi02D;
     phi02D = aTrack->helix().phi0();
     pt2D = aTrack->helix().curv()*0.01*0.3*1.5;
     if(aTrack->charge()<0) {
       phi02D -= m_Trg_PI;
       if (phi02D < 0) phi02D += 2 * m_Trg_PI;
       pt2D = pt2D * -1;
     }
     TVectorD tempFit2D(3);
     tempFit2D[0] = pt2D;
     tempFit2D[1] = phi02D;
     tempFit2D[2] = aTrack->helix().dr()*0.01;
     new((*m_fit2DTrackFitter3D)[iFit]) TVectorD(tempFit2D);

  }

 
  void TRGCDCFitter3D::calculatePhiUsingDrift(unsigned* nSuperLayers, TRGCDCTrack* aTrack, float* evtTime, double (&phi_w)[9], double (&phi)[9], int (&lutv)[9], double (&drift)[9], int (&mclutv)[9], int &ckt){
    //...Super layer loop...
    for (unsigned i = 0; i < (*nSuperLayers); i++) {

      //...Access to track segment list in this super layer...
      //const vector<const TCSegment *> & segments = t.trackSegments(i);
      const vector<TCLink *> & links = aTrack->links(i);
      const unsigned nSegments = links.size();

      //...Presently nSegments should be 1...
      if (nSegments != 1) {
        if (nSegments==0){
          ckt=0;
          //cout << name() << " !!! NO TS assigned" << endl;
          break;
        }
        else{cout<< name()<<"!!!! multiple TS assigned"<< endl;}
      }

      //...using LRLUT to determine Left/Right(assume drift() will return drift distance)
      const TCSegment * s = dynamic_cast<const TCSegment *>(& links[0]->hit()->cell());
      phi_w[i]=(double) s->localId()/m_ni[s->superLayerId()]*4*m_Trg_PI;
      phi[i]=(double) s->localId()/m_ni[s->superLayerId()]*4*m_Trg_PI;
      //cout<<"Hit pattern: "<<s->hitPattern()<<endl;
      //unsigned int tsSize = s->wires().size();
      //if (tsSize == 11) {
      //  cout<<s->name()<<" Hit pattern: "<<bitset<11>(s->hitPattern())<<endl;
      //} else if (tsSize == 15) {
      //  cout<<s->name()<<" Hit pattern: "<<bitset<15>(s->hitPattern())<<endl;
      //}
      lutv[i]=s->LUT()->getLRLUT(s->hitPattern(),s->superLayerId());
      //cout<<"lutv["<<i<<"]: "<<lutv[i]<<endl;

      ///...Using Drift time information
      float dphi=s->hit()->drift()*10;
      if(m_flagEvtTime==1){
        dphi-=(*evtTime);
      }
      drift[i] = dphi;
      mclutv[i] = s->hit()->mcLR();
      if( m_flagWireLRLUT){
        dphi=atan(dphi/m_rr[s->superLayerId()]/1000);
        if(lutv[i]==0){phi[i]-=dphi;}
        else if(lutv[i]==1){phi[i]+=dphi;}
        else{
          phi[i]=phi[i];
        }
      }
      if(m_flagmclr) lutv[i]=s->hit()->mcLR();
    } // End of superlayer loop
  }

  void TRGCDCFitter3D::doit(const vector<TCTrack *> & trackListIn,
      vector<TCTrack *> & trackListOut, int eventNum) {
    m_eventNum = eventNum;
    doit(trackListIn, trackListOut);
  }

  int
    TRGCDCFitter3D::doit(const vector<TCTrack *> & trackListIn,
        vector<TCTrack *> & trackListOut) {

      float evtTime=EvtTime()->getT0();
      evtTime=evtTime*40/1000;

      //double m_Trg_PI = 3.141592653589793; 
      TRGDebug::enterStage("Fitter 3D");

      //double cotnum=0,sxx=0,z0num=0;
      //int z0nump1[4],z0nump2[4],z0den,iz0den;

      initializeEventRoot();

      //...Loop over track list...
      const unsigned nInput = trackListIn.size();
      // Index for number of successful fits
      int iFit=0;

      for (unsigned iInput = 0; iInput < nInput; iInput++) {
        double phi[9]={0,0,0,0,0,0,0,0,0};
        double phi_w[9]={0,0,0,0,0,0,0,0,0};
        double drift[9]={0,0,0,0,0,0,0,0,0};
        int lutv[9];
        int mclutv[9];
        int ckt=1;

        //...Access to a track...
        TCTrack & aTrack = * trackListIn[iInput];


        //       t.dump("detail");

        // Process drift time.
        unsigned nSuperLayers = _cdc.nSuperLayers();
        calculatePhiUsingDrift(&nSuperLayers, &aTrack, &evtTime, phi_w, phi, lutv, drift, mclutv, ckt);


        double z0=-999, cot=-999;
        double zchi2=0.;
        double pt=-999, myphi0=-999;
        double myphi0_w=-999;
        double pt_w=-999;
        //...Do fitting job here (or call a fitting function)...
        if(ckt){
          double rr2D[5]={m_rr[0], m_rr[2], m_rr[4], m_rr[6], m_rr[8]};
          double rr3D[4]={m_rr[1], m_rr[3], m_rr[5], m_rr[7]};
          double phi2D[5]={phi[0], phi[2],phi[4],phi[6],phi[8]};
          double phi2D_w[5]={phi_w[0], phi_w[2],phi_w[4],phi_w[6],phi_w[8]};
          double phi3D[4]={phi[1], phi[3],phi[5],phi[7]};
          int lutv3D[4]={lutv[1],lutv[3],lutv[5],lutv[7]};
          double zz[4], arcS[4];
          TVectorD wStAxPhi(4);
          TVectorD stAxPhi(4);
          double rr_conv[4];
          double ztheta=-999;
          double iezz2[4];
          double iezz21[4];
          double iezz22[4];
          double rho;
          double rho_w;

          //int qqq=0;
          //double myphiz[4];	
          //double ztheta=-999,ss=0.,sx=0.;

          //cout<<"TS: "<<phi2[0]<<" "<<phi2[1]<<endl;

          // Sign Finder
          //int mysign = findSign(phi2);
          int mysign = int(aTrack.charge());

          // r-phi fitter(2D Fitter) ->calculate pt and radius of track-> input for 3D fitter.
          rPhiFit(rr2D,phi2D,m_phierror,rho,myphi0);
          rPhiFit(rr2D,phi2D_w,m_phierror,rho_w,myphi0_w);
          pt = 0.3*rho*1.5;
          pt_w = 0.3*rho_w*1.5;

          if(m_flagRealInt == 1){
            ////Change input into relative values.
            //for(unsigned i=0;i<4;i++){
            //  phi2[i+5]=phi2[i+5]-phi2[2];
            //  //Change the range to [-pi~pi]
            //  if(phi2[i+5]>m_Trg_PI){phi2[i+5]-=2*m_Trg_PI;}
            //  if(phi2[i+5]<-m_Trg_PI){phi2[i+5]+=2*m_Trg_PI;}
            //}
            //myphi0=myphi0-phi2[2];

            ////Change the range to [-pi~pi]
            //if(myphi0>m_Trg_PI){myphi0-=2*m_Trg_PI;}
            //if(myphi0<-m_Trg_PI){myphi0+=2*m_Trg_PI;}
            ////          cout << "track center " << myphi0 << endl;

            ////int phi_st_int[4],myphi_int,rho_int,acos_int;

            ////Change values into integers.
            ////For phi stereos
            //for (unsigned i=0;i<4;i++){
            //  phi2[i+5]=m_intnum2/3.2*phi2[i+5];
            //  if(phi2[i+5]>0){phi2[i+5]+=0.5;}
            //  else{phi2[i+5]-=0.5;}
            //  phi2[i+5]=(int) phi2[i+5];
            //}
            ////For myphi0
            //myphi0=m_intnum2/3.2*myphi0;
            //if(myphi0>0){myphi0+=0.5;}
            //else myphi0-=0.5;
            //myphi0=(int)myphi0;
            ////For rho
            //rho=(int)(m_intnum3/16*rho+0.5);
            ////Interize rr
            //for(unsigned i=0;i<4;i++){rr_conv[i]=(int)(m_rr[i+5]*m_intnum6/m_rr[8]+0.5);}
            ////Change m_zerrorz to iezz2
            //for(unsigned i=0;i<4;i++){
            //  iezz2[i]=(int)(1./m_zerror[i]/m_zerror[i]*m_intnum7*m_zerror[1]*m_zerror[1]+0.5);
            //  iezz21[i]=(int)(1./m_zerror1[i]/m_zerror1[i]*m_intnum7*m_zerror1[1]*m_zerror1[1]+0.5);
            //  iezz22[i]=(int)(1./m_zerror2[i]/m_zerror2[i]*m_intnum7*m_zerror2[1]*m_zerror2[1]+0.5);
            //}

            ////The actual start of the fitter
            ////          cout << "rho " << rho<< " m_lut00 " <<m_lut00[(int) rho]<< " phi 5 " << phi2[5] << endl;

            //if(mysign==1){
            //  myphiz[0]=(m_lut00[(int)rho]+1652)+myphi0-phi2[5];
            //  myphiz[1]=(m_lut01[(int)rho]+1366)+myphi0-phi2[6];
            //  myphiz[2]=(m_lut02[(int)rho]+1039)+myphi0-phi2[7];
            //  myphiz[3]=(m_lut03[(int)rho]+596)+myphi0-phi2[8];
            //}else{
            //  myphiz[0]=-(m_lut00[(int)rho]+1652)+myphi0-phi2[5];
            //  myphiz[1]=-(m_lut01[(int)rho]+1366)+myphi0-phi2[6];
            //  myphiz[2]=-(m_lut02[(int)rho]+1039)+myphi0-phi2[7];
            //  myphiz[3]=-(m_lut03[(int)rho]+596)+myphi0-phi2[8];
            //}

            ////          cout << "myphiz " << myphiz[0]<<" " <<myphiz[1]<<" "<<myphiz[2]<< " " <<myphiz[3]<<endl;


            ////Change myphi to correct z lut address.
            //for(unsigned i=0;i<4;i++){
            //  if(myphiz[i]>=0){myphiz[i]=(int)myphiz[i];}
            //  else{myphiz[i]=(int)(1024+myphiz[i]);}
            //}

            //for(unsigned i=0;i<4;i++){
            //  if(int(myphiz[i])>1023)qqq=1;
            //}	
            //if(qqq) continue;

            //zz[0]=m_zz_0_lut[int(myphiz[0])];
            //zz[1]=m_zz_1_lut[int(myphiz[1])];
            //zz[2]=m_zz_2_lut[int(myphiz[2])];
            //zz[3]=m_zz_3_lut[int(myphiz[3])];

            ////          cout << "zz " << zz[0]<<" " <<zz[1]<<" "<<zz[2]<< " " <<zz[3]<<endl;
            //// Real space converter
            //for(unsigned i=0; i<4; i++){
            //  zz[i]=zz[i]/m_intnum5*1.5;
            //  rr_conv[i]=rr_conv[i]*m_rr[8]/m_intnum6;
            //  iezz2[i]=iezz2[i]/m_zerror[1]/m_zerror[1]/m_intnum7;
            //  iezz21[i]=iezz21[i]/m_zerror1[1]/m_zerror1[1]/m_intnum7;
            //  iezz22[i]=iezz22[i]/m_zerror2[1]/m_zerror2[1]/m_intnum7;
            //  rho=rho/m_intnum3*16;
            //}


          } else {
            // Real space
            // Calculate z position from rho and myphi0 and phi2
            for(unsigned i=0; i<4; i++){
              zz[i]=calZ(mysign, m_anglest[i], m_ztostraw[i],  rr3D[i], phi3D[i], rho, myphi0);
              wStAxPhi[i]=calStAxPhi(mysign, m_anglest[i], m_ztostraw[i],  rr3D[i], rho_w, myphi0_w);
              stAxPhi[i]=calStAxPhi(mysign, m_anglest[i], m_ztostraw[i],  rr3D[i], rho, myphi0);
              rr_conv[i]=rr3D[i];
              iezz2[i] = 1/m_zerror[i]/m_zerror[i];
              iezz21[i] = 1/m_zerror1[i]/m_zerror1[i];
              iezz22[i] = 1/m_zerror2[i]/m_zerror2[i];
            }   
          }

          // Calculate s
          for(unsigned i=0; i<4; i++){
            arcS[i] = calS(rho,rr_conv[i]);
          }
          // Every thing is in real space here


          // RZ Fit
          if(m_flagRealInt == 1){
            //// Integer space converter
            //for(unsigned i=0; i<4; i++){
            //  zz[i] = zz[i]*m_intnum5/1.5;
            //  rr_conv[i] = rr_conv[i]/m_rr[8]*m_intnum6;
            //  iezz2[i] = iezz2[i]*m_zerror[1]*m_zerror[1]*m_intnum7;
            //}

            ////rz fitter
            //z0=-999; ztheta=-999; ss=0.; sx=0.; cot=0.;
            //for(unsigned i=0;i<4;i++){
            //  ss+=iezz2[i];
            //  sx+=rr_conv[i]*iezz2[i];
            //  sxx+=rr_conv[i]*rr_conv[i]*iezz2[i];
            //}

            //for(unsigned i=0;i<4;i++){
            //  cotnum+=(ss*rr_conv[i]-sx)*iezz2[i]*zz[i];
            //  z0nump1[i]=(int)(sxx-sx*rr_conv[i]);
            //  z0nump1[i]=z0nump1[i] >> m_numbit10;
            //  z0nump2[i]=(int)(z0nump1[i]*iezz2[i]*zz[i]);
            //  z0nump2[i]=z0nump2[i] >> m_numbit11;
            //  z0num+=z0nump2[i];
            //}
            //z0den=(int)((ss*sxx)-(sx*sx));
            //z0den=z0den >> m_numbit9;
            //iz0den=m_iz0den_lut[z0den];
            //z0num*=iz0den;
            //z0=z0num/m_intnum8/m_intnum9*m_intnum10*m_intnum11;
            //cot=cotnum*iz0den/m_intnum8/m_intnum9;



            ///*		double sxoss=(int) sx/ss;
            //      for(unsigned i=0;i<4;i++){
            //      tt=(rr_conv[i]-sxoss);
            //      st2+= tt*tt*iezz2[i];
            //      cot+= tt*zz[i]*iezz2[i];
            //      }

            //      cot/=st2;
            //      z0=(sy-sx*cot)/ss;*/


            //// Backwards convertor
            //cot=cot*1.5/m_intnum5*m_intnum6/m_rr[8];
            //z0=z0*1.5/m_intnum5;
            //myphi0 = myphi0*3.2/m_intnum3;
            //for(unsigned i=0;i<4;i++){
            //  rr_conv[i]=rr_conv[i]*m_rr[8]/m_intnum6;
            //  iezz2[i]=iezz2[i]/m_zerror[1]/m_zerror[1]/m_intnum7;
            //  zz[i]=zz[i]*1.5/m_intnum5;
            //}
          } else {
            if(m_flagzierror){
              rSFit2(iezz21,iezz22,arcS, zz,lutv3D,z0,cot,zchi2);
            }
            else{
              //// Make new error vector according to lutv3D;
              //for(unsigned iSL=0; iSL<4; iSL++){
              //  if(lutv3D[iSL]==2) iezz2[iSL] = 1/m_zerror1[iSL]/m_zerror1[iSL];
              //  else iezz2[iSL] = 1/m_zerror2[iSL]/m_zerror2[iSL];
              //}
              rSFit(iezz2, arcS, zz, z0, cot,zchi2);
            }
          }

          // Convert from rad to deg
          //myphi0 = myphi0*180/m_Trg_PI;

          ztheta=m_Trg_PI/2.-atan(cot);
          ztheta*=180./m_Trg_PI;



          // For integer space
          //          cout << "tsimz0/"  << z0*100 <<"]"<<endl;
          //          cout << "tsimpt/"  << pt <<"]"<<endl;
          //          cout << "tsimth/"  << ztheta <<"]"<<endl;
          //          cout << "tsimpi/" << myphi0*180/m_Trg_PI*3.2/m_intnum3 << "]" << endl;

          saveTrackValues(phi, phi_w, pt, myphi0, z0, cot, mysign, zchi2, zz, lutv, 
                          pt_w, myphi0_w, &aTrack, iFit, arcS, &wStAxPhi, &stAxPhi, drift, mclutv);


          //cout<<"3D: "<<pt<<" "<<myphi0<<endl;
          //cout<<"[2D] pt: "<<aTrack.pt()<<" phi0: "<<phi02D<<" dr: "<<aTrack.helix().dr()<<endl;

          iFit += 1;

        } // end of chk

        // Set Helix parameters
        TRGCDCHelix helix(ORIGIN, CLHEP::HepVector(5,0), CLHEP::HepSymMatrix(5,0));
        CLHEP::HepVector a(5);
        a = aTrack.helix().a();
        if(ckt){
          aTrack.setFitted(1);
          a[3] = z0; 
          a[4] = cot;
        } else {
          aTrack.setFitted(0);
        }
        helix.a(a);
        aTrack.setHelix(helix);

        // Fill track list
        trackListOut.push_back(&aTrack);

      } // End trackList loop 

      m_treeTrackFitter3D->Fill();

      //...Termination...
      TRGDebug::leaveStage("Fitter 3D");
      return 0;
    }

  void TRGCDCFitter3D::terminate(void){
    if(m_makeRootFile) {
      m_fileFitter3D->Write();
      m_fileFitter3D->Close();
    }
    // Should clean up allocated memory
  }   



} // namespace Belle2
