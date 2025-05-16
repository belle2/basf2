#pragma once

#include <variant>
#include <string>
#include <functional>
#include <stdexcept>
#include <map>
#include <memory>

namespace Belle2::VariablePersistenceManager {

  enum VariableDataType {
    c_double = 0,
    c_int = 1,
    c_bool = 2
  };

  /**
   * @class Variable
   * @brief Base class representing a generic variable.
   */
  class Variable {
  public:
    /**
     * @brief Constructs a new Variable.
     * @param variableName The name of the variable.
     */
    Variable(const std::string& variableName) : name{variableName} {}

    /**
     * @brief Retrieves the name of the variable.
     * @return The variable name as a std::string.
     */
    virtual std::string getName() const { return name; }

    /**
     * @brief Virtual destructor.
     */
    virtual ~Variable() = default;

  private:
    std::string name; /**< The name of the variable. */
  };

  /**
   * @class TypedVariable
   * @brief A variable with a specified data type.
   */
  class TypedVariable : public Variable {
  public:
    /**
     * @brief Constructs a new TypedVariable.
     * @param variableName The name of the variable.
     * @param variableDataType The data type of the variable (e.g., c_double, c_int, c_bool).
     */
    TypedVariable(const std::string& variableName, const VariableDataType& variableDataType)
      : Variable(variableName)
      , dataType{variableDataType}
    {}

    /**
     * @brief Retrieves the data type of this variable.
     * @return The @c VariableDataType enum value.
     */
    VariableDataType getDataType() const { return dataType; }

  private:
    VariableDataType dataType; /**< The data type of the variable. */
  };

  /**
   * @class BinnedVariable
   * @brief A variable with binning information.
   */
  class BinnedVariable : public Variable {
  public:
    /**
     * @brief Constructs a new BinnedVariable.
     * @param variableName The name of the variable.
     * @param _nBins The number of bins.
     * @param _lowBin The lower edge of the bin range.
     * @param _highBin The upper edge of the bin range.
     */
    BinnedVariable(const std::string& variableName, int _nBins, float _lowBin, float _highBin)
      : Variable(variableName)
      , nBins{_nBins}
      , lowBin{_lowBin}
      , highBin{_highBin}
    {}

    /**
     * @brief Retrieves the number of bins.
     * @return The number of bins.
     */
    int getNbins() const { return nBins; }

    /**
     * @brief Retrieves the lower bin edge.
     * @return The lower bin edge.
     */
    float getLowBin() const { return lowBin; }

    /**
     * @brief Retrieves the upper bin edge.
     * @return The upper bin edge.
     */
    float getHighBin() const { return highBin; }

  private:
    int nBins;    /**< The number of bins. */
    float lowBin; /**< The lower bin boundary. */
    float highBin;/**< The upper bin boundary. */
  };

  /**
   * @typedef VariableType
   * @brief Variant type that can hold a double, an int, or a bool.
   */
  typedef std::variant<double, int, bool> VariableType;

  /**
   * @typedef Variables
   * @brief A vector containing a variant of either TypedVariable or BinnedVariable.
   */
  typedef std::vector<std::variant<TypedVariable, BinnedVariable>> Variables;

  /**
   * @typedef EvaluatedVariables
   * @brief A map from variable names (std::string) to a VariableType.
   */
  typedef std::map<std::string, VariableType> EvaluatedVariables;
}