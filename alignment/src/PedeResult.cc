/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/PedeResult.h>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

namespace Belle2 {
  namespace alignment {

    // ----------
    // PedeResult
    // ----------
    int PedeResult::getParameterIndex(int parameterLabel)
    {
      auto index = labelIndices.find(parameterLabel);
      if (index != labelIndices.end())
        return index->second;
      else
        return -1;
    }
    void PedeResult::readEigenFile(string filename)
    {
      if (!valid)
        return;

      string line;
      ifstream file(filename);

      if (!file.is_open())
        return;

      int eigenPairIndex = -1;

      for (auto& pData : data) {
        pData.eigenweights.clear();
        pData.eigenweights.reserve(20);
      }

      while (getline(file, line)) {
        cout << line << endl;
        stringstream ss;
        ss << line;
        string tmp1, tmp2, tmp3;
        int eigenPairNumber;
        double eigenvalue;
        ss >> tmp1 >> eigenPairNumber >> tmp2 >> tmp3 >> eigenvalue;
        cout << tmp1 << " " << tmp2 << " " << tmp3 << " " << eigenPairNumber << " " << eigenvalue << endl;
        if (tmp1 == "Eigenvector")
          ++eigenPairIndex;
        else
          continue;

        // Read elements
        vector<pair<int, double>> elements;
        while (getline(file, line) && line.length() > 2) {
          int lab1 = 0;
          int lab2 = 0;
          int lab3 = 0;
          double w1, w2, w3;
          stringstream triplet;
          triplet << line;
          triplet >> lab1 >> w1 >> lab2 >> w2 >> lab3 >> w3;
          if (lab1) elements.push_back({lab1, w1});
          if (lab2) elements.push_back({lab2, w2});
          if (lab3) elements.push_back({lab3, w3});
          //cout << lab1 << w1 << lab2 << w2 << lab3 << w3 << endl;
        }
        for (auto& pData : data) {
          pData.eigenweights.push_back(0.);
        }
        for (auto el : elements) {
          int index = getParameterIndex(el.first);
          if (index >= 0)
            data[index].eigenweights[eigenPairIndex] = el.second;
        }
        eigenNumbers.push_back(eigenvalue);
      }


    }
    void PedeResult::read(string filename)
    {
      string line;
      ifstream res(filename);
      // skip comment line
      getline(res, line);

      noDeterminedParams = 0;
      data.clear();
      labelIndices.clear();
      int index(-1);

      while (getline(res, line)) {
        ++index;
        int label = -1;
        double param = 0.;
        double presigma = 0.;
        double differ = 0.;
        double error = 0.;
        stringstream ss;
        ss << line;
        ss >> label >> param >> presigma >> differ >> error;
        data.push_back(parameterData(index, label, param, error, presigma));
        labelIndices.insert(make_pair(label, index));
        if (isParameterDetermined(index))
          ++noDeterminedParams;
      }

      if (data.size() > 0)
        valid = true;
    }
    PedeResult::PedeResult(const string& filename): data(), eigenNumbers(), valid(false), labelIndices()
    {
      read(filename);
    }

    unsigned int PedeResult::getParameterLabel(unsigned int parameterIndex)
    {
      if (outOfBounds(parameterIndex)) return 0;
      return data[parameterIndex].label;
    }
    double PedeResult::getParameterCorrection(unsigned int parameterIndex)
    {
      if (outOfBounds(parameterIndex)) return 0.;
      return data[parameterIndex].correction;
    }
    double PedeResult::getParameterError(unsigned int parameterIndex)
    {
      if (outOfBounds(parameterIndex)) return 0.;
      return data[parameterIndex].error;
    }
    double PedeResult::getParameterPresigma(unsigned int parameterIndex)
    {
      if (outOfBounds(parameterIndex)) return 0.;
      return data[parameterIndex].presigma;
    }
    bool PedeResult::isParameterFixed(unsigned int parameterIndex)
    {
      if (outOfBounds(parameterIndex)) return false;
      return data[parameterIndex].presigma < 0;
    }
    bool PedeResult::isParameterDetermined(unsigned int parameterIndex)
    {
      if (outOfBounds(parameterIndex)) return false;
      return !isParameterFixed(parameterIndex) && data[parameterIndex].correction != 0. && data[parameterIndex].error != 0.;
    }
    double PedeResult::getEigenNumber(unsigned int eigenPairIndex)
    {
      if (eigenPairIndex >= eigenNumbers.size()) return 0.;
      return eigenNumbers[eigenPairIndex];
    }
    double PedeResult::getEigenVectorElement(unsigned int eigenPairIndex, unsigned int parameterIndex)
    {
      if (outOfBounds(parameterIndex))
        return 0.;
      parameterData& element = data[parameterIndex];
      if (eigenPairIndex >= element.eigenweights.size())
        return 0.;

      return data[parameterIndex].eigenweights[eigenPairIndex];
    }
    void PedeResult::dump()
    {
      for (auto pdata : data) {
        std::cout << pdata.index << " : " << pdata.label << " corr: " << pdata.correction << " err: " << pdata.error << " presigma: " <<
                  pdata.presigma << endl;
      }
    }
  }
}


