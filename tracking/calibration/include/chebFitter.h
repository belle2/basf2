#pragma once

#include <vector>
#include <map>
#include <utility>
#include <Eigen/Dense>
#include <functional>
#include "TString.h"

typedef std::map<std::string, double> Pars;
typedef std::map<std::string, std::pair<double, double>> Limits;

//to describe each parameter to minimize
struct par {
  TString name; //name of the parameter
  double v, vMin, vMax; //starting value & limits
};


struct chebFitter {

  Eigen::VectorXd getLogFunction(Pars pars) const;
  double getLogLikelihoodSlow(Pars pars) const;
  double getLogLikelihoodFast(Pars pars) const;
  void init(int Size, double xMin, double xMax);
  Eigen::VectorXd getDataGrid() const;
  std::pair<Eigen::VectorXd, Eigen::MatrixXd> getDataGridWithCov() const;

  double getFunctionFast(Pars pars, double x);

  std::pair<Pars, Eigen::MatrixXd> fitData(Pars pars, Limits limits, bool UseCheb = true);

  double operator()(const double* par) const;


  std::vector<double> data;     //vector with the data points to be fitted
  Eigen::VectorXd dataGrid; //vector with the data points related to the cheb nodes (dataGrid.size = nodes.size)
  Eigen::MatrixXd dataGridCov;
  Eigen::MatrixXd coefsMat; // transofrmation matrix from chebPol to gridPoints

  Eigen::VectorXd nodes;   // vector with cheb nodes
  Eigen::VectorXd weights; // vector with cheb weights for integration
  //int nPars;
  std::vector<std::string> parNames;
  bool useCheb;

  std::function<double(double, Pars)> myFun;

};
