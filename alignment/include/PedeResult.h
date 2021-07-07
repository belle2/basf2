/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <map>
#include <string>
#include <vector>

namespace Belle2 {
  namespace alignment {
    //! Class to process Pede result file(s)
    class PedeResult {
    public:
      //! Default constructor, use read(...) to init the object
      PedeResult() : data(), eigenNumbers(), valid(false), labelIndices() {}
      //! Constructor which loads given file
      //! @param filename Name of the result file (millepede.res)
      explicit PedeResult(const std::string& filename);
      //! Reads the result file and inits the object
      void read(std::string filename = "millepede.res");
      //! Reads file with eigen-vector/numbers
      void readEigenFile(std::string filename = "millepede.eve");
      //! Was the object initialized properly from a result file?
      bool isValid() {return valid;}
      //! Dump the content to std::cout
      void dump();
      //! Get number of parameters in result (for looping over)
      int getNoParameters() const {return data.size();}
      //! Get number of determined parameters (with correction)
      int getNoDeterminedParameters() const {return noDeterminedParams;}
      //! Get index of parameter with given label
      int getParameterIndex(int parameterLabel);
      //! Get label of parameter at index
      unsigned int getParameterLabel(unsigned int parameterIndex);
      //! Get determined correction of parameter at index
      double getParameterCorrection(unsigned int parameterIndex);
      //! Get correction error of parameter at index
      double getParameterError(unsigned int parameterIndex);
      //! Get presigma of parameter at index
      double getParameterPresigma(unsigned int parameterIndex);
      //! Is parameter at given index fixed?
      bool isParameterFixed(unsigned int parameterIndex);
      //! Is parameter at given index determined?
      bool isParameterDetermined(unsigned int parameterIndex);
      //! Get the number of eigenvectors(numbers) in eigen file
      int getNoEigenPairs() const {return eigenNumbers.size();}
      //! Get eigennumber at given index
      double getEigenNumber(unsigned int eigenPairIndex);
      //! Get eigenvector at given index
      double getEigenVectorElement(unsigned int eigenPairIndex, unsigned int parameterIndex);

    private:
      //! Is index out of valid range?
      bool outOfBounds(int iparam) {if (iparam < 0) return true; if ((unsigned int)iparam >= data.size()) return true; return false;}
      //! Struct to hold data for a parameter
    public:
      //! All data for a parameter
      struct parameterData {
        //! Constructor
        parameterData() {}
        //! Constructor from parameter data
        //! @param index_ The index of this parameter
        //! @param label_ Label of the parameter
        //! @param correction_ The Pede correction for the parameter
        //! @param error_ Pede error for the parameter
        //! @param presigma_ The presigma provided to Pede
        parameterData(int index_, int label_, double correction_, double error_, double presigma_)
        {
          index = index_;
          label = label_;
          correction = correction_;
          error = error_;
          presigma = presigma_;
          eigenweights.clear();
        }
        //! param index
        int index;
        //! param label
        int label;
        //! param correction
        double correction;
        //! param error
        double error;
        //! param presigma
        double presigma;
        //! Weights of this param in eigenvectors
        std::vector<double> eigenweights;
      };
    private:
      //! Vector with all the parameter data
      std::vector<parameterData> data;
      //! Vector of loaded eigennumbers
      std::vector<double> eigenNumbers;
      //! Flag to check if data wa loaded
      bool valid;
      //! Map to link parameter labels and their indices in result
      std::map<int, int> labelIndices;
      //! Number of parameters actually determined
      int noDeterminedParams{0};
    };
  }
}
