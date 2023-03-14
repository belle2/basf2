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
