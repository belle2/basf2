/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dbobjects/PIDPriorsTable.h>
#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;


void PIDPriorsTable::setBinEdges(const std::vector<float>& binEdgesX, const std::vector<float>& binEdgesY)
{
  m_binEdgesX = binEdgesX;
  m_binEdgesY = binEdgesY;

  // if less than two edges are provided, assumes the maximum range for floats
  auto checkEdges = [](std::vector<float>& edges, const std::string & axis) {
    if (edges.size() < 2) {
      B2WARNING("You provided less than 2 bin edges for the " << axis <<
                " axis. This is not enough to create one bin, so one will be created with range [-FLT_MAX, +FLT_MAX]");
      edges = { -FLT_MAX, FLT_MAX};
    }
    auto prevVal = edges[0];
    for (int iBin = 1; iBin < static_cast<int>(edges.size()); iBin++) {
      auto edge = edges[iBin];
      if (prevVal >= edge)
        B2FATAL("Null or negative bin size found on the X axis. Please make sure that all the bin edges are sorted and non-equal");
      prevVal = edge;
    }
    return;
  };

  checkEdges(m_binEdgesX, "X");
  checkEdges(m_binEdgesY, "Y");

  m_priors.clear();
  m_errors.clear();

  m_priors.resize((m_binEdgesY.size() - 1) * (m_binEdgesX.size() - 1), 0.);
  m_errors.resize((m_binEdgesY.size() - 1) * (m_binEdgesX.size() - 1), 0.);
  return;
};



void PIDPriorsTable::setPriorValue(float x, float y, float value)
{
  if (!checkRange("The value is out of range for the X axis", x, m_binEdgesX)
      || !checkRange("The value is out of range for the Y axis", y, m_binEdgesY)) return;
  if (value > 1. || value < 0.) {
    B2WARNING("The value " << value  << " you are trying to set for the bin (" << x << ",  " << y <<
              ") does not look like a probability. The table will be filled, but i  will not be usable as a proper prior probability table.");
  }
  auto binX = findBin(x, m_binEdgesX);
  auto binY = findBin(y, m_binEdgesY);
  m_priors[binX + (m_binEdgesX.size() - 1)*binY] = value;
  return;
};


void PIDPriorsTable::setErrorValue(float x, float y, float value)
{
  if (!checkRange("The value is out of range for the X axis", x, m_binEdgesX)
      || !checkRange("The value is out of range for the Y axis", y, m_binEdgesY)) return ;

  auto binX = findBin(x, m_binEdgesX);
  auto binY = findBin(y, m_binEdgesY);
  m_errors[binX + (m_binEdgesX.size() - 1)*binY] = value;
  return ;
};


float PIDPriorsTable::getPriorValue(float x, float y) const
{
  if (!checkRange("The value is out of range for the X axis", x, m_binEdgesX)
      || !checkRange("The value is out of range for the Y axis", y, m_binEdgesY)) return 0;
  auto binX = findBin(x, m_binEdgesX);
  auto binY = findBin(y, m_binEdgesY);
  return getPriorInBin(binX, binY);
};


float PIDPriorsTable::getErrorValue(float x, float y) const
{
  if (!checkRange("The value is out of range for the X axis", x, m_binEdgesX)
      || !checkRange("The value is out of range for the Y axis", y, m_binEdgesY)) return 0;
  auto binX = findBin(x, m_binEdgesX);
  auto binY = findBin(y, m_binEdgesY);
  return getErrorInBin(binX, binY);
};


void  PIDPriorsTable::printPrior() const
{
  std::cout << " --- Prior summary --- " << std::endl;
  std::cout << " Size :  " << m_priors.size() << " (" << m_binEdgesX.size() - 1 <<  " x  " <<   m_binEdgesY.size() - 1 << ")" <<
            std::endl;
  std::cout << " X axis:  " ;
  for (auto edge : m_binEdgesX) {
    std::cout << " "  << edge << " " ;
  }
  std::cout << " " << std::endl;
  std::cout << " Y axis ";
  for (auto edge : m_binEdgesY) {
    std::cout << " " << edge << " " ;
  }
  std::cout << " " << std::endl;
  std::cout << " Values " << std::endl;
  for (int iY = m_binEdgesY.size() - 2; iY >= 0; iY--) {
    for (int iX = 0; iX < static_cast<int>(m_binEdgesX.size() - 1); iX++) {
      std::cout << " " << getPriorInBin(iX, iY) << " ";
    }
    std::cout << " " << std::endl;
  }
  std::cout << " --- End of prior summary --- " << std::endl;
};


void  PIDPriorsTable::printError() const
{
  std::cout << " --- Error summary --- " << std::endl;
  std::cout << " Size :  " << m_errors.size() << " (" << m_binEdgesX.size() - 1 <<  " x  " <<   m_binEdgesY.size() - 1 << ")" <<
            std::endl;
  std::cout << " X axis:  " ;
  for (auto edge : m_binEdgesX) {
    std::cout << " "  << edge << " " ;
  }
  std::cout << " " << std::endl;
  std::cout << " Y axis ";
  for (auto edge : m_binEdgesY) {
    std::cout << " " << edge << " " ;
  }
  std::cout << " " << std::endl;
  std::cout << " Values " << std::endl;
  for (int iY = m_binEdgesY.size() - 2; iY >= 0; iY--) {
    for (int iX = 0; iX < static_cast<int>(m_binEdgesX.size() - 1); iX++) {
      std::cout << " " << getErrorInBin(iX, iY) << " ";
    }
    std::cout << " " << std::endl;
  }
  std::cout << " --- End of error summary --- " << std::endl;
};



bool PIDPriorsTable::checkRange(const std::string& text, float val, const std::vector<float>& edges) const
{
  const float& min = edges.front();
  const float& max = edges.back();
  if (val > max || val < min) {
    B2WARNING("PriorsTable: " << text  << LogVar("value", val) << LogVar("min", min) << LogVar("max", max));
    return false;
  }
  return true;
};


short PIDPriorsTable::findBin(float val, std::vector<float> array) const
{
  auto it = std::lower_bound(array.cbegin(),  array.cend(), val);
  return std::distance(array.cbegin(), it) - 1;
};


short PIDPriorsTable::findBinFast(float val, std::vector<float> array) const
{
  // This function searches for the first bin the axis which is above the
  // value. First it starts assuming the bins are equal, and then moves around
  // the array until the correct bin is found
  float averageBinSize = (array.back() - array.front()) / (array.size() - 1);
  short bin = 1 + (short)((val - array[0]) / averageBinSize);
  //adjusts forward

  while (bin < static_cast<short>(array.size()) && array[bin] < val) {
    bin++;
  }
  //adjusts backward
  while (bin - 1 >  0 && array[bin - 1] > val) {
    bin--;
  }
  return bin - 1;
};


short PIDPriorsTable::findBinWithFixedWidth(float val, std::vector<float> array) const
{
  float binWidth = (array.back() - array.front()) / (array.size() - 1.);
  return (short)((val - array.back()) / binWidth);
};
