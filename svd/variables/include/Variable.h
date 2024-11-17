#pragma once

#include <variant>
#include <string>
#include <functional>
#include <stdexcept>
#include <map>

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

      typedef SVDCluster InputType;
      typedef std::variant<int, double, bool> ReturnType;
      typedef std::function<ReturnType(const InputType*)> FunctionPtr;

      class ComputableVariable {
      public:
        ComputableVariable(const std::string& _name)
          : name{_name}
            // TODO: Pass actual arguments
          , description{""}
          , dataType{VariableDataType::c_double}
          , functionPtr{clusterCharge} {}

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

      typedef std::vector<ComputableVariable> ComputableVariables;
      typedef std::map<std::string, ReturnType> EvaluatedVariables;
    }
  }

}