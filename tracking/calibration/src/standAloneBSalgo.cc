/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Program obtain the Beam Spot properties from mumu track variables
// inspired by https://docs.belle2.org/record/1511/files/BELLE2-NOTE-TE-2019-018.pdf
//          and https://arxiv.org/pdf/1405.6569.pdf

#include <iomanip>
#include <iostream>
#include <tuple>
#include <cassert>

#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TF2.h"
#include "TProfile.h"
#include "TVector3.h"
#include "TFile.h"
#include "TTree.h"
#include "TMatrixD.h"
#include "TMatrixDSym.h"
#include "TVectorD.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TRandom.h"
#include "TRotation.h"
#include "TChain.h"


#include <functional>
#include <vector>
#include <numeric>

#include "tools.h"
#include "standAloneBSalgo.h"
#include "Splitter.h"

using namespace std;


// Sign-sensitive sqr,sqrt
inline double sqrS(double x) {return x >= 0 ? x * x : -x * x; }; //sign-sensitive sqr
inline double sqrtS(double x) {return x >= 0 ? sqrt(x) : -sqrt(-x); }; //sign-sensitive sqrt

TMatrixD getRotatedSizeMatrix(vector<double> xySize, double zzSize, double kX, double kY);


//structore containing most of the beam spot parameters
struct spotParam {
  Spline x, y, z;  //Splines for IP position in time
  Spline kX, kY;   //deviation with z-coordinate

  spotParam() {}

  // Print BeamSpot parameters
  void print()
  {
    cout << "x" << endl;
    x.print();
    cout << "y" << endl;
    y.print();
    cout << "z" << endl;
    z.print();
    cout << "kX" << endl;
    kX.print();
    cout << "kY" << endl;
    kY.print();
  }

  // Constructor based on vals, errors and vector with splines - Assuming linear splines
  // xVals, yVals, kXvals, kYvals, zVals
  spotParam(const vector<double>& vals, const vector<double>& errs, const vector<vector<double>>& spls, int order = 0)
  {
    auto getSize = [order](const vector<double>& sp) {
      if (sp.size() == 0)
        return 1;
      else {
        if (order == 0) {
          assert(sp.size() >= 1);
          return int(sp.size() + 1);
        } else if (order == 1) {
          assert(sp.size() >= 2);
          return int(sp.size());
        } else {
          cout << "Unknown order" << endl;
          exit(0);
        }
      }
    };

    int nx  = getSize(spls[0]);
    int ny  = getSize(spls[1]);
    x.nodes  = spls[0];
    y.nodes  = spls[1];
    x.vals   = Slice(vals, 0, nx);
    y.vals   = Slice(vals, nx, ny);
    x.errs   = Slice(errs, 0, nx);
    y.errs   = Slice(errs, nx, ny);

    if (spls.size() >= 4) {
      int nkx = getSize(spls[2]);
      int nky = getSize(spls[3]);
      kX.nodes = spls[2];
      kY.nodes = spls[3];
      kX.vals  = Slice(vals, nx + ny, nkx);
      kY.vals  = Slice(vals, nx + ny + nkx, nky);

      kX.errs  = Slice(errs, nx + ny, nkx);
      kY.errs  = Slice(errs, nx + ny + nkx, nky);

      if (spls.size() >= 5) {
        int nz = getSize(spls[4]);
        z.nodes  = spls[4];
        z.vals  = Slice(vals, nx + ny + nkx + nky, nz);
        z.errs  = Slice(errs, nx + ny + nkx + nky, nz);
      }
    }
  }


};


// Spline with uncertainity obtained from the replicas
struct unknowSpline {
  vector<Spline> spls; // vector with replicas
  void add(Spline spl) { spls.push_back(spl); } // add boot-strap replica

  // Get mean and 1-sigma errors of the spline values
  Spline getMeanSigma()
  {
    Spline sAvg;
    sAvg = spls[0];
    int nNd = spls[0].vals.size();
    for (int k = 0; k < nNd; ++k) {
      double s = 0, ss = 0;
      for (unsigned i = 0; i < spls.size(); ++i) {
        s  += spls[i].vals[k];
      }
      s  /=  spls.size();

      for (unsigned i = 0; i < spls.size(); ++i) {
        ss += pow(spls[i].vals[k] - s, 2);
      }

      if (spls.size() > 1)
        ss = sqrt(ss / (spls.size() - 1));
      else
        ss = 0;

      sAvg.vals[k] = s;
      sAvg.errs[k] = ss;

    }
    return sAvg;
  }

  //percentil spline, 0.5 : median,  0.16: low interval, 0.84 : up interval
  Spline getLimit(double v)
  {
    Spline sLim = spls[0];

    double indx = (spls.size() - 1) * v;
    int nNd = spls[0].vals.size();
    for (int k = 0; k < nNd; ++k) {
      vector<double> vals;
      for (unsigned i = 0; i < spls.size(); ++i) {
        vals.push_back(spls[i].vals[k]);
      }
      sort(vals.begin(), vals.end());

      int I  = indx;
      double r = indx - I;
      sLim.vals[k] = vals[I] * (1 - r) + vals[I + 1] * r;
      sLim.errs[k] = 0;
    }
    return sLim;
  }

};




// variable with uncertainity from boot-strap replicas
struct unknowVar {
  vector<double> vars;
  void add(double x) { vars.push_back(x); } // add value to the replicas

  double getMean()
  {
    assert(vars.size() >= 1);
    return accumulate(vars.begin(), vars.end(), 0.) / vars.size();
  }

  double getSigma()
  {
    assert(vars.size() >= 1);
    double m = getMean();
    double s = 0;
    for (auto x : vars)
      s += pow(x - m, 2);
    if (vars.size() > 1)
      return sqrt(s / (vars.size() - 1));
    else
      return 0;
  }

  //68 % limit
  double getLimit(double v)
  {
    assert(vars.size() >= 1);
    double indx = (vars.size() - 1) * v;
    sort(vars.begin(), vars.end());
    int I  = indx;
    double r = indx - I;
    return vars[I] * (1 - r) + vars[I + 1] * r;
  }

  // Print variable of name "n" with stat-info
  void printStat(TString n)
  {
    assert(vars.size() >= 1);
    cout << n << " : " <<  getMean() << "+-" << getSigma() << " : " << getLimit(0.50) << " (" << getLimit(0.16) << " , " << getLimit(
           1 - 0.16) << " )" << endl;
  }

  // Get basic stats
  vector<double> getStats()
  {
    return {getLimit(0.50), getLimit(0.16), getLimit(1 - 0.16)};
  }
};


// Get angle in XY-plane from cov-matrix elements, sizes in [um] !
double getAngle(double SizeX, double SizeY, double SizeXY)
{
  double C = sqrS(SizeXY);
  //is this correct?
  double angle = 1. / 2 * atan(2 * C / (pow(SizeX, 2) - pow(SizeY, 2)));
  return angle;
}

//Get the eigen-vals of symetric 2x2 matrix, sizes in [um] !
pair<double, double> getSizeMinMax(double SizeX, double SizeY, double SizeXY)
{
  double A = pow(SizeX, 2) + pow(SizeY, 2);
  double B = pow(SizeX, 2) * pow(SizeY, 2) - pow(SizeXY, 4);
  double D = pow(A, 2) - 4 * B;

  if (D < 0) {
    cout << "Problem with D" << D << endl;
    exit(1);
  }

  double Size2Min = 2 * B / (A + sqrt(D));
  if (abs(Size2Min) < 1e-7) Size2Min = 0;
  if (Size2Min < 0) {
    cout << "Negative size" << endl;
    exit(1);
  }
  double Size2Max = (A + sqrt(D)) / 2;
  return {sqrtS(Size2Min), sqrtS(Size2Max)};
}


//Get the eigen-vals of symetric 2x2 matrix, sizes in [um2] !
double getSize2MinMat(double SizeXX, double SizeYY, double SizeXY)
{
  double A = SizeXX + SizeYY;
  double B = SizeXX * SizeYY - pow(SizeXY, 2);
  double D = pow(A, 2) - 4 * B;

  if (D < 0) {
    cout << "Problem with D" << D << endl;
    exit(1);
  }

  double Size2Min = 2 * B / (A + sqrt(D));

  return Size2Min;
}





// structure including all variables of interest with uncertainity from boot-strap
struct unknownPars {
  unknowSpline x, y, kX, kY, z;
  unknowVar sizeX, sizeY, sizeXY, sizeMin, sizeMax, xyAngle;
  unknowVar sizeZ, crossAngle;

  unknowVar matXX, matYY, matZZ, matXY, matXZ, matYZ;

  void add(spotParam sPar, double SizeX, double SizeY, double SizeXY, double SizeZ)
  {
    x.add(sPar.x);
    y.add(sPar.y);
    kX.add(sPar.kX);
    kY.add(sPar.kY);
    z.add(sPar.z);


    sizeX.add(SizeX);
    sizeY.add(SizeY);
    sizeXY.add(SizeXY);
    sizeZ.add(SizeZ);


    // Calculate the eignen-values
    double SizeMin, SizeMax;
    tie(SizeMin, SizeMax) = getSizeMinMax(SizeX, SizeY, SizeXY);

    sizeMin.add(SizeMin);
    sizeMax.add(SizeMax);

    // and angle in mrad
    double angle = 1e3 * getAngle(SizeX, SizeY, SizeXY);


    xyAngle.add(angle);

    //Get whole cov matrix
    TMatrixD matSize = getRotatedSizeMatrix({sqrS(SizeX), sqrS(SizeY), sqrS(SizeXY)}, sqrS(SizeZ), sPar.kX.val(sPar.kX.center()),
                                            sPar.kY.val(sPar.kY.center()));

    // Store elements in [um]
    matXX.add(sqrtS(matSize(0, 0)));
    matYY.add(sqrtS(matSize(1, 1)));
    matZZ.add(sqrtS(matSize(2, 2)));
    matXY.add(sqrtS(matSize(0, 1)));

    matXZ.add(sqrtS(matSize(0, 2)));
    matYZ.add(sqrtS(matSize(1, 2)));


    // crossing-angle in mrad
    double crossAngleVal = 1e3 * 2 * sqrtS(matSize(0, 0)) / sqrtS(matSize(2, 2));
    crossAngle.add(crossAngleVal);
  }

  // Print interesting statistics from boot-strap
  void printStat()
  {
    x.getMeanSigma().print("x");
    y.getMeanSigma().print("y");
    kX.getMeanSigma().print("kX");
    kY.getMeanSigma().print("kY");
    z.getMeanSigma().print("z");

    sizeX.printStat("sizeX");
    sizeY.printStat("sizeY");
    sizeXY.printStat("sizeXY");


    sizeMin.printStat("sizeMin");
    sizeMax.printStat("sizeMax");
    xyAngle.printStat("xyAngle");
    sizeZ.printStat("sizeZ");
    crossAngle.printStat("crossAngle");


    matXX.printStat("matXX");
    matYY.printStat("matYY");
    matZZ.printStat("matZZ");
    matXY.printStat("matXY");
    matXZ.printStat("matXZ");
    matYZ.printStat("matYZ");
  }

  // get output in Belle2-like format
  void getOutput(vector<TVector3>& vtxPos, vector<TMatrixDSym>& vtxErr, TMatrixDSym& sizeMat)
  {
    //Store the vertex position
    int nVals = x.spls[0].vals.size();

    const double toCm = 1e-4;

    for (int i = 0; i < nVals; ++i) {
      //vertex position
      TVector3 vtx(x.spls[0].vals[i]*toCm, y.spls[0].vals[i]*toCm, z.spls[0].vals[i]*toCm);

      //vertex error matrix (symetric)
      TMatrixDSym mS(3);
      mS(0, 0) = sqrS(x.spls[0].errs[i] * toCm);
      mS(1, 1) = sqrS(y.spls[0].errs[i] * toCm);
      mS(2, 2) = sqrS(z.spls[0].errs[i] * toCm);

      vtxPos.push_back(vtx);
      vtxErr.push_back(mS);
    }

    //BeamSpot size matrix (from iteration 0)

    sizeMat.ResizeTo(3, 3);
    sizeMat(0, 0) = sqrS(matXX.vars[0] * toCm);
    sizeMat(1, 1) = sqrS(matYY.vars[0] * toCm);
    sizeMat(2, 2) = sqrS(matZZ.vars[0] * toCm);

    sizeMat(0, 1) = sqrS(matXY.vars[0] * toCm);
    sizeMat(0, 2) = sqrS(matXZ.vars[0] * toCm);
    sizeMat(1, 2) = sqrS(matYZ.vars[0] * toCm);

    sizeMat(1, 0) = sizeMat(0, 1);
    sizeMat(2, 0) = sizeMat(0, 2);
    sizeMat(2, 1) = sizeMat(1, 2);
  }


  // save bootstrap variable to TTree
  void setBranchVal(TTree* T, vector<double>* vec, TString n)
  {
    T->Branch(n, &vec->at(0), n + "/D");
    T->Branch(n + "_low", &vec->at(1), n + "_low/D");
    T->Branch(n + "_high", &vec->at(2), n + "_high/D");
  }

  // save bootstrap spline to TTree
  void setBranchSpline(TTree* T, Spline* spl, TString n)
  {
    T->Branch(n + "_nodes", &spl->nodes);
    T->Branch(n + "_vals",  &spl->vals);
    T->Branch(n + "_errs",  &spl->errs);
  }



  //save everything to TTree
  void save2tree(TString fName)
  {

    TTree* T = new TTree("runs", "beam conditions of runs");

    int run = -99; //currently dummy
    T->Branch("run", &run, "run/I");

    Spline xAvg = x.getMeanSigma();
    setBranchSpline(T, &xAvg, "x");
    Spline yAvg = y.getMeanSigma();
    setBranchSpline(T, &yAvg, "y");
    Spline zAvg = z.getMeanSigma();
    setBranchSpline(T, &zAvg, "z");

    Spline kxAvg = kX.getMeanSigma();
    setBranchSpline(T, &kxAvg, "kX");
    Spline kyAvg = kY.getMeanSigma();
    setBranchSpline(T, &kyAvg, "kY");

    vector<double> sizeXVar = sizeX.getStats();
    setBranchVal(T, &sizeXVar, "sizeX");
    vector<double> sizeYVar = sizeY.getStats();
    setBranchVal(T, &sizeYVar, "sizeY");
    vector<double> sizeXYVar = sizeXY.getStats();
    setBranchVal(T, &sizeXYVar, "sizeXY");
    vector<double> sizeZVar = sizeZ.getStats();
    setBranchVal(T, &sizeZVar, "sizeZ");

    vector<double> sizeMinVar = sizeMin.getStats();
    setBranchVal(T, &sizeMinVar, "sizeMin");
    vector<double> sizeMaxVar = sizeMax.getStats();
    setBranchVal(T, &sizeMaxVar, "sizeMax");
    vector<double> xyAngleVar = xyAngle.getStats();
    setBranchVal(T, &xyAngleVar, "xyAngle");

    vector<double> crossAngleVar = crossAngle.getStats();
    setBranchVal(T, &crossAngleVar, "crossAngle");


    vector<double> matXXVar = matXX.getStats();
    vector<double> matYYVar = matYY.getStats();
    vector<double> matZZVar = matZZ.getStats();
    vector<double> matXYVar = matXY.getStats();
    vector<double> matXZVar = matXZ.getStats();
    vector<double> matYZVar = matYZ.getStats();

    setBranchVal(T, &matXXVar, "matXX");
    setBranchVal(T, &matYYVar, "matYY");
    setBranchVal(T, &matZZVar, "matZZ");

    setBranchVal(T, &matXYVar, "matXY");
    setBranchVal(T, &matXZVar, "matXZ");
    setBranchVal(T, &matYZVar, "matYZ");


    T->Fill();
    T->SaveAs(fName);
  }
};



// get estimate of the zIP position (recurent function)
// nestMax = number of iter, nest = current iter
double getZIPest(const track& tr, double t, const spotParam& spotPar, int nestMax = 5, int nest = 0)
{
  double x0, y0;
  if (nest < nestMax) {
    double zIP =  getZIPest(tr, t, spotPar, nestMax, nest + 1);

    x0 = spotPar.x.val(t) + spotPar.kX.val(t) * (zIP - spotPar.z.val(t));
    y0 = spotPar.y.val(t) + spotPar.kY.val(t) * (zIP - spotPar.z.val(t));
  } else {
    x0 = spotPar.x.val(t);
    y0 = spotPar.y.val(t);
  }

  double f0 = tr.tanlambda * (x0 * cos(tr.phi0) + y0 * sin(tr.phi0));

  return (tr.z0 + f0);
}


// get the D0 position corrected for IP position
double getCorrD(const track& tr, double t, const spotParam& spotPar)
{
  double zIP =  getZIPest(tr, t, spotPar);

  double x0 = spotPar.x.val(t) + spotPar.kX.val(t) * (zIP - spotPar.z.val(t));
  double y0 = spotPar.y.val(t) + spotPar.kY.val(t) * (zIP - spotPar.z.val(t));

  double f0 = x0 * sin(tr.phi0) - y0 * cos(tr.phi0);

  return (tr.d0 - f0);
}

//Transform D0 to time in the middle of interval
double getDtimeConst(const track& tr, double t, const spotParam& spotPar)
{
  double zIP  =  getZIPest(tr, t,                  spotPar);
  double zIPM =  getZIPest(tr, spotPar.z.center(), spotPar);

  double x0 = spotPar.x.val(t) + spotPar.kX.val(t) * (zIP - spotPar.z.val(t));
  double y0 = spotPar.y.val(t) + spotPar.kY.val(t) * (zIP - spotPar.z.val(t));

  double xM = spotPar.x.val(spotPar.x.center()) + spotPar.kX.val(spotPar.kX.center()) * (zIPM - spotPar.z.val(spotPar.z.center()));
  double yM = spotPar.y.val(spotPar.y.center()) + spotPar.kY.val(spotPar.kY.center()) * (zIPM - spotPar.z.val(spotPar.z.center()));


  double f0 = (x0 - xM) * sin(tr.phi0) - (y0 - yM) * cos(tr.phi0);

  return (tr.d0 - f0);
}


// get Z0 corrected for the IP position
double getCorrZ(const track& tr, double t, const spotParam& spotPar)
{
  double zIP =  getZIPest(tr, t, spotPar);

  double x0 = spotPar.x.val(t) + spotPar.kX.val(t) * (zIP - spotPar.z.val(t));
  double y0 = spotPar.y.val(t) + spotPar.kY.val(t) * (zIP - spotPar.z.val(t));
  double z0 = spotPar.z.val(t);

  double f0 = z0 - tr.tanlambda * (x0 * cos(tr.phi0) + y0 * sin(tr.phi0));

  return (tr.z0 - f0);
}




// get min & max time of the events
pair<double, double> getStartStop(const vector<event>&  evts)
{
  double minT = 1e20, maxT = -1e20;
  for (auto ev : evts) {
    minT = min(minT, ev.tAbs);
    maxT = max(maxT, ev.tAbs);
  }
  return {minT, maxT};
}

//get list of file names from comma-separated string
vector<TString> extractFileNames(TString str)
{
  vector<int> commas;
  for (int i = 0; i < str.Length(); ++i) {
    if (str[i] == ',')
      commas.push_back(i);
  }
  if (commas.size() == 0)
    return {str.Strip()};

  vector<TString> files;
  files.push_back(static_cast<TString>(str(0, commas[0])).Strip());

  for (int i = 0; i < static_cast<int>(commas.size()) - 1; ++i) {
    files.push_back(static_cast<TString>(str(commas[i] + 1, commas[i + 1] - commas[i] - 1)).Strip());
  }
  files.push_back(static_cast<TString>(str(commas.back() + 1, 100000)).Strip());

  return files;
}

// read events from TTree to std::vector
vector<event> getEvents(TTree* tr)
{

  vector<event> events;
  events.reserve(tr->GetEntries());

  event evt;

  tr->SetBranchAddress("run", &evt.run);
  tr->SetBranchAddress("exp", &evt.exp);
  tr->SetBranchAddress("event", &evt.evtNo);
  tr->SetBranchAddress("mu0_d0", &evt.mu0.d0);
  tr->SetBranchAddress("mu1_d0", &evt.mu1.d0);
  tr->SetBranchAddress("mu0_z0", &evt.mu0.z0);
  tr->SetBranchAddress("mu1_z0", &evt.mu1.z0);

  tr->SetBranchAddress("mu0_tanlambda", &evt.mu0.tanlambda);
  tr->SetBranchAddress("mu1_tanlambda", &evt.mu1.tanlambda);


  tr->SetBranchAddress("mu0_phi0", &evt.mu0.phi0);
  tr->SetBranchAddress("mu1_phi0", &evt.mu1.phi0);

  tr->SetBranchAddress("time", &evt.tAbs);


  for (int i = 0; i < tr->GetEntries(); ++i) {
    tr->GetEntry(i);
    evt.toMicroM();

    evt.nBootStrap = 1;
    evt.isSig = true;
    events.push_back(evt);
  }

  //sort by time
  sort(events.begin(), events.end(), [](event e1, event e2) {return e1.tAbs < e2.tAbs;});

  for (auto& e : events) {
    e.t = e.tAbs;
  }

  return events;
}

// Add random booth strp weights to events
void bootStrap(vector<event>& evts)
{
  for (auto& e : evts)
    e.nBootStrap = gRandom->Poisson(1);
}




// Simple linear regresion fit
TVectorD linearFit(TMatrixD mat, TVectorD r)
{
  TMatrixD matT = mat; matT.T();
  TMatrixD A = matT * mat;

  TVectorD v = matT * r;
  TMatrixD Ainv = A; Ainv.Invert();

  return (Ainv * v);
}


// Linear fit with errors & PRESS statistics
pair<vector<double>, vector<double>> linearFitErr(TMatrixD m, TVectorD r, double& err2Mean, double& err2press, double& err2pressErr)
{
  TMatrixD mT = m; mT.T();
  TMatrixD mat = mT * m;

  mat.Invert();
  TMatrixD A = mat * mT;
  TVectorD res = A * r;
  TVectorD dataH = m * res;


  //errs
  double err2 = (dataH - r).Norm2Sqr() / (r.GetNrows() - res.GetNrows());


  // Get PRESS statistics of the linear fit
  {
    //TMatrixD Ahat =  m*A;
    double press = 0;
    double press2 = 0;
    for (int i = 0; i < r.GetNrows(); ++i) {
      double Ahat = 0;
      for (int k = 0; k < m.GetNcols(); ++k)
        Ahat += m(i, k) * A(k, i);

      double v = pow((r(i) - dataH(i)) / (1 - Ahat), 2);
      press += v;
      press2 += v * v;
    }
    press  /= r.GetNrows();
    press2 /= (r.GetNrows() - 1);

    err2press = press;
    err2pressErr = sqrt((press2 - press * press) / r.GetNrows()) / sqrt(r.GetNrows());
  }



  TMatrixD AT = A; A.T();
  TMatrixD errMat = err2 * AT * A;
  TVectorD errs2(errMat.GetNrows());
  for (int i = 0; i < errs2.GetNrows(); ++i)
    errs2(i) = errMat(i, i);

  err2Mean = err2;

  return {vec2vec(res), vec2vec(errs2)};
}




//Linear fit with positivity constraint on the output parameters (for BeamSpot-size fit)
TVectorD linearFitPos(TMatrixD mat, TVectorD r)
{
  const double s2MinLimit = pow(0.05, 2); //Minimal value of the BeamSpot eigenSize
  if (mat.GetNcols() != 3) {
    cout << "Wrong size of matrix for size fit" << endl;
    exit(0);
  }
  TMatrixD matT = mat; matT.T();
  TMatrixD A = matT * mat;
  TVectorD v = matT * r;
  TMatrixD Ainv = A; Ainv.Invert();

  TVectorD pars = Ainv * v;

  //If everyting OK
  double s2Min = getSize2MinMat(pars[0], pars[1], pars[2]);
  if (pars[0] >= 0 && pars[1] >= 0 && s2Min >= s2MinLimit)
    return pars;

  //////////////////////////
  //Get the error matrix
  //////////////////////////

  int nDf = r.GetNrows() - 3;
  //Calculate average error
  double err2 = (mat * pars - r).Norm2Sqr() / nDf;

  //cout << "err2 " << sqrt(err2) << endl;
  TMatrixD wMat = Ainv * matT;
  TMatrixD wMatT = wMat; wMatT.T();

  TMatrixD covMat = err2 * wMat * wMatT;
  TMatrixD covMatI = covMat;
  covMatI.Invert();

  double Norm = 1. / (pow(2 * M_PI, 3 / 2.) *  sqrt(covMat.Determinant()));


  //////////////////////////
  //Get maximum likelihood
  //////////////////////////

  //Maximum likelihood over eigenvector and angle
  TF2* fEig = new TF2(rn(), [Norm, covMatI, pars, s2MinLimit](double * x, double*) {
    double eig1 = x[0];
    double eig2 = s2MinLimit;
    double phi  = x[1];
    double c = cos(phi);
    double s = sin(phi);

    TVectorD xVec(3);
    xVec(0) = eig1 * c * c + eig2 * s * s;
    xVec(1) = eig1 * s * s + eig2 * c * c;
    xVec(2) = s * c * (eig1 - eig2);

    double res = covMatI.Similarity(xVec - pars);
    return res;
  }, s2MinLimit, 400, 0, 2 * M_PI, 0);

  double eigHigh, phi;
  fEig->GetMinimumXY(eigHigh, phi);

  pars[0] = eigHigh * pow(cos(phi), 2) + s2MinLimit * pow(sin(phi), 2);
  pars[1] = eigHigh * pow(sin(phi), 2) + s2MinLimit * pow(cos(phi), 2);
  pars[2] = sin(phi) * cos(phi) * (eigHigh - s2MinLimit);


  return pars;

}




// Get resolution histogram from 2D histo
TH1D* getResolution(TH2D* hRes)
{
  TH1D* hSigmaAll = new TH1D(rn(), "", 50, -M_PI, M_PI);
  for (int i = 1; i <= hRes->GetNbinsX(); ++i) {
    TH1D* hProj = hRes->ProjectionY(rn(), i, i);
    double rms    = hProj->GetRMS();
    double rmsErr = hProj->GetRMSError();
    hSigmaAll->SetBinContent(i,   rms * rms); //from cm2 to um2
    hSigmaAll->SetBinError(i,  2 * abs(rms)*rmsErr);
  }
  return hSigmaAll;
}

// Get meanHisto from 2D histo
TH1D* getMean(const TH2D* hRes)
{
  TH1D* hMean = new TH1D(rn(), "", 50, -M_PI, M_PI);
  for (int i = 1; i <= hRes->GetNbinsX(); ++i) {
    TH1D* hProj = hRes->ProjectionY(rn(), i, i);
    double mean    = hProj->GetMean();
    double meanErr = hProj->GetMeanError();
    hMean->SetBinContent(i,   mean);
    hMean->SetBinError(i,  meanErr);
  }
  return hMean;
}

// Geth theoretical <d0_1 *d0_2>
double getD12th(event e, vector<double> sizesXY)
{
  double sxx = sizesXY[0];
  double syy = sizesXY[1];
  double sxy = sizesXY[2];

  double cc = cos(e.mu0.phi0) * cos(e.mu1.phi0);
  double ss = sin(e.mu0.phi0) * sin(e.mu1.phi0);
  double sc = -(sin(e.mu0.phi0) * cos(e.mu1.phi0) + sin(e.mu1.phi0) * cos(e.mu0.phi0));

  return ss * sxx + cc * syy + sc * sxy;
}

// Geth theoretical <z0_1 *z0_2>
double getZ12th(event e, vector<double> sizesXY)
{
  double sxx = sizesXY[0];
  double syy = sizesXY[1];

  double corr = e.mu0.tanlambda * e.mu1.tanlambda * (sxx * cos(e.mu0.phi0) * cos(e.mu1.phi0) + syy * sin(e.mu0.phi0) * sin(
                                                       e.mu1.phi0) +
                                                     + (sin(e.mu0.phi0) * cos(e.mu1.phi0) + cos(e.mu0.phi0) * sin(e.mu1.phi0)));
  return corr;
}






//Plot TGraph with the fitted function
void plotSpotPositionFit(const vector<event>& evts, spotParam par, TString fName)
{
  TGraph* gr = new TGraph();
  TProfile* dProf     = new TProfile(rn(), "dProf", 100, -M_PI, M_PI, "S");
  TProfile* dProfRes  = new TProfile(rn(), "dProfRes", 100, -M_PI, M_PI, "S");

  for (auto e : evts) {
    if (!e.isSig) continue;

    double d1 = getDtimeConst(e.mu0, e.t, par);
    double d2 = getDtimeConst(e.mu1, e.t, par);

    gr->SetPoint(gr->GetN(), e.mu0.phi0, d1);
    gr->SetPoint(gr->GetN(), e.mu1.phi0, d2);

    dProf->Fill(e.mu0.phi0, d1);
    dProf->Fill(e.mu1.phi0, d2);


    double d1c = getCorrD(e.mu0, e.t, par);
    double d2c = getCorrD(e.mu1, e.t, par);

    dProfRes->Fill(e.mu0.phi0, d1c);
    dProfRes->Fill(e.mu1.phi0, d2c);
  }
  TF1* f = new TF1(rn(), "[0]*sin(x) - [1]*cos(x)", -M_PI, M_PI);
  f->SetParameters(par.x.val(par.x.center()), par.y.val(par.y.center()));

  TCanvas* c = new TCanvas(rn(), "");
  gr->Draw("a p");
  gr->GetXaxis()->SetRangeUser(-M_PI, M_PI);
  gr->SetMaximum(+1.3 * f->GetMaximum());
  gr->SetMinimum(-1.3 * f->GetMaximum());

  gr->GetXaxis()->SetTitle("#phi_{0} [rad]");
  gr->GetYaxis()->SetTitle("d_{0} [#mum]");

  f->Draw("same");
  c->SaveAs(fName + "_dots.pdf");


  TCanvas* d = new TCanvas(rn(), "");
  gStyle->SetOptStat(0);
  dProf->Draw();
  dProf->GetXaxis()->SetRangeUser(-M_PI, M_PI);
  dProf->SetMaximum(+1.3 * f->GetMaximum());
  dProf->SetMinimum(-1.3 * f->GetMaximum());

  dProf->GetXaxis()->SetTitle("#phi_{0} [rad]");
  dProf->GetYaxis()->SetTitle("d_{0} [#mum]");

  f->Draw("same");

  cout << "Saving " << fName << " prof " << endl;
  d->SaveAs(fName + "_prof.pdf");


  TCanvas* e = new TCanvas(rn(), "");
  gStyle->SetOptStat(0);
  dProfRes->Draw();
  dProfRes->GetXaxis()->SetRangeUser(-M_PI, M_PI);

  dProfRes->GetXaxis()->SetTitle("#phi_{0} [rad]");
  dProfRes->GetYaxis()->SetTitle("d_{0} res [#mum]");

  TH1D* errP = new TH1D(rn(), "dErrP", 100, -M_PI, M_PI);
  TH1D* errM = new TH1D(rn(), "dErrM", 100, -M_PI, M_PI);
  for (int i = 1; i <= errP->GetNbinsX(); ++i) {
    errP->SetBinContent(i, dProfRes->GetBinError(i));
    errM->SetBinContent(i, -dProfRes->GetBinError(i));
  }

  errP->Draw("hist same");
  errM->Draw("hist same");

  f->SetParameters(0, 0);
  f->Draw("same");

  cout << "Saving " << fName << " profRes " << endl;
  e->SaveAs(fName + "_profRes.pdf");

}


//Plot TGraph with the fitted function
void plotSpotZPositionFit(const vector<event>& evts, spotParam par, TString fName)
{
  TProfile* zProf  = new TProfile(rn(), "dProf", 100, -M_PI, M_PI, "S");
  TGraph* gr = new TGraph();
  for (auto e : evts) {
    if (!e.isSig) continue;


    double z1ip  = getZIPest(e.mu0, e.t, par);
    double z2ip  = getZIPest(e.mu1, e.t, par);

    double zipT = par.z.val(e.t);
    double zipM = par.z.val(par.z.center());

    double val1 = z1ip - (zipT - zipM);
    double val2 = z2ip - (zipT - zipM);

    gr->SetPoint(gr->GetN(), e.mu0.phi0, val1);
    gr->SetPoint(gr->GetN(), e.mu1.phi0, val2);

    zProf->Fill(e.mu0.phi0, val1);
    zProf->Fill(e.mu1.phi0, val2);
  }
  TF1* f = new TF1(rn(), "[0]", -M_PI, M_PI);
  f->SetParameter(0, par.z.val(par.z.center()));

  TCanvas* c = new TCanvas(rn(), "");
  c->SetLeftMargin(0.12);
  gr->Draw("a p");
  gr->GetXaxis()->SetRangeUser(-M_PI, M_PI);
  gr->SetMaximum(1000);
  gr->SetMinimum(-1000);

  gr->GetXaxis()->SetTitle("#phi_{0} [rad]");
  gr->GetYaxis()->SetTitle("z_{IP} estimate [#mum]");

  f->Draw("same");
  c->SaveAs(fName + "_dots.pdf");


  TCanvas* d = new TCanvas(rn(), "");
  gStyle->SetOptStat(0);
  d->SetLeftMargin(0.12);
  zProf->Draw();
  zProf->GetXaxis()->SetRangeUser(-M_PI, M_PI);
  zProf->SetMaximum(1000);
  zProf->SetMinimum(-1000);

  zProf->GetXaxis()->SetTitle("#phi_{0} [rad]");
  zProf->GetYaxis()->SetTitle("z_{IP} estimate [#mum]");

  f->Draw("same");
  d->SaveAs(fName + +"_prof.pdf");

}





//Plot pull distribution
void plotSpotPositionPull(const vector<event>& evts, const spotParam& par, TString fName, double cut = 70)
{
  TH1D* hPull = new TH1D(rn(), "", 200, -200, 200);

  for (auto& e : evts) {
    if (!e.isSig) continue;

    double d0 = getCorrD(e.mu0, e.t, par);
    double d1 = getCorrD(e.mu1, e.t, par);

    hPull->Fill(d0);
    hPull->Fill(d1);
  }

  TCanvas* c = new TCanvas(rn(), "");
  gStyle->SetOptStat(2210);
  hPull->Draw();

  hPull->GetXaxis()->SetTitle("pull [#mum]");
  hPull->GetYaxis()->SetTitle("#tracks");

  TLine* l = new TLine;
  l->SetLineColor(kRed);
  l->DrawLine(-cut, 0, -cut, 500);
  l->DrawLine(+cut, 0, +cut, 500);

  c->SaveAs(fName + ".pdf");
}


//Plot pull distribution
void plotKxKyFit(const vector<event>& evts, spotParam par, TString fName)
{
  TProfile* profRes    = new TProfile(rn(), "dProf", 100, -800, 800, "S");
  TProfile* profResKx  = new TProfile(rn(), "dProfKx", 100, -800, 800, "S");
  TProfile* profResKy  = new TProfile(rn(), "dProfKy", 100, -800, 800, "S");

  spotParam parNoKx = par;
  spotParam parNoKy = par;
  parNoKx.kX.vals = {0};
  parNoKy.kY.vals = {0};
  parNoKx.kX.nodes = {};
  parNoKy.kY.nodes = {};

  for (auto& e : evts) {
    if (!e.isSig) continue;

    double zDiff1 =  getZIPest(e.mu0, e.t, par) - par.z.val(e.t);
    double zDiff2 =  getZIPest(e.mu1, e.t, par) - par.z.val(e.t);


    double d1 = getCorrD(e.mu0, e.t, par);
    double d2 = getCorrD(e.mu1, e.t, par);

    double d1KX = getCorrD(e.mu0, e.t, parNoKx);
    double d2KX = getCorrD(e.mu1, e.t, parNoKx);


    profResKx->Fill(zDiff1 * sin(e.mu0.phi0), d1KX);
    profResKx->Fill(zDiff2 * sin(e.mu1.phi0), d2KX);

    double d1KY = getCorrD(e.mu0, e.t, parNoKy);
    double d2KY = getCorrD(e.mu1, e.t, parNoKy);
    profResKy->Fill(-zDiff1 * cos(e.mu0.phi0), d1KY);
    profResKy->Fill(-zDiff2 * cos(e.mu1.phi0), d2KY);

    profRes->Fill(zDiff1, d1);
    profRes->Fill(zDiff2, d2);


  }

  TCanvas* cX = new TCanvas(rn(), "");
  gStyle->SetOptStat(0);
  profResKx->Draw();

  profResKx->GetXaxis()->SetTitle("(z_{IP} - z_{IP}^{0}) sin #phi_{0} [#mum]");
  profResKx->GetYaxis()->SetTitle("d_{0} res [#mum]");

  TF1* f = new TF1(rn(), "[0]*x", -800, 800);
  f->SetParameter(0, par.kX.val(par.kX.center()));
  f->Draw("same");

  cX->SaveAs(fName + "_kX.pdf");

  TCanvas* cY = new TCanvas(rn(), "");
  gStyle->SetOptStat(0);
  profResKy->Draw();

  profResKy->GetXaxis()->SetTitle("-(z_{IP} - z_{IP}^{0}) cos #phi_{0} [#mum]");
  profResKy->GetYaxis()->SetTitle("d_{0} res [#mum]");

  f->SetParameter(0, par.kY.val(par.kY.center()));
  f->Draw("same");

  cY->SaveAs(fName + "_kY.pdf");


}

//Plot pull distribution
void plotXYtimeDep(const vector<event>& evts, spotParam par, TString fName)
{
  TProfile* profRes    = new TProfile(rn(), "dProf", 50,   -0.5, 0.5);
  TProfile* profResTx  = new TProfile(rn(), "dProfTx", 50, -0.5, 0.5);
  TProfile* profResTy  = new TProfile(rn(), "dProfTy", 50, -0.5, 0.5);

  spotParam parNoTx = par;
  spotParam parNoTy = par;
  parNoTx.x.nodes = {};
  parNoTx.x.vals  = {par.x.val(par.x.center())};
  parNoTy.y.nodes = {};
  parNoTy.y.vals  = {par.y.val(par.y.center())};

  for (auto& e : evts) {
    if (!e.isSig) continue;

    double tDiff = (e.t - par.x.val(par.x.center()));
    //double tDiff =  e.t;


    double d1 = getCorrD(e.mu0, e.t, par);
    double d2 = getCorrD(e.mu1, e.t, par);

    double d1TX = getCorrD(e.mu0, e.t, parNoTx);
    double d2TX = getCorrD(e.mu1, e.t, parNoTx);

    profResTx->Fill(tDiff * sin(e.mu0.phi0), d1TX);
    profResTx->Fill(tDiff * sin(e.mu1.phi0), d2TX);

    double d1TY = getCorrD(e.mu0, e.t, parNoTy);
    double d2TY = getCorrD(e.mu1, e.t, parNoTy);

    profResTy->Fill(-tDiff * cos(e.mu0.phi0), d1TY);
    profResTy->Fill(-tDiff * cos(e.mu1.phi0), d2TY);


    profRes->Fill(tDiff * sin(e.mu0.phi0), d1);
    profRes->Fill(tDiff * sin(e.mu1.phi0), d2);


  }

  TCanvas* cX = new TCanvas(rn(), "");
  gStyle->SetOptStat(0);
  profResTx->Draw();


  TF1* f = new TF1(rn(), "[0]*x", -1, 1);
  f->SetParameter(0, (par.x.val(1) - par.x.val(0)));
  f->Draw("same");
  cout << "Table value " << par.x.val(1) - par.x.val(0) << endl;

  cX->SaveAs(fName + "_tX.pdf");


}





//Plot pull distribution
void plotSpotZpositionPull(const vector<event>& evts, const spotParam& par, TString fName, double cut = 1000)
{
  TH1D* hPull = new TH1D(rn(), "", 200, -2000, 2000);

  for (auto& e : evts) {
    if (!e.isSig) continue;

    double z0 = getCorrZ(e.mu0, e.t, par);
    double z1 = getCorrZ(e.mu1, e.t, par);

    hPull->Fill(z0);
    hPull->Fill(z1);
  }

  gStyle->SetOptStat(2210);
  TCanvas* c = new TCanvas(rn(), "");
  hPull->Draw();

  hPull->GetXaxis()->SetTitle("pull [#mum]");
  hPull->GetYaxis()->SetTitle("#tracks");

  TLine* l = new TLine;
  l->SetLineColor(kRed);
  l->DrawLine(-cut, 0, -cut, 500);
  l->DrawLine(+cut, 0, +cut, 500);

  c->SaveAs(fName + ".pdf");
}


// Remove outliear from the position fit (for d0)
void removeSpotPositionOutliers(vector<event>& evts,  const spotParam& par, double cut = 70)
{
  int nRem = 0;
  int nAll = 0;
  for (auto& e : evts) {
    if (!e.isSig) continue;

    double d0 = getCorrD(e.mu0, e.t, par);
    double d1 = getCorrD(e.mu1, e.t, par);

    e.isSig = abs(d0) < cut && abs(d1) < cut;
    nRem += !e.isSig;
    ++nAll;
  }
  //cout << "Removed fraction Position " << nRem / (nAll + 0.) << endl;
}


// Remove outliear from the position fit (for z0)
void removeSpotZpositionOutliers(vector<event>& evts,  const spotParam& par, double cut = 1000)
{
  int nRem = 0;
  int nAll = 0;
  for (auto& e : evts) {
    if (!e.isSig) continue;

    double z0 = getCorrZ(e.mu0, e.t, par);
    double z1 = getCorrZ(e.mu1, e.t, par);

    e.isSig = abs(z0) < cut && abs(z1) < cut;
    nRem += !e.isSig;
    ++nAll;
  }
  //cout << "Removed fraction Position " << nRem / (nAll + 0.) << endl;
}



//Fill matrix with bases based on linear splines
vector<vector<double>> fillSplineBasesLinear(const vector<event>& evts, vector<double> spl,
                                             std::function<double(track, double)> fun)
{
  int n = spl.size(); //number of params
  if (n == 0 || (n == 2 && spl[0] > spl[1]))
    n = 1;

  vector<vector<double>> vecs(n);

  if (n == 1) { //no time dependence
    for (const auto& e : evts) {
      for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
        vecs[0].push_back(1 * fun(e.mu0, e.t));
        vecs[0].push_back(1 * fun(e.mu1, e.t));
      }
    }
  } else {
    for (int k = 0; k < n; ++k) {
      double xCnt = spl[k];
      double xLow = (k == 0)   ? spl[0] : spl[k - 1];
      double xHigh = (k == n - 1) ? spl[n - 1] : spl[k + 1];

      for (const auto& e : evts) {
        double x = e.t;
        double v = 0;
        if (xLow <= x && x < xCnt)
          v = (x - xLow) / (xCnt - xLow);
        else if (xCnt < x && x <= xHigh)
          v = (xHigh - x) / (xHigh - xCnt);


        for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
          vecs[k].push_back(v * fun(e.mu0, e.t));
          vecs[k].push_back(v * fun(e.mu1, e.t));
        }
      }
    }
  }

  return vecs;
}


//Fill matrix with bases based on zero-order splines
vector<vector<double>> fillSplineBasesZero(const vector<event>& evts, vector<double> spl, std::function<double(track, double)> fun)
{
  int n = spl.size() + 1; //number of params

  vector<vector<double>> vecs(n);

  if (n == 1) { //no time dependence
    for (const auto& e : evts) {
      for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
        vecs[0].push_back(1 * fun(e.mu0, e.t));
        vecs[0].push_back(1 * fun(e.mu1, e.t));
      }
    }
  } else { // at least two parameters
    for (int k = 0; k < n; ++k) { //loop over spline parameters
      double xLow = -1e30;
      double xHigh = +1e30;

      if (k == 0) {
        xHigh = spl[0];
      } else if (k == n - 1) {
        xLow = spl.back();
      } else {
        xLow  = spl[k - 1];
        xHigh = spl[k];
      }

      for (const auto& e : evts) {
        double x = e.t;
        double v = 0;
        if (xLow <= x && x < xHigh)
          v = 1;

        for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
          vecs[k].push_back(v * fun(e.mu0, e.t));
          vecs[k].push_back(v * fun(e.mu1, e.t));
        }
      }
    }
  }

  return vecs;
}






// compare consistency of two splines, taking errers into account
double compareSplines(const Spline& spl1, const Spline& spl2)
{
  double sum = 0;

  double step = 0.001;
  for (double x = 0; x <= 1 + step / 2; x += step) {
    double v1 = spl1.val(x);
    double e1 = spl1.err(x);
    double v2 = spl2.val(x);
    double e2 = spl2.err(x);

    double d = pow(v2 - v1, 2) / pow(max(e1, e2), 2);
    sum += d * step;
  }
  return sum;
}

// Fit width in z-direction in [um^2]
double fitSpotZwidth(const vector<event>& evts, const spotParam& spotPar, const vector<double>& sizesXY)
{

  vector<double> dataVec;
  vector<double> zzVec;


  for (auto e : evts) {
    double z0 = getCorrZ(e.mu0, e.t, spotPar);
    double z1 = getCorrZ(e.mu1, e.t, spotPar);

    double corr = getZ12th(e, sizesXY);
    double z0z1Corr = z0 * z1 - corr;


    for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
      dataVec.push_back(z0z1Corr);
      zzVec.push_back(1);
    }
  }

  TMatrixD mat = vecs2mat({zzVec});

  vector<double> pars, err2;
  double err2Mean, err2press, err2pressErr;
  tie(pars, err2) = linearFitErr(mat, vec2vec(dataVec), err2Mean, err2press, err2pressErr);

  return pars[0];

}




// Fit xy widths (including XZ, YZ slopes), no prior
spotParam fitSpotPositionSplines(const vector<event>& evts, const vector<double>& splX, const vector<double>& splY,
                                 const vector<double>& splKX, const vector<double>& splKY)
{
  vector<vector<double>> basesX  = fillSplineBasesZero(evts, splX, [](track tr, double) {return  sin(tr.phi0);});
  vector<vector<double>> basesY  = fillSplineBasesZero(evts, splY, [](track tr, double) {return -cos(tr.phi0);});

  vector<vector<double>> basesKX = fillSplineBasesZero(evts, splKX, [](track tr, double) {return  sin(tr.phi0) * tr.z0;});
  vector<vector<double>> basesKY = fillSplineBasesZero(evts, splKY, [](track tr, double) {return -cos(tr.phi0) * tr.z0;});


  vector<double> dataVec;
  for (auto e : evts) {
    for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
      dataVec.push_back(e.mu0.d0);
      dataVec.push_back(e.mu1.d0);
    }
  }

  vector<vector<double>> allVecs = merge({basesX, basesY, basesKX, basesKY});

  TMatrixD A = vecs2mat(allVecs);


  TVectorD vData = vec2vec(dataVec);

  vector<double> pars(A.GetNcols()), err2(A.GetNcols());
  double err2Mean, err2press, err2pressErr;
  tie(pars, err2) = linearFitErr(A, vData, err2Mean, err2press, err2pressErr);

  for (auto& e : err2) e = sqrt(e);
  return spotParam(pars, err2, {splX, splY, splKX, splKY});
}

// Fit xy widths (including XZ, YZ slopes), with prior info from spotPars
spotParam fitSpotPositionSplines(const vector<event>& evts, const vector<double>& splX, const vector<double>& splY,
                                 const vector<double>& splKX, const vector<double>& splKY, const spotParam& spotPars)
{
  vector<vector<double>> basesX  = fillSplineBasesZero(evts, splX, [](track tr, double) {return  sin(tr.phi0);});
  vector<vector<double>> basesY  = fillSplineBasesZero(evts, splY, [](track tr, double) {return -cos(tr.phi0);});

  vector<vector<double>> basesKX = fillSplineBasesZero(evts, splKX, [ = ](track tr, double t) {return  sin(tr.phi0) * (getZIPest(tr, t, spotPars) - spotPars.z.val(t));});
  vector<vector<double>> basesKY = fillSplineBasesZero(evts, splKY, [ = ](track tr, double t) {return -cos(tr.phi0) * (getZIPest(tr, t, spotPars) - spotPars.z.val(t));});


  vector<double> dataVec;
  for (auto e : evts) {
    for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
      dataVec.push_back(e.mu0.d0);
      dataVec.push_back(e.mu1.d0);
    }
  }

  vector<vector<double>> allVecs = merge({basesX, basesY, basesKX, basesKY});

  TMatrixD A = vecs2mat(allVecs);


  TVectorD vData = vec2vec(dataVec);

  vector<double> pars(A.GetNcols()), err2(A.GetNcols());
  double err2Mean, err2press, err2pressErr;
  tie(pars, err2) = linearFitErr(A, vData, err2Mean, err2press, err2pressErr);

  for (auto& e : err2) e = sqrt(e);
  auto res = spotParam(pars, err2, {splX, splY, splKX, splKY});
  res.z = spotPars.z;
  return res;
}





// simple fit of position splines, without kX, kY
spotParam fitSpotPositionSplines(const vector<event>& evts, const vector<double>& splX, const vector<double>& splY)
{
  vector<vector<double>> basesX  = fillSplineBasesZero(evts, splX, [](track tr, double) {return  sin(tr.phi0);});
  vector<vector<double>> basesY  = fillSplineBasesZero(evts, splY, [](track tr, double) {return -cos(tr.phi0);});

  vector<double> dataVec;
  for (auto e : evts) {
    for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
      dataVec.push_back(e.mu0.d0);
      dataVec.push_back(e.mu1.d0);
    }
  }

  vector<vector<double>> allVecs = merge({basesX, basesY});

  TMatrixD A = vecs2mat(allVecs);

  TVectorD vData = vec2vec(dataVec);

  vector<double> pars(A.GetNcols()), err2(A.GetNcols());
  double err2Mean, err2press, err2pressErr;
  //cout << "Starting linear fit ev : "  << dataVec.size()  << endl;
  tie(pars, err2) = linearFitErr(A, vData, err2Mean, err2press, err2pressErr);
  //cout << "Mean err xyfit " << fixed << setprecision(3) << err2Mean << " : " << err2press << " " << err2pressErr << " :  " <<
  //err2press - err2Mean << endl;

  for (auto& e : err2) e = sqrt(e);
  return spotParam(pars, err2, {splX, splY});
}






//Fit Zposition
spotParam fitZpositionSplines(const vector<event>& evts, const vector<double>& splX, const vector<double>& splY,
                              const vector<double>& splKX, const vector<double>& splKY,
                              const vector<double>& splZ)
{
  vector<vector<double>> basesX  = fillSplineBasesZero(evts, splX, [](track tr, double) {return -tr.tanlambda * cos(tr.phi0);});
  vector<vector<double>> basesY  = fillSplineBasesZero(evts, splY, [](track tr, double) {return -tr.tanlambda * sin(tr.phi0);});

  vector<vector<double>> basesKX = fillSplineBasesZero(evts, splKX, [](track tr, double) {return -tr.z0 * tr.tanlambda * cos(tr.phi0);});
  vector<vector<double>> basesKY = fillSplineBasesZero(evts, splKY, [](track tr, double) {return -tr.z0 * tr.tanlambda * sin(tr.phi0);});

  vector<vector<double>> basesZ  = fillSplineBasesZero(evts, splZ,  [](track , double) {return 1;});


  vector<double> dataVec;
  for (auto e : evts) {
    for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
      dataVec.push_back(e.mu0.z0);
      dataVec.push_back(e.mu1.z0);
    }
  }

  vector<vector<double>> allVecs = merge({basesX, basesY, basesKX, basesKY, basesZ});

  TMatrixD A = vecs2mat(allVecs);

  TVectorD vData = vec2vec(dataVec);

  vector<double> pars(A.GetNcols()), err2(A.GetNcols());
  double err2Mean, err2press, err2pressErr;
  tie(pars, err2) = linearFitErr(A, vData, err2Mean, err2press, err2pressErr);

  for (auto& e : err2) e = sqrt(e);
  return spotParam(pars, err2, {splX, splY, splKX, splKY, splZ});
}



//Fit Zposition, xIP, yIP fixed from d0 fit
spotParam fitZpositionSplinesSimple(const vector<event>& evts, const vector<double>& splZ, const spotParam& spotPars)
{
  vector<vector<double>> basesZ  = fillSplineBasesZero(evts, splZ,  [](track, double) {return 1;});

  vector<double> dataVec;
  for (auto e : evts) {
    for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
      double z1 =  getZIPest(e.mu0, e.t, spotPars);
      double z2 =  getZIPest(e.mu1, e.t, spotPars);
      dataVec.push_back(z1);
      dataVec.push_back(z2);
    }
  }

  TMatrixD A = vecs2mat({basesZ});

  TVectorD vData = vec2vec(dataVec);

  vector<double> pars(A.GetNcols()), err2(A.GetNcols());
  double err2Mean, err2press, err2pressErr;
  tie(pars, err2) = linearFitErr(A, vData, err2Mean, err2press, err2pressErr);
  //cout << "Mean err zfit " << sqrt(err2Mean) << endl;

  for (auto& e : err2) e = sqrt(e);

  spotParam parsUpd = spotPars;
  parsUpd.z.vals = pars;
  parsUpd.z.errs = err2;
  parsUpd.z.nodes = splZ;

  return parsUpd;
}



// Returns x-y sizes in um^2
vector<double> fitSpotWidthCMS(const vector<event>& evts, const spotParam& spotPar)
{

  vector<double> dataVec, ccVec, ssVec, scVec;


  for (auto e : evts) {
    double d0 = getCorrD(e.mu0, e.t, spotPar);
    double d1 = getCorrD(e.mu1, e.t, spotPar);

    for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
      dataVec.push_back(d0 * d1);

      ccVec.push_back(cos(e.mu0.phi0)*cos(e.mu1.phi0));
      ssVec.push_back(sin(e.mu0.phi0)*sin(e.mu1.phi0));
      scVec.push_back(-(sin(e.mu0.phi0)*cos(e.mu1.phi0) + sin(e.mu1.phi0)*cos(e.mu0.phi0)));
    }
  }


  TMatrixD mat = vecs2mat({ssVec, ccVec, scVec});

  // Linear fit with constraint on positivness
  TVectorD resPhys = linearFitPos(mat, vec2vec(dataVec));

  return {resPhys(0), resPhys(1), resPhys(2)};
}


// Plot pulls in xy size fit
void plotSpotSizePull(const vector<event>& evts, const spotParam& spotPar, const vector<double>& sizesXY)
{
  TH1D* hPull = new TH1D(rn(), "", 100, -2000, 2000);
  for (auto& e : evts) {
    if (!e.isSig) continue;

    double d0 = getCorrD(e.mu0, e.t, spotPar);
    double d1 = getCorrD(e.mu1, e.t, spotPar);

    double d12Th =  getD12th(e, sizesXY);

    hPull->Fill(d0 * d1 - d12Th);
  }
  TCanvas* c = new TCanvas(rn(), "");
  hPull->Draw();
  c->SaveAs("pullsSize.pdf");
}


// Plot pulls in Zsize fit
void plotSpotSizeZPull(const vector<event>& evts, const spotParam& spotPar, const vector<double>& sizesXY,  double sizeZZ)
{
  TH1D* hPull = new TH1D(rn(), "", 100, -300e3, 600e3);
  for (auto& e : evts) {
    if (!e.isSig) continue;

    double z0 = getCorrZ(e.mu0, e.t, spotPar);
    double z1 = getCorrZ(e.mu1, e.t, spotPar);

    double corr = getZ12th(e, sizesXY);
    double res = z0 * z1 - corr - sizeZZ;

    hPull->Fill(res);
  }

  gStyle->SetOptStat(2210);
  TCanvas* c = new TCanvas(rn(), "");
  hPull->Draw();
  cout << "zSizeFit mean " << hPull->GetMean() << endl;
  cout << "zSizeFit rms " << hPull->GetRMS() << endl;

  c->SaveAs("pullsZSize.pdf");
}




// Plot size fit control plots
void plotSpotSizeFit(const vector<event>& evts, const spotParam& par, const vector<double>& sizeXY)
{
  double sxx = sizeXY[0];
  double syy = sizeXY[1];
  double sxy = sizeXY[2];

  gStyle->SetOptStat(0);

  TProfile* profSxx = new TProfile(rn(), "", 50, -1, 1);
  TProfile* profSyy = new TProfile(rn(), "", 50, -1, 1);
  TProfile* profSxy = new TProfile(rn(), "", 50, -1, 1);
  for (auto e : evts) {
    if (!e.isSig) continue;

    double cc = cos(e.mu0.phi0) * cos(e.mu1.phi0);
    double ss = sin(e.mu0.phi0) * sin(e.mu1.phi0);
    double sc = - (sin(e.mu0.phi0) * cos(e.mu1.phi0) + sin(e.mu1.phi0) * cos(e.mu0.phi0));

    double d0 = getCorrD(e.mu0, e.t, par);
    double d1 = getCorrD(e.mu1, e.t, par);

    double data = d0 * d1;

    profSxx->Fill(ss, data - syy * cc - sxy * sc);
    profSyy->Fill(cc, data - sxx * ss - sxy * sc);
    profSxy->Fill(sc, data - syy * cc - sxx * ss);
  }

  TCanvas* c = new TCanvas(rn(), "", 1200, 500);
  c->Divide(3, 1);
  c->cd(1);
  profSxx->Draw();
  profSxx->GetXaxis()->SetTitle("sin #phi_{1} sin #phi_{2}");
  profSxx->GetYaxis()->SetTitle("#LTd_{1} d_{2}#GT - corr_{xx} [#mum^{2}]");
  TF1* fxx = new TF1(rn(), "[0]*x", -1, 1);
  fxx->SetParameter(0, sxx);
  fxx->Draw("same");

  c->cd(2);
  profSyy->Draw();
  profSyy->GetXaxis()->SetTitle("cos #phi_{1} cos #phi_{2}");
  profSyy->GetYaxis()->SetTitle("#LTd_{1} d_{2}#GT  - corr_{yy} [#mum^{2}]");
  TF1* fyy = new TF1(rn(), "[0]*x", -1, 1);
  fyy->SetParameter(0, syy);
  fyy->Draw("same");

  c->cd(3);
  profSxy->Draw();
  profSxy->GetXaxis()->SetTitle("-(sin #phi_{1} cos #phi_{2} + sin #phi_{2} cos #phi_{1})");
  profSxy->GetYaxis()->SetTitle("#LTd_{1} d_{2}#GT  - corr_{xy}  [#mum^{2}]");
  TF1* fxy = new TF1(rn(), "[0]*x", -1, 1);
  fxy->SetParameter(0, sxy);
  fxy->Draw("same");

  c->SaveAs("SizeFit.pdf");
}


// Plot zSizeFit control plots
void plotSpotZSizeFit(const vector<event>& evts, const spotParam& par, const vector<double>& sizesXY, double sizeZZ)
{

  gStyle->SetOptStat(0);


  TProfile* zzProfPhi = new TProfile(rn(), "", 100, -M_PI, M_PI);
  TProfile* zzProfXX  = new TProfile(rn(), "", 100, -M_PI / 4, 2 * M_PI);
  TProfile* zzProfYY  = new TProfile(rn(), "", 100, -M_PI / 4, 2 * M_PI);
  TProfile* zzProfXY  = new TProfile(rn(), "", 100, -2 * M_PI, 2 * M_PI);
  TProfile* zzProfXZ  = new TProfile(rn(), "", 100, -2 * M_PI, 2 * M_PI);
  TProfile* zzProfYZ  = new TProfile(rn(), "", 100, -2 * M_PI, 2 * M_PI);


  for (auto e : evts) {
    double z0 = getCorrZ(e.mu0, e.t, par);
    double z1 = getCorrZ(e.mu1, e.t, par);

    double corr = getZ12th(e, sizesXY);
    double z0z1Corr = z0 * z1 - corr;

    if (e.isSig) {

      double xx = e.mu0.tanlambda * e.mu1.tanlambda * cos(e.mu0.phi0) * cos(e.mu1.phi0);
      double yy = e.mu0.tanlambda * e.mu1.tanlambda * sin(e.mu0.phi0) * sin(e.mu1.phi0);
      double xy = e.mu0.tanlambda * e.mu1.tanlambda * (sin(e.mu0.phi0) * cos(e.mu1.phi0) + cos(e.mu0.phi0) * sin(e.mu1.phi0));
      double xz = - (e.mu0.tanlambda * cos(e.mu0.phi0) + e.mu1.tanlambda * cos(e.mu1.phi0));
      double yz = - (e.mu0.tanlambda * sin(e.mu0.phi0) + e.mu1.tanlambda * sin(e.mu1.phi0));


      zzProfPhi->Fill(e.mu0.phi0, z0z1Corr);
      zzProfPhi->Fill(e.mu1.phi0, z0z1Corr);
      zzProfXX->Fill(xx, z0z1Corr);
      zzProfYY->Fill(yy, z0z1Corr);
      zzProfXY->Fill(xy, z0z1Corr);
      zzProfXZ->Fill(xz, z0z1Corr);
      zzProfYZ->Fill(yz, z0z1Corr);
    }
  }

  TF1* f = new TF1(rn(), "[0]", -2 * M_PI, 2 * M_PI);
  f->SetParameter(0, sizeZZ);

  TCanvas* c = new TCanvas(rn(), "", 1200, 500);
  c->Divide(3, 2);
  c->cd(1);
  zzProfPhi->Draw();
  zzProfPhi->GetXaxis()->SetTitle("#phi_{0} [rad]");
  zzProfPhi->GetYaxis()->SetTitle("#LTz_{1} z_{2}#GT - corr [#mum^{2}]");
  f->Draw("same");

  c->cd(2);
  zzProfXX->Draw();
  zzProfXX->GetXaxis()->SetTitle("xx sensitive");
  zzProfXX->GetYaxis()->SetTitle("#LTz_{1} z_{2}#GT - corr [#mum^{2}]");
  f->Draw("same");

  c->cd(3);
  zzProfYY->Draw();
  zzProfYY->GetXaxis()->SetTitle("yy sensitive");
  zzProfYY->GetYaxis()->SetTitle("#LTz_{1} z_{2}#GT - corr [#mum^{2}]");
  f->Draw("same");

  c->cd(4);
  zzProfXY->Draw();
  zzProfXY->GetXaxis()->SetTitle("xy sensitive");
  zzProfXY->GetYaxis()->SetTitle("#LTz_{1} z_{2}#GT - corr [#mum^{2}]");
  f->Draw("same");

  c->cd(5);
  zzProfXZ->Draw();
  zzProfXZ->GetXaxis()->SetTitle("xz sensitive");
  zzProfXZ->GetYaxis()->SetTitle("#LTz_{1} z_{2}#GT - corr [#mum^{2}]");
  f->Draw("same");

  c->cd(6);
  zzProfYZ->Draw();
  zzProfYZ->GetXaxis()->SetTitle("yz sensitive");
  zzProfYZ->GetYaxis()->SetTitle("#LTz_{1} z_{2}#GT - corr [#mum^{2}]");
  f->Draw("same");

  c->SaveAs("SizeZFit.pdf");
}




void removeSpotSizeOutliers(vector<event>& evts, const spotParam& spotPar, const vector<double>& sizesXY, double cut = 1500)
{

  int nRem = 0;
  int nAll = 0;
  for (auto& e : evts) {
    if (!e.isSig) continue;

    double d0 = getCorrD(e.mu0, e.t, spotPar);
    double d1 = getCorrD(e.mu1, e.t, spotPar);
    double d12Th =  getD12th(e, sizesXY);

    e.isSig = abs(d0 * d1 - d12Th) < cut;
    nRem += !e.isSig;
    ++nAll;
  }
  //cout << "Removed fraction Size " << nRem / (nAll + 0.) << endl;
}


// Remove outliers in spotSize
void removeSpotSizeZOutliers(vector<event>& evts, const spotParam& spotPar, const vector<double>& sizesXY, double sizeZZ,
                             double cut = 150000)
{

  int nRem = 0;
  int nAll = 0;
  for (auto& e : evts) {
    if (!e.isSig) continue;

    double z0 = getCorrZ(e.mu0, e.t, spotPar);
    double z1 = getCorrZ(e.mu1, e.t, spotPar);

    double corr = getZ12th(e, sizesXY);
    double res = z0 * z1 - corr - sizeZZ;


    e.isSig = abs(res) < cut;
    nRem += !e.isSig;
    ++nAll;
  }
  cout << "Removed fraction Size " << nRem / (nAll + 0.) << endl;
}


// TRotation to TMatrixD
TMatrixD toMat(TRotation rot)
{
  TMatrixD rotM(3, 3);
  rotM(0, 0) = rot.XX();
  rotM(0, 1) = rot.XY();
  rotM(0, 2) = rot.XZ();
  rotM(1, 0) = rot.YX();
  rotM(1, 1) = rot.YY();
  rotM(1, 2) = rot.YZ();
  rotM(2, 0) = rot.ZX();
  rotM(2, 1) = rot.ZY();
  rotM(2, 2) = rot.ZZ();

  return rotM;
}

// Rotate the BeamSpot ellipsoid by angles kX and kY
TMatrixD getRotatedSizeMatrix(vector<double> xySize, double zzSize, double kX, double kY)
{
  TRotation rot; // rotation moving eZ=(0,0,1) to (kX, kY, 1)
  rot.RotateX(-kY); //x-rot
  rot.RotateY(+kX); //y-rot

  TMatrixD rotM = toMat(rot);
  TMatrixD rotMT = rotM; rotMT.T();

  TMatrixD eigenMat(3, 3); //z-rot included in eigenMat
  eigenMat(0, 0) = xySize[0];
  eigenMat(1, 1) = xySize[1];
  eigenMat(0, 1) = xySize[2];
  eigenMat(1, 0) = xySize[2];
  eigenMat(2, 2) = zzSize;

  return (rotM * eigenMat * rotMT);
}


// Get exp,run,evtNum from the time tEdge
ExpRunEvt getPosition(const vector<event>& events, double tEdge)
{
  ExpRunEvt evt(-1, -1, -1);
  double tBreak = -1e10;
  for (auto& e : events) {
    if (e.t < tEdge) {
      if (e.t > tBreak) {
        tBreak = e.t;
        evt =  ExpRunEvt(e.exp, e.run, e.evtNo);
      }
    }
  }
  return evt;
}



// convert splitPoints [in utc time] to expRunEvt
vector<ExpRunEvt> convertSplitPoints(const vector<event>& events, vector<double> splitPoints)
{

  vector<ExpRunEvt>  breakPos;
  for (auto p : splitPoints) {
    auto pos = getPosition(events, p);
    breakPos.push_back(pos);
  }
  return breakPos;
}



//Get map with runs and startTime,endTime
map<ExpRun, pair<double, double>> getRunInfo(const vector<event>& evts)
{
  map<ExpRun, pair<double, double>> runsInfo;

  for (auto& evt : evts) {
    int Exp = evt.exp;
    int Run = evt.run;
    double time = evt.t;
    //tracks->GetEntry(i);
    if (runsInfo.count(ExpRun(Exp, Run))) {
      double tMin, tMax;
      tie(tMin, tMax) = runsInfo.at(ExpRun(Exp, Run));
      tMin = min(tMin, time);
      tMax = max(tMax, time);
      runsInfo.at(ExpRun(Exp, Run)) = {tMin, tMax};
    } else {
      runsInfo[ExpRun(Exp, Run)] = {time, time};
    }

  }
  return runsInfo;
}





// Returns tuple with the beamspot parameters
tuple<vector<TVector3>, vector<TMatrixDSym>, TMatrixDSym>  runBeamSpotAnalysis(vector<event> evts,
    const vector<double>& splitPoints)
{
  const double xyPosLimit  = 70; //um
  const double xySize2Limit = pow(40, 2); //um^2
  const double zPosLimit   = 1200; //um


  vector<double> indX = splitPoints;
  vector<double> indY = splitPoints;
  vector<double> indZ = splitPoints;

  //no time detepndence, as for beam size
  vector<double> indKX =  {};
  vector<double> indKY =  {};

  unknownPars allPars, allParsZ;
  const int kPlot = -1; //do plots for index kPlot
  for (int k = 0; k < 1; ++k) { //loop over BootStrap replicas
    for (auto& e : evts) e.isSig = true; //reset cuts
    if (k != 0) bootStrap(evts);


    //simple XY pos fit
    auto resTemp = fitSpotPositionSplines(evts, indX, indY);

    if (k == kPlot) {
      plotSpotPositionFit(evts, resTemp, "positionFitSimpe");
      plotSpotPositionPull(evts, resTemp, "pullsPositionSimple",  xyPosLimit);
    }
    removeSpotPositionOutliers(evts, resTemp, xyPosLimit);

    //simple XY pos fit (with outliers removed)
    auto resFin = fitSpotPositionSplines(evts, indX, indY);
    if (k == kPlot) {
      plotSpotPositionFit(evts, resFin, "positionFitSimpleC");
      plotSpotPositionPull(evts, resFin, "pullsPositionSimpleC",  xyPosLimit);
      plotXYtimeDep(evts, resFin, "simplePosTimeDep");
    }

    //Z position fit
    auto resZmy = fitZpositionSplinesSimple(evts, indZ, resFin);
    if (k == kPlot) {
      plotSpotZPositionFit(evts, resZmy, "positionFitSimpleZ");
      plotSpotZpositionPull(evts, resZmy, "zPositionPull", zPosLimit);
    }

    removeSpotZpositionOutliers(evts,  resZmy, zPosLimit);

    //Z position fit (with outliers removed)
    resZmy = fitZpositionSplinesSimple(evts, indZ, resZmy);


    //complete XY pos fit
    auto resNew = fitSpotPositionSplines(evts, indX, indY, indKX, indKY, resZmy);
    if (k == kPlot) {
      plotSpotPositionFit(evts, resNew, "positionFitFull");
      plotKxKyFit(evts, resNew, "slopes");
    }

    //Z position fit (iteration 2)
    resZmy = fitZpositionSplinesSimple(evts, indZ, resNew);
    if (k == kPlot) plotSpotZPositionFit(evts, resZmy, "positionFitSimpleZLast");


    //complete XY pos fit (iteration 2)
    resNew = fitSpotPositionSplines(evts, indX, indY, indKX, indKY, resZmy);

    //XYZ pos fits (iteration 3)
    resZmy = fitZpositionSplinesSimple(evts, indZ, resNew);
    resNew = fitSpotPositionSplines(evts, indX, indY, indKX, indKY, resZmy);


    // fit of XY sizes (original + with outliers removed)
    auto vecXY = fitSpotWidthCMS(evts, resNew);
    if (k == kPlot) plotSpotSizePull(evts, resNew, vecXY);
    removeSpotSizeOutliers(evts, resNew, vecXY, xySize2Limit);
    vecXY = fitSpotWidthCMS(evts, resNew);
    if (k == kPlot) plotSpotSizeFit(evts, resNew,  vecXY);


    // fit of Z size
    double sizeZZ = fitSpotZwidth(evts, resNew, vecXY);

    if (k == kPlot) {
      plotSpotZSizeFit(evts, resNew, vecXY, sizeZZ);
      plotSpotSizeZPull(evts, resNew, vecXY,  sizeZZ);
    }

    //removeSpotSizeZOutliers(evts, resNew, vecXY, sizeZZ, 150000);
    //sizeZZ = fitSpotZwidth(evts, resNew, vecXY);

    allPars.add(resNew, sqrtS(vecXY[0]), sqrtS(vecXY[1]), sqrtS(vecXY[2]), sqrtS(sizeZZ));

  }

  //allPars.printStat();

  vector<TVector3> vtxPos;
  vector<TMatrixDSym> vtxErr;
  TMatrixDSym sizeMat;

  allPars.getOutput(vtxPos, vtxErr, sizeMat);

  return make_tuple(vtxPos, vtxErr, sizeMat);
}

