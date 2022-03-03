/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef __EXTERNAL__
#include "trg/cdc/Fitter3DUtility.h"
#include "trg/cdc/FpgaUtility.h"
#include "trg/cdc/JLUT.h"
#include "trg/cdc/JSignal.h"
#include "trg/cdc/JSignalData.h"
#include <cmath>
#else
#include "Fitter3DUtility.h"
#include "JLUT.h"
#include "JSignal.h"
#include "JSignalData.h"
#endif
#include <utility>
#include <iostream>
#include <fstream>
#include "TVector3.h"
#include "TVector2.h"

using std::cout;
using std::endl;
using std::pair;
using std::make_pair;
using std::tuple;
using std::vector;
using std::map;
using std::string;
using std::to_string;
using std::get;
using std::function;
using std::ofstream;
using std::ifstream;
using std::make_tuple;

int Fitter3DUtility::findSign(double* phi2)
{
  double Trg_PI = 3.141592653589793;
  int mysign;
  double sign_phi[2];

  if ((phi2[0] - phi2[4]) > Trg_PI || (phi2[0] - phi2[4]) < -Trg_PI) {
    if (phi2[0] > Trg_PI) {sign_phi[0] = phi2[0] - 2 * Trg_PI;}
    else {sign_phi[0] = phi2[0];}
    if (phi2[4] > Trg_PI) {sign_phi[1] = phi2[4] - 2 * Trg_PI;}
    else {sign_phi[1] = phi2[4];}
  } else {
    sign_phi[0] = phi2[0];
    sign_phi[1] = phi2[4];
  }
  if ((sign_phi[1] - sign_phi[0]) > 0) {mysign = 0;}
  else {mysign = 1;}

  return mysign;
}

void Fitter3DUtility::rPhiFit(double* rr, double* phi2, double* phierror, double& rho, double& myphi0)
{

  cout << "Fitter3DUtility::rPhiFit() will be deprecated. Please use Fitter3DUtility::rPhiFitter(). phierror was changed to inv phi error."
       << endl;
  double invphierror[5];
  for (unsigned i = 0; i < 5; i++) {
    invphierror[i] = 1 / phierror[i];
  }
  rPhiFitter(rr, phi2, invphierror, rho, myphi0);
}

void Fitter3DUtility::rPhiFitter(double* rr, double* phi2, double* invphierror, double& rho, double& myphi0)
{

  double chi2;
  rPhiFitter(rr, phi2, invphierror, rho, myphi0, chi2);

}

void Fitter3DUtility::rPhiFitter(double* rr, double* phi2, double* invphierror, double& rho, double& myphi0, double& chi2)
{

  // Print input values
  //for(unsigned iSL=0; iSL<5; iSL++){
  //  cout<<"SL["<<iSL<<"] rr: "<<rr[iSL]<<" phi: "<<phi2[iSL]<<" phiError: "<<phierror[iSL]<<endl;
  //}

  double Trg_PI = 3.141592653589793;
  double A, B, C, D, E, hcx, hcy;
  double invFiterror[5];
  //double G;
  //Calculate fit error
  for (unsigned i = 0; i < 5; i++) {
    // Sometimes SL8 and SL4 will not be hit. So cannot use below calculation.
    //invFiterror[i]=1/sqrt((rr[4]*rr[4]-2*rr[4]*rr[2]*cos(phi2[4]-phi2[2])+rr[2]*rr[2])/(sin(phi2[4]-phi2[2])*sin(phi2[4]-phi2[2]))-rr[i]*rr[i])*invphierror[i];
    //invFiterror[i]=invphierror[i];
    //invFiterror[i]=invphierror[i]*rr[i];
    invFiterror[i] = invphierror[i] / rr[i];
  }

  //r-phi fitter(2D Fitter) ->calculate pt and radius of track-> input for 3D fitter.
  A = 0, B = 0, C = 0, D = 0, E = 0, hcx = 0, hcy = 0;
  //G=0;
  for (unsigned i = 0; i < 5; i++) {
    A += cos(phi2[i]) * cos(phi2[i]) * (invFiterror[i] * invFiterror[i]);
    B += sin(phi2[i]) * sin(phi2[i]) * (invFiterror[i] * invFiterror[i]);
    C += cos(phi2[i]) * sin(phi2[i]) * (invFiterror[i] * invFiterror[i]);
    D += rr[i] * cos(phi2[i]) * (invFiterror[i] * invFiterror[i]);
    E += rr[i] * sin(phi2[i]) * (invFiterror[i] * invFiterror[i]);
    //G+=rr[i]*rr[i]/(fiterror[i]*fiterror[i]);
  }
  hcx = D * B - E * C; //helix center x
  hcx /= 2 * (A * B - C * C);
  hcy = E * A - D * C; //helix center y
  hcy /= 2 * (A * B - C * C);
  rho = sqrt(hcx * hcx + hcy * hcy); //radius of helix
  myphi0 = atan2(hcy, hcx);
  if (myphi0 < 0) myphi0 += 2 * Trg_PI;
  //myphi0=atan(hcy/hcx);
  //if(hcx<0 && hcy>0) myphi0 += Trg_PI;
  //if(hcx<0 && hcy<0) myphi0 += Trg_PI;
  //if(hcx>0 && hcy<0) myphi0 += Trg_PI*2.0;

  //// For chi2
  // double pchi2 = -2*hcx*D-2*hcy*E+G;
  // pchi2/=3;

  // Count number of total TS hits.
  int nTSHits = 0;
  for (unsigned iAx = 0; iAx < 5; iAx++) {
    if (invphierror[iAx] != 0) nTSHits++;
  }
  // Another way to calculate chi2
  chi2 = 0.;
  for (unsigned i = 0; i < 5; i++) {
    chi2 += (2 * (hcx * cos(phi2[i]) + hcy * sin(phi2[i])) - rr[i]) * (2 * (hcx * cos(phi2[i]) + hcy * sin(phi2[i])) - rr[i]) /
            (invFiterror[i] * invFiterror[i]);
  }
  chi2 /= nTSHits - 2;

}

void Fitter3DUtility::chargeFinder(double* nTSs, double* tsIds, double* tsHitmap, double phi0, double inCharge,
                                   double& foundCharge)
{
  double Trg_PI = 3.141592653589793;
  vector<double> tsPhi(5);
  vector<double> dPhi(5);
  vector<double> dPhi_c(5);
  vector<double> charge(5);
  for (unsigned iAx = 0; iAx < 5; iAx++) {
    //cout<<"iAx:"<<iAx<<" nTSs:"<<nTSs[iAx]<<" tsId:"<<tsIds[iAx]<<" hitmap:"<<tsHitmap[iAx]<<" phi0:"<<phi0<<" inCharge:"<<inCharge<<endl;
    // Change tsId to rad unit.
    tsPhi[iAx] = tsIds[iAx] * 2 * Trg_PI / nTSs[iAx];
    dPhi[iAx] = tsPhi[iAx] - phi0;
    //cout<<"iAx:"<<iAx<<" dPhi:"<<dPhi[iAx]<<endl;
    // Constrain dPhi to [-pi, pi]
    if (dPhi[iAx] < -Trg_PI) dPhi_c[iAx] = dPhi[iAx] + 2 * Trg_PI;
    else if (dPhi[iAx] > Trg_PI) dPhi_c[iAx] = dPhi[iAx] - 2 * Trg_PI;
    else dPhi_c[iAx] = dPhi[iAx];
    //cout<<"iAx:"<<iAx<<" dPhi_c:"<<dPhi_c[iAx]<<endl;
    // Calculate charge
    if (tsHitmap[iAx] == 0) charge[iAx] = 0;
    else if (dPhi_c[iAx] == 0) charge[iAx] = 0;
    else if (dPhi_c[iAx] > 0) charge[iAx] = 1;
    else charge[iAx] = -1;
    //cout<<"iAx:"<<iAx<<" charge:"<<charge[iAx]<<endl;
  }
  // Sum up charge
  double sumCharge = charge[0] + charge[1] + charge[2] + charge[3] + charge[4];
  //cout<<"sumCharge:"<<sumCharge<<endl;
  // Calculate foundCharge
  if (sumCharge == 0) foundCharge = inCharge;
  else if (sumCharge > 0) foundCharge = 1;
  else foundCharge = -1;
  //cout<<"foundCharge:"<<foundCharge<<endl;
  //cout<<"inCharge:"<<inCharge<<endl;
}


void Fitter3DUtility::rPhiFit2(double* rr, double* phi2, double* phierror, double& rho, double& myphi0, int nTS)
{

  double Trg_PI = 3.141592653589793;
  double A, B, C, D, E, hcx, hcy;
  //double G;
  double fiterror[5];
  //Calculate fit error
  for (int i = 0; i < nTS; i++) {
    //fiterror[i]=sqrt((rr[4]*rr[4]-2*rr[4]*rr[2]*cos(phi2[4]-phi2[2])+rr[2]*rr[2])/(sin(phi2[4]-phi2[2])*sin(phi2[4]-phi2[2]))-rr[i]*rr[i])*phierror[i];
    fiterror[i] = 1 + 0 * phierror[i];
  }

  //r-phi fitter(2D Fitter) ->calculate pt and radius of track-> input for 3D fitter.
  A = 0, B = 0, C = 0, D = 0, E = 0, hcx = 0, hcy = 0;
  //G=0;
  for (int i = 0; i < nTS; i++) {
    A += cos(phi2[i]) * cos(phi2[i]) / (fiterror[i] * fiterror[i]);
    B += sin(phi2[i]) * sin(phi2[i]) / (fiterror[i] * fiterror[i]);
    C += cos(phi2[i]) * sin(phi2[i]) / (fiterror[i] * fiterror[i]);
    D += rr[i] * cos(phi2[i]) / (fiterror[i] * fiterror[i]);
    E += rr[i] * sin(phi2[i]) / (fiterror[i] * fiterror[i]);
    //G+=rr[i]*rr[i]/(fiterror[i]*fiterror[i]);
  }
  hcx = D * B - E * C; //helix center x
  hcx /= 2 * (A * B - C * C);
  hcy = E * A - D * C; //helix center y
  hcy /= 2 * (A * B - C * C);
  rho = sqrt(hcx * hcx + hcy * hcy); //radius of helix
  myphi0 = atan2(hcy, hcx);
  if (myphi0 < 0) myphi0 += 2 * Trg_PI;
  //myphi0=atan(hcy/hcx);
  //if(hcx<0 && hcy>0) myphi0 += Trg_PI;
  //if(hcx<0 && hcy<0) myphi0 += Trg_PI;
  //if(hcx>0 && hcy<0) myphi0 += Trg_PI*2.0;

  //// For chi2
  // double pchi2 = -2*hcx*D-2*hcy*E+G;
  // pchi2/=nTS-2;

  //// Another way to calculate chi2
  //double pchi3 = 0.; //iw
  //for(int i=0;i<nTS;i++){
  //  pchi3+=(2*(hcx*cos(phi2[i])+hcy*sin(phi2[i]))-rr[i])*(2*(hcx*cos(phi2[i])+hcy*sin(phi2[i]))-rr[i])/(fiterror[i]*fiterror[i]);
  //}
  //pchi3/=3;
}

unsigned Fitter3DUtility::toUnsignedTdc(int tdc, int nBits)
{
  bool notDone = 1;
  int tdcMax = pow(2, nBits) - 1;
  int resultTdc = tdc;
  while (notDone) {
    if (resultTdc > tdcMax) resultTdc -= (tdcMax + 1);
    else if (resultTdc < 0) resultTdc += (tdcMax + 1);
    else notDone = 0;
  }
  return (unsigned) resultTdc;
}

double Fitter3DUtility::calPhi(double wirePhi, double driftLength, double rr, int lr)
{
  //cout<<"rr:"<<rr<<" lr:"<<lr;
  double result = wirePhi;
  //cout<<" wirePhi:"<<result;
  // time is in 2ns rms clock.
  double t_dPhi = driftLength;
  //cout<<" driftLength:"<<t_dPhi;
  // Change to radian
  // rr is cm scale.
  t_dPhi = atan(t_dPhi / rr);
  //cout<<" driftPhi:"<<t_dPhi;
  // Use LR to add dPhi
  if (lr == 1) result -= t_dPhi;
  else if (lr == 2) result += t_dPhi;
  //cout<<" phi:"<<result<<endl;
  return result;
}

double Fitter3DUtility::calPhi(double wirePhi, double driftTime, double eventTime, double rr, int lr)
{
  // time is in 2ns rms clock.
  double t_driftLength = (driftTime - eventTime) * 1000 / 1017.774 * 2 * 40 / 1000 / 10;
  return calPhi(wirePhi, t_driftLength, rr, lr);
}

double Fitter3DUtility::calPhi(int localId, int nWires, double driftTime, double eventTime, double rr, int lr)
{
  double wirePhi = (double)localId / nWires * 4 * M_PI;
  return Fitter3DUtility::calPhi(wirePhi, driftTime, eventTime, rr, lr);
}

void Fitter3DUtility::calPhi(std::map<std::string, double> const& mConstD,
                             std::map<std::string, std::vector<double> > const& mConstV, std::map<std::string, Belle2::TRGCDCJSignal>& mSignalStorage,
                             std::map<std::string, Belle2::TRGCDCJLUT* >& mLutStorage)
{
  Belle2::TRGCDCJSignalData* commonData = mSignalStorage.begin()->second.getCommonData();
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<tsId_"<<iSt<<">>>"<<endl; mSignalStorage["tsId_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<tdc_"<<iSt<<">>>"<<endl; mSignalStorage["tdc_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<lr_"<<iSt<<">>>"<<endl; mSignalStorage["lr_"+to_string(iSt)].dump();}
  // Make phiFactor constants
  if (mSignalStorage.find("phiFactor_0") == mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      int nShiftBits = int(log(pow(2, 24) / 2 / mConstD.at("Trg_PI") * mConstV.at("nTSs")[2 * iSt + 1] *
                               mSignalStorage["phi0"].getToReal()) / log(2));
      string t_name;
      t_name = "phiFactor_" + to_string(iSt);
      mSignalStorage[t_name] = Belle2::TRGCDCJSignal(2 * mConstD.at("Trg_PI") / mConstV.at("nTSs")[2 * iSt + 1],
                                                     mSignalStorage["phi0"].getToReal() / pow(2, nShiftBits), commonData);
    }
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<phiFactor_"<<iSt<<">>>"<<endl; mSignalStorage["phiFactor_"+to_string(iSt)].dump();}
  // wirePhi <= tsId * phiFactor
  for (unsigned iSt = 0; iSt < 4;
       iSt++) mSignalStorage["wirePhi_" + to_string(iSt)] <= mSignalStorage["tsId_" + to_string(iSt)] * mSignalStorage["phiFactor_" +
             to_string(iSt)];
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<wirePhi_"<<iSt<<">>>"<<endl; mSignalStorage["wirePhi_"+to_string(iSt)].dump();}

  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<tdc_"<<iSt<<">>>"<<endl; mSignalStorage["tdc_"+to_string(iSt)].dump();}
  //cout<<"<<<eventTime>>>"<<endl; mSignalStorage["eventTime"].dump();

  // Calculate driftTimeRaw (= tdc - eventTime)
  for (unsigned iSt = 0; iSt < 4;
       iSt++) mSignalStorage["driftTimeRaw_" + to_string(iSt)] <= mSignalStorage["tdc_" + to_string(iSt)] - mSignalStorage["eventTime"];

  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<driftTimeRaw_"<<iSt<<">>>"<<endl; mSignalStorage["driftTimeRaw_"+to_string(iSt)].dump();}

  // Confine drift time
  // Constants for driftTime
  if (mSignalStorage.find("maxDriftTimeLow") == mSignalStorage.end()) {
    int maxDriftTime = 287;
    mSignalStorage["maxDriftTimeLow"] = Belle2::TRGCDCJSignal(-512 + maxDriftTime + 1, mSignalStorage["eventTime"].getToReal(),
                                                              commonData);
    mSignalStorage["maxDriftTimeHigh"] = Belle2::TRGCDCJSignal(maxDriftTime, mSignalStorage["eventTime"].getToReal(), commonData);
    mSignalStorage["maxDriftTimeMax"] = Belle2::TRGCDCJSignal(512, mSignalStorage["eventTime"].getToReal(), commonData);
  }
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_in1Name = "driftTimeRaw_" + to_string(iSt);
    string t_valueName = "driftTime_c_" + to_string(iSt);
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare (eventTimeValid == 0)
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["eventTimeValid"], "=", Belle2::TRGCDCJSignal(0,
                                      mSignalStorage["eventTimeValid"].getToReal(), commonData));
    // Assignments (driftTime = 0)
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage[t_valueName], Belle2::TRGCDCJSignal(0, mSignalStorage["eventTime"].getToReal(), commonData))
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (driftTime < maxDriftTimeLow)
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], "<", mSignalStorage["maxDriftTimeLow"]);
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], (mSignalStorage[t_in1Name] + mSignalStorage["maxDriftTimeMax"]).limit(mSignalStorage["maxDriftTimeLow"], mSignalStorage["maxDriftTimeHigh"]))
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (driftTime < maxDriftTimeHigh)
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], "<=", mSignalStorage["maxDriftTimeHigh"]);
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_in1Name].limit(mSignalStorage["maxDriftTimeLow"], mSignalStorage["maxDriftTimeHigh"]))
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (else)
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], (mSignalStorage[t_in1Name] - mSignalStorage["maxDriftTimeMax"]).limit(mSignalStorage["maxDriftTimeLow"], mSignalStorage["maxDriftTimeHigh"]))
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<driftTime_c_"<<iSt<<">>>"<<endl; mSignalStorage["driftTime_c_"+to_string(iSt)].dump();}

  //// Notify case when driftTimeRaw is larger than maxDriftTime or smaller than maxDriftTimeLow
  //for(unsigned iSt=0; iSt<4; iSt++) {
  //  if(mSignalStorage["driftTimeRaw_"+to_string(iSt)].getInt() > maxDriftTime) cout<<"driftTimeRaw_"<<iSt<<":"<<mSignalStorage["driftTimeRaw_"+to_string(iSt)].getInt()<<" is larger than maxDriftTime:"<<maxDriftTime<<" changing to "<<mSignalStorage["driftTimeRaw_"+to_string(iSt)].getInt()-512<<endl;
  //  if(mSignalStorage["driftTimeRaw_"+to_string(iSt)].getInt() < -512+maxDriftTime+1) cout<<"driftTimeRaw_"<<iSt<<":"<<mSignalStorage["driftTimeRaw_"+to_string(iSt)].getInt()<<" is smaller than -512+maxDriftTime+1:"<<-512+maxDriftTime+1<<" changed to "<<mSignalStorage["driftTimeRaw_"+to_string(iSt)].getInt()+512<<endl;
  //}

  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<invErrorRho_"<<iSt<<">>>"<<endl; mSignalStorage["invErrorRho_"+to_string(iSt)].dump();}
  // Remove negative driftTime
  // Constants for driftTime
  if (mSignalStorage.find("minDriftTime") == mSignalStorage.end()) {
    int minDriftTime = -8;
    mSignalStorage["minDriftTime"] = Belle2::TRGCDCJSignal(minDriftTime, mSignalStorage["eventTime"].getToReal(), commonData);
    mSignalStorage["minDriftTimeLow"] = Belle2::TRGCDCJSignal(0, mSignalStorage["eventTime"].getToReal(), commonData);
  }
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_in1Name = "driftTime_c_" + to_string(iSt);
    string t_valueName = "driftTime_" + to_string(iSt);
    string t_in2Name = "invErrorRho_" + to_string(iSt);
    string t_value2Name = "iZError2_" + to_string(iSt);
    string t_noneErrorName = "iZNoneError2_" + to_string(iSt);
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare (driftTime_c[i]>0)
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">=", mSignalStorage["minDriftTimeLow"]);
    // Assignments
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_in1Name].limit(mSignalStorage["minDriftTimeLow"], mSignalStorage["maxDriftTimeHigh"])),
      make_pair(&mSignalStorage[t_value2Name], mSignalStorage[t_in2Name])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (driftTime_c[i]>=minDriftTime)
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">=", mSignalStorage["minDriftTime"]);
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage["minDriftTimeLow"]),
      make_pair(&mSignalStorage[t_value2Name], mSignalStorage[t_in2Name])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (else)
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage["minDriftTimeLow"]),
      make_pair(&mSignalStorage[t_value2Name], mSignalStorage[t_noneErrorName])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<driftTime_"<<iSt<<">>>"<<endl; mSignalStorage["driftTime_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<iZError2_"<<iSt<<">>>"<<endl; mSignalStorage["iZError2_"+to_string(iSt)].dump();}

  //// Notify case when driftTime_c is smaller than 0 or minDriftTime
  //for(unsigned iSt=0; iSt<4; iSt++) {
  //  if(mSignalStorage["driftTime_c_"+to_string(iSt)].getInt() < minDriftTime) cout<<"driftTime_c_"<<iSt<<":"<<mSignalStorage["driftTime_c_"+to_string(iSt)].getInt()<<" is smaller than minDriftTime:"<<minDriftTime<<endl;
  //  else if(mSignalStorage["driftTime_c_"+to_string(iSt)].getInt() < 0) cout<<"driftTime_c_"<<iSt<<":"<<mSignalStorage["driftTime_c_"+to_string(iSt)].getInt()<<" is smaller than 0."<<endl;
  //}

  // Calculate minInvValue, maxInvValue for LUTs
  if (!mSignalStorage.count("invDriftPhiMin")) { //nothing found
    mSignalStorage["invDriftPhiMin"] = Belle2::TRGCDCJSignal(0, mSignalStorage["driftTime_0"].getToReal(), commonData);
    mSignalStorage["invDriftPhiMax"] = Belle2::TRGCDCJSignal(pow(2, mConstD.at("tdcBitSize")) - 1,
                                                             mSignalStorage["driftTime_0"].getToReal(), commonData);
  }
  //cout<<"<<<invDriftPhiMin>>>"<<endl; mSignalStorage["invDriftPhiMin"].dump();
  //cout<<"<<<invDriftPhiMax>>>"<<endl; mSignalStorage["invDriftPhiMax"].dump();
  // Generate LUT(driftPhi[i]=arctan(driftLengthFunction(driftTime))/r[i])
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_valueName = "driftPhi_" + to_string(iSt);
    string t_inName = "driftTime_" + to_string(iSt);
    if (!mLutStorage.count(t_valueName)) { //if nothing found
      mLutStorage[t_valueName] = new Belle2::TRGCDCJLUT(t_valueName);
      // Lambda can not copy maps.
      double t_parameter = mConstV.at("rr3D")[iSt];
      mLutStorage[t_valueName]->setFloatFunction(
        //[=](double aValue) -> double{return mConstV.at("driftLengthTableSL"+to_string(2*iSt+1))[aValue]/t_parameter;},
        [ = ](double aValue) -> double{return atan(mConstV.at("driftLengthTableSL" + to_string(2 * iSt + 1))[aValue] / t_parameter);},
        mSignalStorage[t_inName],
        mSignalStorage["invDriftPhiMin"], mSignalStorage["invDriftPhiMax"], mSignalStorage["phi0"].getToReal(),
        (int)mConstD.at("driftPhiLUTInBitSize"), (int)mConstD.at("driftPhiLUTOutBitSize"));
      //mLutStorage[t_valueName]->makeCOE("./LutData/"+t_valueName+".coe");
    }
  }
  //// driftPhi[i]=arctan(driftLengthFunction(driftTime))/r[i]
  //// Operate using LUT(driftPhi[i]).
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    //cout<<"driftTime:"<<mSignalStorage["driftTime_"+to_string(iSt)].getActual()<<endl;
    //cout<<"actual:"<<mLutStorage["driftPhi_"+to_string(iSt)]->getFloatOutput(mSignalStorage["driftTime_"+to_string(iSt)].getActual())<<endl;
    mLutStorage["driftPhi_" + to_string(iSt)]->operate(mSignalStorage["driftTime_" + to_string(iSt)],
                                                       mSignalStorage["driftPhi_" + to_string(iSt)]);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<driftPhi_"<<iSt<<">>>"<<endl; mSignalStorage["driftPhi_"+to_string(iSt)].dump();}

  // Calculate finePhi depending on lr(0:no hit, 1: left, 2: right, 3: not determined)
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<lr_"<<iSt<<">>>"<<endl; mSignalStorage["lr_"+to_string(iSt)].dump();}
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_in1Name = "lr_" + to_string(iSt);
    string t_valueName = "finePhi_" + to_string(iSt);
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare (lr == 1)
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], "=", Belle2::TRGCDCJSignal(1,
                                      mSignalStorage[t_in1Name].getToReal(), commonData));
    // Assignments
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage["wirePhi_" + to_string(iSt)] - mSignalStorage["driftPhi_" + to_string(iSt)])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (lr == 2)
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], "=", Belle2::TRGCDCJSignal(2,
                                            mSignalStorage[t_in1Name].getToReal(), commonData));
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage["wirePhi_" + to_string(iSt)] + mSignalStorage["driftPhi_" + to_string(iSt)])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (else)
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage["wirePhi_" + to_string(iSt)])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<finePhi_"<<iSt<<">>>"<<endl; mSignalStorage["finePhi_"+to_string(iSt)].dump();}

  // Constrain phi[i] to [-pi, pi]
  if (mSignalStorage.find("phiMax_0") == mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_valueName = "finePhi_" + to_string(iSt);
      string t_maxName = "phiMax_" + to_string(iSt);
      string t_minName = "phiMin_" + to_string(iSt);
      string t_2PiName = "phi2Pi_" + to_string(iSt);
      mSignalStorage[t_maxName] = Belle2::TRGCDCJSignal(mConstD.at("Trg_PI"), mSignalStorage[t_valueName].getToReal(), commonData);
      mSignalStorage[t_minName] = Belle2::TRGCDCJSignal(-mConstD.at("Trg_PI"), mSignalStorage[t_valueName].getToReal(), commonData);
      mSignalStorage[t_2PiName] = Belle2::TRGCDCJSignal(2 * mConstD.at("Trg_PI"), mSignalStorage[t_valueName].getToReal(), commonData);
    }
  }
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_in1Name = "finePhi_" + to_string(iSt);
    string t_valueName = "phi_" + to_string(iSt);
    string t_maxName = "phiMax_" + to_string(iSt);
    string t_minName = "phiMin_" + to_string(iSt);
    string t_2PiName = "phi2Pi_" + to_string(iSt);
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare (dPhi[i] > dPhiPiMax)
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", mSignalStorage[t_maxName]);
    // Assignments (dPhiPi_c[i] = dPhi[i] - dPhiPi2Pi[i])
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage[t_valueName], (mSignalStorage[t_in1Name] - mSignalStorage[t_2PiName]).limit(mSignalStorage[t_minName], mSignalStorage[t_maxName])),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (dPhi[i] > dPhiPiMin)
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", mSignalStorage[t_minName]);
    // Assignments (dPhiPi_c[i] = dPhi[i])
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_in1Name].limit(mSignalStorage[t_minName], mSignalStorage[t_maxName])),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (else)
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments (dPhiPi_c[i] = dPhi[i] + dPhiPi2Pi[i])
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], (mSignalStorage[t_in1Name] + mSignalStorage[t_2PiName]).limit(mSignalStorage[t_minName], mSignalStorage[t_maxName])),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<phi_"<<iSt<<">>>"<<endl; mSignalStorage["phi_"+to_string(iSt)].dump();}

}

void Fitter3DUtility::saveStereoXt(std::vector<std::vector<double> >& stXts, string const& filePrefix)
{
  for (unsigned iSt = 0; iSt < stXts.size(); ++iSt) {
    string filename = filePrefix + ".st" + to_string(iSt);
    ofstream xtFile;
    cout << "Fitter3DUtility::saveStereoXt() Saving xt file to " << filename << endl;
    xtFile.open(filename);
    for (unsigned iTick = 0; iTick < stXts[iSt].size(); ++iTick) {
      xtFile << stXts[iSt][iTick] << endl;
    }
    xtFile.close();
  }
}

void Fitter3DUtility::loadStereoXt(string const& filePrefix, int nFiles, std::vector<std::vector<double> >& stXts)
{
  stXts = vector<vector<double> > (nFiles);
  for (unsigned iSt = 0; iSt < stXts.size(); ++iSt) {
    string filename = filePrefix + ".st" + to_string(iSt);
    ifstream xtFile;
    xtFile.open(filename.c_str());
    if (xtFile.fail()) {
      cout << "Fitter3DUtility::loadStereoXt() Cannot load " << filename << endl;
      return;
    }
    string line;
    while (getline(xtFile, line)) {
      stXts[iSt].push_back(std::stof(line));
    }
    xtFile.close();
  }
}

double Fitter3DUtility::stereoIdToPhi(vector<double>& stNWires, int iSt, int id)
{
  return (double)id / stNWires[iSt] * 4 * M_PI;
}

// rawStTSs[iSt] = [TS ID, TS LR, TS driftTime]
// stTSs is 9999 when not valid
void Fitter3DUtility::calPhiFast(map<string, vector<double> >& stGeometry, vector<vector<double> > const& stXts, int eventTimeValid,
                                 int eventTime, vector<vector<int> > const& rawStTSs, vector<double>& stTSs)
{
  stTSs = vector<double> (4, 9999);
  if (eventTimeValid == 0) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      stTSs[iSt] = stereoIdToPhi(stGeometry["nWires"], iSt, rawStTSs[iSt][0]);
    }
  } else {
    //cout<<"eventTime: "<<eventTime;
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      if (rawStTSs[iSt][1] != 0) {
        // Get drift length from table.
        int t = rawStTSs[iSt][2] - eventTime;
        //cout<<" iSt:"<<iSt<<" rt: "<<rawStTSs[iSt][2];
        if (t < 0) t = 0;
        if (t > 511) t = 511;
        double driftLength = stXts[iSt][t];
        stTSs[iSt] = Fitter3DUtility::calPhi(stereoIdToPhi(stGeometry["nWires"], iSt, rawStTSs[iSt][0]), driftLength,
                                             stGeometry["cdcRadius"][iSt], rawStTSs[iSt][1]);
      }
    }
    //cout<<endl;
  }
}

double Fitter3DUtility::rotatePhi(double value, double refPhi)
{
  double Trg_PI = 3.141592653589793;
  double phiMin = -Trg_PI;
  double phiMax = Trg_PI;
  double result = value - refPhi;
  bool rangeOK = 0;
  while (rangeOK == 0) {
    if (result > phiMax) result -= 2 * Trg_PI;
    else if (result < phiMin) result += 2 * Trg_PI;
    else rangeOK = 1;
  }
  return result;
}

double Fitter3DUtility::rotatePhi(double value, int refId, int nTSs)
{
  double refPhi = (double)refId / nTSs * 2 * M_PI;
  return rotatePhi(value, refPhi);
}

int Fitter3DUtility::rotateTsId(int value, int refId, int nTSs)
{
  int result = value - refId;
  bool rangeOk = 0;
  while (rangeOk == 0) {
    if (result >= nTSs) result -= nTSs;
    else if (result < 0) result += nTSs;
    else rangeOk = 1;
  }
  return result;
}

int Fitter3DUtility::findQuadrant(double value)
{
  // Rotate to [-pi,pi] range.
  double Trg_PI = 3.141592653589793;
  double phiMin = -Trg_PI;
  double phiMax = Trg_PI;
  bool rangeOK = 0;
  while (rangeOK == 0) {
    if (value > phiMax) value -= 2 * Trg_PI;
    else if (value < phiMin) value += 2 * Trg_PI;
    else rangeOK = 1;
  }
  // Find quadrant.
  int result = -1;
  if (value > Trg_PI / 2) result = 2;
  else if (value > 0) result = 1;
  else if (value > -Trg_PI / 2) result = 4;
  else if (value > -Trg_PI) result = 3;
  return result;
}


// rawStTSs[iSt] = [TS ID, TS LR, TS driftTime]
void Fitter3DUtility::setErrorFast(std::vector<std::vector<int> > const& rawStTSs, int eventTimeValid,
                                   std::vector<double>& invZError2)
{
  vector<double> driftZError({0.7676, 0.9753, 1.029, 1.372});
  vector<double> wireZError({0.7676, 0.9753, 1.029, 1.372});
  vector<double> zError(4, 9999);
  invZError2 = vector<double> (4, 0);
  for (unsigned iSt = 0; iSt < 4; ++iSt) {
    if (rawStTSs[iSt][1] != 0) {
      // Check LR and eventTime
      if (rawStTSs[iSt][1] != 3 && eventTimeValid != 0) zError[iSt] = driftZError[iSt];
      else zError[iSt] = wireZError[iSt];
      // Get inverse zerror ^ 2
      invZError2[iSt] = 1 / pow(zError[iSt], 2);
    }
  }
}

void Fitter3DUtility::setError(std::map<std::string, double> const& mConstD,
                               std::map<std::string, std::vector<double> > const& mConstV, std::map<std::string, Belle2::TRGCDCJSignal>& mSignalStorage)
{
  Belle2::TRGCDCJSignalData* commonData = mSignalStorage.begin()->second.getCommonData();
  // Make constants for wireError,driftError,noneError
  if (mSignalStorage.find("iZWireError2_0") == mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_name;
      t_name = "iZWireError2_" + to_string(iSt);
      mSignalStorage[t_name] = Belle2::TRGCDCJSignal(mConstD.at("iError2BitSize"), 1 / pow(mConstV.at("wireZError")[iSt], 2), 0,
                                                     mConstD.at("iError2Max"), -1, commonData);
      t_name = "iZDriftError2_" + to_string(iSt);
      mSignalStorage[t_name] = Belle2::TRGCDCJSignal(mConstD.at("iError2BitSize"), 1 / pow(mConstV.at("driftZError")[iSt], 2), 0,
                                                     mConstD.at("iError2Max"), -1, commonData);
      t_name = "iZNoneError2_" + to_string(iSt);
      mSignalStorage[t_name] = Belle2::TRGCDCJSignal(mConstD.at("iError2BitSize"), 0, 0, mConstD.at("iError2Max"), -1, commonData);
    }
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<iZWireError2_"<<iSt<<">>>"<<endl; mSignalStorage["iZWireError2_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<iZDriftError2_"<<iSt<<">>>"<<endl; mSignalStorage["iZDriftError2_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<iZNoneError2_"<<iSt<<">>>"<<endl; mSignalStorage["iZNoneError2_"+to_string(iSt)].dump();}

  // Set error depending on lr(0:no hit, 1: left, 2: right, 3: not determined)
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<lr_"<<iSt<<">>>"<<endl; mSignalStorage["lr_"+to_string(iSt)].dump();}
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_in1Name = "lr_" + to_string(iSt);
    string t_valueName = "invErrorLR_" + to_string(iSt);
    string t_noneErrorName = "iZNoneError2_" + to_string(iSt);
    string t_driftErrorName = "iZDriftError2_" + to_string(iSt);
    string t_wireErrorName = "iZWireError2_" + to_string(iSt);
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare (lr == 0)
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], "=", Belle2::TRGCDCJSignal(0,
                                      mSignalStorage[t_in1Name].getToReal(), commonData));
    // Assignments (invErrorLR <= iZNoneError2)
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_noneErrorName])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (lr == 3)
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], "=", Belle2::TRGCDCJSignal(3,
                                            mSignalStorage[t_in1Name].getToReal(), commonData));
    // Assignments (invErrorLR <= iZWireError)
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_wireErrorName])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (else)
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments (invErrorLR <= iZDriftError)
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_driftErrorName])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<invErrorLR_"<<iSt<<">>>"<<endl; mSignalStorage["invErrorLR_"+to_string(iSt)].dump();}

  // Make constants for half radius of SL
  if (mSignalStorage.find("halfRadius_0") == mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_name;
      t_name = "halfRadius_" + to_string(iSt);
      mSignalStorage[t_name] = Belle2::TRGCDCJSignal(mConstV.at("rr")[iSt * 2 + 1] / 2, mSignalStorage["rho"].getToReal(), commonData);
    }
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<halfRadius_"<<iSt<<">>>"<<endl; mSignalStorage["halfRadius_"+to_string(iSt)].dump();}
  // Set error depending on R(helix radius)
  //cout<<"<<<rho>>>"<<endl; mSignalStorage["rho"].dump();
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_compareName = "halfRadius_" + to_string(iSt);
    //string t_valueName = "iZError2_" + to_string(iSt);
    string t_valueName = "invErrorRho_" + to_string(iSt);
    string t_noneErrorName = "iZNoneError2_" + to_string(iSt);
    string t_in1Name = "invErrorLR_" + to_string(iSt);
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare (R < r/2)
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["rho"], "<", mSignalStorage[t_compareName]);
    // Assignments (invError <= 0)
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_noneErrorName])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (else)
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments (invError <= invErrorLR)
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_in1Name])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<invError_"<<iSt<<">>>"<<endl; mSignalStorage["invError_"+to_string(iSt)].dump();}
}

double Fitter3DUtility::calStAxPhi(int mysign, double anglest, double ztostraw, double rr, double rho, double myphi0)
{
  if (false) cout << anglest << ztostraw << endl; // Removes warnings when compileing

  double myphiz, acos_real;
  double Trg_PI = 3.141592653589793;
  //Find phifit-phist
  double t_rho = rho;
  if (rr > (2 * rho)) t_rho = rr / 2;
  acos_real = acos(rr / (2 * t_rho));
  if (mysign == 1) {
    myphiz = +acos_real + myphi0;
  } else {
    myphiz = -acos_real + myphi0;
  }
  if (myphiz > Trg_PI) myphiz -= 2 * Trg_PI;
  if (myphiz < -Trg_PI) myphiz += 2 * Trg_PI;

  return myphiz;
}

double Fitter3DUtility::calDeltaPhi(int mysign, double anglest, double ztostraw, double rr, double phi2, double rho, double myphi0)
{
  if (false) cout << anglest << ztostraw << endl; // Removes warnings when compileing

  double myphiz, acos_real;
  double Trg_PI = 3.141592653589793;
  //Find phifit-phist
  double t_rho = rho;
  if (rr > (2 * rho)) t_rho = rr / 2;
  acos_real = acos(rr / (2 * t_rho));
  if (mysign == 1) {
    myphiz = -acos_real - myphi0 + phi2;
  } else {
    myphiz = +acos_real - myphi0 + phi2;
  }
  if (myphiz > Trg_PI) myphiz -= 2 * Trg_PI;
  if (myphiz < -Trg_PI) myphiz += 2 * Trg_PI;

  return myphiz;
}

void Fitter3DUtility::constrainRPerStSl(std::map<std::string, std::vector<double> > const& mConstV,
                                        std::map<std::string, Belle2::TRGCDCJSignal>& mSignalStorage)
{
  Belle2::TRGCDCJSignalData* commonData = mSignalStorage.begin()->second.getCommonData();
  // Make constants for min and max values for each super layer.
  if (mSignalStorage.find("rhoMin_0") == mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      // Min value
      string t_minName = "rhoMin_" + to_string(iSt);
      double t_actual = (mConstV.find("rr")->second)[2 * iSt + 1] / 2;
      double t_toReal = mSignalStorage["rho"].getToReal();
      mSignalStorage[t_minName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
      // Add one to prevent arccos, arcsin becoming infinity.
      if (mSignalStorage[t_minName].getRealInt() < t_actual) {
        t_actual += 1 * t_toReal;
        mSignalStorage[t_minName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
        //cout<<"[Warning] "<<t_minName<<" is too small. Adding one unit to prevent arccos output being nan."<<endl;
        //cout<<t_minName<<".getRealInt():"<<mSignalStorage[t_minName].getRealInt()<<" is new min."<<endl;
      }
      // Max value
      string t_maxName = "rhoMax_" + to_string(iSt);
      t_actual = mSignalStorage["rho"].getMaxActual();
      mSignalStorage[t_maxName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
    }
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<rhoMin_"<<iSt<<">>>"<<endl; mSignalStorage["rhoMin_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<rhoMax_"<<iSt<<">>>"<<endl; mSignalStorage["rhoMax_"+to_string(iSt)].dump();}

  //cout<<"<<<rho>>>"<<endl; mSignalStorage["rho"].dump();
  // Constrain rho to min and max per layer.
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_in1Name = "rho";
    string t_valueName = "rho_c_" + to_string(iSt);
    string t_maxName = "rhoMax_" + to_string(iSt);
    string t_minName = "rhoMin_" + to_string(iSt);
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", mSignalStorage[t_maxName]);
    // Assignments
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_maxName]),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", mSignalStorage[t_minName]);
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_in1Name].limit(mSignalStorage[t_minName], mSignalStorage[t_maxName]))
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_minName])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<rho_c_"<<iSt<<">>>"<<endl; mSignalStorage["rho_c_"+to_string(iSt)].dump();}

}

double Fitter3DUtility::calZ(int mysign, double anglest, double ztostraw, double rr, double phi2, double rho, double myphi0)
{
  double myphiz = 0, acos_real = 0, dPhiAx = 0;
  double Trg_PI = 3.141592653589793;
  //Find phifit-phist
  double t_rho = rho;
  if (rr > (2 * rho)) t_rho = rr / 2;
  acos_real = acos(rr / (2 * t_rho));
  if (mysign == 1) {
    dPhiAx = -acos_real - myphi0;
  } else {
    dPhiAx = acos_real - myphi0;
  }
  myphiz = dPhiAx + phi2;
  if (myphiz > Trg_PI) myphiz -= 2 * Trg_PI;
  if (myphiz < -Trg_PI) myphiz += 2 * Trg_PI;

  return (ztostraw - rr * 2 * sin(myphiz / 2) / anglest);
}

void Fitter3DUtility::calZ(std::map<std::string, double> const& mConstD, std::map<std::string, std::vector<double> > const& mConstV,
                           std::map<std::string, Belle2::TRGCDCJSignal>& mSignalStorage, std::map<std::string, Belle2::TRGCDCJLUT* >& mLutStorage)
{
  Belle2::TRGCDCJSignalData* commonData = mSignalStorage.begin()->second.getCommonData();
  // Calculate zz[i]
  if (mSignalStorage.find("invPhiAxMin_0") == mSignalStorage.end()) {
    //for(unsigned iSt=0; iSt<4; iSt++) {
    //  string t_invMinName = "invPhiAxMin_" + to_string(iSt);
    //  double t_actual = (mConstV.find("rr")->second)[2*iSt+1]/2;
    //  double t_toReal = mSignalStorage["rho"].getToReal();
    //  mSignalStorage[t_invMinName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
    //  // Add one to prevent arccos becoming infinity.
    //  if(mSignalStorage[t_invMinName].getRealInt() < t_actual) {
    //    t_actual += 1*t_toReal;
    //    mSignalStorage[t_invMinName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
    //    //cout<<"[Warning] "<<t_invMinName<<" is too small. Adding one unit to prevent arccos output being nan."<<endl;
    //    //cout<<t_invMinName<<".getRealInt():"<<mSignalStorage[t_invMinName].getRealInt()<<" is new min."<<endl;
    //  }
    //  string t_invMaxName = "invPhiAxMax_" + to_string(iSt);
    //  t_actual = mSignalStorage["rho"].getMaxActual();
    //  mSignalStorage[t_invMaxName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
    //}
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_invMinName = "invPhiAxMin_" + to_string(iSt);
      //double t_actual = (mConstV.find("rr")->second)[2*iSt+1]/2;
      double t_actual = mSignalStorage["rho_c_" + to_string(iSt)].getMinActual();
      double t_toReal = mSignalStorage["rho_c_" + to_string(iSt)].getToReal();
      mSignalStorage[t_invMinName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
      string t_invMaxName = "invPhiAxMax_" + to_string(iSt);
      t_actual = mSignalStorage["rho_c_" + to_string(iSt)].getMaxActual();
      mSignalStorage[t_invMaxName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
    }
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<rho_c_"<<iSt<<">>>"<<endl; mSignalStorage["rho_c_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<invPhiAxMin_"<<iSt<<">>>"<<endl; mSignalStorage["invPhiAxMin_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<invPhiAxMax_"<<iSt<<">>>"<<endl; mSignalStorage["invPhiAxMax_"+to_string(iSt)].dump();}
  // Generate LUT(phiAx[i]=acos(rr[i]/2/rho)).
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_valueName = "phiAx_" + to_string(iSt);
    string t_minName = "phiAxMin_" + to_string(iSt);
    string t_maxName = "phiAxMax_" + to_string(iSt);
    string t_invMinName = "invPhiAxMin_" + to_string(iSt);
    string t_invMaxName = "invPhiAxMax_" + to_string(iSt);
    if (!mLutStorage.count(t_valueName)) { //if nothing found
      mLutStorage[t_valueName] = new Belle2::TRGCDCJLUT(t_valueName);
      // Lambda can not copy maps.
      double t_parameter = mConstV.at("rr3D")[iSt];
      mLutStorage[t_valueName]->setFloatFunction(
        [ = ](double aValue) -> double{return acos(t_parameter / 2 / aValue);},
        //mSignalStorage["rho"],
        mSignalStorage["rho_c_" + to_string(iSt)],
        mSignalStorage[t_invMinName], mSignalStorage[t_invMaxName], mSignalStorage["phi0"].getToReal(),
        (int)mConstD.at("acosLUTInBitSize"), (int)mConstD.at("acosLUTOutBitSize"));
      //mLutStorage[t_valueName]->makeCOE("./LutData/"+t_valueName+".coe");
    }
  }
  // phiAx[i] = acos(rr[i]/2/rho).
  // Operate using LUT(phiAx[i]).
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    // Set output name
    string t_valueName =  "phiAx_" + to_string(iSt);
    //mLutStorage[t_valueName]->operate(mSignalStorage["rho"], mSignalStorage[t_valueName]);
    mLutStorage[t_valueName]->operate(mSignalStorage["rho_c_" + to_string(iSt)], mSignalStorage[t_valueName]);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<phiAx_"<<iSt<<">>>"<<endl; mSignalStorage["phiAx_"+to_string(iSt)].dump();}
  //cout<<"<<<phi0>>>"<<endl; mSignalStorage["phi0"].dump();
  //cout<<"<<<charge>>>"<<endl; mSignalStorage["charge"].dump();
  // dPhiAx[i] = -+phiAx[i] - phi0
  {
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(Belle2::TRGCDCJSignal(1, mSignalStorage["charge"].getToReal(),
                                      commonData), "=", mSignalStorage["charge"]);
    // Assignments
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage["dPhiAx_0"], -mSignalStorage["phiAx_0"] - mSignalStorage["phi0"]),
      make_pair(&mSignalStorage["dPhiAx_1"], -mSignalStorage["phiAx_1"] - mSignalStorage["phi0"]),
      make_pair(&mSignalStorage["dPhiAx_2"], -mSignalStorage["phiAx_2"] - mSignalStorage["phi0"]),
      make_pair(&mSignalStorage["dPhiAx_3"], -mSignalStorage["phiAx_3"] - mSignalStorage["phi0"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage["dPhiAx_0"], mSignalStorage["phiAx_0"] - mSignalStorage["phi0"]),
      make_pair(&mSignalStorage["dPhiAx_1"], mSignalStorage["phiAx_1"] - mSignalStorage["phi0"]),
      make_pair(&mSignalStorage["dPhiAx_2"], mSignalStorage["phiAx_2"] - mSignalStorage["phi0"]),
      make_pair(&mSignalStorage["dPhiAx_3"], mSignalStorage["phiAx_3"] - mSignalStorage["phi0"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhiAx_"<<iSt<<">>>"<<endl; mSignalStorage["dPhiAx_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<phi_"<<iSt<<">>>"<<endl; mSignalStorage["phi_"+to_string(iSt)].dump();}
  // dPhi[i] = dPhiAx[i] + phi[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["dPhi_" + to_string(iSt)] <= mSignalStorage["dPhiAx_" + to_string(iSt)] + mSignalStorage["phi_" + to_string(iSt)];
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhi_"<<iSt<<">>>"<<endl; mSignalStorage["dPhi_"+to_string(iSt)].dump();}

  // Rotate dPhi to [-pi, pi]
  if (mSignalStorage.find("dPhiPiMax_0") == mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_valueName = "dPhi_" + to_string(iSt);
      string t_maxName = "dPhiPiMax_" + to_string(iSt);
      string t_minName = "dPhiPiMin_" + to_string(iSt);
      string t_2PiName = "dPhiPi2Pi_" + to_string(iSt);
      mSignalStorage[t_maxName] = Belle2::TRGCDCJSignal(mConstD.at("Trg_PI"), mSignalStorage[t_valueName].getToReal(), commonData);
      mSignalStorage[t_minName] = Belle2::TRGCDCJSignal(-mConstD.at("Trg_PI"), mSignalStorage[t_valueName].getToReal(), commonData);
      mSignalStorage[t_2PiName] = Belle2::TRGCDCJSignal(2 * mConstD.at("Trg_PI"), mSignalStorage[t_valueName].getToReal(), commonData);
    }
  }
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_in1Name = "dPhi_" + to_string(iSt);
    string t_valueName = "dPhiPi_c_" + to_string(iSt);
    string t_maxName = "dPhiPiMax_" + to_string(iSt);
    string t_minName = "dPhiPiMin_" + to_string(iSt);
    string t_2PiName = "dPhiPi2Pi_" + to_string(iSt);
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare (dPhi[i] > dPhiPiMax)
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", mSignalStorage[t_maxName]);
    // Assignments (dPhiPi_c[i] = dPhi[i] - dPhiPi2Pi[i])
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage[t_valueName], (mSignalStorage[t_in1Name] - mSignalStorage[t_2PiName]).limit(mSignalStorage[t_minName], mSignalStorage[t_maxName])),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (dPhi[i] > dPhiPiMin)
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", mSignalStorage[t_minName]);
    // Assignments (dPhiPi_c[i] = dPhi[i])
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_in1Name].limit(mSignalStorage[t_minName], mSignalStorage[t_maxName])),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (else)
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments (dPhiPi_c[i] = dPhi[i] + dPhiPi2Pi[i])
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], (mSignalStorage[t_in1Name] + mSignalStorage[t_2PiName]).limit(mSignalStorage[t_minName], mSignalStorage[t_maxName])),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhiPi_c_"<<iSt<<">>>"<<endl; mSignalStorage["dPhiPi_c_"+to_string(iSt)].dump();}

  // Calculate dPhiMax[i], dPhiMin[i]=0
  if (mSignalStorage.find("dPhiMax_0") == mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_maxName = "dPhiMax_" + to_string(iSt);
      string t_minName = "dPhiMin_" + to_string(iSt);
      string t_valueName = "dPhi_" + to_string(iSt);
      double t_value = 2 * mConstD.at("Trg_PI") * fabs(mConstV.at("nShift")[iSt]) / (mConstV.at("nWires")[2 * iSt + 1]);
      mSignalStorage[t_maxName] = Belle2::TRGCDCJSignal(t_value, mSignalStorage[t_valueName].getToReal(), commonData);
      mSignalStorage[t_minName] = Belle2::TRGCDCJSignal(0, mSignalStorage[t_valueName].getToReal(), commonData);
    }
  }
  // Constrain dPhiPi to [0, 2*pi*#shift/#holes]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_maxName = "dPhiMax_" + to_string(iSt);
    string t_minName = "dPhiMin_" + to_string(iSt);
    string t_valueName = "dPhi_c_" + to_string(iSt);
    string t_in1Name = "dPhiPi_c_" + to_string(iSt);
    // For SL1, SL5
    if (iSt % 2 == 0) {
      // Create data for ifElse
      vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
      // Compare
      Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", Belle2::TRGCDCJSignal(0,
                                        mSignalStorage[t_in1Name].getToReal(), commonData));
      // Assignments
      vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
        make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_minName]),
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      // Compare
      t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", -mSignalStorage[t_maxName]);
      // Assignments
      t_assigns = {
        make_pair(&mSignalStorage[t_valueName], Belle2::TRGCDCJSignal::absolute(mSignalStorage[t_in1Name]).limit(mSignalStorage[t_minName], mSignalStorage[t_maxName])),
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      // Compare
      t_compare = Belle2::TRGCDCJSignal();
      // Assignments
      t_assigns = {
        make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_maxName]),
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      // Process ifElse data.
      Belle2::TRGCDCJSignal::ifElse(t_data);
      // For SL3, SL7
    } else {
      // Create data for ifElse
      vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
      // Compare (dPhi > dPhiMax)
      Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", mSignalStorage[t_maxName]);
      // Assignments (dPhi_c = dPhiMax)
      vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
        make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_maxName]),
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      // Compare (dPhi > 0)
      t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", Belle2::TRGCDCJSignal(0,
                                              mSignalStorage[t_in1Name].getToReal(), commonData));
      // Assignments (dPhi_c = dPhi)
      t_assigns = {
        make_pair(&mSignalStorage[t_valueName], Belle2::TRGCDCJSignal::absolute(mSignalStorage[t_in1Name]).limit(mSignalStorage[t_minName], mSignalStorage[t_maxName])),
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      // Compare (else) => (dPhi > -Pi)
      t_compare = Belle2::TRGCDCJSignal();
      // Assignments (dPhi_c = dPhiMin)
      t_assigns = {
        make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_minName]),
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      // Process ifElse.
      Belle2::TRGCDCJSignal::ifElse(t_data);
    }
  }

  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhiMax_"<<iSt<<">>>"<<endl; mSignalStorage["dPhiMax_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhiMin_"<<iSt<<">>>"<<endl; mSignalStorage["dPhiMin_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhi_c_"<<iSt<<">>>"<<endl; mSignalStorage["dPhi_c_"+to_string(iSt)].dump();}
  // Calculate minInvValue, maxInvValue for LUTs
  if (mSignalStorage.find("invZMin_0") == mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_minName = "invZMin_" + to_string(iSt);
      string t_maxName = "invZMax_" + to_string(iSt);
      string t_valueName = "dPhi_c_" + to_string(iSt);
      unsigned long long t_int = mSignalStorage[t_valueName].getMinInt();
      double t_toReal = mSignalStorage[t_valueName].getToReal();
      double t_actual = mSignalStorage[t_valueName].getMinActual();
      mSignalStorage[t_minName] = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1, commonData);
      t_int = mSignalStorage[t_valueName].getMaxInt();
      t_actual = mSignalStorage[t_valueName].getMaxActual();
      mSignalStorage[t_maxName] = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1, commonData);
    }
  }
  // Generate LUT(zz[i] = ztostraw[0]-+rr[0]*2*sin(dPhi_c[i]/2)/angleSt[i], -+ depends on SL)
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_inputName = "dPhi_c_" + to_string(iSt);
    string t_outputName = "zz_" + to_string(iSt);
    string t_invMinName = "invZMin_" + to_string(iSt);
    string t_invMaxName = "invZMax_" + to_string(iSt);

    if (!mLutStorage.count(t_outputName)) { //if nothing found
      mLutStorage[t_outputName] = new Belle2::TRGCDCJLUT(t_outputName);
      // Lambda can not copy maps.
      double t_zToStraw = mConstV.at("zToStraw")[iSt];
      double t_rr3D = mConstV.at("rr3D")[iSt];
      double t_angleSt = mConstV.at("angleSt")[iSt];

      if (iSt % 2 == 0) {
        mLutStorage[t_outputName]->setFloatFunction(
          [ = ](double aValue) -> double{return t_zToStraw + t_rr3D * 2 * sin(aValue / 2) / t_angleSt;},
          mSignalStorage[t_inputName],
          mSignalStorage[t_invMinName], mSignalStorage[t_invMaxName], mSignalStorage["rho"].getToReal(),
          (int) mConstD.at("zLUTInBitSize"), (int) mConstD.at("zLUTOutBitSize"));
      } else {
        mLutStorage[t_outputName]->setFloatFunction(
          [ = ](double aValue) -> double{return t_zToStraw - t_rr3D * 2 * sin(aValue / 2) / t_angleSt;},
          mSignalStorage[t_inputName],
          mSignalStorage[t_invMinName], mSignalStorage[t_invMaxName], mSignalStorage["rho"].getToReal(),
          (int) mConstD.at("zLUTInBitSize"), (int) mConstD.at("zLUTOutBitSize"));
      }
      //mLutStorage[t_outputName]->makeCOE("./LutData/"+t_outputName+".coe");
    }
  }
  // zz[i] = ztostraw[0]-+rr[0]*2*sin(dPhi_c[i]/2)/angleSt[i], -+ depends on SL)
  // Operate using LUT(zz[i]).
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_outputName = "zz_" + to_string(iSt);
    string t_inputName = "dPhi_c_" + to_string(iSt);
    mLutStorage[t_outputName]->operate(mSignalStorage[t_inputName], mSignalStorage[t_outputName]);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<zz_"<<iSt<<">>>"<<endl; mSignalStorage["zz_"+to_string(iSt)].dump();}
}
double Fitter3DUtility::calS(double rho, double rr)
{
  double result;
  result = rho * 2 * asin(rr / 2 / rho);
  return result;
}

void Fitter3DUtility::calS(std::map<std::string, double> const& mConstD, std::map<std::string, std::vector<double> > const& mConstV,
                           std::map<std::string, Belle2::TRGCDCJSignal>& mSignalStorage, std::map<std::string, Belle2::TRGCDCJLUT* >& mLutStorage)
{
  Belle2::TRGCDCJSignalData* commonData = mSignalStorage.begin()->second.getCommonData();
  // Calculate minInvValue, maxInvValue for LUTs
  if (mSignalStorage.find("invArcSMin_0") == mSignalStorage.end()) {
    //for(unsigned iSt=0; iSt<4; iSt++) {
    //  string t_invMinName = "invArcSMin_" + to_string(iSt);
    //  double t_actual = mSignalStorage["rho"].getMaxActual();
    //  double t_toReal = mSignalStorage["rho"].getToReal();
    //  mSignalStorage[t_invMinName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
    //  string t_invMaxName = "invArcSMax_" + to_string(iSt);
    //  t_actual = (mConstV.find("rr")->second)[2*iSt+1]/2;
    //  mSignalStorage[t_invMaxName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
    //  // Subtract one to prevent arcsin becoming infinity.
    //  if(mSignalStorage[t_invMaxName].getRealInt() < t_actual) {
    //    t_actual += 1*t_toReal;
    //    mSignalStorage[t_invMaxName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
    //    //cout<<"[Warning] "<<t_invMaxName<<" is too small. Adding one unit to prevent arcsin output being nan."<<endl;
    //    //cout<<t_invMaxName<<".getRealInt():"<<mSignalStorage[t_invMaxName].getRealInt()<<" is new max."<<endl;
    //  }
    //}
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_invMinName = "invArcSMin_" + to_string(iSt);
      //double t_actual = (mConstV.find("rr")->second)[2*iSt+1]/2;
      double t_actual = mSignalStorage["rho_c_" + to_string(iSt)].getMaxActual();
      double t_toReal = mSignalStorage["rho_c_" + to_string(iSt)].getToReal();
      mSignalStorage[t_invMinName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
      string t_invMaxName = "invArcSMax_" + to_string(iSt);
      t_actual = mSignalStorage["rho_c_" + to_string(iSt)].getMinActual();
      mSignalStorage[t_invMaxName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, commonData);
    }
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<invArcSMin_"<<iSt<<">>>"<<endl; mSignalStorage["invArcSMin_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<invArcSMax_"<<iSt<<">>>"<<endl; mSignalStorage["invArcSMax_"+to_string(iSt)].dump();}

  // Generate LUT(arcS[i]=2*rho*asin(rr[i]/2/rho).
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_valueName = "arcS_" + to_string(iSt);
    string t_invMinName = "invArcSMin_" + to_string(iSt);
    string t_invMaxName = "invArcSMax_" + to_string(iSt);
    if (!mLutStorage.count(t_valueName)) { //if nothing found
      mLutStorage[t_valueName] = new Belle2::TRGCDCJLUT(t_valueName);
      // Lambda can not copy maps.
      double t_parameter = mConstV.at("rr3D")[iSt];
      mLutStorage[t_valueName]->setFloatFunction(
        [ = ](double aValue) -> double{return 2 * aValue * asin(t_parameter / 2 / aValue);},
        //mSignalStorage["rho"],
        mSignalStorage["rho_c_" + to_string(iSt)],
        mSignalStorage[t_invMinName], mSignalStorage[t_invMaxName], mSignalStorage["rho"].getToReal(),
        (int)mConstD.at("acosLUTInBitSize"), (int)mConstD.at("acosLUTOutBitSize"));
      //mLutStorage[t_valueName]->makeCOE("./LutData/"+t_valueName+".coe");
    }
  }
  // arcS[i]=2*rho*asin(rr[i]/2/rho).
  // Operate using LUT(arcS[i]).
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    // Set output name
    string t_valueName =  "arcS_" + to_string(iSt);
    //mLutStorage[t_valueName]->operate(mSignalStorage["rho"], mSignalStorage[t_valueName]);
    mLutStorage[t_valueName]->operate(mSignalStorage["rho_c_" + to_string(iSt)], mSignalStorage[t_valueName]);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<arcS_"<<iSt<<">>>"<<endl; mSignalStorage["arcS_"+to_string(iSt)].dump();}
}

double Fitter3DUtility::calDen(double* arcS, double* zError)
{
  double t_sum1 = 0;
  double t_sumSS = 0;
  double t_sumS = 0;
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    t_sum1 += pow(1 / zError[iSt], 2);
    t_sumSS += pow(arcS[iSt] / zError[iSt], 2);
    t_sumS += arcS[iSt] / pow(zError[iSt], 2);
  }
  return t_sum1 * t_sumSS - pow(t_sumS, 2);
}

double Fitter3DUtility::calDenWithIZError(double* arcS, double* iZError)
{
  double t_sum1 = 0;
  double t_sumSS = 0;
  double t_sumS = 0;
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    t_sum1 += pow(iZError[iSt], 2);
    t_sumSS += pow(arcS[iSt] * iZError[iSt], 2);
    t_sumS += arcS[iSt] * pow(iZError[iSt], 2);
  }
  return t_sum1 * t_sumSS - pow(t_sumS, 2);
}


void Fitter3DUtility::rSFit(double* iezz2, double* arcS, double* zz, double& z0, double& cot, double& zchi2)
{

  double ss = 0, sx = 0, sxx = 0, cotnum = 0, z0num = 0;
  double z0nump1[4], z0nump2[4];
  double z0den, iz0den;

  for (unsigned i = 0; i < 4; i++) {
    ss += iezz2[i];
    sx += arcS[i] * iezz2[i];
    sxx += arcS[i] * arcS[i] * iezz2[i];
  }

  for (unsigned i = 0; i < 4; i++) {
    cotnum += (ss * arcS[i] - sx) * iezz2[i] * zz[i];
    z0nump1[i] = sxx - sx * arcS[i];
    z0nump2[i] = z0nump1[i] * iezz2[i] * zz[i];
    z0num += z0nump2[i];
  }
  z0den = (ss * sxx) - (sx * sx);
  iz0den = 1. / z0den;
  z0num *= iz0den;
  cotnum *= iz0den;
  z0 = z0num;
  cot = cotnum;

  // Count number of total TS hits.
  int nTSHits = 0;
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    if (iezz2[iSt] != 0) nTSHits++;
  }
  // Calculate chi2 of z0
  zchi2 = 0.;
  for (unsigned i = 0; i < 4; i++) {
    zchi2 += (zz[i] - z0 - cot * arcS[i]) * (zz[i] - z0 - cot * arcS[i]) * iezz2[i];
  }
  zchi2 /= (nTSHits - 2);

}

void Fitter3DUtility::rSFit2(double* iezz21, double* iezz22, double* arcS, double* zz, int* lutv, double& z0, double& cot,
                             double& zchi2)
{
//      cout << "rs2" << endl;

  double ss = 0, sx = 0, sxx = 0, cotnum = 0, z0num = 0;
  double z0nump1[4], z0nump2[4];
  double z0den, iz0den;

  for (unsigned i = 0; i < 4; i++) {
    if (lutv[i] == 2) {
      ss += iezz21[i];
      sx += arcS[i] * iezz21[i];
      sxx += arcS[i] * arcS[i] * iezz21[i];
    } else {
      ss += iezz22[i];
      sx += arcS[i] * iezz22[i];
      sxx += arcS[i] * arcS[i] * iezz22[i];
    }
  }

  for (unsigned i = 0; i < 4; i++) {
    if (lutv[i] == 2) {
      cotnum += (ss * arcS[i] - sx) * iezz21[i] * zz[i];
      z0nump1[i] = sxx - sx * arcS[i];
      z0nump2[i] = z0nump1[i] * iezz21[i] * zz[i];
      z0num += z0nump2[i];
    } else {
      cotnum += (ss * arcS[i] - sx) * iezz22[i] * zz[i];
      z0nump1[i] = sxx - sx * arcS[i];
      z0nump2[i] = z0nump1[i] * iezz22[i] * zz[i];
      z0num += z0nump2[i];
    }
  }
  z0den = (ss * sxx) - (sx * sx);
  iz0den = 1. / z0den;
  z0num *= iz0den;
  cotnum *= iz0den;
  z0 = z0num;
  cot = cotnum;

  // Calculate chi2 of z0
//  double zchi2 = 0.;
  for (unsigned i = 0; i < 4; i++) {
    if (lutv[i] == 2) {
      zchi2 += (zz[i] - z0 - cot * arcS[i]) * (zz[i] - z0 - cot * arcS[i]) * iezz21[i];
    } else {
      zchi2 += (zz[i] - z0 - cot * arcS[i]) * (zz[i] - z0 - cot * arcS[i]) * iezz22[i];
    }
  }
  zchi2 /= (4 - 2);
}

void Fitter3DUtility::rSFit(std::map<std::string, double> const& mConstD,
                            std::map<std::string, std::vector<double> > const& mConstV, std::map<std::string, Belle2::TRGCDCJSignal>& mSignalStorage,
                            std::map<std::string, Belle2::TRGCDCJLUT* >& mLutStorage)
{
  Belle2::TRGCDCJSignalData* commonData = mSignalStorage.begin()->second.getCommonData();
  // sum1_p1_0 = iZError2[0] + iZError2[1]
  mSignalStorage["sum1_p1_0"] <= mSignalStorage["iZError2_0"] + mSignalStorage["iZError2_1"];
  // sum1_p1_1 = iZError2[2] + iZError2[3]
  mSignalStorage["sum1_p1_1"] <= mSignalStorage["iZError2_2"] + mSignalStorage["iZError2_3"];
  //for(int iSt=0; iSt<2; iSt++) {cout<<"<<<sum1_p1_"<<iSt<<">>>"<<endl; mSignalStorage["sum1_p1_"+to_string(iSt)].dump();}
  // sum1 = sum1_p1[0] + sum1_p1[1]
  mSignalStorage["sum1"] <= mSignalStorage["sum1_p1_0"] + mSignalStorage["sum1_p1_1"];
  //cout<<"<<<sum1>>>"<<endl; mSignalStorage["sum1"].dump();
  // sumS_p1[i] = arcS[i] * iZError2[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["sumS_p1_" + to_string(iSt)] <= mSignalStorage["arcS_" + to_string(iSt)] * mSignalStorage["iZError2_" + to_string(
          iSt)];
  }
  // sumSS_p1[i] = arcS[i] * arcS[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["sumSS_p1_" + to_string(iSt)] <= mSignalStorage["arcS_" + to_string(iSt)] * mSignalStorage["arcS_" + to_string(iSt)];
  }
  // cotNumS_p1[i] = sum1 * arcS[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["cotNumS_p1_" + to_string(iSt)] <= mSignalStorage["sum1"] * mSignalStorage["arcS_" + to_string(iSt)];
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<sumS_p1_"<<iSt<<">>>"<<endl; mSignalStorage["sumS_p1_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<sumSS_p1_"<<iSt<<">>>"<<endl; mSignalStorage["sumSS_p1_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<cotNumS_p1_"<<iSt<<">>>"<<endl; mSignalStorage["cotNumS_p1_"+to_string(iSt)].dump();}
  // sumS_p2[0] = sumS_p1[0] + sumS_p1[1]
  mSignalStorage["sumS_p2_0"] <= mSignalStorage["sumS_p1_0"] + mSignalStorage["sumS_p1_1"];
  // sumS_p2[1] = sumS_p1[2] + sumS_p1[3]
  mSignalStorage["sumS_p2_1"] <= mSignalStorage["sumS_p1_2"] + mSignalStorage["sumS_p1_3"];
  // sumSS_p2[i] = sumSS_p1[i] * iZError2[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["sumSS_p2_" + to_string(iSt)] <= mSignalStorage["sumSS_p1_" + to_string(iSt)] * mSignalStorage["iZError2_" +
        to_string(iSt)];
  }
  //for(int iSt=0; iSt<2; iSt++) {cout<<"<<<sumS_p2_"<<iSt<<">>>"<<endl; mSignalStorage["sumS_p2_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<sumSS_p2_"<<iSt<<">>>"<<endl; mSignalStorage["sumSS_p2_"+to_string(iSt)].dump();}
  // sumS = sumS_p2[0] + sumS_p2[1]
  mSignalStorage["sumS"] <= mSignalStorage["sumS_p2_0"] + mSignalStorage["sumS_p2_1"];
  // sumSS_p3[0] = sumSS_p2[0] + sumSS_p2[1]
  mSignalStorage["sumSS_p3_0"] <= mSignalStorage["sumSS_p2_0"] + mSignalStorage["sumSS_p2_1"];
  // sumSS_p3[1] = sumSS_p2[2] + sumSS_p2[3]
  mSignalStorage["sumSS_p3_1"] <= mSignalStorage["sumSS_p2_2"] + mSignalStorage["sumSS_p2_3"];
  //cout<<"<<<sumS>>>"<<endl; mSignalStorage["sumS"].dump();
  //for(int iSt=0; iSt<2; iSt++) {cout<<"<<<sumSS_p3_"<<iSt<<">>>"<<endl; mSignalStorage["sumSS_p3_"+to_string(iSt)].dump();}
  //cout<<"===================PIPELINE7============================"<<endl;
  // z0NumS_p1[i] = arcS[i] * sumS
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["z0NumS_p1_" + to_string(iSt)] <= mSignalStorage["arcS_" + to_string(iSt)] * mSignalStorage["sumS"];
  }
  // den_p1 = sumS * sumS
  mSignalStorage["den_p1"] <= mSignalStorage["sumS"] * mSignalStorage["sumS"];
  // cotNumS[i] = cotNumS_p1[i] - sumS
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["cotNumS_" + to_string(iSt)] <= mSignalStorage["cotNumS_p1_" + to_string(iSt)] - mSignalStorage["sumS"];
  }
  // sumSS = sumSS_p3[0] + sumSS_p3[1]
  mSignalStorage["sumSS"] <= mSignalStorage["sumSS_p3_0"] + mSignalStorage["sumSS_p3_1"];
  // zxiZError[i] = zz[i] * iZError2[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["zxiZError_" + to_string(iSt)] <= mSignalStorage["zz_" + to_string(iSt)] * mSignalStorage["iZError2_" + to_string(
          iSt)];
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<z0NumS_p1_"<<iSt<<">>>"<<endl; mSignalStorage["z0NumS_p1_"+to_string(iSt)].dump();}
  //cout<<"<<<den_p1>>>"<<endl; mSignalStorage["den_p1"].dump();
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<cotNumS_"<<iSt<<">>>"<<endl; mSignalStorage["cotNumS_"+to_string(iSt)].dump();}
  //cout<<"<<<sumSS>>>"<<endl; mSignalStorage["sumSS"].dump();
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<zxiZError_"<<iSt<<">>>"<<endl; mSignalStorage["zxiZError_"+to_string(iSt)].dump();}
  // den_p2 = sum1 * sumSS
  mSignalStorage["den_p2"] <= mSignalStorage["sum1"] * mSignalStorage["sumSS"];
  // z0NumS[i] =sumSS - z0NumS_p1[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["z0NumS_" + to_string(iSt)] <= mSignalStorage["sumSS"] - mSignalStorage["z0NumS_p1_" + to_string(iSt)];
  }
  //cout<<"<<<den_p2>>>"<<endl; mSignalStorage["den_p2"].dump();
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<z0NumS_"<<iSt<<">>>"<<endl; mSignalStorage["z0NumS_"+to_string(iSt)].dump();}
  // cotNumSZ[i] = cotNumS[i] * zxiZError[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["cotNumSZ_" + to_string(iSt)] <= mSignalStorage["cotNumS_" + to_string(iSt)] * mSignalStorage["zxiZError_" +
        to_string(iSt)];
  }
  // den = den_p2 - den_p1
  mSignalStorage["den"] <= mSignalStorage["den_p2"] - mSignalStorage["den_p1"];
  // Calculate denMax and denMin.
  if (mSignalStorage.find("denMin") == mSignalStorage.end()) {
    // Create input for calDen.
    double t_rr3D[4], t_wireZError[4];
    for (int iSt = 0; iSt < 2; iSt++) {
      t_rr3D[iSt] = mConstV.at("rr3D")[iSt];
      t_wireZError[iSt] = 1 / mConstV.at("wireZError")[iSt];
    }
    for (int iSt = 2; iSt < 4; iSt++) {
      t_rr3D[iSt] = mConstV.at("rr3D")[iSt];
      t_wireZError[iSt] = 0;
    }
    double t_denMin = Fitter3DUtility::calDenWithIZError(t_rr3D, t_wireZError);
    mSignalStorage["denMin"] = Belle2::TRGCDCJSignal(t_denMin, mSignalStorage["den"].getToReal(), commonData);
    double t_arcSMax[4], t_driftZError[4];
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      t_arcSMax[iSt] = mConstD.at("Trg_PI") * mConstV.at("rr3D")[iSt] / 2;
      t_driftZError[iSt] = 1 / mConstV.at("driftZError")[iSt];
    }
    double t_denMax = Fitter3DUtility::calDenWithIZError(t_arcSMax, t_driftZError);
    mSignalStorage["denMax"] = Belle2::TRGCDCJSignal(t_denMax, mSignalStorage["den"].getToReal(), commonData);
  }
  // Constrain den.
  {
    // Make ifElse data.
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["den"], ">", mSignalStorage["denMax"]);
    // Assignments
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage["den_c"], mSignalStorage["denMax"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["den"], ">", mSignalStorage["denMin"]);
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage["den_c"], mSignalStorage["den"].limit(mSignalStorage["denMin"], mSignalStorage["denMax"]))
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage["den_c"], mSignalStorage["denMin"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  // z0NumSZ[i] = z0NumS[i]*zxiZError[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["z0NumSZ_" + to_string(iSt)] <= mSignalStorage["z0NumS_" + to_string(iSt)] * mSignalStorage["zxiZError_" + to_string(
          iSt)];
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<cotNumSZ_"<<iSt<<">>>"<<endl; mSignalStorage["cotNumSZ_"+to_string(iSt)].dump();}
  //cout<<"<<<den>>>"<<endl; mSignalStorage["den"].dump();
  //cout<<"<<<den_c>>>"<<endl; mSignalStorage["den_c"].dump();
  //cout<<"<<<denMin>>>"<<endl; mSignalStorage["denMin"].dump();
  //cout<<"<<<denMax>>>"<<endl; mSignalStorage["denMax"].dump();
  //cout<<"<<<offsetDen>>>"<<endl; mSignalStorage["offsetDen"].dump();
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<z0NumSZ_"<<iSt<<">>>"<<endl; mSignalStorage["z0NumSZ_"+to_string(iSt)].dump();}
  // cot_p1[0] = cotNumSZ[0] + cotNumSZ[1]
  mSignalStorage["cot_p1_0"] <= mSignalStorage["cotNumSZ_0"] + mSignalStorage["cotNumSZ_1"];
  // cot_p1[1] = cotNumSZ[2] + cotNumSZ[3]
  mSignalStorage["cot_p1_1"] <= mSignalStorage["cotNumSZ_2"] + mSignalStorage["cotNumSZ_3"];
  // z0_p1_0 = z0NumSZ_0 + z0NumSZ_1
  mSignalStorage["z0_p1_0"] <= mSignalStorage["z0NumSZ_0"] + mSignalStorage["z0NumSZ_1"];
  // z0_p1_1 = z0NumSZ_2 + z0NumSZ_3
  mSignalStorage["z0_p1_1"] <= mSignalStorage["z0NumSZ_2"] + mSignalStorage["z0NumSZ_3"];
  //// Calculate iDenMin, iDenMax.
  //mSignalStorage["iDenMin"] <= Belle2::TRGCDCJSignal(1/t_denMax, pow(1/mSignalStorage["rho"].getToReal()/mSignalStorage["iZError2_0"].getToReal(),2));
  //mSignalStorage["iDenMax"] <= Belle2::TRGCDCJSignal(1/t_denMin, pow(1/mSignalStorage["rho"].getToReal()/mSignalStorage["iZError2_0"].getToReal(),2));
  // Calculate minInvValue, maxInvValue for LUTs
  if (mSignalStorage.find("invIDenMin") == mSignalStorage.end()) {
    unsigned long long t_int = mSignalStorage["den_c"].getMaxInt();
    double t_toReal = mSignalStorage["den_c"].getToReal();
    double t_actual = mSignalStorage["den_c"].getMaxActual();
    mSignalStorage["invIDenMin"] = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1, commonData);
    t_int = mSignalStorage["den_c"].getMinInt();
    t_actual = mSignalStorage["den_c"].getMinActual();
    mSignalStorage["invIDenMax"] = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1, commonData);
  }
  //cout<<"<<<invIDenMin>>>"<<endl; mSignalStorage["invIDenMin"].dump();
  //cout<<"<<<invIDenMax>>>"<<endl; mSignalStorage["invIDenMax"].dump();
  // Generate LUT(iDen = 1/den)
  if (!mLutStorage.count("iDen")) {
    mLutStorage["iDen"] = new Belle2::TRGCDCJLUT("iDen");
    mLutStorage["iDen"]->setFloatFunction(
      [ = ](double aValue) -> double{return 1 / aValue;},
      mSignalStorage["den_c"],
      mSignalStorage["invIDenMin"], mSignalStorage["invIDenMax"],
      pow(1 / mSignalStorage["rho"].getToReal() / mSignalStorage["iZError2_0"].getToReal(), 2),
      (int)mConstD.at("iDenLUTInBitSize"), (int)mConstD.at("iDenLUTOutBitSize"));
    //mLutStorage["iDen"]->makeCOE("./LutData/iDen.coe");
  }
  // Operate using LUT(iDen = 1/den)
  mLutStorage["iDen"]->operate(mSignalStorage["den_c"], mSignalStorage["iDen"]);
  //for(int iSt=0; iSt<2; iSt++) {cout<<"<<<cot_p1_"<<iSt<<">>>"<<endl; mSignalStorage["cot_p1_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<2; iSt++) {cout<<"<<<z0_p1_"<<iSt<<">>>"<<endl; mSignalStorage["z0_p1_"+to_string(iSt)].dump();}
  //cout<<"<<<offsetDen_c>>>"<<endl; mSignalStorage["offsetDen_c"].dump();
  //cout<<"<<<denOffset_c>>>"<<endl; mSignalStorage["denOffset_c"].dump();
  //cout<<"<<<iDen>>>"<<endl; mSignalStorage["iDen"].dump();
  // iDen = offsetIDen + iDenOffset (Already done in LUT->operate)
  // cot_p2 = cot_p1_0 + cot_p1_1
  mSignalStorage["cot_p2"] <= mSignalStorage["cot_p1_0"] + mSignalStorage["cot_p1_1"];
  // z0_p2 = z0_p1_0 + z0_p1_1
  mSignalStorage["z0_p2"] <= mSignalStorage["z0_p1_0"] + mSignalStorage["z0_p1_1"];
  //cout<<"<<<cot_p2>>>"<<endl; mSignalStorage["cot_p2"].dump();
  //cout<<"<<<z0_p2>>>"<<endl; mSignalStorage["z0_p2"].dump();
  // cot = cot_p2 * iDen
  mSignalStorage["cot"] <= mSignalStorage["cot_p2"] * mSignalStorage["iDen"];
  // z0 = z0_p2 * iDen
  mSignalStorage["z0"] <= mSignalStorage["z0_p2"] * mSignalStorage["iDen"];
  if (mConstD.at("JB") == 1) {
    cout << "<<<cot>>>" << endl; mSignalStorage["cot"].dump();
    cout << "<<<z0>>>" << endl; mSignalStorage["z0"].dump();
  }

  // Constrain z0 to [-30,30]
  /* cppcheck-suppress variableScope */
  double z0Min = -30;
  /* cppcheck-suppress variableScope */
  double z0Max = 30;
  // Calculate z0Max and z0Min
  if (!mSignalStorage.count("z0Min")) {
    mSignalStorage["z0Min"] = Belle2::TRGCDCJSignal(z0Min, mSignalStorage["z0"].getToReal(), commonData);
    mSignalStorage["z0Max"] = Belle2::TRGCDCJSignal(z0Max, mSignalStorage["z0"].getToReal(), commonData);
  }
  {
    // Make ifElse data.
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["z0"], ">", mSignalStorage["z0Max"]);
    // Assignments
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage["z0_c"], mSignalStorage["z0Max"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["z0"], ">", mSignalStorage["z0Min"]);
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage["z0_c"], mSignalStorage["z0"].limit(mSignalStorage["z0Min"], mSignalStorage["z0Max"]))
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage["z0_c"], mSignalStorage["z0Min"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //cout<<"<<<z0Min>>>"<<endl; mSignalStorage["z0Min"].dump();
  //cout<<"<<<z0Max>>>"<<endl; mSignalStorage["z0Max"].dump();
  //cout<<"<<<z0_c>>>"<<endl; mSignalStorage["z0_c"].dump();
  // Constraint cot to [-0.753, 1.428]
  double cotMin = cos(127 * mConstD.at("Trg_PI") / 180) / sin(127 * mConstD.at("Trg_PI") / 180);
  double cotMax = cos(35 * mConstD.at("Trg_PI") / 180) / sin(35 * mConstD.at("Trg_PI") / 180);
  if (!mSignalStorage.count("cotMin")) {
    mSignalStorage["cotMin"] = Belle2::TRGCDCJSignal(cotMin, mSignalStorage["cot"].getToReal(), commonData);
    mSignalStorage["cotMax"] = Belle2::TRGCDCJSignal(cotMax, mSignalStorage["cot"].getToReal(), commonData);
  }
  {
    // Make ifElse data.
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["cot"], ">", mSignalStorage["cotMax"]);
    // Assignments
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage["cot_c"], mSignalStorage["cotMax"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["cot"], ">", mSignalStorage["cotMin"]);
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage["cot_c"], mSignalStorage["cot"].limit(mSignalStorage["cotMin"], mSignalStorage["cotMax"]))
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage["cot_c"], mSignalStorage["cotMin"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //cout<<"<<<cotMin>>>"<<endl; mSignalStorage["cotMin"].dump();
  //cout<<"<<<cotMax>>>"<<endl; mSignalStorage["cotMax"].dump();
  //cout<<"<<<cot_c>>>"<<endl; mSignalStorage["cot_c"].dump();
  // Reduce number of bits for z0 to 11 bits and cot to 11 bits
  /* cppcheck-suppress variableScope */
  int z0Bits = 11;
  /* cppcheck-suppress variableScope */
  int cotBits = 11;
  {
    int z0Shift = mSignalStorage["z0_c"].getBitsize() - z0Bits;
    mSignalStorage["z0_r"] <= mSignalStorage["z0_c"].shift(z0Shift, 0);
    int cotShift = mSignalStorage["cot_c"].getBitsize() - cotBits;
    mSignalStorage["cot_r"] <= mSignalStorage["cot_c"].shift(cotShift, 0);
  }
  //cout<<"<<<z0_c>>>"<<endl; mSignalStorage["z0_c"].dump();
  //cout<<"<<<cot_c>>>"<<endl; mSignalStorage["cot_c"].dump();
  //cout<<"<<<z0_r>>>"<<endl; mSignalStorage["z0_r"].dump();
  //cout<<"<<<cot_r>>>"<<endl; mSignalStorage["cot_r"].dump();


  // fitDistFromZ0[i] = cot*arcS[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["fitDistFromZ0_" + to_string(iSt)] <= mSignalStorage["cot"] * mSignalStorage["arcS_" + to_string(iSt)];
  }
  // distFromZ0[i] = z[i] - z0
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["distFromZ0_" + to_string(iSt)] <= mSignalStorage["zz_" + to_string(iSt)] - mSignalStorage["z0"];
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<fitDistFromZ0_"<<iSt<<">>>"<<endl; mSignalStorage["fitDistFromZ0_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<distFromZ0_"<<iSt<<">>>"<<endl; mSignalStorage["distFromZ0_"+to_string(iSt)].dump();}
  // chiNum[i] = distFromZ0[i] - fitDistFromZ0[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["chiNum_" + to_string(iSt)] <= mSignalStorage["distFromZ0_" + to_string(iSt)] - mSignalStorage["fitDistFromZ0_" +
        to_string(iSt)];
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<chiNum_"<<iSt<<">>>"<<endl; mSignalStorage["chiNum_"+to_string(iSt)].dump();}
  // Calculate chiNumMax[i], chiNumMin[i].
  if (mSignalStorage.find("chiNumMax_0") == mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_maxName = "chiNumMax_" + to_string(iSt);
      string t_minName = "chiNumMin_" + to_string(iSt);
      string t_valueName = "chiNum_" + to_string(iSt);
      double t_maxValue = 2 * mConstV.at("zToOppositeStraw")[iSt];
      double t_minValue = 2 * mConstV.at("zToStraw")[iSt];
      mSignalStorage[t_maxName] = Belle2::TRGCDCJSignal(t_maxValue, mSignalStorage[t_valueName].getToReal(), commonData);
      mSignalStorage[t_minName] = Belle2::TRGCDCJSignal(t_minValue, mSignalStorage[t_valueName].getToReal(), commonData);
    }
  }
  // Constrain chiNum[i] to [-2*z_min[i], 2*z_max[i]]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_maxName = "chiNumMax_" + to_string(iSt);
    string t_minName = "chiNumMin_" + to_string(iSt);
    string t_valueName = "chiNum_c_" + to_string(iSt);
    string t_in1Name = "chiNum_" + to_string(iSt);
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", mSignalStorage[t_maxName]);
    // Assignments
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_maxName]),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage[t_in1Name], ">", mSignalStorage[t_minName]);
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_in1Name].limit(mSignalStorage[t_minName], mSignalStorage[t_maxName])),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage[t_valueName], mSignalStorage[t_minName]),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<chiNumMax_"<<iSt<<">>>"<<endl; mSignalStorage["chiNumMax_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<chiNumMin_"<<iSt<<">>>"<<endl; mSignalStorage["chiNumMin_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<chiNum_c_"<<iSt<<">>>"<<endl; mSignalStorage["chiNum_c_"+to_string(iSt)].dump();}
  // chi2_p1[i] = chiNum_c[i] * chiNum_c[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["chi2_p1_" + to_string(iSt)] <= mSignalStorage["chiNum_c_" + to_string(iSt)] * mSignalStorage["chiNum_c_" +
        to_string(iSt)];
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<chi2_p1_"<<iSt<<">>>"<<endl; mSignalStorage["chi2_p1_"+to_string(iSt)].dump();}
  // chi2[i] = chi2_p1[i] * iError2[i]
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    mSignalStorage["chi2_" + to_string(iSt)] <= mSignalStorage["chi2_p1_" + to_string(iSt)] * mSignalStorage["iZError2_" + to_string(
          iSt)];
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<chi2_"<<iSt<<">>>"<<endl; mSignalStorage["chi2_"+to_string(iSt)].dump();}
  // chi2Sum_p1_0 = chi2_0 + chi2_1
  mSignalStorage["chi2Sum_p1_0"] <= mSignalStorage["chi2_0"] + mSignalStorage["chi2_1"];
  // chi2Sum_p1_1 = chi2_2 + chi2_3
  mSignalStorage["chi2Sum_p1_1"] <= mSignalStorage["chi2_2"] + mSignalStorage["chi2_3"];
  //cout<<"<<<chi2Sum_p1_0>>>"<<endl; mSignalStorage["chi2Sum_p1_0"].dump();
  //cout<<"<<<chi2Sum_p1_1>>>"<<endl; mSignalStorage["chi2Sum_p1_1"].dump();
  // chi2Sum = chi2Sum_p1_0 + chi2Sum_p1_1
  mSignalStorage["zChi2"] <= (mSignalStorage["chi2Sum_p1_0"] + mSignalStorage["chi2Sum_p1_1"]).shift(1);
  if (mConstD.at("JB") == 1) {cout << "<<<zChi2>>>" << endl; mSignalStorage["zChi2"].dump();}

  // Constrain zChi2 to [0, 100]
  /* cppcheck-suppress variableScope */
  double zChi2Min = 0;
  /* cppcheck-suppress variableScope */
  double zChi2Max = 100;
  /* cppcheck-suppress variableScope */
  int zChi2Bits = 4;
  // Calculate zChi2Max and zChi2Min
  if (!mSignalStorage.count("zChi2Min")) {
    mSignalStorage["zChi2Min"] = Belle2::TRGCDCJSignal(zChi2Min, mSignalStorage["zChi2"].getToReal(), commonData);
    mSignalStorage["zChi2Max"] = Belle2::TRGCDCJSignal(zChi2Max, mSignalStorage["zChi2"].getToReal(), commonData);
  }
  {
    // Make ifElse data.
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["zChi2"], ">", mSignalStorage["zChi2Max"]);
    // Assignments
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&mSignalStorage["zChi2_c"], mSignalStorage["zChi2Max"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["zChi2"], ">", mSignalStorage["zChi2Min"]);
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage["zChi2_c"], mSignalStorage["zChi2"].limit(mSignalStorage["zChi2Min"], mSignalStorage["zChi2Max"]))
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&mSignalStorage["zChi2_c"], mSignalStorage["zChi2Min"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //cout<<"<<<zChi2Min>>>"<<endl; mSignalStorage["zChi2Min"].dump();
  //cout<<"<<<zChi2Max>>>"<<endl; mSignalStorage["zChi2Max"].dump();
  //cout<<"<<<zChi2_c>>>"<<endl; mSignalStorage["zChi2_c"].dump();
  {
    int zChi2Shift = mSignalStorage["zChi2_c"].getBitsize() - zChi2Bits;
    mSignalStorage["zChi2_r"] <= mSignalStorage["zChi2_c"].shift(zChi2Shift, 0);
  }
  //cout<<"<<<zChi2_c>>>"<<endl; mSignalStorage["zChi2_c"].dump();
  //cout<<"<<<zChi2_r>>>"<<endl; mSignalStorage["zChi2_r"].dump();

}

void Fitter3DUtility::fitter3D(std::map<std::string, std::vector<double> >& stGeometry,
                               std::vector<std::vector<double> > const& stXts,
                               int eventTimeValid, int eventTime,
                               std::vector<std::vector<int> > const& rawStTSs,
                               int charge, double radius, double phi_c, double& z0, double& cot, double& chi2)
{
  vector<double> zz;
  vector<double> arcS;
  vector<double> invZError2;
  fitter3D(stGeometry, stXts, eventTimeValid, eventTime, rawStTSs, charge, radius, phi_c, z0, cot, chi2, arcS, zz, invZError2);
}

void Fitter3DUtility::fitter3D(std::map<std::string, std::vector<double> >& stGeometry,
                               std::vector<std::vector<double> > const& stXts,
                               int eventTimeValid, int eventTime,
                               std::vector<std::vector<int> > const& rawStTSs,
                               int charge, double radius, double phi_c, double& z0, double& cot, double& chi2, vector<double>& arcS, vector<double>& zz,
                               vector<double>& invZError2)
{
  vector<double> stTSs(4);
  Fitter3DUtility::calPhiFast(stGeometry, stXts, eventTimeValid, eventTime, rawStTSs, stTSs);
  Fitter3DUtility::setErrorFast(rawStTSs, eventTimeValid, invZError2);
  zz = vector<double> (4, 0);
  arcS = vector<double> (4, 0);
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    if (rawStTSs[iSt][1] != 0) {
      zz[iSt] = Fitter3DUtility::calZ(charge, stGeometry["angleSt"][iSt], stGeometry["zToStraw"][iSt],
                                      stGeometry["cdcRadius"][iSt], stTSs[iSt], radius, phi_c);
      arcS[iSt] = Fitter3DUtility::calS(radius, stGeometry["cdcRadius"][iSt]);
    }
  }
  Fitter3DUtility::rSFit(&invZError2[0], &arcS[0], &zz[0], z0, cot, chi2);
}

void Fitter3DUtility::fitter3DFirm(std::map<std::string, double>& mConstD,
                                   const std::map<std::string, std::vector<double> >& mConstV,
                                   int eventTimeValid, int eventTime,
                                   std::vector<std::vector<int> > const& rawStTSs,
                                   int charge, double radius, double phi_c,
                                   Belle2::TRGCDCJSignalData* commonData, std::map<std::string, Belle2::TRGCDCJSignal>& mSignalStorage,
                                   std::map<std::string, Belle2::TRGCDCJLUT*>& mLutStorage)
{
  // Change to Signals.
  // rawStTSs[iSt] = [TS ID, TS LR, TS driftTime]
  vector<tuple<string, double, int, double, double, int> > t_values = {
    make_tuple("phi0", phi_c, mConstD["phiBitSize"], mConstD["phiMin"], mConstD["phiMax"], 0),
    make_tuple("rho", radius, mConstD["rhoBitSize"], mConstD["rhoMin"], mConstD["rhoMax"], 0),
    make_tuple("charge", (int)(charge == 1 ? 1 : 0), 1, 0, 1.5, 0),
    make_tuple("lr_0", rawStTSs[0][1], 2, 0, 3.5, 0),
    make_tuple("lr_1", rawStTSs[1][1], 2, 0, 3.5, 0),
    make_tuple("lr_2", rawStTSs[2][1], 2, 0, 3.5, 0),
    make_tuple("lr_3", rawStTSs[3][1], 2, 0, 3.5, 0),
    make_tuple("tsId_0", rawStTSs[0][0], ceil(log(mConstV.at("nTSs")[1]) / log(2)), 0, pow(2, ceil(log(mConstV.at("nTSs")[1]) / log(2))) - 0.5, 0),
    make_tuple("tsId_1", rawStTSs[1][0], ceil(log(mConstV.at("nTSs")[3]) / log(2)), 0, pow(2, ceil(log(mConstV.at("nTSs")[3]) / log(2))) - 0.5, 0),
    make_tuple("tsId_2", rawStTSs[2][0], ceil(log(mConstV.at("nTSs")[5]) / log(2)), 0, pow(2, ceil(log(mConstV.at("nTSs")[5]) / log(2))) - 0.5, 0),
    make_tuple("tsId_3", rawStTSs[3][0], ceil(log(mConstV.at("nTSs")[7]) / log(2)), 0, pow(2, ceil(log(mConstV.at("nTSs")[7]) / log(2))) - 0.5, 0),
    make_tuple("tdc_0", rawStTSs[0][2], mConstD["tdcBitSize"], 0, pow(2, mConstD["tdcBitSize"]) - 0.5, 0),
    make_tuple("tdc_1", rawStTSs[1][2], mConstD["tdcBitSize"], 0, pow(2, mConstD["tdcBitSize"]) - 0.5, 0),
    make_tuple("tdc_2", rawStTSs[2][2], mConstD["tdcBitSize"], 0, pow(2, mConstD["tdcBitSize"]) - 0.5, 0),
    make_tuple("tdc_3", rawStTSs[3][2], mConstD["tdcBitSize"], 0, pow(2, mConstD["tdcBitSize"]) - 0.5, 0),
    make_tuple("eventTime", eventTime, mConstD["tdcBitSize"], 0, pow(2, mConstD["tdcBitSize"]) - 0.5, 0),
    make_tuple("eventTimeValid", eventTimeValid, 1, 0, 1.5, 0),
  };
  Belle2::TRGCDCJSignal::valuesToMapSignals(t_values, commonData, mSignalStorage);
  // Calculate complex fit3D
  Fitter3DUtility::setError(mConstD, mConstV, mSignalStorage);
  Fitter3DUtility::calPhi(mConstD, mConstV, mSignalStorage, mLutStorage);
  Fitter3DUtility::constrainRPerStSl(mConstV, mSignalStorage);
  Fitter3DUtility::calZ(mConstD, mConstV, mSignalStorage, mLutStorage);
  Fitter3DUtility::calS(mConstD, mConstV, mSignalStorage, mLutStorage);
  Fitter3DUtility::rSFit(mConstD, mConstV, mSignalStorage, mLutStorage);
  // Post handling of signals.
  // Name all signals.
  if ((*mSignalStorage.begin()).second.getName() == "") {
    for (auto it = mSignalStorage.begin(); it != mSignalStorage.end(); ++it) {
      (*it).second.setName((*it).first);
    }
  }
}

void Fitter3DUtility::findImpactPosition(TVector3* mcPosition, ROOT::Math::PxPyPzEVector* mcMomentum, int charge,
                                         TVector2& helixCenter,
                                         TVector3& impactPosition)
{

  // Finds the impact position. Everything is in cm, and GeV.
  // Input:   production vertex (mcx, mcy, mcz),
  //          momentum at production vertex (px, py, pz)
  //          charge of particle.
  // Output:  helix center's coordiante (hcx, hcy)
  //          impact position (impactX, impactY, impactZ)

  double rho = sqrt(pow(mcMomentum->Px(), 2) + pow(mcMomentum->Py(), 2)) / 0.3 / 1.5 * 100;
  double hcx = mcPosition->X() + rho * cos(atan2(mcMomentum->Py(), mcMomentum->Px()) - charge * TMath::Pi() / 2);
  double hcy = mcPosition->Y() + rho * sin(atan2(mcMomentum->Py(), mcMomentum->Px()) - charge * TMath::Pi() / 2);
  helixCenter.Set(hcx, hcy);
  double impactX = (helixCenter.Mod() - rho) / helixCenter.Mod() * helixCenter.X();
  double impactY = (helixCenter.Mod() - rho) / helixCenter.Mod() * helixCenter.Y();
  int signdS;
  if (atan2(impactY, impactX) < atan2(mcPosition->Y(), mcPosition->X())) signdS = -1;
  else signdS = 1;
  double dS = 2 * rho * asin(sqrt(pow(impactX - mcPosition->X(), 2) + pow(impactY - mcPosition->Y(), 2)) / 2 / rho);
  double impactZ = mcMomentum->Pz() / mcMomentum->Pt() * dS * signdS + mcPosition->Z();
  impactPosition.SetXYZ(impactX, impactY, impactZ);

}

void Fitter3DUtility::calHelixParameters(TVector3 position, TVector3 momentum, int charge, TVectorD& helixParameters)
{
  double Trg_PI = 3.141592653589793;
  // HelixParameters: dR, phi0, keppa, dz, tanLambda
  double t_alpha = 1 / 0.3 / 1.5 * 100;
  double t_pT = momentum.Perp();
  double t_R = t_pT * t_alpha;
  helixParameters.Clear();
  helixParameters.ResizeTo(5);
  helixParameters[2] = t_alpha / t_R;
  helixParameters[1] = atan2(position.Y() - t_R * momentum.X() / t_pT * charge, position.X() + t_R * momentum.Y() / t_pT * charge);
  helixParameters[0] = (position.X() + t_R * momentum.Y() / t_pT * charge) / cos(helixParameters[1]) - t_R;
  double t_phi = atan2(-momentum.X() * charge, momentum.Y() * charge) - helixParameters[1];
  if (t_phi > Trg_PI) t_phi -= 2 * Trg_PI;
  if (t_phi < -Trg_PI) t_phi += 2 * Trg_PI;
  helixParameters[4] = momentum.Z() / t_pT * charge;
  helixParameters[3] = position.Z() + helixParameters[4] * t_R * t_phi;
}
void Fitter3DUtility::calVectorsAtR(const TVectorD& helixParameters, int charge, double cdcRadius, TVector3& position,
                                    TVector3& momentum)
{
  // HelixParameters: dR, phi0, keppa, dz, tanLambda
  double t_alpha = 1 / 0.3 / 1.5 * 100;
  double t_R = t_alpha / helixParameters[2];
  double t_pT = t_R / t_alpha;
  double t_phi = -1 * charge * acos((-pow(cdcRadius, 2) + pow(t_R + helixParameters[0], 2) + pow(t_R,
                                     2)) / (2 * t_R * (t_R + helixParameters[0])));
  double t_X = (helixParameters[0] + t_R) * cos(helixParameters[1]) - t_R * cos(helixParameters[1] + t_phi);
  double t_Y = (helixParameters[0] + t_R) * sin(helixParameters[1]) - t_R * sin(helixParameters[1] + t_phi);
  double t_Z = helixParameters[3] - helixParameters[4] * t_R * t_phi;
  double t_Px = -t_pT * sin(helixParameters[1] + t_phi) * charge;
  double t_Py = t_pT * cos(helixParameters[1] + t_phi) * charge;
  double t_Pz = t_pT * helixParameters[4] * charge;
  position.SetXYZ(t_X, t_Y, t_Z);
  momentum.SetXYZ(t_Px, t_Py, t_Pz);
}

int Fitter3DUtility::bitSize(int numberBits, int mode)
{
  int bitsize = 1;
  if (mode == 1) {
    for (int i = 0; i < numberBits - 1; i++) {
      bitsize *= 2;
    }
    bitsize -= 1;
  } else if (mode == 0) {
    for (int i = 0; i < numberBits; i++) {
      bitsize *= 2;
    }
  }
  return bitsize;
}

void Fitter3DUtility::changeInteger(int& integer, double real, double minValue, double maxValue, int bitSize)
{
  // Version 1.
  //double range = maxValue - minValue;
  //double convert = bitSize/range;
  ////cout<<"maxValue: "<<bitSize<<" realValue: "<<(real-minValue)*convert<<endl;
  //integer = int( (real - minValue) * convert +0.5);
  // Version 2.
  double range = maxValue - minValue;
  double convert = (bitSize - 0.5) / range;
  integer = FpgaUtility::roundInt((real - minValue) * convert);
}

void Fitter3DUtility::changeReal(double& real, int integer, double minValue, double maxValue, int bitSize)
{
  //// Version 1.
  //double range = maxValue - minValue;
  //double convert = bitSize/range;
  //real = (integer/convert) + minValue;
  // Version 2.
  double range = maxValue - minValue;
  double convert = (bitSize - 0.5) / range;
  real = (integer / convert) + minValue;
}


void Fitter3DUtility::findExtreme(double& m_max, double& m_min, double value)
{
  if (value > m_max) m_max = value;
  if (value < m_min) m_min = value;
}
