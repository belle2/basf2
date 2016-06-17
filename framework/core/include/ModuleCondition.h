#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  class Path;
  /** Wraps a condition set on a Module instance.
   *
   * It supports conditions of the form [comparison operator][integer]
   * e.g. ">5", "=7", "!= 0"
   *
   * Additional spaces in front of the operator, between operator and integer number
   * and after the integer number are allowed.
   *
   * Supported operators are: "> , < , = , == , >= , <= , !="
   */
  class ModuleCondition {
  public:
    /** The supported condition operators. */
    enum EConditionOperators {
      c_GT,    /**< Greater than:          ">"  */
      c_ST,    /**< Smaller than:          "<"  */
      c_GE,    /**< Greater or equal than: ">=" */
      c_SE,    /**< Smaller or equal than: "<=" */
      c_EQ,    /**< Equal:                 "=" or "=="  */
      c_NE     /**< Not equal:             "!=" */
    };

    /** initialize from string expression (see class doc). Throws runtime_error if expression is invalid. */
    ModuleCondition(std::string expression, boost::shared_ptr<Path> conditionPath, Module::EAfterConditionPath afterConditionPath);
    /** copy ctor (uses Path::clone()). */
    ModuleCondition(const ModuleCondition& other);
    ~ModuleCondition() { };

    /** evaluate the condition using the given value. E.g. for a condition ">5", this would return "value>5" */
    bool evaluate(int value) const;

    /** Returns the path of the condition.  */
    const boost::shared_ptr<Path>& getPath() const {return m_conditionPath; };

    /** Returns the value of the condition.  */
    int getConditionValue() const {return m_conditionValue; };

    /** Returns the value of the condition.  */
    EConditionOperators getConditionOperator() const {return m_conditionOperator; };

    /** What to do after a conditional path is finished. */
    Module::EAfterConditionPath getAfterConditionPath() const { return m_afterConditionPath; }

    /** A string representation of this condition. */
    std::string getString() const;

  private:
    /** no default constructed objects. */
    ModuleCondition() = delete;

    boost::shared_ptr<Path> m_conditionPath; /**< The path which which will be executed if the condition is evaluated to true. */
    EConditionOperators m_conditionOperator;  /**< The operator of the condition (set by parsing the condition expression). */
    int m_conditionValue;                    /**< Numeric value used in the condition (set by parsing the condition expression). */
    Module::EAfterConditionPath m_afterConditionPath; /**< What to do after a conditional path is finished. */
  };
}
