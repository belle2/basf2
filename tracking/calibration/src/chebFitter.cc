#include <cmath>
#include <iostream>
#include <cstdlib>
#include <cassert>


#include "TGraph.h"
#include "TString.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"

#include <Eigen/Core>

//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/chebFitter.h>
#include <tracking/calibration/nodes.h>
#else
#include <chebFitter.h>
#include <nodes.h>
#endif


using Eigen::MatrixXd;

using namespace std;



//return values of -2*log(p(x)), where p(x) is normalized to 1 over the fitted range
VectorXd chebFitter::getLogFunction(Pars pars) const
{
  VectorXd fVals(nodes.size());

  //calc function values
  fVals = nodes.unaryExpr([&](double x) { return myFun(x, pars); });

  //normalize the function
  double I = fVals.dot(weights);

  fVals = -2 * log(fVals.array() / I); //normalize by integral

  return fVals;

}

void chebFitter::init(int Size, double xMin, double xMax)
{
  // loading the Cheb nodes
  nodes = (xMax - xMin) * GetNodes(Size).array() + xMin;

  // loding the weights for integration
  weights = (xMax - xMin) * GetWeights(Size);


  // calculate the transformation matrix from pol coefs to grid points
  coefsMat = GetCoefsCheb(Size).transpose();

  //cout << "Loading data grid" << endl;
  dataGrid = getDataGrid();
  //tie(dataGrid, dataGridCov) = getDataGridWithCov();

}

//function assumed to be normalized !!!
double chebFitter::getLogLikelihoodSlow(Pars pars) const
{

  double L = 0;
  for (double d : data) {
    double v = myFun(d, pars);
    L += -2 * log(v);
  }

  return L;
}

//evaluation using cheb pols
double chebFitter::getLogLikelihoodFast(Pars pars) const
{
  VectorXd funVals = getLogFunction(pars);
  double LL = funVals.dot(dataGrid);

  return LL;
}

double chebFitter::operator()(const double* par) const
{
  Pars pars;
  for (unsigned i = 0; i < parNames.size(); ++i)
    pars[parNames[i]] = par[i];

  double LL = useCheb ? getLogLikelihoodFast(pars) : getLogLikelihoodSlow(pars);

  return LL;
}

// get data transformed into the grid such that (chebFunVals dot dataGrid) == logL
VectorXd chebFitter::getDataGrid() const
{
  double a = nodes[0];
  double b = nodes[nodes.size() - 1];


  VectorXd polSum = VectorXd::Zero(nodes.size());
  for (double x : data) {
    double xx = (x - a) / (b - a); //normalize between 0 and 1
    polSum += getPols(nodes.size(), xx);
  }
  //cout << "Done " << endl;



  /*
  //new method
  VectorXd dataNew(data.size());
  for(int i = 0; i < data.size(); ++i)
     dataNew[i] = (data[i] - a) / (b - a);

  cout << "Starting " << endl;
  VectorXd polSum = getPolsSum(nodes.size(), dataNew);
  */


  //transform to the basis of the cheb nodes
  VectorXd gridVals = coefsMat * polSum;

  return gridVals;
}


// get data transformed into the grid such that (chebFunVals dot dataGrid) == logL
pair<VectorXd, MatrixXd> chebFitter::getDataGridWithCov() const
{
  double a = nodes[0];
  double b = nodes[nodes.size() - 1];

  MatrixXd polSum2 = MatrixXd::Zero(nodes.size(), nodes.size());
  VectorXd polSum  = VectorXd::Zero(nodes.size());
  for (double x : data) {
    double xx = (x - a) / (b - a); //normalize between 0 and 1
    VectorXd pol = getPols(nodes.size(), xx);
    polSum  += pol;
    polSum2 += pol * pol.transpose();
  }
  //cout << "Done " << endl;


  //transform to the basis of the cheb nodes
  MatrixXd coefs = GetCoefsCheb(polSum.size()).transpose();
  VectorXd gridVals = coefs * polSum;
  MatrixXd gridValsCov = coefs * polSum2 * coefs.transpose();

  return make_pair(gridVals, gridValsCov);
}


//Minimize using ROOT minimizer
pair<Pars, MatrixXd> chebFitter::fitData(Pars pars, Limits limits, bool UseCheb)
{
  useCheb = UseCheb;

  ROOT::Math::Minimizer* minimum =
    ROOT::Math::Factory::CreateMinimizer("Minuit2", "");

  // set tolerance , etc...
  minimum->SetMaxFunctionCalls(10000000); // for Minuit/Minuit2
  minimum->SetMaxIterations(100000);  // for GSL
  minimum->SetTolerance(10.0);
  //minimum->SetPrecision(1e-5);

  //minimum->SetPrintLevel(3); //many outputs
  minimum->SetPrintLevel(0); //few outputs
  minimum->SetStrategy(2);
  minimum->SetErrorDef(1);


  // Set the free variables to be minimized !
  parNames.clear();
  int k = 0;
  for (auto p : pars) {
    string n    = p.first;
    double vCnt = p.second;
    if (limits.count(n) == 1) {
      double vMin = limits.at(n).first;
      double vMax = limits.at(n).second;
      double step = (vMax - vMin) / 100;
      minimum->SetLimitedVariable(k, n, vCnt, step, vMin, vMax);
    } else {
      double step = 1;
      minimum->SetVariable(k, n, vCnt, step);
    }
    parNames.push_back(n);
    ++k;
  }

  // create function wrapper for minimizer
  // a IMultiGenFunction type
  ROOT::Math::Functor f(*this, pars.size());
  minimum->SetFunction(f);


  // do the minimization
  minimum->Minimize();


  Pars parsF;
  for (unsigned i = 0; i < parNames.size(); ++i)
    parsF[parNames[i]] = minimum->X()[i];

  MatrixXd covMat(parsF.size(), parsF.size());
  for (unsigned i = 0; i < parsF.size(); ++i)
    for (unsigned j = 0; j < parsF.size(); ++j)
      covMat(i, j) = minimum->CovMatrix(i, j);

  // Pars
  cout << "Minuit status : " << minimum->Status() << ", ";
  for (auto p : parsF)
    cout << "\"" << p.first << "\" : " << p.second << ", ";
  cout << endl;

  return make_pair(parsF, covMat);
}


double chebFitter::getFunctionFast(Pars pars, double x)
{
  static VectorXd funVals = getLogFunction(pars);


  return  exp(-0.5 * interpol(nodes, funVals, x));

}
