/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <vector>
#include <iostream>
#include <string>
#include <array>

namespace Belle2 {

  /** bundle together the classifier input and the target value into one struct for easier passing around. */
  template<size_t Ndims = 9>
  struct FBDTTrainSample {
    /** constructor from array of inputs and a target value */
    explicit FBDTTrainSample(const std::array<double, Ndims>& values, bool sig) : hits(values), signal(sig) {}

    /** default constructor */
    FBDTTrainSample() { };

    /** inputs */
    std::array<double, Ndims> hits{};

    /** target */
    bool signal{};
  };

  /** read samples from stream and append them to samples */
  template<size_t Ndims>
  static void readSamplesFromStream(std::istream& is, std::vector<FBDTTrainSample<Ndims> >& samples)
  {
    size_t nSamplesBefore = samples.size();
    std::string line;
    while (!is.eof()) {
      getline(is, line);
      if (line.empty()) continue; // ignore empty lines
      std::stringstream ss(line);
      std::array<double, 9> coords;
      for (double& c : coords) ss >> c;
      bool sig; ss >> sig;

      samples.push_back(FBDTTrainSample<9>(coords, sig));
    }

    B2INFO("Read in " << (samples.size() - nSamplesBefore) << " samples.");
  }

  /** write all samples to stream */
  template<size_t Ndims>
  static void writeSamplesToStream(std::ostream& os, const std::vector<FBDTTrainSample<Ndims> >& samples)
  {
    for (const auto& event : samples) {
      for (const auto& val : event.hits) {
        os << val << " ";
      }
      os << event.signal << std::endl;
    }
    B2INFO("Wrote out " << samples.size() << " samples.");
  }
}
