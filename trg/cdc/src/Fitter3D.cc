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
#include "generators/dataobjects/MCParticle.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/HoughFinder.h"
#include "trg/cdc/LUT.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/trg/Constants.h"
#include "trg/cdc/Helix.h"

#include "trg/cdc/Fitter3DUtility.h"

#include "trg/cdc/EventTime.h"

//...Global varibles...
double rr[9];
double rro[9];
double anglest[4];
double ztostraw[4];
int ni[9];

int lut00[4096];
int lut01[4096];
int lut02[4096];
int lut03[4096];
int zz_0_lut[1024];
int zz_1_lut[1024];
int zz_2_lut[1024];
int zz_3_lut[1024];
int iz0den_lut[8192];
//For signed bits for myphi0
int intnum2;
unsigned int numbit2;	
//For signed bits for rho
int intnum3;
unsigned numbit3;	
//For signed bits for zz
int intnum5;
unsigned numbit5;	
//For signed bits for rr
int intnum6;
unsigned numbit6;	
//For signed bits for iezz2
int intnum7;
unsigned numbit7;	
//For unsigned iz0den
int intnum8;
unsigned numbit8;	
//For unsigned z0den
int intnum9;
unsigned numbit9;	
//For unsinged z0num_p1
int intnum10;
unsigned numbit10;	
//For unsigned z0num_p2
int intnum11;
unsigned numbit11;	
int kkk;

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

      kkk=0;
      int lutindex=0;

      intnum2=1,numbit2=13;	//For signed bits for myphi0
      for(unsigned i=0;i<numbit2-1;i++){intnum2*=2;}
      intnum2-=1;
      intnum3=1,numbit3=12;	//For signed bits for rho
      for(unsigned i=0;i<numbit3;i++){intnum3*=2;}
      intnum3-=1;
      intnum5=1,numbit5=9;	//For signed bits for zz
      for(unsigned i=0;i<numbit5-1;i++){intnum5*=2;}
      intnum5-=1;
      //intnum6=1,numbit6=9;	//For signed bits for rr
      intnum6=1,numbit6=7;	//For signed bits for rr
      for(unsigned i=0;i<numbit6;i++){intnum6*=2;}
      intnum6-=1;
      intnum7=1,numbit7=4;	//For signed bits for iezz2
      for(unsigned i=0;i<numbit7;i++){intnum7*=2;}
      intnum7-=1;

      intnum8=1;numbit8=23;
      for(unsigned i=0;i<numbit8;i++){intnum8*=2;}
      intnum9=1,numbit9=9;
      for(unsigned i=0;i<numbit9;i++){intnum9*=2;}
      intnum10=1,numbit10=10;
      for(unsigned i=0;i<numbit10;i++){intnum10*=2;}
      intnum11=1,numbit11=8;
      for(unsigned i=0;i<numbit11;i++){intnum11*=2;}

      string first,second;
      ifstream f_lut00;
      f_lut00.open("trg/data/lut00.coe");
      while(1){
        if (lutindex == 4097) break;
        f_lut00 >> first >> second;
        istringstream isecond(second);
        if(lutindex != 0) {
          isecond >> lut00[lutindex-1];
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
          isecond >> lut01[lutindex-1];
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
          isecond >> lut02[lutindex-1];
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
          isecond >> lut03[lutindex-1];
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
          isecond >> zz_0_lut[lutindex-1];
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
          isecond >> zz_1_lut[lutindex-1];
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
          isecond >> zz_2_lut[lutindex-1];
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
          isecond >> zz_3_lut[lutindex-1];
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
          isecond >> iz0den_lut[lutindex-1];
        }
        lutindex += 1;
      }
      f_iz0den_lut.close();
    }

  void TRGCDCFitter3D::initialize(){
    callLUT();
    m_Trg_PI = 3.141592653589793; 
    m_zerror[0] = 0.0319263; m_zerror[1] = 0.028765; m_zerror[2] = 0.0290057; m_zerror[3]=0.0396206;
    m_zerror1[0]=0.0581; m_zerror1[1]=0.0785; m_zerror1[2]=0.0728; m_zerror1[3]=0.0767;
    m_zerror2[0]=0.00388; m_zerror2[1]=0.00538; m_zerror2[2]=0.00650; m_zerror2[3]=0.00842;
    m_phierror[0] = 0.0085106; m_phierror[1] = 0.0039841; m_phierror[2] = 0.0025806; m_phierror[3] = 0.0019084; m_phierror[4] = 0.001514;

    if(m_makeRootFile) m_fileFitter3D = new TFile((char*)m_rootFitter3DFilename.c_str(),"RECREATE");
    m_treeTrackFitter3D = new TTree("m_treeTrackFitter3D","track");

    m_tSTrackFitter3D = new TClonesArray("TVectorD");
    m_mcTSTrackFitter3D = new TClonesArray("TVectorD");
    m_fitTrackFitter3D = new TClonesArray("TVectorD");
    m_wFit2DTrackFitter3D = new TClonesArray("TVectorD");
    m_fit2DTrackFitter3D = new TClonesArray("TVectorD");
    m_szTrackFitter3D = new TClonesArray("TVectorD");
    m_wStAxPhiTrackFitter3D = new TClonesArray("TVectorD");
    m_stAxPhiTrackFitter3D = new TClonesArray("TVectorD");
    m_mcTrackFitter3D = new TClonesArray("TVectorD");
    m_mcStatusTrackFitter3D = new TClonesArray("TVectorD");
    m_stTSsTrackFitter3D = new TClonesArray("TVectorD");
    m_mcVertexTrackFitter3D = new TClonesArray("TVector3");
    m_mc4VectorTrackFitter3D = new TClonesArray("TLorentzVector");
    m_mcHelixCenterTrackFitter3D = new TClonesArray("TVector2");
    m_mcImpactPositionTrackFitter3D = new TClonesArray("TVector3");
    m_eventTrackIDTrackFitter3D = new TClonesArray("TVectorD");

    m_treeTrackFitter3D->Branch("tSTrackFitter3D", &m_tSTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mcTSTrackFitter3D", &m_mcTSTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("fitTrackFitter3D", &m_fitTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("wFit2DTrackFitter3D", &m_wFit2DTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("fit2DTrackFitter3D", &m_fit2DTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("szTrackFitter3D", &m_szTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("wStAxPhiTrackFitter3D", &m_wStAxPhiTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("stAxPhiTrackFitter3D", &m_stAxPhiTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mcTrackFitter3D", &m_mcTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("mcStatusTrackFitter3D", &m_mcStatusTrackFitter3D,32000,0);
    m_treeTrackFitter3D->Branch("stTSsTrackFitter3D", &m_stTSsTrackFitter3D,32000,0);
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


    // Flags
    m_flagRealInt = 0;
    m_flagNonTSStudy = 0;

    // Geometry
    CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
    //Initialize rr,ztostarw,anglest,ni
    rr[0]=cdcp.senseWireR(2)*0.01;
    rro[0]=cdcp.senseWireR(2)*0.01;
    ni[0]=cdcp.nWiresInLayer(2)*2;
    for(int axSuperLayer=1;axSuperLayer<5;axSuperLayer++){
      rr[axSuperLayer]=cdcp.senseWireR(12*axSuperLayer+4)*0.01;
      rro[2*axSuperLayer]=cdcp.senseWireR(12*axSuperLayer+4)*0.01;
      ni[2*axSuperLayer]=cdcp.nWiresInLayer(12*axSuperLayer+4)*2;
    }
    for(int stSuperLayer=0;stSuperLayer<4;stSuperLayer++){
      rr[stSuperLayer+5]=cdcp.senseWireR(12*stSuperLayer+10)*0.01;
      ztostraw[stSuperLayer]=cdcp.senseWireBZ(12*stSuperLayer+10)*0.01;
      anglest[stSuperLayer]=2*rr[stSuperLayer+5]*sin(m_Trg_PI*cdcp.nShifts(12*stSuperLayer+10)/(2*cdcp.nWiresInLayer(12*stSuperLayer+10)))/(cdcp.senseWireFZ(12*stSuperLayer+10)-cdcp.senseWireBZ(12*stSuperLayer+10))/0.01;
      rro[2*stSuperLayer+1]=cdcp.senseWireR(12*stSuperLayer+10)*0.01;
      ni[2*stSuperLayer+1]=cdcp.nWiresInLayer(12*stSuperLayer+10)*2;
    }

    // Save geometry to root file
    TVectorD &geometryFitter3D = *m_geometryFitter3D;
    TVectorD &nWiresFitter3D = *m_nWiresFitter3D;
    for(int i=0; i<9; i++){
      geometryFitter3D[i] = rr[i];
    }
    for(int i=0; i<5; i++){
      nWiresFitter3D[i] = ni[2*i];
    }
    for(int i=0; i<4; i++){
      geometryFitter3D[i+9] = anglest[i];
      geometryFitter3D[i+13] = ztostraw[i];
      nWiresFitter3D[i+5] = ni[2*i+1];
    }

    // Save error to root file
    TVectorD &errorFitter3D = *m_errorFitter3D;
    for(int i=0; i<5; i++){
      errorFitter3D[i] = m_phierror[i];
    }
    for(int i=0; i<4; i++){
      errorFitter3D[i+5] = m_zerror[i];
    }

    m_treeConstantsFitter3D->Fill();

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
      //	StoreArray<CDCSimHit> cdcArray("CDCSimHIts");
      //	StoreArray<CDCHit> CDCHits("CDCHits");
      //	RelationArray cdcSimHitsToCDCHits(cdcArray,CDCHits);
      StoreArray<MCParticle> MCParticles("");
      StoreArray<CDCSimHit> SimHits("CDCSimHits");
      TClonesArray &tSTrackFitter3D = *m_tSTrackFitter3D;
      TClonesArray &mcTSTrackFitter3D = *m_mcTSTrackFitter3D;
      TClonesArray &fitTrackFitter3D = *m_fitTrackFitter3D;
      TClonesArray &wFit2DTrackFitter3D = *m_wFit2DTrackFitter3D;
      TClonesArray &fit2DTrackFitter3D = *m_fit2DTrackFitter3D;
      TClonesArray &szTrackFitter3D = *m_szTrackFitter3D;
      TClonesArray &wStAxPhiTrackFitter3D = *m_wStAxPhiTrackFitter3D;
      TClonesArray &stAxPhiTrackFitter3D = *m_stAxPhiTrackFitter3D;
      TClonesArray &mcTrackFitter3D = *m_mcTrackFitter3D;
      TClonesArray &mcStatusTrackFitter3D = *m_mcStatusTrackFitter3D;
      TClonesArray &stTSsTrackFitter3D = *m_stTSsTrackFitter3D;
      TClonesArray &mcVertexTrackFitter3D = *m_mcVertexTrackFitter3D;
      TClonesArray &mc4VectorTrackFitter3D = *m_mc4VectorTrackFitter3D;
      TClonesArray &mcHelixCenterTrackFitter3D = *m_mcHelixCenterTrackFitter3D;
      TClonesArray &mcImpactPositionTrackFitter3D = *m_mcImpactPositionTrackFitter3D;
      TClonesArray &eventTrackIDTrackFitter3D = *m_eventTrackIDTrackFitter3D;

      //...TS study (loop over all TS's)(for L/R LUT)...
      if(m_flagNonTSStudy == 1){
        const TRGCDC & cdc = * TRGCDC::getTRGCDC();
        for(unsigned i=0;i<cdc.nSegmentLayers();i++){
          const Belle2::TRGCDCLayer * l=cdc.segmentLayer(i);
          const unsigned nWires=l->nCells();
          if(!nWires) continue;
          for(unsigned j=0;j<nWires;j++){
            const TCSegment & t=(TCSegment &)*(*l)[j];
            unsigned int ptn=t.hitPattern();
            const std::vector<const TCWire*> &wires = t.wires();
            int nfl=0;
            int segsize=wires.size();
            if(segsize==15){
              if(ptn/1024){
                ptn=ptn%1024;
                nfl++;
              }
              if(ptn/64){
                ptn=ptn%64;
                nfl++;
              }
              if (ptn/8){
                ptn=ptn%8;
                nfl++;
              }
              if (ptn/2){
                ptn=ptn%2;
                nfl++;
              }
              if(ptn) nfl++;
            }
            else{
              if(ptn/256){
                ptn=ptn%256;
                nfl++;
              }
              if(ptn/64){
                ptn=ptn%64;
                nfl++;
              }
              if (ptn/32){
                ptn=ptn%32;
                nfl++;
              }
              if (ptn/8){
                ptn=ptn%8;
                nfl++;
              }
              if(ptn) nfl++;
            }

            if(nfl>=4){
              for(int k=0;k<segsize;k++){
                const TCWHit* wh = wires[k]->hit();
                if(!wh) continue;
                unsigned ind=wh->iCDCSimHit();
                CDCSimHit &h=*SimHits[ind];
                if(segsize==15){
                  if((k==0)||(k==1)||(k==2)){
                    cout << "TSflag: " <<h.getWireID().getICLayer()<< " " <<h.getWireID().getIWire()<< " " << t.hitPattern() << " " <<h.getPosFlag() << endl;
                  }
                }
                else{
                  if((k==5)||(k==6)||(k==7)){
                    cout << "TSflag: " <<h.getWireID().getICLayer()<< " " <<h.getWireID().getIWire()<< " " << t.hitPattern() << " " <<h.getPosFlag() << endl;
                  }
                }
              }
            }
          }
        }
      }

      double cotnum=0,sxx=0,z0num=0;
      int z0nump1[4],z0nump2[4],z0den,iz0den;

      tSTrackFitter3D.Clear();
      mcTSTrackFitter3D.Clear();
      fitTrackFitter3D.Clear();
      wFit2DTrackFitter3D.Clear();
      fit2DTrackFitter3D.Clear();
      szTrackFitter3D.Clear();
      wStAxPhiTrackFitter3D.Clear();
      stAxPhiTrackFitter3D.Clear();
      mcTrackFitter3D.Clear();
      mcStatusTrackFitter3D.Clear();
      stTSsTrackFitter3D.Clear();
      mcVertexTrackFitter3D.Clear();
      mc4VectorTrackFitter3D.Clear();
      mcHelixCenterTrackFitter3D.Clear();
      mcImpactPositionTrackFitter3D.Clear();
      eventTrackIDTrackFitter3D.Clear();


      //...Loop over track list...
      const unsigned nInput = trackListIn.size();
      // Index for number of successful fits
      int iFit=0;

      for (unsigned iInput = 0; iInput < nInput; iInput++) {
        double phi[9]={0,0,0,0,0,0,0,0,0};
        double phi_w[9]={0,0,0,0,0,0,0,0,0};
        int lutv[9];
        int ckt=1;

        //...Access to a track...
        TCTrack & aTrack = * trackListIn[iInput];

        // Access to track's MC particle.
        const TCRelation & trackRelation = aTrack.relation();
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

        //       t.dump("detail");

        //...Super layer loop...
        for (unsigned i = 0; i < _cdc.nSuperLayers(); i++) {

          //...Access to track segment list in this super layer...
          //const vector<const TCSegment *> & segments = t.trackSegments(i);
          const vector<TCLink *> & links = aTrack.links(i);
          const unsigned nSegments = links.size();

          //...Presently nSegments should be 1...
          if (nSegments != 1) {
            if (nSegments==0){
              ckt=0;
              cout << name() << " !!! NO TS assigned" << endl;
              break;
            }
            else{cout<< name()<<"!!!! multiple TS assigned"<< endl;}
          }

          //...Access to a track segment...
          //         links[0]->dump("detail");
          //          const TCSegment & s =  (TCSegment &) links[0]->hit()->cell();
          //         const TRGCDCCell *s = & links[0]->hit()->cell();
          //      phi[i]=(double) s->localId()/ni[i]*4*m_Trg_PI;
          //		double dphi=s->hit()->drift()*10;
          //		s.drift();
          //	cout << lutcomp << endl;
          //...using LRLUT to determine Left/Right(assume drift() will return drift distance)
          const TCSegment * s = dynamic_cast<const TCSegment *>(& links[0]->hit()->cell());
          phi_w[i]=(double) s->localId()/ni[s->superLayerId()]*4*m_Trg_PI;
          phi[i]=(double) s->localId()/ni[s->superLayerId()]*4*m_Trg_PI;
          //phi[i]=s->phiPosition();
          //cout<<"Hit pattern: "<<s->hitPattern()<<endl;
          //unsigned int tsSize = s->wires().size();
          //if (tsSize == 11) {
          //  cout<<s->name()<<" Hit pattern: "<<bitset<11>(s->hitPattern())<<endl;
          //} else if (tsSize == 15) {
          //  cout<<s->name()<<" Hit pattern: "<<bitset<15>(s->hitPattern())<<endl;
          //}
          lutv[i]=s->LUT()->getLRLUT(s->hitPattern(),s->superLayerId());
          //cout<<"lutv["<<i<<"]: "<<lutv[i]<<endl;

          if( m_flagWireLRLUT){
            ///...Using Drift time information
            float dphi=s->hit()->drift()*10;
            if(m_flagEvtTime==1){
              dphi-=evtTime;
            }
            dphi=atan(dphi/rro[s->superLayerId()]/1000);
            if(lutv[i]==0){phi[i]-=dphi;}
            else if(lutv[i]==1){phi[i]+=dphi;}
            else{
              phi[i]=phi[i];
            }
          }
          //else lutv[i]=2;
          if(m_flagmclr) lutv[i]=s->hit()->mcLR();
        } // End of superlayer loop

        double z0=-999, cot=-999;
        double zchi2=0.;
        double pt=-999, myphi0=-999;
        double myphi0_w=-999;
        double pt_w=-999;
        //...Do fitting job here (or call a fitting function)...
        if(ckt){
          int lutv2[4]={lutv[1],lutv[3],lutv[5],lutv[7]};
          double phi2[9]={phi[0], phi[2],phi[4],phi[6],phi[8],phi[1],phi[3],phi[5],phi[7]};
          double phi2_w[9]={phi_w[0], phi_w[2],phi_w[4],phi_w[6],phi_w[8],phi_w[1],phi_w[3],phi_w[5],phi_w[7]};
          double zz[4], arcS[4];
          double mczz[4];
          TVectorD wStAxPhi(4);
          TVectorD stAxPhi(4);
          int qqq=0;
          double rr_conv[4];
          double iezz2[4];
          double iezz21[4];
          double iezz22[4];
          double myphiz[4];	
          double ztheta=-999,ss=0.,sx=0.;
          double rho;
          double rho_w;

          // Initialize the candidates
          TVectorD stTSs(100);
          for(unsigned i=0; i<100; i++){
            stTSs[i] = 999.;
          }
          // Save the stereo candidates
          // Stereo 1
          for(unsigned i=0; i< _cdc.nStereoSuperLayers(); i++){
            vector<const TCSHit*> hits = _cdc.stereoSegmentHits(i);
            for(unsigned j=0; j<hits.size(); j++){
              if(j>24) continue;
              stTSs[25*i+j] = (double)hits[j]->cell().localId()/ni[2*i+1]*4*m_Trg_PI;
            }
          }
          // Save the candidates
          new(stTSsTrackFitter3D[iFit]) TVectorD(stTSs);

          // Save track segment information
          TVectorD tempPhi(9,phi2);
          new(tSTrackFitter3D[iFit]) TVectorD(tempPhi);
          //cout<<"TS: "<<phi2[0]<<" "<<phi2[1]<<endl;

          // Save mc track segment information
          // Initialize values for track
          int tSLayerId[9] = {2,16,28,40,52,10,22,34,46};
          double driftTS[9] = {999,999,999,999,999,999,999,999,999};
          TVectorD mcTS(27);
          for( int iHits = 0; iHits < SimHits.getEntries(); iHits++){
            CDCSimHit* aCDCSimHit = SimHits[iHits];
            TVector3 posTrack = aCDCSimHit->getPosTrack();
            int hitLayerId = aCDCSimHit->getWireID().getICLayer(); 
            double hitDriftLength = aCDCSimHit->getDriftLength() * Unit::cm;
            // Find 9 TS values
            for( int iTS = 0; iTS < 9; iTS++) {
              if(hitLayerId == tSLayerId[iTS]) {
                if(driftTS[iTS]>hitDriftLength) {
                  mcTS[3*iTS] = posTrack.x();
                  mcTS[3*iTS+1] = posTrack.y();
                  mcTS[3*iTS+2] = posTrack.z();
                  if(iTS>=5) mczz[iTS-5]=posTrack.z()/100;
                }
              }
            } // Find 9 TS values
          } // Loop for all CDC Sim hit values 
          //cout<<"mcTS: "<<atan2(mcTS[1],mcTS[0])<<" "<<atan2(mcTS[4],mcTS[3])<<endl;
          new(mcTSTrackFitter3D[iFit]) TVectorD(mcTS);


          // Sign Finder
          //int mysign = findSign(phi2);
          int mysign = int(aTrack.charge());

          // r-phi fitter(2D Fitter) ->calculate pt and radius of track-> input for 3D fitter.
          rPhiFit(rr,phi2,m_phierror,rho,myphi0);
          rPhiFit(rr,phi2_w,m_phierror,rho_w,myphi0_w);
          pt = 0.3*rho*1.5;
          pt_w = 0.3*rho_w*1.5;

          if(m_flagRealInt == 1){
            //Change input into relative values.
            for(unsigned i=0;i<4;i++){
              phi2[i+5]=phi2[i+5]-phi2[2];
              //Change the range to [-pi~pi]
              if(phi2[i+5]>m_Trg_PI){phi2[i+5]-=2*m_Trg_PI;}
              if(phi2[i+5]<-m_Trg_PI){phi2[i+5]+=2*m_Trg_PI;}
            }
            myphi0=myphi0-phi2[2];

            //Change the range to [-pi~pi]
            if(myphi0>m_Trg_PI){myphi0-=2*m_Trg_PI;}
            if(myphi0<-m_Trg_PI){myphi0+=2*m_Trg_PI;}
            //          cout << "track center " << myphi0 << endl;

            //int phi_st_int[4],myphi_int,rho_int,acos_int;

            //Change values into integers.
            //For phi stereos
            for (unsigned i=0;i<4;i++){
              phi2[i+5]=intnum2/3.2*phi2[i+5];
              if(phi2[i+5]>0){phi2[i+5]+=0.5;}
              else{phi2[i+5]-=0.5;}
              phi2[i+5]=(int) phi2[i+5];
            }
            //For myphi0
            myphi0=intnum2/3.2*myphi0;
            if(myphi0>0){myphi0+=0.5;}
            else myphi0-=0.5;
            myphi0=(int)myphi0;
            //For rho
            rho=(int)(intnum3/16*rho+0.5);
            //Interize rr
            for(unsigned i=0;i<4;i++){rr_conv[i]=(int)(rr[i+5]*intnum6/rr[8]+0.5);}
            //Change m_zerrorz to iezz2
            for(unsigned i=0;i<4;i++){
              iezz2[i]=(int)(1./m_zerror[i]/m_zerror[i]*intnum7*m_zerror[1]*m_zerror[1]+0.5);
              iezz21[i]=(int)(1./m_zerror1[i]/m_zerror1[i]*intnum7*m_zerror1[1]*m_zerror1[1]+0.5);
              iezz22[i]=(int)(1./m_zerror2[i]/m_zerror2[i]*intnum7*m_zerror2[1]*m_zerror2[1]+0.5);
            }

            //The actual start of the fitter
            //          cout << "rho " << rho<< " lut00 " <<lut00[(int) rho]<< " phi 5 " << phi2[5] << endl;

            if(mysign==1){
              myphiz[0]=(lut00[(int)rho]+1652)+myphi0-phi2[5];
              myphiz[1]=(lut01[(int)rho]+1366)+myphi0-phi2[6];
              myphiz[2]=(lut02[(int)rho]+1039)+myphi0-phi2[7];
              myphiz[3]=(lut03[(int)rho]+596)+myphi0-phi2[8];
            }else{
              myphiz[0]=-(lut00[(int)rho]+1652)+myphi0-phi2[5];
              myphiz[1]=-(lut01[(int)rho]+1366)+myphi0-phi2[6];
              myphiz[2]=-(lut02[(int)rho]+1039)+myphi0-phi2[7];
              myphiz[3]=-(lut03[(int)rho]+596)+myphi0-phi2[8];
            }

            //          cout << "myphiz " << myphiz[0]<<" " <<myphiz[1]<<" "<<myphiz[2]<< " " <<myphiz[3]<<endl;


            //Change myphi to correct z lut address.
            for(unsigned i=0;i<4;i++){
              if(myphiz[i]>=0){myphiz[i]=(int)myphiz[i];}
              else{myphiz[i]=(int)(1024+myphiz[i]);}
            }

            for(unsigned i=0;i<4;i++){
              if(int(myphiz[i])>1023)qqq=1;
            }	
            if(qqq) continue;

            zz[0]=zz_0_lut[int(myphiz[0])];
            zz[1]=zz_1_lut[int(myphiz[1])];
            zz[2]=zz_2_lut[int(myphiz[2])];
            zz[3]=zz_3_lut[int(myphiz[3])];

            //          cout << "zz " << zz[0]<<" " <<zz[1]<<" "<<zz[2]<< " " <<zz[3]<<endl;
            // Real space converter
            for(unsigned i=0; i<4; i++){
              zz[i]=zz[i]/intnum5*1.5;
              rr_conv[i]=rr_conv[i]*rr[8]/intnum6;
              iezz2[i]=iezz2[i]/m_zerror[1]/m_zerror[1]/intnum7;
              iezz21[i]=iezz21[i]/m_zerror1[1]/m_zerror1[1]/intnum7;
              iezz22[i]=iezz22[i]/m_zerror2[1]/m_zerror2[1]/intnum7;
              rho=rho/intnum3*16;
            }
          } else {
            // Real space
            // Calculate z position from rho and myphi0 and phi2
            for(unsigned i=0; i<4; i++){
              zz[i]=calZ(mysign, anglest[i], ztostraw[i],  rr[i+5], phi2[i+5], rho, myphi0);
              wStAxPhi[i]=calStAxPhi(mysign, anglest[i], ztostraw[i],  rr[i+5], rho_w, myphi0_w);
              stAxPhi[i]=calStAxPhi(mysign, anglest[i], ztostraw[i],  rr[i+5], rho, myphi0);
              rr_conv[i]=rr[i+5];
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

          // Save s and z hits for stand alone HoughFinder
          TVectorD tempSz(8);
          for(unsigned i=0; i<4; i++){
            tempSz[i] = arcS[i];
            tempSz[i+4] = zz[i];
          }
          new(szTrackFitter3D[iFit]) TVectorD(tempSz);

          // Save stereo's axial phi
          new(wStAxPhiTrackFitter3D[iFit]) TVectorD(wStAxPhi);
          new(stAxPhiTrackFitter3D[iFit]) TVectorD(stAxPhi);

          // RZ Fit
          if(m_flagRealInt == 1){
            // Integer space converter
            for(unsigned i=0; i<4; i++){
              zz[i] = zz[i]*intnum5/1.5;
              rr_conv[i] = rr_conv[i]/rr[8]*intnum6;
              iezz2[i] = iezz2[i]*m_zerror[1]*m_zerror[1]*intnum7;
            }

            //rz fitter
            z0=-999; ztheta=-999; ss=0.; sx=0.; cot=0.;
            for(unsigned i=0;i<4;i++){
              ss+=iezz2[i];
              sx+=rr_conv[i]*iezz2[i];
              sxx+=rr_conv[i]*rr_conv[i]*iezz2[i];
            }

            for(unsigned i=0;i<4;i++){
              cotnum+=(ss*rr_conv[i]-sx)*iezz2[i]*zz[i];
              z0nump1[i]=(int)(sxx-sx*rr_conv[i]);
              z0nump1[i]=z0nump1[i] >> numbit10;
              z0nump2[i]=(int)(z0nump1[i]*iezz2[i]*zz[i]);
              z0nump2[i]=z0nump2[i] >> numbit11;
              z0num+=z0nump2[i];
            }
            z0den=(int)((ss*sxx)-(sx*sx));
            z0den=z0den >> numbit9;
            iz0den=iz0den_lut[z0den];
            z0num*=iz0den;
            z0=z0num/intnum8/intnum9*intnum10*intnum11;
            cot=cotnum*iz0den/intnum8/intnum9;



            /*		double sxoss=(int) sx/ss;
                  for(unsigned i=0;i<4;i++){
                  tt=(rr_conv[i]-sxoss);
                  st2+= tt*tt*iezz2[i];
                  cot+= tt*zz[i]*iezz2[i];
                  }

                  cot/=st2;
                  z0=(sy-sx*cot)/ss;*/


            // Backwards convertor
            cot=cot*1.5/intnum5*intnum6/rr[8];
            z0=z0*1.5/intnum5;
            myphi0 = myphi0*3.2/intnum3;
            for(unsigned i=0;i<4;i++){
              rr_conv[i]=rr_conv[i]*rr[8]/intnum6;
              iezz2[i]=iezz2[i]/m_zerror[1]/m_zerror[1]/intnum7;
              zz[i]=zz[i]*1.5/intnum5;
            }
          } else {
            if(m_flagzierror){
              rSFit2(iezz21,iezz22,arcS, zz,lutv2,z0,cot,zchi2);
            }
            else{
              rSFit(iezz2, arcS, zz, z0, cot,zchi2);
            }
          }



          // Convert from rad to deg
          //myphi0 = myphi0*180/m_Trg_PI;

          ztheta=m_Trg_PI/2.-atan(cot);
          ztheta*=180./m_Trg_PI;

          // Save fit values
          TVectorD tempFit(14);
          tempFit[0]=pt;
          tempFit[1]=myphi0;
          tempFit[2]=z0;
          tempFit[3]=cot;
          tempFit[4]=mysign;
          tempFit[5]=zchi2;
          tempFit[6]=zz[0];
          tempFit[7]=zz[1];
          tempFit[8]=zz[2];
          tempFit[9]=zz[3];
          tempFit[10]=lutv2[0];
          tempFit[11]=lutv2[1];
          tempFit[12]=lutv2[2];
          tempFit[13]=lutv2[3];
          new(fitTrackFitter3D[iFit]) TVectorD(tempFit);

          // Save fit values
          TVectorD tempWFit(3);
          tempWFit[0]=pt_w;
          tempWFit[1]=myphi0_w;
          tempWFit[2]=mysign;
          new(wFit2DTrackFitter3D[iFit]) TVectorD(tempWFit);

          TVectorD tempMC(9);
          tempMC[0] = mcPt;
          tempMC[1] = mcPhi0;
          tempMC[2] = mcZ0;
          tempMC[3] = mcCot;
          tempMC[4] = mcCharge;
          tempMC[5] = mczz[0];
          tempMC[6] = mczz[1];
          tempMC[7] = mczz[2];
          tempMC[8] = mczz[3];
          new(mcTrackFitter3D[iFit]) TVectorD(tempMC);

          // Save mc information
          new(mcStatusTrackFitter3D[iFit]) TVectorD(mcStatus);
          new(mcVertexTrackFitter3D[iFit]) TVector3(vertex);
          new(mc4VectorTrackFitter3D[iFit]) TLorentzVector(vector4);
          new(mcHelixCenterTrackFitter3D[iFit]) TVector2(helixCenter);
          new(mcImpactPositionTrackFitter3D[iFit]) TVector3(impactPosition);

          // Save event and track ID
          TVectorD eventTrackID(2);
          eventTrackID[0] = m_eventNum;
          eventTrackID[1] = aTrack.getTrackID();
          new(eventTrackIDTrackFitter3D[iFit]) TVectorD(eventTrackID);


          // For integer space
          //          cout << "tsimz0/"  << z0*100 <<"]"<<endl;
          //          cout << "tsimpt/"  << pt <<"]"<<endl;
          //          cout << "tsimth/"  << ztheta <<"]"<<endl;
          //          cout << "tsimpi/" << myphi0*180/m_Trg_PI*3.2/intnum3 << "]" << endl;

          // Save 2D fit values from 2D fitter
          double pt2D;
          double phi02D;
          phi02D = aTrack.helix().phi0();
          pt2D = aTrack.helix().curv()*0.01*0.3*1.5;
          if(aTrack.charge()<0) {
            phi02D -= m_Trg_PI;
            if (phi02D < 0) phi02D += 2 * m_Trg_PI;
            pt2D = pt2D * -1;
          }
          TVectorD tempFit2D(3);
          tempFit2D[0] = pt2D;
          tempFit2D[1] = phi02D;
          tempFit2D[2] = aTrack.helix().dr()*0.01;
          new(fit2DTrackFitter3D[iFit]) TVectorD(tempFit2D);

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
