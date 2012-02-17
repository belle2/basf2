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
#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Link.h"
#include <cstdlib>

#include "framework/datastore/StoreArray.h"
#include "cdc/hitcdc/HitCDC.h"
#include "cdc/hitcdc/CDCSimHit.h"
#include "cdc/geometry/CDCGeometryPar.h"
#include "trg/trg/Time.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Link.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/HoughFinder.h"

//...Global varibles...
double rr[9]={0.188,0.4016, 0.620,0.8384,1.0568,0.2934,0.5128,0.7312,0.9496};	//can be replaced with geo-information
double anglest[4]={0.0702778, -0.06176, 0.069542, -0.07489};
double ztostraw[4]={-0.508184,-0.645228,-0.687681,-0.730134};	//can be replaced with geo-information
int ni[9]={320,320,384,448,512,576,640,704,768};
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
      return string("TRGCDCFitter3D 5.04");
    }

  TRGCDCFitter3D::TRGCDCFitter3D(const string & name,
      const TRGCDC & TRGCDC)
    : _name(name),
    _cdc(TRGCDC) {

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

  int
    TRGCDCFitter3D::doit(vector<TCTrack *> & trackListIn,
        vector<TCTrack *> & trackListOut) {

      TRGDebug::enterStage("Fitter 3D");

      //...TS study (loop over all TS's)...
      const TRGCDC & cdc = * TRGCDC::getTRGCDC();
      for (unsigned i = 0; i < cdc.nTrackSegmentLayers(); i++) {
	  const Belle2::TRGCDCLayer * l = cdc.trackSegmentLayer(i);
	  const unsigned nWires = l->nWires();
	  if (! nWires) continue;
	  unsigned ptn = 0;
	  for (unsigned j = 0; j < nWires; j++) {
	      const TCTSegment & s = (TCTSegment &) * (* l)[j];

	      //...Get hit pattern...
	      unsigned ptn = s.hitPattern();

	      if (ptn != 0)
		  cout << s.name() << " ... ptn=" << ptn << endl;

	      //...Or cal. hit pattern by my self...
	      const std::vector<const TCWire *> & wires = s.wires();
	      unsigned ptn2 = 0;
	      for (unsigned j = 0; j < wires.size(); j++) {
		  const TRGSignal & s = wires[j]->triggerOutput();
		  if (s.active()) {
		      ptn2 |= (1 << j);

		      //...Get index for CDCHit...
		      unsigned ind = wires[j]->hit()->iCDCHit();
		      // Use 'ind' to access CDCSimHit.
		  }
	      }

	      if (ptn != 0)
		  cout << s.name() << " ... ptn2=" << ptn2 << endl;
	  }
      }
    




      double cotnum=0,sxx=0,z0num=0;
      int z0nump1[4],z0nump2[4],z0den,iz0den;

      //...Loop over track list...
      const unsigned nInput = trackListIn.size();
      for (unsigned i = 0; i < nInput; i++) {
        double phi[9]={0,0,0,0,0,0,0,0,0};
        int ckt=1;
        int chk[9]={1,1,1,1,1,1,1,1,1};

        //...Access to a track...
        const TCTrack & t = * trackListIn[i];

        t.dump("detail");

        //...Super layer loop...
        for (unsigned i = 0; i < _cdc.nSuperLayers(); i++) {

          //...Access to track segment list in this super layer...
          //const vector<const TCTSegment *> & segments = t.trackSegments(i);
          const vector<TCLink *> & links = t.links(i);
          const unsigned nSegments = links.size();

          //...Presently nSegments should be 1...
          if (nSegments != 1) {
            if (nSegments==0){
              ckt=0;
              break;
              cout << name() << " !!! NO TS assigned" << endl;
            }
            else{cout<< name()<<"!!!! multiple TS assigned"<< endl;}
          }
          if (nSegments ==0){chk[i]=0;}
          ckt=ckt*chk[i];

          //...Access to a track segment...
          links[0]->dump("detail");
          //const TCTSegment & s = * (TCTSegment *) links[0]->hit();
          const TRGCDCWire *s = & links[0]->hit()->wire();
          phi[i]=(double) s->localId()/ni[i]*4*M_PI;
        }

        //...Do fitting job here (or call a fitting function)...
        if(ckt){
          double phi2[9]={phi[0], phi[2],phi[4],phi[6],phi[8],phi[1],phi[3],phi[5],phi[7]};
          double zz[4];
          double zerror[4]={0.0319263,0.028765,0.0290057,0.0396206};
          double A,B,C,D,E,G,hcx,hcy;
          double fiterror[5];
          double phierror[5]={0.0085106,0.0039841,0.0025806,0.0019084,0.001514};
          int qqq=0;

          kkk++;
          //re-ordering
          cout << "tsimTS/"<<phi2[0]<<" " <<phi2[1]<<" "<<phi2[2]<<" "<<phi2[3]<<" "<<phi2[4]<<" "<<phi2[5]<<" "<<phi2[6]<<" "<<phi2[7]<<" "<<phi2[8] <<"]" << endl;

          //Sign Finder
          int mysign;
          double sign_phi[2];
          if((phi2[0]-phi2[4])>M_PI||(phi2[0]-phi2[4])<-M_PI){
            if(phi2[0]>M_PI){sign_phi[0]=phi2[0]-2*M_PI;}
            else{sign_phi[0]=phi[0];}
            if(phi2[4]>M_PI){sign_phi[1]=phi2[4]-2*M_PI;}
            else{sign_phi[1]=phi[4];}
          }
          else{
            sign_phi[0]=phi2[0];
            sign_phi[1]=phi2[4];
          }
          if((sign_phi[1]-sign_phi[0])>0){mysign=0;}
          else{mysign=1;}


          //fiterror(added)
          for(unsigned i=0;i<5;i++){
            fiterror[i]=sqrt((rr[4]*rr[4]-2*rr[4]*rr[2]*cos(phi[4]-phi[2])+rr[2]*rr[2])/(sin(phi[4]-phi[2])*sin(phi[4]-phi[2]))-rr[i]*rr[i])*phierror[i];
          }

          //r-phi fitter(2D Fitter) ->calculate pt and radius of track-> input for 3D fitter.
          A=0,B=0,C=0,D=0,E=0,G=0,hcx=0,hcy=0;
          for(unsigned i=0;i<5;i++){
            A+=cos(phi2[i])*cos(phi2[i])/(fiterror[i]*fiterror[i]);
            B+=sin(phi2[i])*sin(phi2[i])/(fiterror[i]*fiterror[i]);
            C+=cos(phi2[i])*sin(phi2[i])/(fiterror[i]*fiterror[i]);
            D+=rr[i]*cos(phi2[i])/(fiterror[i]*fiterror[i]);
            E+=rr[i]*sin(phi2[i])/(fiterror[i]*fiterror[i]);
          }
          hcx=D*B-E*C;		//helix center x
          hcx/=2*(A*B-C*C);
          hcy=E*A-D*C;		//helix center y
          hcy/=2*(A*B-C*C);
          double rho=sqrt(hcx*hcx + hcy*hcy);	//radius of helix
          double pt=0.3*rho*1.5;

          //Calculate phi0(center of track)->this should be input for 3D fitter.
          double myphi0=atan(hcy/hcx);
          if(hcx<0 && hcy>0) myphi0+=M_PI;
          if(hcx<0 && hcy<0) myphi0+=M_PI;
          if(hcx>0 && hcy<0) myphi0+=M_PI*2.0;

          //Change input into relative values.
          for(unsigned i=0;i<4;i++){
            phi2[i+5]=phi2[i+5]-phi2[2];
            //Change the range to [-pi~pi]
            if(phi2[i+5]>M_PI){phi2[i+5]-=2*M_PI;}
            if(phi2[i+5]<-M_PI){phi2[i+5]+=2*M_PI;}
          }
          myphi0=myphi0-phi2[2];

          //Change the range to [-pi~pi]
          if(myphi0>M_PI){myphi0-=2*M_PI;}
          if(myphi0<-M_PI){myphi0+=2*M_PI;}
          cout << "track center " << myphi0 << endl;

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
          double rr_conv[4];
          for(unsigned i=0;i<4;i++){rr_conv[i]=(int)(rr[i+5]*intnum6/rr[8]+0.5);}
          //Change zerrorz to iezz2
          double iezz2[4];
          for(unsigned i=0;i<4;i++){
            iezz2[i]=(int)(1./zerror[i]/zerror[i]*intnum7*zerror[1]*zerror[1]+0.5);
          }

          //The actual start of the fitter
          double myphiz[4];	
          cout << "rho " << rho<< " lut00 " <<lut00[(int) rho]<< " phi 5 " << phi2[5] << endl;

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

          for(unsigned i=0;i<4;i++){
            if(myphiz[i]>1023)qqq=1;
          }	
          if(qqq) continue;
          cout << "myphiz " << myphiz[0]<<" " <<myphiz[1]<<" "<<myphiz[2]<< " " <<myphiz[3]<<endl;


          //Change myphi to correct z lut address.
          for(unsigned i=0;i<4;i++){
            if(myphiz[i]>=0){myphiz[i]=(int)myphiz[i];}
            else{myphiz[i]=(int)(1024+myphiz[i]);}
          }
          zz[0]=zz_0_lut[(int)myphiz[0]];
          zz[1]=zz_1_lut[(int)myphiz[1]];
          zz[2]=zz_2_lut[(int)myphiz[2]];
          zz[3]=zz_3_lut[(int)myphiz[3]];

          cout << "zz " << zz[0]<<" " <<zz[1]<<" "<<zz[2]<< " " <<zz[3]<<endl;


          //rz fitter
          double z0=-999,ztheta=-999,ss=0.,sx=0.,cot=0.;
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


          //Backwards convertor
          cot=cot*1.5/intnum5*intnum6/rr[8];
          z0=z0*1.5/intnum5;
          for(unsigned i=0;i<4;i++){
            rr_conv[i]=rr_conv[i]*rr[8]/intnum6;
            iezz2[i]=iezz2[i]/zerror[1]/zerror[1]/intnum7;
            zz[i]=zz[i]*1.5/intnum5;
          }


          ztheta=M_PI/2.-atan(cot);
          ztheta*=180./M_PI;

          cout << "tsimz0/"  << z0*100 <<"]"<<endl;
          cout << "tsimpt/"  << pt <<"]"<<endl;
          cout << "tsimth/"  << ztheta <<"]"<<endl;
          cout << "tsimpi/" << myphi0*180/M_PI*3.2/intnum3 << "]" << endl;

        }  
      }

      //...Termination...
      TRGDebug::leaveStage("Fitter 3D");
      return 0;
    }

} // namespace Belle2
