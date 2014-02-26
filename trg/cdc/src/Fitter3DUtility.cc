#ifndef __CINT__
#include "trg/cdc/Fitter3DUtility.h"
#include <cmath>
#else
#include "Fitter3DUtility.h"
#endif

#include <iostream>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector2.h"

using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::string;

int Fitter3DUtility::bitSize(int numberBits, int mode) {
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


void Fitter3DUtility::changeInteger(int &integer, double real, double minValue, double maxValue, int bitSize) {
  double range = maxValue - minValue; 
  double convert = bitSize/range;
  //cout<<"maxValue: "<<bitSize<<" realValue: "<<(real-minValue)*convert<<endl;
  integer = int( (real - minValue) * convert +0.5);
}

void Fitter3DUtility::changeReal(double &real, int integer, double minValue, double maxValue, int bitSize) {
  double range = maxValue - minValue;
  double convert = bitSize/range;
  real = (integer/convert) + minValue;
}


void Fitter3DUtility::findExtreme(double &m_max, double &m_min, double value) {
  if(value > m_max) m_max = value;
  if(value < m_min) m_min = value;
}

int Fitter3DUtility::findSign(double *phi2){
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

void Fitter3DUtility::rPhiFit(double *rr, double *phi2, double *phierror, double &rho, double &myphi0){

  // Print input values
  //for(unsigned iSL=0; iSL<5; iSL++){
  //  cout<<"SL["<<iSL<<"] rr: "<<rr[iSL]<<" phi: "<<phi2[iSL]<<" phiError: "<<phierror[iSL]<<endl;
  //}

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
  double pchi3 = 0.; //iw
  for(unsigned i=0;i<5;i++){
    pchi3+=(2*(hcx*cos(phi2[i])+hcy*sin(phi2[i]))-rr[i])*(2*(hcx*cos(phi2[i])+hcy*sin(phi2[i]))-rr[i])/(fiterror[i]*fiterror[i]);
  }
  pchi3/=3;
  
}

void Fitter3DUtility::rPhiFit2(double *rr, double *phi2, double *phierror, double &rho, double &myphi0, int nTS){

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
  double pchi3 = 0.; //iw
  for(int i=0;i<nTS;i++){
    pchi3+=(2*(hcx*cos(phi2[i])+hcy*sin(phi2[i]))-rr[i])*(2*(hcx*cos(phi2[i])+hcy*sin(phi2[i]))-rr[i])/(fiterror[i]*fiterror[i]);
  }
  pchi3/=3;
  
}

double Fitter3DUtility::calPhi(double wirePhi, double driftLength, double eventTime, double rr, int lr){
  double result = wirePhi;
  double t_dPhi=driftLength*10 - eventTime;
  // Change to radian
  t_dPhi=atan(t_dPhi/rr/10);
  // Use LR to add dPhi
  if(lr == 0) result -= t_dPhi;
  else if(lr == 1) result += t_dPhi;
  return result;
}

double Fitter3DUtility::calStAxPhi(int mysign, double anglest, double ztostraw, double rr, double rho, double myphi0){
  if(1==2) cout<<anglest<<ztostraw<<endl; // Removes warnings when compileing

  double myphiz, acos_real;
  double Trg_PI=3.141592653589793; 
  //Find phifit-phist
  double t_rho = rho;
  if(rr>(2*rho)) t_rho = rr/2;
  acos_real=acos(rr/(2*t_rho));
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

double Fitter3DUtility::calDeltaPhi(int mysign, double anglest, double ztostraw, double rr, double phi2, double rho, double myphi0){
  if(1==2) cout<<anglest<<ztostraw<<endl; // Removes warnings when compileing

  double myphiz, acos_real;
  double Trg_PI=3.141592653589793; 
  //Find phifit-phist
  double t_rho = rho;
  if(rr>(2*rho)) t_rho = rr/2;
  acos_real=acos(rr/(2*t_rho));
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

double Fitter3DUtility::calZ(int mysign, double anglest, double ztostraw, double rr, double phi2, double rho, double myphi0){
  double myphiz, acos_real;
  double Trg_PI=3.141592653589793; 
  //Find phifit-phist
  double t_rho = rho;
  if(rr>(2*rho)) t_rho = rr/2;
  acos_real=acos(rr/(2*t_rho));
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

double Fitter3DUtility::calS(double rho, double rr){
  double result;
  result = rho*2*asin(rr/2/rho);
  return result;
}

void Fitter3DUtility::rSFit(double *iezz2, double *arcS, double *zz, double &z0, double &cot, double &zchi2){

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
  zchi2 = 0.;
  for(unsigned i=0;i<4;i++){
    zchi2 += (zz[i]-z0-cot*arcS[i])*(zz[i]-z0-cot*arcS[i])*iezz2[i];
  }
  zchi2 /= (4-2);

}

void Fitter3DUtility::rSFit2(double *iezz21,double *iezz22, double *arcS, double *zz, int *lutv, double &z0, double &cot,double &zchi2){
//      cout << "rs2" << endl;

  double ss=0, sx=0, sxx=0, cotnum=0, z0num=0;
  double z0nump1[4], z0nump2[4];
  double z0den, iz0den;

  for(unsigned i=0;i<4;i++){
    if (lutv[i]==2){
      ss+=iezz21[i];
      sx+=arcS[i]*iezz21[i];
      sxx+=arcS[i]*arcS[i]*iezz21[i];
    }
    else{
      ss+=iezz22[i];
      sx+=arcS[i]*iezz22[i];
      sxx+=arcS[i]*arcS[i]*iezz22[i];
    }
  }

  for(unsigned i=0;i<4;i++){
    if(lutv[i]==2){
      cotnum+=(ss*arcS[i]-sx)*iezz21[i]*zz[i];
      z0nump1[i]=sxx-sx*arcS[i];
      z0nump2[i]=z0nump1[i]*iezz21[i]*zz[i];
      z0num+=z0nump2[i];
    }
    else{
      cotnum+=(ss*arcS[i]-sx)*iezz22[i]*zz[i];
      z0nump1[i]=sxx-sx*arcS[i];
      z0nump2[i]=z0nump1[i]*iezz22[i]*zz[i];
      z0num+=z0nump2[i];
    }
  }
  z0den=(ss*sxx)-(sx*sx);
  iz0den=1./z0den;
  z0num*=iz0den;
  cotnum*=iz0den;
  z0=z0num;
  cot=cotnum;

  // Calculate chi2 of z0
//  double zchi2 = 0.;
  for(unsigned i=0;i<4;i++){
    if(lutv[i]==2){
      zchi2 += (zz[i]-z0-cot*arcS[i])*(zz[i]-z0-cot*arcS[i])*iezz21[i];
    }
    else{
      zchi2 += (zz[i]-z0-cot*arcS[i])*(zz[i]-z0-cot*arcS[i])*iezz22[i];
    }
  }
  zchi2 /= (4-2);
}


void Fitter3DUtility::findImpactPosition(TVector3 * mcPosition, TLorentzVector * mcMomentum, int charge, TVector2 & helixCenter, TVector3 & impactPosition){

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

void Fitter3DUtility::calHelixParameters(TVector3 position, TVector3 momentum, int charge, TVectorD& helixParameters){
  double Trg_PI=3.141592653589793; 
  // HelixParameters: dR, phi0, keppa, dz, tanLambda
  double t_alpha = 1/0.3/1.5*100;
  double t_pT = momentum.Perp();
  double t_R = t_pT * t_alpha;
  helixParameters.Clear();
  helixParameters.ResizeTo(5);
  helixParameters[2] = t_alpha/t_R;
  helixParameters[1] = atan2(position.Y() - t_R * momentum.X() / t_pT * charge, position.X() + t_R * momentum.Y() / t_pT * charge);
  helixParameters[0] = (position.X() + t_R * momentum.Y() / t_pT * charge ) / cos(helixParameters[1]) - t_R;
  double t_phi = atan2(-momentum.X()*charge, momentum.Y()*charge) - helixParameters[1];
  if(t_phi > Trg_PI) t_phi -= 2*Trg_PI;
  if(t_phi < -Trg_PI) t_phi += 2*Trg_PI;
  helixParameters[4] = momentum.Z() / t_pT * charge;
  helixParameters[3] = position.Z() + helixParameters[4] * t_R * t_phi;
}
void Fitter3DUtility::calVectorsAtR(TVectorD& helixParameters, int charge, double cdcRadius, TVector3& position, TVector3& momentum){
  // HelixParameters: dR, phi0, keppa, dz, tanLambda
  double t_alpha = 1/0.3/1.5*100;
  double t_R = t_alpha / helixParameters[2];
  double t_pT = t_R / t_alpha;
  double t_phi = -1*charge*acos((-pow(cdcRadius,2) + pow(t_R+helixParameters[0],2) + pow(t_R,2)) / (2*t_R*(t_R+helixParameters[0])));
  double t_X = (helixParameters[0]+t_R)*cos(helixParameters[1])-t_R*cos(helixParameters[1]+t_phi);
  double t_Y = (helixParameters[0]+t_R)*sin(helixParameters[1])-t_R*sin(helixParameters[1]+t_phi);
  double t_Z = helixParameters[3] - helixParameters[4]*t_R*t_phi;
  double t_Px = -t_pT * sin(helixParameters[1]+t_phi)*charge;
  double t_Py = t_pT * cos(helixParameters[1]+t_phi)*charge;
  double t_Pz = t_pT * helixParameters[4]*charge;
  position.SetXYZ(t_X, t_Y, t_Z);
  momentum.SetXYZ(t_Px, t_Py, t_Pz);
}


// Convert method: abs(min)=abs(max): signed. min=0: unsigned. else: const+unsigned.
// [TODO] Make function that calculates "const" in else case.
int Fitter3DUtility::convertToInt(double value, std::vector<double> convertInformation){
  // convertInformation: (min, max, #bits): 
  double min = convertInformation[0];
  double max = convertInformation[1];
  int nBits = convertInformation[2];
  double result;
  // signed case
  if(abs(min)==abs(max)){
    double t_bitRange = pow(2,nBits)/2 - 0.5;
    double t_realRange = max;
    double t_min = 0;
    result = (value-t_min)*float(t_bitRange)/t_realRange;
    if(result > 0){
      result = result + 0.5;
      result = (result == int(result) ? result - 1 : int(result) );
    } else {
      result = result - 0.5;
      result = (result == int(result) ? result + 1 : int(result) );
    }
  // unsigned case
  } else if (min == 0){
    int t_bitRange = pow(2,nBits);
    double t_min = max / (t_bitRange-0.5) * (-0.5);
    double t_realRange = max - t_min;
    result = (value-t_min)*float(t_bitRange)/t_realRange;
    result = (result == int(result) ? result - 1 : int(result) );
  // unsigned + const case
  } else {
    int t_bitRange = pow(2,nBits);
    double t_realRange = max - min;
    result = (value-min)*float(t_bitRange)/t_realRange;
    result = (result == int(result) ? result - 1 : int(result) );
    if(value == min ) result = 0;
  }
  return int(result);
}

// Convert method: abs(min)=abs(max): signed. min=0: unsigned. else: const+unsigned.
double Fitter3DUtility::convertToDouble(int value, std::vector<double> convertInformation){
  // convertInformation: (min, max, #bits): 
  double min = convertInformation[0];
  double max = convertInformation[1];
  int nBits = convertInformation[2];
  double result;
  // signed case
  if(abs(min)==abs(max)){
    double t_bitRange = pow(2,nBits)/2 - 0.5;
    double t_realRange = max;
    double t_min = 0;
    result = (value-t_min)*t_realRange/float(t_bitRange);
  // unsigned case
  } else if (min == 0){
    int t_bitRange = pow(2,nBits);
    double t_min = max / (t_bitRange-0.5) * (-0.5);
    double t_realRange = max - t_min;
    result = value*t_realRange/float(t_bitRange);
  } else {
  // unsigned + const case
    int t_bitRange = pow(2,nBits);
    double t_realRange = max - min;
    result = (value+0.5)*t_realRange/float(t_bitRange) + min;
  }
  return result;
}
