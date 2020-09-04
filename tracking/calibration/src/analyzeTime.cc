// Program to fit vertex parameters
// based on https://docs.belle2.org/record/1511/files/BELLE2-NOTE-TE-2019-018.pdf

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


#include "Math/GSLMinimizer.h"
#include "Math/Functor.h"
#include "Minuit2/Minuit2Minimizer.h"

#include <functional>
#include <vector>

#include <tracking/calibration/tools.h>
#include <tracking/calibration/analyzeTime.h>

using namespace std;

int run = -99;

//Time shift to reduce huge number
double runStart = -1;
double runEnd = -1;

inline double Sqr(double x) {return x >= 0 ? x * x : -x * x; }; //sign-sensitive sqr
inline double Sqrt(double x) {return x >= 0 ? sqrt(x) : -sqrt(-x); }; //sign-sensitive sqrt

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
  spotParam(vector<double> vals, vector<double> errs, vector<vector<double>> spls, int order = 0)
  {
    auto getSize = [order](vector<double>& sp) {
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

  /*
  void printStat(TString n)
  {
      cout <<n <<" : "<<  getMean()<< "+-" << getSigma() << " : " << getLimit(0.50) << " ("<< getLimit(0.16) <<" , "<< getLimit(1-0.16) <<" )" << endl;
  }

  vector<double> getStats()
  {
      return {getLimit(0.50), getLimit(0.16), getLimit(1-0.16)};
  }
  */
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
  double C = Sqr(SizeXY);
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
  return {Sqrt(Size2Min), Sqrt(Size2Max)};

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

    //return;
    /*
    x.add(sPar.x);
    y.add(sPar.y);
    kX.add(sPar.kX);
    kY.add(sPar.kY);
    dX.add(sPar.dX*3600);
    dY.add(sPar.dY*3600);
    */

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

    /*
    //Validate the angle
    TMatrixD mat(2,2);
    mat(0,0) = SizeX*SizeX;
    mat(1,1) = SizeY*SizeY;
    mat(0,1) = C;
    mat(1,0) = C;

    TVectorD eigVals(2);
    TMatrixD myMat = mat.EigenVectors(eigVals);
    cout << "My angle " << atan2(myMat(1,0), myMat(0,0)) << endl;
    cout << "extracted angle " << angle << endl;
    cout << "myEigVals " << Sqrt(SizeMin) << " "<< Sqrt(SizeMax) << endl;
    cout << "tabVals " << Sqrt(eigVals(0)) << " "<< Sqrt(eigVals(1)) << endl;
    exit(0);
    */


    //
    xyAngle.add(angle);

    //Get whole cov matrix
    TMatrixD matSize = getRotatedSizeMatrix({Sqr(SizeX), Sqr(SizeY), Sqr(SizeXY)}, Sqr(SizeZ), sPar.kX.val(0.5), sPar.kY.val(0.5));

    // Store elements in [um]
    matXX.add(Sqrt(matSize(0, 0)));
    matYY.add(Sqrt(matSize(1, 1)));
    matZZ.add(Sqrt(matSize(2, 2)));
    matXY.add(Sqrt(matSize(0, 1)));

    matXZ.add(Sqrt(matSize(0, 2)));
    matYZ.add(Sqrt(matSize(1, 2)));


    // crossing-angle in mrad
    double crossAngleVal = 1e3 * 2 * Sqrt(matSize(0, 0)) / Sqrt(matSize(2, 2));
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
      mS(0, 0) = Sqr(x.spls[0].errs[i] * toCm);
      mS(1, 1) = Sqr(y.spls[0].errs[i] * toCm);
      mS(2, 2) = Sqr(z.spls[0].errs[i] * toCm);

      vtxPos.push_back(vtx);
      vtxErr.push_back(mS);
    }

    //BeamSpot size matrix (from iteration 0)

    sizeMat.ResizeTo(3, 3);
    sizeMat(0, 0) = Sqr(matXX.vars[0] * toCm);
    sizeMat(1, 1) = Sqr(matYY.vars[0] * toCm);
    sizeMat(2, 2) = Sqr(matZZ.vars[0] * toCm);

    sizeMat(0, 1) = Sqr(matXY.vars[0] * toCm);
    sizeMat(0, 2) = Sqr(matXZ.vars[0] * toCm);
    sizeMat(1, 2) = Sqr(matYZ.vars[0] * toCm);

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
    //convert nodes to unix time
    for (auto& xx : spl->nodes)
      xx = (1 - xx) * runStart + xx * runEnd;

    T->Branch(n + "_nodes", &spl->nodes);
    T->Branch(n + "_vals",  &spl->vals);
    T->Branch(n + "_errs",  &spl->errs);
  }



  //save everything to TTree
  void save2tree(TString fName)
  {

    TTree* T = new TTree("runs", "beam conditions of runs");

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



// get estimate of the zIP position
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
  //cout << "Hope0 " <<setprecision(10)<<  getZIPest(tr, t, spotPar,0) << endl;
  //cout << "Hope1 " << getZIPest(tr, t, spotPar,1) << endl;
  //cout << "Hope2 " << getZIPest(tr, t, spotPar,2) << endl;
  //cout << "Hope3 " << getZIPest(tr, t, spotPar,3) << endl;

  /*
  cout << "Spline test" << endl;
  cout << spotPar.x.val(0) <<" "<< spotPar.x.val(1) << endl;
  cout << spotPar.x.val(0.3) <<" "<< spotPar.x.val(0.5) << endl;
  cout << spotPar.x.vals[0] <<" "<< spotPar.x.vals[1] << endl;
  cout << endl;
  exit(0);
  */


  double x0 = spotPar.x.val(t) + spotPar.kX.val(t) * (zIP - spotPar.z.val(t));
  double y0 = spotPar.y.val(t) + spotPar.kY.val(t) * (zIP - spotPar.z.val(t));

  double f0 = x0 * sin(tr.phi0) - y0 * cos(tr.phi0);

  return (tr.d0 - f0);
}

//Transform D0 to time t=0.5
double getDtimeConst(const track& tr, double t, const spotParam& spotPar)
{
  double zIP  =  getZIPest(tr, t, spotPar);
  double zIPM =  getZIPest(tr, 0.5, spotPar);

  double x0 = spotPar.x.val(t) + spotPar.kX.val(t) * (zIP - spotPar.z.val(t));
  double y0 = spotPar.y.val(t) + spotPar.kY.val(t) * (zIP - spotPar.z.val(t));

  double xM = spotPar.x.val(0.5) + spotPar.kX.val(0.5) * (zIPM - spotPar.z.val(0.5));
  double yM = spotPar.y.val(0.5) + spotPar.kY.val(0.5) * (zIPM - spotPar.z.val(0.5));


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

  //To relative time (0 is start, 1 is end)
  double start, stop;
  tie(start, stop) = getStartStop(events);
  for (auto& e : events) {
    e.t = (e.tAbs - start) / (stop - start);
    //e.tAbs /= 1e9;
  }
  events[0].t += 1e-11;
  events.back().t -= 1e-11;

  //file->Close();

  runStart = start;
  runEnd   = stop;


  return events;
}

// Add random booth strp weights to events
void bootStrap(vector<event>& evts)
{
  for (auto& e : evts)
    e.nBootStrap = gRandom->Poisson(1);
}


// get center time of the evnt sample
double getAvgTime(const vector<event>&  evts)
{
  double minT = 1e20, maxT = -1e20;
  for (auto ev : evts) {
    minT = min(minT, ev.t);
    maxT = max(maxT, ev.t);
  }
  return (minT + maxT) / 2;
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
  //mat.Print();

  mat.Invert();
  TMatrixD A = mat * mT;
  TVectorD res = A * r;
  TVectorD dataH = m * res;

  cout << "Mean residuals " << (dataH - r).Sum() / r.GetNrows() << endl;
  //exit(0);

  //errs
  double err2 = (dataH - r).Norm2Sqr() / (r.GetNrows() - res.GetNrows());
  //double err2 = (dataH-r).Norm2Sqr() / r.GetNrows();// - res.GetNrows());
  //cout << "Train AvgErr2 " << err2 << endl;


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
  if (pars[0] >= 0 && pars[1] >= 0 && pars[0]*pars[1] >= pars[2]*pars[2])
    return pars;

  //////////////////////////
  //Get the error matrix
  //////////////////////////

  int nDf = r.GetNrows() - 3;
  //Calculate average error
  double err2 = (mat * pars - r).Norm2Sqr() / nDf;

  cout << "err2 " << sqrt(err2) << endl;
  TMatrixD wMat = Ainv * matT;
  TMatrixD wMatT = wMat; wMatT.T();

  TMatrixD covMat = err2 * wMat * wMatT;
  TMatrixD covMatI = covMat;
  covMatI.Invert();

  double Norm = 1. / (pow(2 * M_PI, 3 / 2.) *  sqrt(covMat.Determinant()));


  //////////////////////////
  //Get maximum likelihood
  //////////////////////////



  TF2* f = new TF2(rn(), [Norm, covMatI, pars](double * x, double*) {
    TVectorD xVec(3); xVec(0) = x[0]; xVec(1) = x[1];

    xVec(2) = +sqrt(x[0] * x[1]);
    double r1 = covMatI.Similarity(xVec - pars);
    xVec(2) = -sqrt(x[0] * x[1]);
    double r2 = covMatI.Similarity(xVec - pars);

    return min(r1, r2);
  }, 0, 250, 0, 100, 0);

  double xx, yy, xy;
  f->GetMinimumXY(xx, yy);
  delete f;
  cout << "Helenka " << xx << " " << yy << endl;
  //f->Draw("colz");

  //////////////////////////
  //Is it the positive or negative leave?
  //////////////////////////

  TVectorD xVec(3); xVec(0) = xx; xVec(1) = yy; xVec(2) = sqrt(xx * yy);
  double rP = covMatI.Similarity(xVec - pars);
  xVec(2) = -sqrt(xx * yy);
  double rN = covMatI.Similarity(xVec - pars);


  if (rP < rN) xy = +sqrt(xx * yy);
  else        xy = -sqrt(xx * yy);


  pars(0) = xx;
  pars(1) = yy;
  pars(2) = xy;
  return pars;


  /*
  pars.Print();
  covMat.Print();

  //If solution is not in the domain, scan the borders
  //TODO

  return pars;
  */
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
TH1D* getMean(TH2D* hRes)
{
  TH1D* hMean = new TH1D(rn(), "", 50, -M_PI, M_PI);
  for (int i = 1; i <= hRes->GetNbinsX(); ++i) {
    TH1D* hProj = hRes->ProjectionY(rn(), i, i);
    double mean    = hProj->GetMean();
    double meanErr = hProj->GetMeanError();
    hMean->SetBinContent(i,   mean); //from cm2 to um2
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
  //double sxy = sizesXY[2];

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
  f->SetParameters(par.x.val(0.5), par.y.val(0.5));

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
  //dProfRes->SetMaximum(+1.3* f->GetMaximum());
  //dProfRes->SetMinimum(-1.3* f->GetMaximum());

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

    //double z1ipM = getZIPest(e.mu0, 0.5, par);
    //double z2ipM = getZIPest(e.mu1, 0.5, par);

    double zipT = par.z.val(e.t);
    double zipM = par.z.val(0.5);

    double val1 = z1ip - (zipT - zipM);
    double val2 = z2ip - (zipT - zipM);

    gr->SetPoint(gr->GetN(), e.mu0.phi0, val1);
    gr->SetPoint(gr->GetN(), e.mu1.phi0, val2);

    zProf->Fill(e.mu0.phi0, val1);
    zProf->Fill(e.mu1.phi0, val2);
  }
  TF1* f = new TF1(rn(), "[0]", -M_PI, M_PI);
  f->SetParameter(0, par.z.val(0.5));

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


  //gr->Fit(f);
}





//Plot pull distribution
void plotSpotPositionPull(const vector<event>& evts, spotParam par, TString fName, double cut = 70)
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

    //cout <<"Radek " << zDiff1 <<" "<< d1 - d1KX << " : " << (d1-d1KX)/(zDiff1*sin(e.mu0.phi0))  << endl;

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
  f->SetParameter(0, par.kX.val(0.5));
  f->Draw("same");

  cX->SaveAs(fName + "_kX.pdf");

  TCanvas* cY = new TCanvas(rn(), "");
  gStyle->SetOptStat(0);
  profResKy->Draw();

  profResKy->GetXaxis()->SetTitle("-(z_{IP} - z_{IP}^{0}) cos #phi_{0} [#mum]");
  profResKy->GetYaxis()->SetTitle("d_{0} res [#mum]");

  f->SetParameter(0, par.kY.val(0.5));
  f->Draw("same");

  cY->SaveAs(fName + "_kY.pdf");


}

//Plot pull distribution
void plotXYtimeDep(const vector<event>& evts, spotParam par, TString fName)
{
  TProfile* profRes    = new TProfile(rn(), "dProf", 50, -0.5, 0.5);
  TProfile* profResTx  = new TProfile(rn(), "dProfTx", 50, -0.5, 0.5);
  TProfile* profResTy  = new TProfile(rn(), "dProfTy", 50, -0.5, 0.5);

  spotParam parNoTx = par;
  spotParam parNoTy = par;
  parNoTx.x.nodes = {};
  parNoTx.x.vals  = {par.x.val(0.5)};
  parNoTy.y.nodes = {};
  parNoTy.y.vals  = {par.y.val(0.5)};

  for (auto& e : evts) {
    if (!e.isSig) continue;

    double tDiff = (e.t - 0.5);
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
  //profRes->Draw();

  //profResKx->GetXaxis()->SetTitle("(z_{IP} - z_{IP}^{0}) sin #phi_{0} [#mum]");
  //profResKx->GetYaxis()->SetTitle("d_{0} res [#mum]");

  TF1* f = new TF1(rn(), "[0]*x", -1, 1);
  f->SetParameter(0, (par.x.val(1) - par.x.val(0)));
  f->Draw("same");
  //profResTx->Fit(f);
  cout << "Table value " << par.x.val(1) - par.x.val(0) << endl;

  cX->SaveAs(fName + "_tX.pdf");

  /*
  TCanvas *cY = new TCanvas(rn(), "");
  gStyle->SetOptStat(0);
  profResKy->Draw();

  profResKy->GetXaxis()->SetTitle("-(z_{IP} - z_{IP}^{0}) cos #phi_{0} [#mum]");
  profResKy->GetYaxis()->SetTitle("d_{0} res [#mum]");

  f->SetParameter(0, par.kY.val(0.5));
  f->Draw("same");

  cY->SaveAs(fName+"_kY.pdf");
  */


}





//Plot pull distribution
void plotSpotZpositionPull(const vector<event>& evts, spotParam par, TString fName, double cut = 1000)
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
void removeSpotPositionOutliers(vector<event>& evts,  spotParam par, double cut = 70)
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
  cout << "Removed fraction Position " << nRem / (nAll + 0.) << endl;
}


// Remove outliear from the position fit (for z0)
void removeSpotZpositionOutliers(vector<event>& evts,  spotParam par, double cut = 1000)
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
  cout << "Removed fraction Position " << nRem / (nAll + 0.) << endl;
}



//Fill matrix with bases based on linear splines
vector<vector<double>> fillSplineBasesLinear(const vector<event>& evts, vector<double> spl,
                                             std::function<double(track, double)> fun)
{
  int n = spl.size(); //number of params
  if (n == 0)

    if (n == 2 && spl[0] > spl[1])
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

      //cout << "Helenka "<< k<<":" <<xLow <<" "<< xCnt <<" "<< xHigh << endl;

      for (const auto& e : evts) {
        double x = e.t;// - start) / (stop-start);
        double v = 0;
        if (xLow <= x && x < xCnt)
          v = (x - xLow) / (xCnt - xLow);
        else if (xCnt < x && x <= xHigh)
          v = (xHigh - x) / (xHigh - xCnt);


        for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
          //if(!isfinite(v*fun1(e))) cout << "Problem " << v <<" "<< x <<" "<< xLow <<" "<<xHigh <<" "<< xCnt - xLow <<" "<< xHigh-xCnt<< endl;
          //if(!isfinite(v*fun2(e))) cout << "Problem " << v << endl;

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
        double x = e.t;// - start) / (stop-start);
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
double compareSplines(Spline spl1, Spline spl2)
{
  double sum = 0;
  //double Max = 0;

  double step = 0.001;
  for (double x = 0; x <= 1 + step / 2; x += step) {
    double v1 = spl1.val(x);
    double e1 = spl1.err(x);
    double v2 = spl2.val(x);
    double e2 = spl2.err(x);

    //double d = pow(v2-v1,2) / (e1*e1 + e2*e2);
    double d = pow(v2 - v1, 2) / pow(max(e1, e2), 2);
    sum += d * step;
    //Max = max(Max, d);
  }
  return sum;
}

// Fit width in z-direction
double fitSpotZwidth(const vector<event>& evts, spotParam spotPar, vector<double> sizesXY)
{

  vector<double> dataVec;
  vector<double> zzVec;
  //vector<double> xxVec, yyVec, xyVec, xzVec, yzVec, zzVec, zzTVec;

  //TProfile *zProf  = new TProfile(rn(), "dProf", 100, -M_PI, M_PI, "S");
  //TProfile *zzProf = new TProfile(rn(), "dProf", 100, -M_PI, M_PI, "S");
  //TProfile *zProfRes = new TProfile(rn(), "dProf", 100, -M_PI, M_PI, "S");


  for (auto e : evts) {
    double z0 = getCorrZ(e.mu0, e.t, spotPar);
    double z1 = getCorrZ(e.mu1, e.t, spotPar);

    //double corr = e.mu0.tanlambda*e.mu1.tanlambda * (sxx*cos(e.mu0.phi0)*cos(e.mu1.phi0) + syy*sin(e.mu0.phi0)*sin(e.mu1.phi0) +
    //+  (sin(e.mu0.phi0)*cos(e.mu1.phi0) + cos(e.mu0.phi0)*sin(e.mu1.phi0)) );

    double corr = getZ12th(e, sizesXY);
    double z0z1Corr = z0 * z1 - corr;

    /*
    if(e.isSig) {
        zProfRes->Fill(e.mu0.phi0, (z0-z1)/sqrt(2));
        zProfRes->Fill(e.mu1.phi0, (z1-z0)/sqrt(2));
        zProf->Fill(e.mu0.phi0, z0);
        zProf->Fill(e.mu1.phi0, z1);
        zzProf->Fill(e.mu0.phi0, z0z1Corr);
        zzProf->Fill(e.mu1.phi0, z0z1Corr);
    }
    */

    for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
      dataVec.push_back(z0z1Corr);

      /*
      double xx = e.mu0.tanlambda * e.mu1.tanlambda * cos(e.mu0.phi0)*cos(e.mu1.phi0);
      double yy = e.mu0.tanlambda * e.mu1.tanlambda * sin(e.mu0.phi0)*sin(e.mu1.phi0);
      double xy = e.mu0.tanlambda * e.mu1.tanlambda * (sin(e.mu0.phi0)*cos(e.mu1.phi0) + cos(e.mu0.phi0)*sin(e.mu1.phi0));
      double xz = - (e.mu0.tanlambda*cos(e.mu0.phi0) + e.mu1.tanlambda*cos(e.mu1.phi0));
      double yz = - (e.mu0.tanlambda*sin(e.mu0.phi0) + e.mu1.tanlambda*sin(e.mu1.phi0));

      xxVec.push_back(xx);
      yyVec.push_back(yy);
      xyVec.push_back(xy);
      xzVec.push_back(xz);
      yzVec.push_back(yz);
      */
      zzVec.push_back(1);
      //zzTVec.push_back(e.t-0.5);
    }
  }

  /*
  double eTotSum = 0, eSpotSum = 0;
  for(int i = 1; i <= zProf->GetNbinsX(); ++i) {
      double eTot = zProf->GetBinError(i);
      double eRes = zProfRes->GetBinError(i);
      eTotSum  += eTot;
      eSpotSum += sqrt(eTot*eTot - eRes*eRes);
      cout << i <<" "<< eTot <<" "<< eRes <<" " << sqrt(eTot*eTot - eRes*eRes) << endl;
  }
  eTotSum /= zProf->GetNbinsX();
  eSpotSum /= zProf->GetNbinsX();
  cout << "HelenkaHelenka " << eTotSum <<" "<< eSpotSum << endl;
  cout << "Done"  << endl;
  */

  TMatrixD mat = vecs2mat({zzVec /*, zzTVec  , xzVec, yzVec, xxVec, yyVec, xyVec*/});

  //TVectorD resPhys = linearFit(mat, vec2vec(dataVec));

  vector<double> pars, err2;
  double err2Mean, err2press, err2pressErr;
  tie(pars, err2) = linearFitErr(mat, vec2vec(dataVec), err2Mean, err2press, err2pressErr);

  for (auto p : pars) cout << "RadekP " << Sqrt(p) << endl;
  for (auto e : err2) cout << "RadekE " << Sqrt(e) / (2 * sqrt(pars[0])) << endl;


  /*
  TCanvas *c = new TCanvas(rn(), "", 600, 600);
  zzProf->Draw();
  TF1 * f = new TF1("f", "[0]", -M_PI, M_PI);
  f->SetParameter(0, pars[0]);
  f->Draw("same");

  c->SaveAs("zSizeFit.pdf");
  */

  //resPhys.Print();
  //exit(0);


  return pars[0];

  /*
  double xSizeMy  = Sqrt(resPhys(0));
  double ySizeMy  = Sqrt(resPhys(1));
  double xySizeMy = Sqrt(resPhys(2));

  //cout << "xRadek " << xSize <<" "<< xSizeMy << endl;
  //cout << "yRadek " << ySize <<" "<< ySizeMy << endl;
  //cout << "xyRadek " << xySize <<" "<< xySizeMy << endl;

  return  {xSizeMy, ySizeMy, xySizeMy};
  */
}






// Fit xy widths (including XZ, YZ slopes), no prior
spotParam fitSpotPositionSplines(const vector<event>& evts, vector<double> splX, vector<double> splY, vector<double> splKX,
                                 vector<double> splKY)
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
spotParam fitSpotPositionSplines(const vector<event>& evts, vector<double> splX, vector<double> splY, vector<double> splKX,
                                 vector<double> splKY, spotParam spotPars)
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


// fit position splines, with given number of points
spotParam fitSpotPositionSplines(const vector<event>& evts, int nX, int nY, int nKX, int nKY, spotParam pars)
{
  auto rX  = getRangeZero(nX,  0, 1);
  auto rY  = getRangeZero(nY,  0, 1);
  auto rKX = getRangeZero(nKX, 0, 1);
  auto rKY = getRangeZero(nKY, 0, 1);

  return fitSpotPositionSplines(evts, rX, rY, rKX, rKY, pars);
}





// simple fit of position splines, without kX, kY
spotParam fitSpotPositionSplines(const vector<event>& evts, vector<double> splX, vector<double> splY)
{
  vector<vector<double>> basesX  = fillSplineBasesZero(evts, splX, [](track tr, double) {return  sin(tr.phi0);});
  vector<vector<double>> basesY  = fillSplineBasesZero(evts, splY, [](track tr, double) {return -cos(tr.phi0);});

  /*
  cout << "Test Test Radek " << basesX.size() << endl;

  for(int i = 0; i < basesX[0].size(); ++i) {
      cout << basesX[0][i] << " "<< basesX[1][i] << endl;
  }

  for(auto e : evts) {
    cout << e.t << endl;
  }
  exit(0);
  */

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
  tie(pars, err2) = linearFitErr(A, vData, err2Mean, err2press, err2pressErr);
  cout << "Mean err xyfit " << fixed << setprecision(3) << err2Mean << " : " << err2press << " " << err2pressErr << " :  " <<
       err2press - err2Mean << endl;

  for (auto& e : err2) e = sqrt(e);
  return spotParam(pars, err2, {splX, splY});
}




spotParam fitSpotPositionSplines(const vector<event>& evts, int nX, int nY, int nKX, int nKY)
{
  auto rX  = getRangeZero(nX,  0, 1);
  auto rY  = getRangeZero(nY,  0, 1);
  auto rKX = getRangeZero(nKX, 0, 1);
  auto rKY = getRangeZero(nKY, 0, 1);

  return fitSpotPositionSplines(evts, rX, rY, rKX, rKY);
}

spotParam fitSpotPositionSplines(const vector<event>& evts, int nX, int nY)
{
  auto rX  = getRangeZero(nX,  0, 1);
  auto rY  = getRangeZero(nY,  0, 1);

  return fitSpotPositionSplines(evts, rX, rY);
}





//Fit Zposition
spotParam fitZpositionSplines(const vector<event>& evts, vector<double> splX, vector<double> splY,  vector<double> splKX,
                              vector<double> splKY, vector<double> splZ)
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



spotParam fitZpositionSplines(const vector<event>& evts, int nX, int nY, int nKX, int nKY, int nZ)
{
  auto rX  = getRangeZero(nX,  0, 1);
  auto rY  = getRangeZero(nY,  0, 1);
  auto rKX = getRangeZero(nKX, 0, 1);
  auto rKY = getRangeZero(nKY, 0, 1);
  auto rZ  = getRangeZero(nZ,  0, 1);

  return fitZpositionSplines(evts, rX, rY, rKX, rKY, rZ);
}


//Fit Zposition, xIP, yIP fixed from d0 fit
spotParam fitZpositionSplinesSimple(const vector<event>& evts, vector<double> splZ, spotParam spotPars)
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
  cout << "Mean err zfit " << sqrt(err2Mean) << endl;

  for (auto& e : err2) e = sqrt(e);

  spotParam parsUpd = spotPars;
  parsUpd.z.vals = pars;
  parsUpd.z.errs = err2;
  parsUpd.z.nodes = splZ;

  return parsUpd;
}





/*
//Fit Z-sizes
spotParam fitZsizesSplines(const vector<event> &evts, spotParam pars)
{
    vector<vector<double>> basesX  = fillSplineBases(evts, splX, [](track tr){return -tr.tanlambda * cos(tr.phi0);} );
    vector<vector<double>> basesY  = fillSplineBases(evts, splY, [](track tr){return -tr.tanlambda * sin(tr.phi0);} );

    vector<vector<double>> basesKX = fillSplineBases(evts, splKX, [](track tr){return -tr.z0*tr.tanlambda * cos(tr.phi0);} );
    vector<vector<double>> basesKY = fillSplineBases(evts, splKY, [](track tr){return -tr.z0*tr.tanlambda * sin(tr.phi0);} );

    vector<vector<double>> basesZ  = fillSplineBases(evts, splZ,  [](track tr){return 1;} );


    vector<double> dataVec;
    for(auto e : evts) {
        for(int i = 0; i < e.nBootStrap*e.isSig; ++i) {
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

    for(auto &e : err2) e = sqrt(e);
    return spotParam(pars, err2, {splX, splY, splKX, splKY, splZ});
}
*/








// Scan over the negative index
int scanOrder(const vector<event>&  evts, int iX, int iY)
{
  TString type;
  int* indx = nullptr;
  if (iX < 0) {
    indx = &iX;
    type = "X";
  } else if (iY < 0) {
    indx = &iY;
    type = "Y";
  } else {
    cout << "Nothing found " << endl;
    exit(1);
  }
  assert(*indx < 0);

  //cout << "Type is " << type << endl;
  vector<spotParam> spotPars;

  int nMax = abs(*indx);
  for (*indx = 1; *indx <= nMax; ++(*indx)) {
    cout << "Fitting order " << *indx << endl;
    auto resTemp = fitSpotPositionSplines(evts, iX, iY);
    spotPars.push_back(resTemp);
    //resTemp.x.print();
  }


  int indOpt = 1; //time-independent function

  //index starts at 0
  for (int n = spotPars.size() - 1; n >= 1; --n) {
    double errMin = 1e10;
    for (int k = 0; k < n; ++k) {
      double err = 0;
      if (type == "X")  err = compareSplines(spotPars[n].x, spotPars[k].x);
      if (type == "Y")  err = compareSplines(spotPars[n].y, spotPars[k].y);
      //if(type == "KX") err = compareSplines(spotPars[n].kX, spotPars[k].kX);
      //if(type == "KY") err = compareSplines(spotPars[n].kY, spotPars[k].kY);
      errMin = min(errMin, err);
    }

    if (errMin > 1.1) {
      indOpt = n + 1;
      //break;
    }
    cout << "Helenka " << n + 1 << " " << errMin << endl;
  }
  return indOpt;
}


//x-y sizes in um^2
vector<double> fitSpotWidthCMS(vector<event> evts, spotParam spotPar)
{

  vector<double> dataVec, cDiffVec, cSumVec, sSumVec;
  vector<double> ccVec, ssVec, scVec;


  for (auto e : evts) {
    double d0 = getCorrD(e.mu0, e.t, spotPar);
    double d1 = getCorrD(e.mu1, e.t, spotPar);

    for (int i = 0; i < e.nBootStrap * e.isSig; ++i) {
      dataVec.push_back(d0 * d1);
      //cDiffVec.push_back(cos(e.mu0.phi0 - e.mu1.phi0));
      //cSumVec.push_back (cos(e.mu0.phi0 + e.mu1.phi0));
      //sSumVec.push_back (sin(e.mu0.phi0 + e.mu1.phi0));

      ccVec.push_back(cos(e.mu0.phi0)*cos(e.mu1.phi0));
      ssVec.push_back(sin(e.mu0.phi0)*sin(e.mu1.phi0));
      scVec.push_back(-(sin(e.mu0.phi0)*cos(e.mu1.phi0) + sin(e.mu1.phi0)*cos(e.mu0.phi0)));
    }
  }

  cout << "Done"  << endl;

  TMatrixD mat = vecs2mat({ssVec, ccVec, scVec});

  TVectorD resPhys = linearFitPos(mat, vec2vec(dataVec));

  //double xSizeMy  = Sqrt(resPhys(0));
  //double ySizeMy  = Sqrt(resPhys(1));
  //double xySizeMy = Sqrt(resPhys(2));

  //return  {xSizeMy, ySizeMy, xySizeMy};
  return {resPhys(0), resPhys(1), resPhys(2)};
}


void plotSpotSizePull(const vector<event>& evts, spotParam spotPar, vector<double> sizesXY)
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

void plotSpotSizeZPull(const vector<event>& evts, spotParam spotPar, vector<double> sizesXY,  double sizeZZ)
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
  cout << "Helenka mean " << hPull->GetMean() << endl;
  cout << "Helenka rms " << hPull->GetRMS() << endl;

  c->SaveAs("pullsZSize.pdf");
}





void plotSpotSizeFit(const vector<event>& evts, spotParam par, vector<double> sizeXY)
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
    //double d12Th =  getD12th(e, sizesXY);

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


void plotSpotZSizeFit(const vector<event>& evts, spotParam par, vector<double> sizesXY, double sizeZZ)
{
  //double sxx = sizeXY[0];
  //double syy = sizeXY[1];
  //double sxy = sizeXY[2];

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

    //double corr = e.mu0.tanlambda*e.mu1.tanlambda * (sxx*cos(e.mu0.phi0)*cos(e.mu1.phi0) + syy*sin(e.mu0.phi0)*sin(e.mu1.phi0) +
    //+  (sin(e.mu0.phi0)*cos(e.mu1.phi0) + cos(e.mu0.phi0)*sin(e.mu1.phi0)) );

    double corr = getZ12th(e, sizesXY);
    double z0z1Corr = z0 * z1 - corr;

    if (e.isSig) {
      //zProfRes->Fill(e.mu0.phi0, (z0-z1)/sqrt(2));
      //ProfRes->Fill(e.mu1.phi0, (z1-z0)/sqrt(2));

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






void removeSpotSizeOutliers(vector<event>& evts, spotParam spotPar, vector<double> sizesXY, double cut = 1500)
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
  cout << "Removed fraction Size " << nRem / (nAll + 0.) << endl;
}


void removeSpotSizeZOutliers(vector<event>& evts, spotParam spotPar, vector<double> sizesXY, double sizeZZ, double cut = 150000)
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


ExpRunEvt getPostion(const vector<event>& events, double tRel)
{
  ExpRunEvt evt(-1, -1, -1);
  double tBreak = -1e10;
  for (auto& e : events) {
    if (e.t < tRel) {
      if (e.t > tBreak) {
        tBreak = e.t;
        evt =  ExpRunEvt(e.exp, e.run, e.evtNo);
      }
    }
  }
  return evt;

}

//splitPoints should be sorted in time
vector<ExpRunEvt> convertSplitPoints(const vector<event>& events, vector<double> splitPoints)
{

  vector<ExpRunEvt>  breakPos;
  for (auto p : splitPoints) {
    auto pos = getPostion(events, p);
    breakPos.push_back(pos);
  }
  return breakPos;
}


map<pair<int, int>, pair<double, double>> getRunInfo(const vector<event>& evts)
{
  map<pair<int, int>, pair<double, double>> runsInfo;

  for (auto& evt : evts) {
    int Exp = evt.exp;
    int Run = evt.run;
    double time = evt.t;
    //tracks->GetEntry(i);
    if (runsInfo.count({Exp, Run})) {
      double tMin, tMax;
      tie(tMin, tMax) = runsInfo.at({Exp, Run});
      tMin = min(tMin, time);
      tMax = max(tMax, time);
      runsInfo.at({Exp, Run}) = {tMin, tMax};
    }
    else {
      runsInfo[ {Exp, Run}] = {time, time};
    }

  }
  return runsInfo;
}





pair<double, double> getMinMaxTime(const vector<event>& evts)
{
  double tMin = 1e40, tMax = -1e40;
  for (const auto& evt : evts) {
    double time = evt.tAbs;
    tMin = min(tMin, time);
    tMax = max(tMax, time);
  }
  return {tMin, tMax};
}







// Returns tuple with the beamspot parameters
tuple<vector<TVector3>, vector<TMatrixDSym>, TMatrixDSym>  runBeamSpotAnalysis(vector<event> evts, vector<double> splitPoints)
{
  vector<double> indX = splitPoints;
  vector<double> indY = splitPoints;
  vector<double> indZ = splitPoints;

  //no time detepndence, as for beam size
  vector<double> indKX =  {};
  vector<double> indKY =  {};

  unknownPars allPars, allParsZ;
  const int kPlot = -1;
  for (int k = 0; k < 1; ++k) {
    cout << "Radek " << k << endl;
    for (auto& e : evts) e.isSig = true; //reset cuts
    if (k != 0) bootStrap(evts);

    cout << "Iteration numberr " << k << endl;

    //auto resTemp = fitSpotPositionSplines(evts, indX, indY, indKX, indKY);
    auto resTemp = fitSpotPositionSplines(evts, indX, indY);


    if (k == kPlot) plotSpotPositionFit(evts, resTemp, "positionFitSimpe");
    if (k == kPlot) plotSpotPositionPull(evts, resTemp, "pullsPositionSimple",  70);
    removeSpotPositionOutliers(evts, resTemp, 70);
    //auto resOld = fitSpotPositionSplines(evts, indX, indY, indKX, indKY);
    auto resFin = fitSpotPositionSplines(evts, indX, indY);
    if (k == kPlot) plotSpotPositionFit(evts, resFin, "positionFitSimpleC");
    if (k == kPlot) plotSpotPositionPull(evts, resFin, "pullsPositionSimpleC",  70);

    if (k == kPlot) plotXYtimeDep(evts, resFin, "simplePosTimeDep");


    //vector<double> splZ = indZ; //getRangeZero(indZ, 0., 1.);//   (indZ == 2) ? vector<double>({0,1}) : vector<double>({1, 0});

    auto resZmy = fitZpositionSplinesSimple(evts, indZ, resFin);
    if (k == kPlot) plotSpotZPositionFit(evts, resZmy, "positionFitSimpleZ");
    if (k == kPlot) plotSpotZpositionPull(evts, resZmy, "zPositionPull", 1000);

    removeSpotZpositionOutliers(evts,  resZmy, 1000);
    resZmy = fitZpositionSplinesSimple(evts, indZ, resZmy);

    //resTemp.print();
    //exit(0);


    auto resNew = fitSpotPositionSplines(evts, indX, indY, indKX, indKY, resZmy);
    if (k == kPlot) plotSpotPositionFit(evts, resNew, "positionFitFull");


    //cout << "Fit with K " << endl;
    //resNew.print();
    if (k == kPlot) plotKxKyFit(evts, resNew, "slopes");


    resZmy = fitZpositionSplinesSimple(evts, indZ, resNew);
    if (k == kPlot) plotSpotZPositionFit(evts, resZmy, "positionFitSimpleZLast");

    //exit(0);

    resNew = fitSpotPositionSplines(evts, indX, indY, indKX, indKY, resZmy);

    //cout << "Fit with K iter" << endl;
    //resNew.print();

    resZmy = fitZpositionSplinesSimple(evts, indZ, resNew);
    resNew = fitSpotPositionSplines(evts, indX, indY, indKX, indKY, resZmy);

    //if(k==0) plotXYtimeDep(evts, resNew, "simplePosTimeDepUpdated");

    //cout << "Fit with K iter-iter" << endl;
    //resNew.print();



    //auto resZ = fitZpositionSplines(evts, indX, indY, indKX, indKY, indZ);
    //if(k==0)plotSpotZpositionPull(evts, resZ, 1000);
    //removeSpotZpositionOutliers(evts,  resZ, 1000);
    //auto resZfin = fitZpositionSplines(evts, indX, indY, indKX, indKY, indZ);

    auto vecXY = fitSpotWidthCMS(evts, resNew);
    if (k == kPlot) plotSpotSizePull(evts, resNew, vecXY);
    removeSpotSizeOutliers(evts, resNew, vecXY, 1500);
    vecXY = fitSpotWidthCMS(evts, resNew);
    if (k == kPlot) plotSpotSizeFit(evts, resNew,  vecXY);


    double sizeZZ = fitSpotZwidth(evts, resNew, vecXY);

    if (k == kPlot) plotSpotZSizeFit(evts, resNew, vecXY, sizeZZ);
    if (k == kPlot) plotSpotSizeZPull(evts, resNew, vecXY,  sizeZZ);


    //removeSpotSizeZOutliers(evts, resNew, vecXY, sizeZZ, 150000);
    //sizeZZ = fitSpotZwidth(evts, resNew, vecXY);

    allPars.add(resNew, Sqrt(vecXY[0]), Sqrt(vecXY[1]), Sqrt(vecXY[2]), Sqrt(sizeZZ));

    //return;

    /*
    allPars.add(resFin,1, 1, 0);
    allParsZ.add(resZfin, 1, 1, 0);
    */

    //xUn.add(resTemp.x);
    //yUn.add(resTemp.y);
    //resFin.x.print();
  }
  //xUn.getMeanSigma().print();
  //yUn.getMeanSigma().print();

  allPars.printStat();
  //allParsZ.printStat();
  //allPars.save2tree(outFile);

  vector<TVector3> vtxPos;
  vector<TMatrixDSym> vtxErr;
  TMatrixDSym sizeMat;

  allPars.getOutput(vtxPos, vtxErr, sizeMat);
  cout << "Marcela " << __LINE__ << endl;

  return make_tuple(vtxPos, vtxErr, sizeMat);
}

//TString inFile = "data/ntuple1797.root"
//"data/ntupleMChuge.root"
void analyzeTime(TString inFile = "data/ntuple1797.root", TString outFile = "data/out.root")
{

//TMatrixD getRotatedSizeMatrix(vector<double> xySize, double zzSize, double kX, double kY)
  //getRotatedSizeMatrix({10*10, 1*1, 1*7} , 250*250, -0.027, -0.009);//.Print();
  //return;

  /*
  cout <<  "angleX  " << getAngle(Sqrt(resM(2,2)), Sqrt(resM(0,0)), Sqrt(resM(2,0))) << endl;
  cout <<  "angleY  " << getAngle(Sqrt(resM(2,2)), Sqrt(resM(1,1)), Sqrt(resM(2,1))) << endl;
  cout <<  "angleXY " << getAngle(Sqrt(resM(1,1)), Sqrt(resM(0,0)), Sqrt(resM(0,1))) << endl;
  cout <<  "angleXY " << getAngle(Sqrt(eigenMat(1,1)), Sqrt(eigenMat(0,0)), Sqrt(eigenMat(0,1))) << endl;
  resM.Print();
  return;
  */


  //TFile *file = TFile::Open("ntuple1797.root");
  //TTree *tr = (TTree *) file->Get("variables");
  //if(!tr) exit(0);

  //vector<event> evts = getEvents("ntuple1797Test.root");
  //

  cout << "Processing file" << inFile << endl;

  vector<event> evts = {}; // getEvents(inFile);

  runBeamSpotAnalysis(evts, {0.5});

  return;
  /*
  auto resTemp = fitSpotPositionSplines(evts, 2, 2);
  //spotParam spotPar = fitSpotPosition(evts);
  removeSpotPositionOutliers(evts, resTemp, 70);
  //spotPar = fitSpotPosition(evts);

  //scanOrder(evts, -30, 2);
  scanOrder(evts, 2, -20);
  return 0;
  */

  /*

  int indY = scanOrder(evts, 2, -10, 1, 1);

  int indKX = scanOrder(evts, indX, indY,-4, 1);
  int indKY = scanOrder(evts, indX, indY, 1, -4);

  cout << "Ind is " <<indX <<" "<< indY << endl;
  cout << "IndK is " <<indKX <<" "<< indKY << endl;
  //return;
  //unknowSpline xUn, yUn;
  */

  double duration = runEnd - runStart;
  int indX, indY, indZ;
  if (duration > 40 * 60) { // if longer than 40m, choose 2 params
    indX = indY = indZ = 2;
  } else {
    indX = indY = indZ = 1;
  }

  int indKX = 1;
  int indKY = 1;

  unknownPars allPars, allParsZ;
  for (int k = 0; k < 1; ++k) {
    cout << "Radek " << k << endl;
    for (auto& e : evts) e.isSig = true; //reset cuts
    if (k != 0) bootStrap(evts);

    cout << "Iteration number " << k << endl;

    //auto resTemp = fitSpotPositionSplines(evts, indX, indY, indKX, indKY);
    auto resTemp = fitSpotPositionSplines(evts, indX, indY);

    cout << "Terezka " << __LINE__ << endl;

    if (k == 0) plotSpotPositionFit(evts, resTemp, "positionFitSimpe");
    if (k == 0) plotSpotPositionPull(evts, resTemp, "pullsPositionSimple",  70);
    removeSpotPositionOutliers(evts, resTemp, 70);
    //auto resOld = fitSpotPositionSplines(evts, indX, indY, indKX, indKY);
    auto resFin = fitSpotPositionSplines(evts, indX, indY);
    if (k == 0) plotSpotPositionFit(evts, resFin, "positionFitSimpleC");
    if (k == 0) plotSpotPositionPull(evts, resFin, "pullsPositionSimpleC",  70);
    cout << "Terezka " << __LINE__ << endl;

    if (k == 0) plotXYtimeDep(evts, resFin, "simplePosTimeDep");

    cout << "Terezka " << __LINE__ << endl;

    vector<double> splZ = getRangeZero(indZ, 0., 1.);//   (indZ == 2) ? vector<double>({0,1}) : vector<double>({1, 0});

    auto resZmy = fitZpositionSplinesSimple(evts, splZ, resFin);
    if (k == 0) plotSpotZPositionFit(evts, resZmy, "positionFitSimpleZ");
    if (k == 0) plotSpotZpositionPull(evts, resZmy, "zPositionPull", 1000);

    cout << "Terezka " << __LINE__ << endl;
    removeSpotZpositionOutliers(evts,  resZmy, 1000);
    resZmy = fitZpositionSplinesSimple(evts, splZ, resZmy);

    //resTemp.print();
    //exit(0);


    cout << "Terezka " << __LINE__ << endl;
    auto resNew = fitSpotPositionSplines(evts, indX, indY, indKX, indKY, resZmy);
    if (k == 0) plotSpotPositionFit(evts, resNew, "positionFitFull");

    cout << "Terezka " << __LINE__ << endl;

    //cout << "Fit with K " << endl;
    //resNew.print();
    if (k == 0) plotKxKyFit(evts, resNew, "slopes");

    cout << "Terezka " << __LINE__ << endl;

    resZmy = fitZpositionSplinesSimple(evts, splZ, resNew);
    if (k == 0) plotSpotZPositionFit(evts, resZmy, "positionFitSimpleZLast");

    cout << "Terezka " << __LINE__ << endl;
    //exit(0);

    resNew = fitSpotPositionSplines(evts, indX, indY, indKX, indKY, resZmy);

    //cout << "Fit with K iter" << endl;
    //resNew.print();

    resZmy = fitZpositionSplinesSimple(evts, splZ, resNew);
    resNew = fitSpotPositionSplines(evts, indX, indY, indKX, indKY, resZmy);

    //if(k==0) plotXYtimeDep(evts, resNew, "simplePosTimeDepUpdated");

    //cout << "Fit with K iter-iter" << endl;
    //resNew.print();


    cout << "Terezka " << __LINE__ << endl;

    //auto resZ = fitZpositionSplines(evts, indX, indY, indKX, indKY, indZ);
    //if(k==0)plotSpotZpositionPull(evts, resZ, 1000);
    //removeSpotZpositionOutliers(evts,  resZ, 1000);
    //auto resZfin = fitZpositionSplines(evts, indX, indY, indKX, indKY, indZ);

    auto vecXY = fitSpotWidthCMS(evts, resNew);
    if (k == 0) plotSpotSizePull(evts, resNew, vecXY);
    removeSpotSizeOutliers(evts, resNew, vecXY, 1500);
    vecXY = fitSpotWidthCMS(evts, resNew);
    if (k == 0) plotSpotSizeFit(evts, resNew,  vecXY);

    cout << "Terezka " << __LINE__ << endl;

    double sizeZZ = fitSpotZwidth(evts, resNew, vecXY);

    if (k == 0) plotSpotZSizeFit(evts, resNew, vecXY, sizeZZ);
    if (k == 0) plotSpotSizeZPull(evts, resNew, vecXY,  sizeZZ);

    cout << "Terezka " << __LINE__ << endl;

    //removeSpotSizeZOutliers(evts, resNew, vecXY, sizeZZ, 150000);
    //sizeZZ = fitSpotZwidth(evts, resNew, vecXY);

    allPars.add(resNew, Sqrt(vecXY[0]), Sqrt(vecXY[1]), Sqrt(vecXY[2]), Sqrt(sizeZZ));

    //return;

    /*
    allPars.add(resFin,1, 1, 0);
    allParsZ.add(resZfin, 1, 1, 0);
    */

    //xUn.add(resTemp.x);
    //yUn.add(resTemp.y);
    //resFin.x.print();
  }
  //xUn.getMeanSigma().print();
  //yUn.getMeanSigma().print();

  allPars.printStat();
  //allParsZ.printStat();
  allPars.save2tree(outFile);


  return;

  for (int n = 1; n <= 50; ++n) {
    cout << "n is " << n << endl;
    auto rX = getRangeZero(n, 0, 1);
    auto rY  = getRangeZero(1, 0, 1);

    auto res1 = fitSpotPositionSplines(evts, rX, rY, {1, 0}, {1, 0});


    res1.x.print();
    for (int k = n - 1; k < n && k >= 1; ++k) {
      auto rX2 = getRangeZero(k, 0, 1);
      auto res2 = fitSpotPositionSplines(evts, rX2, rY, {1, 0}, {1, 0});

      //cout << "Before error " << endl;
      //cout << "Sizes " << res1.first.size() <<" "<< res1.second.size() <<     endl;
      double err = compareSplines(res1.x, res2.x);
      cout << "Radek Diff " << n << " " << k << " : " << err << endl;
      //res2.x.print();
    }
  }

  return;
  //fitZposition(evts);
  //return;

  /*

  return;
  */

  unknownPars StatVars, StatVarsZ;

  /*
  int n = 1;
  for(int i = 0; i < n; ++i) {
    for(auto &e : evts) e.isSig = true;
    if(i!=0) bootStrap(evts);
    spotParam par = fitSpotPosition(evts);


    cout << "Others " << par.x << " " << par.y << endl;
    spotParam parZ = fitZposition(evts);
    StatVars.add(par, 0, 0, 0);
    StatVarsZ.add(parZ, 0, 0, 0);
    continue;

    //if(i==0) plotSpotPositionFit(evts, par);
    if(i==0) plotSpotPositionPull(evts, par);
    removeSpotPositionOutliers(evts, par, 70);
    par = fitSpotPosition(evts);
    if(i==0) plotSpotPositionFit(evts, par);

    auto vec = fitSpotWidthCMS(evts, par);
    if(i==0) plotSpotSizePull(evts, par, Sqr(vec[0]), Sqr(vec[1]), Sqr(vec[2]));
    removeSpotSizeOutliers(evts, par, Sqr(vec[0]), Sqr(vec[1]), Sqr(vec[2]), 1500);
    vec = fitSpotWidthCMS(evts, par);
    if(i==0) plotSpotSizeFit(evts, par,  Sqr(vec[0]), Sqr(vec[1]), Sqr(vec[2]));

    double x = vec[0], y = vec[1], c = vec[2];
    cout << "Hela " << y << " "<< x*y-c*c << endl;
    StatVars.add(par, x, y, c);
  }

  StatVars.printStat();
  StatVarsZ.printStat();
  return;
  StatVars.save2tree(outFile);


  return;
  spotPar = fitSpotPosition(evts);
  spotPar = fitSpotPosition(evts);

  */


  /*
  TH1D *hPhi = new TH1D("hPhi", "", 30, -0.5+M_PI, 0.5+M_PI);
  for(auto ev : evts)
    hPhi->Fill(abs(ev.mu1.phi0 - ev.mu0.phi0));

  hPhi->Draw();
  return;
  */


  /*
  TH1D *hErr = new TH1D("hErr", "histErr", 100, -0.065, -0.04);
  TH1D *h = new TH1D("h", "hist", 100, -0.065, -0.04);
  //tr->Draw("x >> hErr", "(abs(x) < 0.08) / x_uncertainty^2");
  //tr->Draw("x", "(abs(x) < 0.08)");
  //tr->Draw("y", "(abs(y) < 0.08)");
  //tr->Draw("mu0.d0 + mu1.d0 : mu0.phi0", "","" );


  TProfile *hProf = new TProfile("hProf", "dProf", 100, -M_PI, M_PI);
  tr->Draw("mu0.d0 : mu0.phi0 >> hProf", "", "profs goff");
  //tr->Draw("mu1.d0 : mu1.phi0", "", "");
  //tr->Draw("abs(mu0.phi0 - mu1.phi0)");

  //return;
  hProf->Draw();
  TF1 * f = new TF1("f", "[0]*sin(x) - [1]*cos(x)");
  hProf->Fit(f);
  //return;
  //return;
  double x0 = f->GetParameter(0);
  double y0 = f->GetParameter(1);
  */


  //fitSpotWidth(evts, spotPar);


  return;

  /*
  cout << h->GetMean() << endl;
  cout << "comparison     " << h->GetRMS() <<" " << hErr->GetRMS() <<  endl;
  h->Print();

  h->Scale(1/h->Integral());
  hErr->Scale(1/hErr->Integral());
  h->Draw();
  hErr->Draw("same");
  */
}
