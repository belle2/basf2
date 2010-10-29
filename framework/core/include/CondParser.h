/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CONDPARSER_H_
#define CONDPARSER_H_

#include <string>
#include <map>

namespace Belle2 {

  /**
   * The condition parser class.
   * Implements a condition parser.
   *
   * It supports conditions of the form [comparison operator][integer]
   * e.g. ">5", "=7", "!= 0"
   *
   * Additional spaces in front of the operator, between operator and integer number
   * and after the integer number are allowed.
   *
   * Supported operators are: "> , < , = , == , >= , <= , !="
   */
  class CondParser {

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

    /**
     * Constructor.
     */
    CondParser();

    /**
     * Destructor.
     */
    virtual ~CondParser();

    /**
     * Parses a given condition expression and returns the operator and the integer value.
     *
     * The condition has to be of the form: [comparison operator][integer]
     * e.g. ">5", "=7", "!= 0"
     * Additional spaces in front of the operator, between operator and integer number
     * and after the integer number are allowed.
     * Supported operators are: "> , < , = , == , >= , <= , !="
     *
     * @param expression The expression of the condition.
     * @return Returns true of the expression is valid.
     */
    bool parseCondition(std::string expression, EConditionOperators& condOperator, int& condValue) const;

    /**
     * Evaluates a condition, given by two input values and a condition (comparison) operator.
     *
     * The evaluation is as follows: value1 [operator] value2.
     *
     * @param value1 The first value which should be compared to the second value
     * @param value2 The second value which should be compared to the first value
     * @param condOperator The condition (comparison) operator
     * @return True if the condition (comparison) is evaluated to true.
     */
    static bool evalCondition(const int& value1, const int& value2, const EConditionOperators& condOperator);


  protected:

  private:

    std::map<std::string, EConditionOperators> m_conditionOperatorMap; /**< Maps the operator string to a EConditionOperators value. */

  };

} //end of Belle2 namespace

#endif /* CONDPARSER_H_ */
