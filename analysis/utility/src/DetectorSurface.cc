/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/utility/DetectorSurface.h>

using namespace Belle2;

const std::unordered_map<std::string, DetSurfCylBoundaries> DetectorSurface::detToSurfBoundaries = {
  {"CDC", DetSurfCylBoundaries(16.8, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
  {"TOP", DetSurfCylBoundaries(117.8, 193.0, -94.0, 0.24, 0.52, 2.23, 3.14)},
  {"ARICH", DetSurfCylBoundaries(117.8, 193.0, -94.0, 0.24, 0.52, 2.23, 3.14)},
  {"ECL", DetSurfCylBoundaries(125.0, 196.0, -102.0, 0.21, 0.56, 2.24, 2.70)},
  {"KLM", DetSurfCylBoundaries(202.0, 283.9, -189.9, 0.40, 0.82, 2.13, 2.60)},
};

const std::unordered_map<std::string, DetSurfCylBoundaries> DetectorSurface::detLayerToSurfBoundaries = {
  {"CDC0", DetSurfCylBoundaries(16.8, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
  {"CDC1", DetSurfCylBoundaries(25.7, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
  {"CDC2", DetSurfCylBoundaries(36.52, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
  {"CDC3", DetSurfCylBoundaries(47.69, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
  {"CDC4", DetSurfCylBoundaries(58.41, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
  {"CDC5", DetSurfCylBoundaries(69.53, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
  {"CDC6", DetSurfCylBoundaries(80.25, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
  {"CDC7", DetSurfCylBoundaries(91.37, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
  {"CDC8", DetSurfCylBoundaries(102.09, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
  {"TOP0", DetSurfCylBoundaries(117.8, 193.0, -94.0, 0.24, 0.52, 2.23, 3.14)},
  {"ARICH0", DetSurfCylBoundaries(117.8, 193.0, -94.0, 0.24, 0.52, 2.23, 3.14)},
  {"ECL0", DetSurfCylBoundaries(125.0, 196.0, -102.0, 0.21, 0.56, 2.24, 2.70)},
  {"ECL1", DetSurfCylBoundaries(140.0, 211.0, -117.0, 0.21, 0.56, 2.24, 2.70)},
  {"KLM0", DetSurfCylBoundaries(202.0, 283.9, -189.9, 0.40, 0.82, 2.13, 2.60)},
};

const std::unordered_map<std::string, std::vector<int>> DetectorSurface::detToLayers = {
  {"CDC", {0, 1, 2, 3, 4, 5, 6, 7, 8}},
  {"TOP", {0}},
  {"ARICH", {0}},
  {"ECL", {0, 1}},
  {"KLM", {0}}
};

const std::unordered_map<int, double> DetectorSurface::cdcWireRadiuses = {
  {0, 16.80},
  {1, 17.80},
  {2, 18.80},
  {3, 19.80},
  {4, 20.80},
  {5, 21.80},
  {6, 22.80},
  {7, 23.80},
  {8, 25.70},
  {9, 27.52},
  {10, 29.34},
  {11, 31.16},
  {12, 32.98},
  {13, 34.80},
  {14, 36.52},
  {15, 38.34},
  {16, 40.16},
  {17, 41.98},
  {18, 43.80},
  {19, 45.57},
  {20, 47.69},
  {21, 49.46},
  {22, 51.28},
  {23, 53.10},
  {24, 54.92},
  {25, 56.69},
  {26, 58.41},
  {27, 60.18},
  {28, 62.00},
  {29, 63.82},
  {30, 65.64},
  {31, 67.41},
  {32, 69.53},
  {33, 71.30},
  {34, 73.12},
  {35, 74.94},
  {36, 76.76},
  {37, 78.53},
  {38, 80.25},
  {39, 82.02},
  {40, 83.84},
  {41, 85.66},
  {42, 87.48},
  {43, 89.25},
  {44, 91.37},
  {45, 93.14},
  {46, 94.96},
  {47, 96.78},
  {48, 98.60},
  {49, 100.37},
  {50, 102.09},
  {51, 103.86},
  {52, 105.68},
  {53, 107.50},
  {54, 109.32},
  {55, 111.14},
};
