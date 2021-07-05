/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <functional>
#include <cmath>
#include <utility>
#include <iostream>



/** Get minimum inside and outside of the smaller window defined by i0, j0 */
inline std::pair<double, double> getMinima(std::vector<std::vector<double>> vals, int i0, int j0)
{
  int N = vals.size();
  int Nzoom = (N + 1) / 2;


  double minIn  = 1e50;
  double minOut = 1e50;

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      if ((i0 <= i && i < i0 + Nzoom) &&
          (j0 <= j && j < j0 + Nzoom))
        minIn = std::min(minIn, vals[i][j]);
      else
        minOut = std::min(minOut, vals[i][j]);
    }
  return {minIn, minOut};
}

/** Get minimum of 2D function in the rectangular domain defined by xMin,xMax & yMin,yMax */
inline std::vector<double> getMinimum(std::function<double(double, double)> fun, double xMin, double xMax, double yMin, double yMax)
{
  const int N = 17; //the grid has size N x N
  const int Nzoom = (N + 1) / 2; // the size of the zoomed rectangle where minimum is


  const int kMax = 35; //number of iterations

  std::vector<std::vector<double>> vals(N);
  for (auto& v : vals) v.resize(N);

  for (int k = 0; k < kMax; ++k) {

    // get values of the function
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j) {
        double x = xMin + i * (xMax - xMin) / (N - 1.);
        double y = yMin + j * (yMax - yMin) / (N - 1.);
        vals[i][j] = fun(x, y);
      }

    if (k == kMax - 1) break;

    double mOutMax = -1e50;
    int iOpt = -1, jOpt = -1;
    //find optimal rectangle
    for (int i = 0; i < N - Nzoom; ++i)
      for (int j = 0; j < N - Nzoom; ++j) {
        double mIn, mOut;
        std::tie(mIn, mOut) = getMinima(vals, i, j);

        if (mOut > mOutMax) {
          mOutMax = mOut;
          iOpt = i;
          jOpt = j;
        }
      }

    //Zoom to the optimal rectangle
    // get values of the function

    double xMinNow = xMin + iOpt * (xMax - xMin) / (N - 1.);
    double xMaxNow = xMin + (iOpt + Nzoom - 1) * (xMax - xMin) / (N - 1.);

    double yMinNow = yMin + jOpt * (yMax - yMin) / (N - 1.);
    double yMaxNow = yMin + (jOpt + Nzoom - 1) * (yMax - yMin) / (N - 1.);

    xMin = xMinNow;
    xMax = xMaxNow;
    yMin = yMinNow;
    yMax = yMaxNow;

  }


  //get the overall minimum
  double minTot = 1e50;
  int iOpt = -1, jOpt = -1;

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      if (vals[i][j] < minTot) {
        minTot = vals[i][j];
        iOpt = i;
        jOpt = j;
      }
    }

  double xMinNow = xMin + iOpt * (xMax - xMin) / (N - 1.);
  double yMinNow = yMin + jOpt * (yMax - yMin) / (N - 1.);

  return {xMinNow, yMinNow};
}

