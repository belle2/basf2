#include <alignment/PedeResult.h>
#include <fstream>
#include <sstream>
#include <iostream>

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
      //TODO: not implemented
      ifstream file(filename);
      if (!file.is_open())
        return;
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
    PedeResult::PedeResult(string filename): data(), eigenNumbers(), valid(false), labelIndices()
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


