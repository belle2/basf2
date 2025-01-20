#pragma once

#include <variant>
#include <string>
#include <functional>
#include <stdexcept>
#include <map>
#include <memory>

#include <svd/variables/ClusterVariables.h>
// #include <svd/variables/Utils.h>

namespace Belle2 {
  class SVDCluster;
  namespace SVD {

    namespace Variables {
      enum class VariableDataType {
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

      // Do wywalenia
      typedef SVDCluster InputType;
      typedef std::variant<double, int, bool> ReturnType;
      typedef std::function<ReturnType(const InputType*)> FunctionPtr;

      class ComputableVariable {
      public:
        ComputableVariable(const std::string& _name)
          : name{_name}
            // TODO: Pass actual arguments
          , description{""}
          , dataType{VariableDataType::c_double}
        {
          if (name == "clusterCharge") {
            functionPtr = clusterCharge;
          } else if (name == "clusterChargeNormalized") {
            functionPtr = clusterChargeNormalized;
          } else if (name == "clusterSNR") {
            functionPtr = clusterSNR;
          } else if (name == "clusterSize") {
            functionPtr = clusterSize;
          }
        }

        const std::string& getName() const { return name; }

        const std::string& getDescription() const { return description; }

        const VariableDataType& getDataType() const { return dataType; }

        const ReturnType operator()(const InputType* input) const
        {
          if (not functionPtr) {
            // TODO: Improve exception handling
            throw std::runtime_error("Function pointer not initialized!");
          }
          return functionPtr(input);
        }

      private:
        std::string name;
        std::string description;
        VariableDataType dataType;
        FunctionPtr functionPtr;
      };

      typedef std::vector<std::variant<TypedVariable, BinnedVariable>> Variables;
      typedef std::vector<ComputableVariable> ComputableVariables;
      typedef std::map<std::string, ReturnType> EvaluatedVariables;
    }
  }
}