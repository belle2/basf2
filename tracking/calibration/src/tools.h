#pragma once

//Hack for ROOT macros
#undef assert
#define assert(arg)  { if((arg) == false) {std::cout << __FILE__ <<", line "<<__LINE__ << std::endl << "" << #arg << " failed" << std::endl;   exit(0);} }

#include <vector>
#include <iostream>
#include <cstdlib>
#include "TMatrixD.h"
#include "TString.h"


// Get random string
inline TString rn() {return Form("%d", std::rand()); }

//merge { vector<double> a, vector<double> b} into {a, b}
inline std::vector<std::vector<double>> merge(std::vector<std::vector<std::vector<double>>> toMerge)
{
  std::vector<std::vector<double>> allVecs;
  for (const auto& v : toMerge)
    allVecs.insert(allVecs.end(), v.begin(), v.end());
  return allVecs;
}


// std vector -> ROOT vector
inline TVectorD vec2vec(std::vector<double> vec)
{
  TVectorD v(vec.size());
  for (unsigned i = 0; i < vec.size(); ++i) {
    v(i) = vec[i];
  }
  return v;
}

// ROOT vector -> std vector
inline std::vector<double> vec2vec(TVectorD v)
{
  std::vector<double> vNew(v.GetNrows());
  for (int i = 0; i < v.GetNrows(); ++i)
    vNew[i] = v(i);
  return vNew;
}



// merge columns (from std::vectors) into ROOT matrix
inline TMatrixD vecs2mat(std::vector<std::vector<double>> vecs)
{
  TMatrixD m(vecs[0].size(), vecs.size());
  for (unsigned i = 0; i < vecs[0].size(); ++i)
    for (unsigned j = 0; j < vecs.size(); ++j) {
      m(i, j) = vecs[j][i];
    }
  return m;
}

// for splines of the first order
inline std::vector<double> getRangeLin(int nVals, double xMin, double xMax)
{
  assert(nVals >= 1);
  if (nVals == 1) return {};
  std::vector<double> v(nVals);
  for (int i = 0; i < nVals; ++i)
    v[i] = xMin + i * (xMax - xMin) / (nVals - 1);
  return v;
}

//for splines of zero order
inline std::vector<double> getRangeZero(int nVals, double xMin, double xMax)
{
  assert(nVals >= 1);
  if (nVals == 1) return {};
  std::vector<double> v(nVals - 1);
  for (int i = 1; i < nVals; ++i)
    v[i - 1] = xMin + i * (xMax - xMin) / (nVals);
  return v;
}


// put slice of original vecotr v[ind:ind+n] into new one
inline std::vector<double> Slice(std::vector<double> v, unsigned ind, unsigned n)
{
  std::vector<double> vNew;
  for (unsigned i = ind; i < ind + n && i < v.size(); ++i)
    vNew.push_back(v[i]);
  return vNew;
}

//To evaluate spline
inline double Eval(const std::vector<double>& spl, const std::vector<double>& vals, double x)
{
  int order = -1;
  if (spl.size() == 0)
    order = 0;
  else if (spl.size() == vals.size() - 1)
    order = 0;
  else if (spl.size() == vals.size())
    order = 1;
  else {
    std::cout << "Unknown order of spline" << std::endl;
    std::exit(0);
  }
  assert(0 <= order && order <= 1);

  if (order == 1) {
    assert(spl.size() >= 2);
    assert(spl.size() == vals.size());

    if (x <= spl[0]) return vals[0];
    if (x >= spl.back()) return vals.back();

    int i1 = lower_bound(spl.begin(), spl.end(), x) - spl.begin() - 1;

    if (!(spl[i1] <= x && x <= spl[i1 + 1])) {
      std::cout << spl[i1] << " " << x << " " << spl[i1 + 1] << std::endl;
      std::cout << "Problem"  << std::endl;
      exit(1);
    }

    double v = ((spl[i1 + 1] - x) * vals[i1] + (x - spl[i1]) * vals[i1 + 1]) / (spl[i1 + 1] - spl[i1]);
    return v;
  } else if (order == 0) { //zero order polynomial
    //std::cout << "Radek " << spl.size() <<" : " << vals.size() << std::endl;
    assert(spl.size() + 1 == vals.size());
    if (vals.size() == 1) {
      return vals[0];
    } else {
      double res = vals[0]; //by default value from lowest node
      for (unsigned i = 0; i < spl.size(); ++i) {
        if (spl[i] <= x) res = vals[i + 1];
        else break;
      }
      return res;
    }
  }
  return -99;
}


// Spline structure for zero-order & linear splines
struct Spline {
  //spl.size() <= 1 -> order=0 , spl.size() == vals.size() -> order=1
  std::vector<double> nodes, vals, errs;
  double val(double x) const {return Eval(nodes, vals, x);}
  double err(double x) const {return Eval(nodes, errs, x);}

  double center() const
  {
    if (nodes.size() == 0)
      return 0;
    else if (nodes.size() % 2 == 1)
      return nodes[nodes.size() / 2];
    else
      return (nodes[nodes.size() / 2 - 1]  +  nodes[nodes.size() / 2]) / 2;
  }

  void print(TString tag = "")
  {
    std::cout << tag << " : nodes: ";
    for (auto n : nodes) {
      std::cout << n << " ";
    }
    std::cout << std::endl;
    std::cout << tag << " : vals: ";
    for (auto n : vals) {
      std::cout << n << " ";
    }
    std::cout << std::endl;

    std::cout << tag << " : errs: ";
    for (auto n : errs) {
      std::cout << n << " ";
    }
    std::cout << std::endl;
  }

  Spline() { nodes = {}; vals = {0}; errs = {0}; }

};




