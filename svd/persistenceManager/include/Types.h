#pragma once

#include <variant>
#include <string>
#include <functional>
#include <stdexcept>
#include <map>
#include <memory>

namespace Belle2 {
  class SVDCluster;
  namespace SVD {

    namespace Variables {
      enum VariableDataType {
        c_double = 0,
        c_int = 1,
        c_bool = 2
      };

      class Variable {
      public:
        Variable(const std::string& variableName) : name{variableName} {}

        virtual std::string getName() const { return name; }

        virtual ~Variable() = default;

      private:
        std::string name;
      };

      class TypedVariable : public Variable {
      public:
        TypedVariable(const std::string& variableName, const VariableDataType& variableDataType)
          : Variable(variableName), dataType{variableDataType} {}

        VariableDataType getDataType() const { return dataType; }

      private:
        VariableDataType dataType;
      };

      class BinnedVariable : public Variable {
      public:
        BinnedVariable(const std::string& variableName, int _nBins, float _lowBin, float _highBin)
          : Variable(variableName), nBins{_nBins}, lowBin{_lowBin}, highBin{_highBin} {}

        int getNbins() const { return nBins; }

        float getLowBin() const { return lowBin; }

        float getHighBin() const { return highBin; }

      private:
        int nBins;
        float lowBin;
        float highBin;
      };

      typedef std::variant<double, int, bool> VariableType;
      typedef std::vector<std::variant<TypedVariable, BinnedVariable>> Variables;
      typedef std::map<std::string, VariableType> EvaluatedVariables;
    }
  }
}