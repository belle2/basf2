#ifndef __CINT__
#include "trg/cdc/Fitter3DUtility.h"
#include <cmath>
#endif

#include <iostream>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector2.h"

using std::cout;
using std::endl;

int bitSize(int numberBits, int mode) {
  int bitsize = 1;
  if( mode == 1 ) {
    for(int i=0; i<numberBits-1; i++) {
      bitsize *= 2;
    }
      bitsize -= 1;
  } else if ( mode == 0 ) {
    for(int i=0; i<numberBits; i++) {
      bitsize *= 2;
    }
  }
  return bitsize;
}


void changeInteger(int &integer, double real, double minValue, double maxValue, int bitSize) {
  double range = maxValue - minValue; 
  double convert = bitSize/range;
  //cout<<"maxValue: "<<bitSize<<" realValue: "<<(real-minValue)*convert<<endl;
  integer = int( (real - minValue) * convert +0.5);
}

void changeReal(double &real, int integer, double minValue, double maxValue, int bitSize) {
  double range = maxValue - minValue;
  double convert = bitSize/range;
  real = (integer/convert) + minValue;
}


void findExtreme(double &m_max, double &m_min, double value) {
  if(value > m_max) m_max = value;
  if(value < m_min) m_min = value;
}

int findSign(double *phi2){
  double Trg_PI=3.141592653589793; 
  int mysign;
  double sign_phi[2];

  if((phi2[0]-phi2[4])>Trg_PI||(phi2[0]-phi2[4])<-Trg_PI){
    if(phi2[0]>Trg_PI){sign_phi[0]=phi2[0]-2*Trg_PI;}
    else{sign_phi[0]=phi2[0];}
    if(phi2[4]>Trg_PI){sign_phi[1]=phi2[4]-2*Trg_PI;}
    else{sign_phi[1]=phi2[4];}
  }
  else{
    sign_phi[0]=phi2[0];
    sign_phi[1]=phi2[4];
  }
  if((sign_phi[1]-sign_phi[0])>0){mysign=0;}
  else{mysign=1;}

  return mysign;
}

void rPhiFit(double *rr, double *phi2, double *phierror, double &rho, double &myphi0){

  double Trg_PI=3.141592653589793; 
  double A,B,C,D,E,G,hcx,hcy;
  double fiterror[5];
  //Calculate fit error
  for(unsigned i=0;i<5;i++){
    fiterror[i]=sqrt((rr[4]*rr[4]-2*rr[4]*rr[2]*cos(phi2[4]-phi2[2])+rr[2]*rr[2])/(sin(phi2[4]-phi2[2])*sin(phi2[4]-phi2[2]))-rr[i]*rr[i])*phierror[i];
  }

  //r-phi fitter(2D Fitter) ->calculate pt and radius of track-> input for 3D fitter.
  A=0,B=0,C=0,D=0,E=0,G=0,hcx=0,hcy=0;
  for(unsigned i=0;i<5;i++){
    A+=cos(phi2[i])*cos(phi2[i])/(fiterror[i]*fiterror[i]);
    B+=sin(phi2[i])*sin(phi2[i])/(fiterror[i]*fiterror[i]);
    C+=cos(phi2[i])*sin(phi2[i])/(fiterror[i]*fiterror[i]);
    D+=rr[i]*cos(phi2[i])/(fiterror[i]*fiterror[i]);
    E+=rr[i]*sin(phi2[i])/(fiterror[i]*fiterror[i]);
    G+=rr[i]*rr[i]/(fiterror[i]*fiterror[i]);
  }
  hcx=D*B-E*C;    //helix center x
  hcx/=2*(A*B-C*C);
  hcy=E*A-D*C;    //helix center y
  hcy/=2*(A*B-C*C);
  rho=sqrt(hcx*hcx + hcy*hcy);  //radius of helix
  myphi0=atan2(hcy,hcx);
  if(myphi0<0) myphi0 += 2*Trg_PI;
  //myphi0=atan(hcy/hcx);
  //if(hcx<0 && hcy>0) myphi0 += Trg_PI;
  //if(hcx<0 && hcy<0) myphi0 += Trg_PI;
  //if(hcx>0 && hcy<0) myphi0 += Trg_PI*2.0;

  // For chi2
  double pchi2 = -2*hcx*D-2*hcy*E+G;
  pchi2/=3;
  // Another way to calculate chi2
  double pchi3;
  for(unsigned i=0;i<5;i++){
    pchi3+=(2*(hcx*cos(phi2[i])+hcy*sin(phi2[i]))-rr[i])*(2*(hcx*cos(phi2[i])+hcy*sin(phi2[i]))-rr[i])/(fiterror[i]*fiterror[i]);
  }
  pchi3/=3;
  
}

void rPhiFit2(double *rr, double *phi2, double *phierror, double &rho, double &myphi0, int nTS){

  double Trg_PI=3.141592653589793; 
  double A,B,C,D,E,G,hcx,hcy;
  double fiterror[5];
  //Calculate fit error
  for(int i=0;i<nTS;i++){
    //fiterror[i]=sqrt((rr[4]*rr[4]-2*rr[4]*rr[2]*cos(phi2[4]-phi2[2])+rr[2]*rr[2])/(sin(phi2[4]-phi2[2])*sin(phi2[4]-phi2[2]))-rr[i]*rr[i])*phierror[i];
    fiterror[i] = 1+0*phierror[i];
  }

  //r-phi fitter(2D Fitter) ->calculate pt and radius of track-> input for 3D fitter.
  A=0,B=0,C=0,D=0,E=0,G=0,hcx=0,hcy=0;
  for(int i=0;i<nTS;i++){
    A+=cos(phi2[i])*cos(phi2[i])/(fiterror[i]*fiterror[i]);
    B+=sin(phi2[i])*sin(phi2[i])/(fiterror[i]*fiterror[i]);
    C+=cos(phi2[i])*sin(phi2[i])/(fiterror[i]*fiterror[i]);
    D+=rr[i]*cos(phi2[i])/(fiterror[i]*fiterror[i]);
    E+=rr[i]*sin(phi2[i])/(fiterror[i]*fiterror[i]);
    G+=rr[i]*rr[i]/(fiterror[i]*fiterror[i]);
  }
  hcx=D*B-E*C;    //helix center x
  hcx/=2*(A*B-C*C);
  hcy=E*A-D*C;    //helix center y
  hcy/=2*(A*B-C*C);
  rho=sqrt(hcx*hcx + hcy*hcy);  //radius of helix
  myphi0=atan2(hcy,hcx);
  if(myphi0<0) myphi0 += 2*Trg_PI;
  //myphi0=atan(hcy/hcx);
  //if(hcx<0 && hcy>0) myphi0 += Trg_PI;
  //if(hcx<0 && hcy<0) myphi0 += Trg_PI;
  //if(hcx>0 && hcy<0) myphi0 += Trg_PI*2.0;

  // For chi2
  double pchi2 = -2*hcx*D-2*hcy*E+G;
  pchi2/=nTS-2;
  // Another way to calculate chi2
  double pchi3;
  for(int i=0;i<nTS;i++){
    pchi3+=(2*(hcx*cos(phi2[i])+hcy*sin(phi2[i]))-rr[i])*(2*(hcx*cos(phi2[i])+hcy*sin(phi2[i]))-rr[i])/(fiterror[i]*fiterror[i]);
  }
  pchi3/=3;
  
}

double calStAxPhi(int &mysign, double &anglest, double &ztostraw, double &rr, double &rho, double &myphi0){
  if(1==2) cout<<anglest<<ztostraw<<endl; // Removes warnings when compileing

  double myphiz, acos_real;
  double Trg_PI=3.141592653589793; 
  //Find phifit-phist
  acos_real=acos(rr/(2*rho));
  if(mysign==1){
    myphiz = +acos_real+myphi0;
  }
  else{
    myphiz = -acos_real+myphi0;
  }
  if(myphiz>Trg_PI) myphiz-=2*Trg_PI;
  if(myphiz<-Trg_PI) myphiz+=2*Trg_PI;

  return myphiz;
}

double calDeltaPhi(int &mysign, double &anglest, double &ztostraw, double &rr, double &phi2, double &rho, double &myphi0){
  if(1==2) cout<<anglest<<ztostraw<<endl; // Removes warnings when compileing

  double myphiz, acos_real;
  double Trg_PI=3.141592653589793; 
  //Find phifit-phist
  acos_real=acos(rr/(2*rho));
  if(mysign==1){
    myphiz = -acos_real-myphi0+phi2;
  }
  else{
    myphiz = +acos_real-myphi0+phi2;
  }
  if(myphiz>Trg_PI) myphiz-=2*Trg_PI;
  if(myphiz<-Trg_PI) myphiz+=2*Trg_PI;

  return myphiz;
}

double calZ(int &mysign, double &anglest, double &ztostraw, double &rr, double &phi2, double &rho, double &myphi0){
  double myphiz, acos_real;
  double Trg_PI=3.141592653589793; 
  //Find phifit-phist
  acos_real=acos(rr/(2*rho));
  if(mysign==1){
    myphiz = -acos_real-myphi0+phi2;
  }
  else{
    myphiz = +acos_real-myphi0+phi2;
  }
  if(myphiz>Trg_PI) myphiz-=2*Trg_PI;
  if(myphiz<-Trg_PI) myphiz+=2*Trg_PI;

  return (ztostraw - rr*2*sin(myphiz/2)/anglest);
}

double calS(double &rho, double &rr){
  double result;
  result = rho*2*asin(rr/2/rho);
  return result;
}

double rSFit(double *iezz2, double *arcS, double *zz, double &z0, double &cot){

  double ss=0, sx=0, sxx=0, cotnum=0, z0num=0;
  double z0nump1[4], z0nump2[4];
  double z0den, iz0den;

  for(unsigned i=0;i<4;i++){
    ss+=iezz2[i];
    sx+=arcS[i]*iezz2[i];
    sxx+=arcS[i]*arcS[i]*iezz2[i];
  }

  for(unsigned i=0;i<4;i++){
    cotnum+=(ss*arcS[i]-sx)*iezz2[i]*zz[i];
    z0nump1[i]=sxx-sx*arcS[i];
    z0nump2[i]=z0nump1[i]*iezz2[i]*zz[i];
    z0num+=z0nump2[i];
  }
  z0den=(ss*sxx)-(sx*sx);
  iz0den=1./z0den;
  z0num*=iz0den;
  cotnum*=iz0den;
  z0=z0num;
  cot=cotnum;

  // Calculate chi2 of z0
  double zchi2 = 0.;
  for(unsigned i=0;i<4;i++){
    zchi2 += (zz[i]-z0-cot*arcS[i])*(zz[i]-z0-cot*arcS[i])*iezz2[i];
  }
  zchi2 /= (4-2);

  return zchi2;

}

void findImpactPosition(TVector3 * mcPosition, TLorentzVector * mcMomentum, int charge, TVector2 & helixCenter, TVector3 & impactPosition){

  // Finds the impact position. Everything is in cm, and GeV.
  // Input:   production vertex (mcx, mcy, mcz),
  //          momentum at production vertex (px, py, pz)
  //          charge of particle.
  // Output:  helix center's coordiante (hcx, hcy)
  //          impact position (impactX, impactY, impactZ)

  double rho = sqrt(pow(mcMomentum->Px(),2)+pow(mcMomentum->Py(),2))/0.3/1.5*100;
  double hcx = mcPosition->X()+rho*cos(atan2(mcMomentum->Py(),mcMomentum->Px())-charge*TMath::Pi()/2);
  double hcy = mcPosition->Y()+rho*sin(atan2(mcMomentum->Py(),mcMomentum->Px())-charge*TMath::Pi()/2);
  helixCenter.Set(hcx,hcy);
  double impactX = (helixCenter.Mod()-rho)/helixCenter.Mod()*helixCenter.X();
  double impactY = (helixCenter.Mod()-rho)/helixCenter.Mod()*helixCenter.Y();
  int signdS;
  if(atan2(impactY, impactX) < atan2(mcPosition->Y(),mcPosition->X())) signdS = -1;
  else signdS = 1;
  double dS = 2*rho*asin(sqrt(pow(impactX-mcPosition->X(),2)+pow(impactY-mcPosition->Y(),2))/2/rho);
  double impactZ = mcMomentum->Pz()/mcMomentum->Pt()*dS*signdS+mcPosition->Z();
  impactPosition.SetXYZ(impactX, impactY, impactZ);

}
