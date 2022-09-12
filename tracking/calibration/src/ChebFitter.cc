/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <cmath>
#include <iostream>
#include <sstream>

#include <Math/Minimizer.h>
#include <Math/Factory.h>
#include <Math/Functor.h>

#include <Eigen/Core>

//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/ChebFitter.h>
#include <tracking/calibration/nodes.h>
#include <framework/logging/Logger.h>
#else
#include <ChebFitter.h>
#include <nodes.h>
#define B2INFO(arg) { std::cout << arg << std::endl;}
#endif


namespace Belle2 {

  using Eigen::VectorXd;
  using Eigen::MatrixXd;


//return values of -2*log(p(x)), where p(x) is normalized to 1 over the fitted range
  VectorXd ChebFitter::getLogFunction(Pars pars) const
  {
    VectorXd fVals(m_nodes.size());

    //calc function values
    fVals = m_nodes.unaryExpr([&](double x) { return m_myFun(x, pars); });

    //normalize the function
    double I = fVals.dot(m_weights);

    fVals = -2 * log(fVals.array() / I); //normalize by integral

    return fVals;

  }

  void ChebFitter::init(int Size, double xMin, double xMax)
  {
    // loading the Cheb nodes
    m_nodes = (xMax - xMin) * getNodes(Size).array() + xMin;

    // loding the weights for integration
    m_weights = (xMax - xMin) * getWeights(Size);


    // calculate the transformation matrix from pol coefs to grid points
    m_coefsMat = getCoefsCheb(Size).transpose();

    m_dataGrid = getDataGrid();

  }

//function assumed to be normalized !!!
  double ChebFitter::getLogLikelihoodSlow(const Pars& pars) const
  {

    double L = 0;
    for (double d : m_data) {
      double v = m_myFun(d, pars);
      L += -2 * log(v);
    }

    return L;
  }

//evaluation using cheb pols
  double ChebFitter::getLogLikelihoodFast(const Pars& pars) const
  {
    VectorXd funVals = getLogFunction(pars);
    double LL = funVals.dot(m_dataGrid);

    return LL;
  }

  double ChebFitter::operator()(const double* par) const
  {
    Pars pars;
    for (unsigned i = 0; i < m_parNames.size(); ++i)
      pars[m_parNames[i]] = par[i];

    double LL = m_useCheb ? getLogLikelihoodFast(pars) : getLogLikelihoodSlow(pars);

    return LL;
  }

// get data transformed into the grid such that (chebFunVals dot m_dataGrid) == logL
  VectorXd ChebFitter::getDataGrid() const
  {
    double a = m_nodes[0];
    double b = m_nodes[m_nodes.size() - 1];


    VectorXd polSum = VectorXd::Zero(m_nodes.size());
    for (double x : m_data) {
      double xx = (x - a) / (b - a); //normalize between 0 and 1
      polSum += getPols(m_nodes.size(), xx);
    }


    //transform to the basis of the cheb m_nodes
    VectorXd gridVals = m_coefsMat * polSum;

    return gridVals;
  }


// get data transformed into the grid such that (chebFunVals dot m_dataGrid) == logL
  std::pair<VectorXd, MatrixXd> ChebFitter::getDataGridWithCov() const
  {
    double a = m_nodes[0];
    double b = m_nodes[m_nodes.size() - 1];

    MatrixXd polSum2 = MatrixXd::Zero(m_nodes.size(), m_nodes.size());
    VectorXd polSum  = VectorXd::Zero(m_nodes.size());
    for (double x : m_data) {
      double xx = (x - a) / (b - a); //normalize between 0 and 1
      VectorXd pol = getPols(m_nodes.size(), xx);
      polSum  += pol;
      polSum2 += pol * pol.transpose();
    }


    //transform to the basis of the cheb nodes
    MatrixXd coefs = getCoefsCheb(polSum.size()).transpose();
    VectorXd gridVals = coefs * polSum;
    MatrixXd gridValsCov = coefs * polSum2 * coefs.transpose();

    return std::make_pair(gridVals, gridValsCov);
  }


//Minimize using ROOT minimizer
  std::pair<Pars, MatrixXd> ChebFitter::fitData(Pars pars, Limits limits, bool UseCheb)
  {
    m_useCheb = UseCheb;

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
    m_parNames.clear();
    int k = 0;
    for (auto p : pars) {
      std::string n    = p.first;
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
      m_parNames.push_back(n);
      ++k;
    }

    // create function wrapper for minimizer
    // a IMultiGenFunction type
    ROOT::Math::Functor f(*this, pars.size());
    minimum->SetFunction(f);


    // do the minimization
    minimum->Minimize();


    Pars parsF;
    for (unsigned i = 0; i < m_parNames.size(); ++i)
      parsF[m_parNames[i]] = minimum->X()[i];

    MatrixXd covMat(parsF.size(), parsF.size());
    for (unsigned i = 0; i < parsF.size(); ++i)
      for (unsigned j = 0; j < parsF.size(); ++j)
        covMat(i, j) = minimum->CovMatrix(i, j);

    // print pars
    std::stringstream log;
    log << "Minuit status : " << minimum->Status() << ", ";
    for (auto p : parsF)
      log << "\"" << p.first << "\" : " << p.second << ", ";

    B2INFO(log.str());

    delete minimum;

    return std::make_pair(parsF, covMat);
  }


  double ChebFitter::getFunctionFast(const Pars& pars, double x)
  {
    static VectorXd funVals = getLogFunction(pars);


    return  exp(-0.5 * interpol(m_nodes, funVals, x));

  }

}
