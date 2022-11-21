/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <iostream>
#include <TString.h>
#include <TRandom.h>

#include <Eigen/Dense>

//If compiled within BASF2
#ifdef _PACKAGE_
#include <framework/logging/Logger.h>
#else
#ifndef B2FATAL
#define B2FATAL(arg) { std::cout << arg << std::endl; std::exit(1);}
#define B2ASSERT(str, cond) { if((cond) == false) {std::cout << __FILE__ <<", line "<<__LINE__ << std::endl << "" << #cond << " failed" << std::endl;   std::exit(1);} }
#endif
#endif




namespace Belle2 {

  /// Get random string
  inline TString rn() {return Form("%d", gRandom->Integer(1000000000)); }

  ///merge { vector<double> a, vector<double> b} into {a, b}
  inline std::vector<std::vector<double>> merge(std::vector<std::vector<std::vector<double>>> toMerge)
  {
    std::vector<std::vector<double>> allVecs;
    for (const auto& v : toMerge)
      allVecs.insert(allVecs.end(), v.begin(), v.end());
    return allVecs;
  }


  /// std vector -> ROOT vector
  inline Eigen::VectorXd vec2vec(std::vector<double> vec)
  {
    Eigen::VectorXd v(vec.size());
    for (unsigned i = 0; i < vec.size(); ++i) {
      v[i] = vec[i];
    }
    return v;
  }

  /// ROOT vector -> std vector
  inline std::vector<double> vec2vec(Eigen::VectorXd v)
  {
    std::vector<double> vNew(v.rows());
    for (int i = 0; i < v.rows(); ++i)
      vNew[i] = v(i);
    return vNew;
  }



  /// merge columns (from std::vectors) into ROOT matrix
  inline Eigen::MatrixXd vecs2mat(std::vector<std::vector<double>> vecs)
  {
    Eigen::MatrixXd m(vecs[0].size(), vecs.size());
    for (unsigned i = 0; i < vecs[0].size(); ++i)
      for (unsigned j = 0; j < vecs.size(); ++j) {
        m(i, j) = vecs[j][i];
      }
    return m;
  }

  /// Equidistant range between xMin and xMax for spline of the first order
  inline std::vector<double> getRangeLin(int nVals, double xMin, double xMax)
  {
    B2ASSERT("At least one value in the spline required", nVals >= 1);
    if (nVals == 1) return {};
    std::vector<double> v(nVals);
    for (int i = 0; i < nVals; ++i)
      v[i] = xMin + i * (xMax - xMin) / (nVals - 1);
    return v;
  }

  /// Equidistant range between xMin and xMax for spline of the zero order
  inline std::vector<double> getRangeZero(int nVals, double xMin, double xMax)
  {
    B2ASSERT("At least one value in the spline required", nVals >= 1);
    if (nVals == 1) return {};
    std::vector<double> v(nVals - 1);
    for (int i = 1; i < nVals; ++i)
      v[i - 1] = xMin + i * (xMax - xMin) / (nVals);
    return v;
  }


  /// put slice of original vector v[ind:ind+n] into new one, n is number of elements
  inline std::vector<double> slice(std::vector<double> v, unsigned ind, unsigned n)
  {
    std::vector<double> vNew;
    for (unsigned i = ind; i < ind + n && i < v.size(); ++i)
      vNew.push_back(v[i]);
    return vNew;
  }

  /// Evaluate spline (zero order or first order) in point x
  inline double eval(const std::vector<double>& spl, const std::vector<double>& vals, double x)
  {
    int order = -1;
    if (spl.size() == 0)
      order = 0;
    else if (spl.size() == vals.size() - 1)
      order = 0;
    else if (spl.size() == vals.size())
      order = 1;
    else {
      B2FATAL("Unknown order of spline");
    }
    B2ASSERT("Spline order should be zero or one", order == 0 || order == 1);

    if (order == 1) {
      B2ASSERT("Linear spline only meaningful for two or more nodes", spl.size() >= 2);
      B2ASSERT("As nodes as values in lin. spline", spl.size() == vals.size());

      if (x <= spl[0]) return vals[0];
      if (x >= spl.back()) return vals.back();

      // binary search for position
      int i1 = lower_bound(spl.begin(), spl.end(), x) - spl.begin() - 1;

      if (!(spl[i1] <= x && x <= spl[i1 + 1])) {
        B2FATAL("Wrong place founded : " <<  spl[i1] << " " << x << " " << spl[i1 + 1]);
      }

      // Linear interpolation between neighbouring nodes
      double v = ((spl[i1 + 1] - x) * vals[i1] + (x - spl[i1]) * vals[i1 + 1]) / (spl[i1 + 1] - spl[i1]);
      return v;
    } else if (order == 0) { //zero order polynomial
      B2ASSERT("#values vs #nodes in zero-order spline", spl.size() + 1 == vals.size());
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


  /** Spline structure for zero-order & linear splines */
  /** nodes.size() <= 1 -> order=0 , nodes.size() == vals.size() -> order=1 */
  struct Spline {
    std::vector<double> nodes; ///< vector of spline nodes
    std::vector<double> vals;  ///< vector of spline values
    std::vector<double> errs;  ///< vector of spline errors

    /** get value of spline at point x */
    double val(double x) const {return eval(nodes, vals, x);}

    /** get error of spline at point x */
    double err(double x) const {return eval(nodes, errs, x);}

    /** Get center of the spline domain */
    double center() const
    {
      if (nodes.size() == 0)  //dummy situation
        return 0;
      else if (nodes.size() % 2 == 1)
        return nodes[nodes.size() / 2];
      else
        return (nodes[nodes.size() / 2 - 1]  +  nodes[nodes.size() / 2]) / 2;
    }

    /** print the spline */
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

    Spline() : nodes{}, vals{0}, errs{0}   {}

  };


}

