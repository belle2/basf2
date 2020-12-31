/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/utilities/Utils.h>
#include <sstream>
#include <variant>
#include <stack>
#include <string>


namespace Belle2 {

  /** Base class with the non-templated part of the formula parser. Not very
   * useful on its own.  @see FormulaParser
   */
  class FormulaParserBase {
  public:
    /** List of known operators.
     * The second word encodes the operator precedence */
    enum class EOperator : unsigned char {
      c_noop               = 0x00, /**< No operation */
      c_roundBracketOpen   = 0x01, /**< Open round bracket */
      c_roundBracketClose  = 0x02, /**< Close round bracket */
      c_squareBracketOpen  = 0x03, /**< Open square bracket */
      c_squareBracketClose = 0x04, /**< Close square bracket */
      c_plus               = 0x11, /**< Addition */
      c_minus              = 0x12, /**< Subtraction */
      c_multiply           = 0x21, /**< Multiply */
      c_divide             = 0x22, /**< Division */
      c_power              = 0x31  /**< Exponentation */
    };

    /** States of a string literal when checking if it is a valid number */
    enum class ENumberStatus {
      c_Invalid,      /**< Not a valid number */
      c_Empty,        /**< Empty string */
      c_Sign,         /**< Leading sign */
      c_Int,          /**< [leading sign] + digits */
      c_Dot,          /**< [leading sign] + digits + dot */
      c_LeadingDot,   /**< leading dot without preceding digits */
      c_Float,        /**< [leading sign] + [digits] + dot + digits */
      c_Exponent,     /**< [float] + E or e */
      c_ExponentSign, /**< exponent followed by plus or minus */
      c_Scientific    /**< exponent followed by sign and digits */
    };

    /** Input token type: an input tokein is either a string or a float variable */
    typedef std::variant<std::string, double> InputToken;

    /** Convert operator code to character */
    static char operatorToChar(EOperator op) noexcept;

    /** Apply operator on two values */
    static double applyOperator(EOperator op, double a, double b);

    /** Check if a string literal with a given number status continues to be a
     * valid number if next is appended to it */
    static ENumberStatus checkNumber(ENumberStatus current, char next);

    /** Default constructor */
    FormulaParserBase() = default;

    /** virtual, but empty destructor */
    virtual ~FormulaParserBase() = default;
  protected:
    /** Make sure we have enough operands to use an operator */
    static void assertOperatorUsable(size_t stacksize);
    /** Execute an operator on the current state */
    virtual void executeOperator(EOperator op) = 0;
    /** Add a variable token to the current state */
    virtual void addVariable(const InputToken& token) = 0;
    /** Process the given formula and store the final state */
    void processString(const std::string& formula);
    /** Format the given runtime_error with context information and rethrow a
     * new one */
    [[noreturn]] void raiseError(const std::runtime_error& e);
  private:
    /** Add an operator to the internal state, convert them to reverse polish
     * notation using the shunting yard algorithm and execute them as they
     * become available */
    void addOperator(EOperator op);
    /** Flush the currently parsed variable name and add it to the state either
     * as variable or number */
    void flushCurrentVariable();
    /** Flush all pending operators at the end of processing */
    void flushPendingOperators();
    /** Check if the next character is a operator */
    EOperator checkForOperator(char next);
    /** Bool to check whether there were consecutive operators or variables */
    bool m_lastTokenWasOperator;
    /** Buffer for the formula */
    std::istringstream m_buffer;
    /** collect characters into a variable name */
    std::string m_currentVariableName;
    /** State of the current variable name being a valid float literal */
    ENumberStatus m_currentVariableNameNumberStatus;
    /** Stack of operators for the Shunting-yard algorithm */
    std::stack<EOperator> m_operatorStack;
  };


  /** FormulaParser to parse a text formula like "a + b * c ^ d" where the
   * separate parts can be either variables (with and without arguments) or
   * numbers.
   *
   * The output can be configured by providing a custom VariableConstructor
   * which creates the desired type of output variables. This
   * VariableConstructor object needs to have
   *
   * 1. a member type `type` specifying the desired type of the output object
   * 2. overloads for the call operator to create such objects from strings,
   *    doubles and to combine variables and doubles with a given EOperator.
   *    The necessary overloads are
   *
   *    - `type operator()(const std::string &name)`
   *    - `type operator()(double value)`
   *    - `type operator()(EOperator op, const type& a, const type& b)`
   *    - `type operator()(EOperator op, double a, const type &b)`
   *    - `type operator()(EOperator op, const type &a, double b)`
   *
   * @see StringFormulaConstructor for an example
   */
  template<class VariableConstructor> class FormulaParser: public FormulaParserBase {
  public:
    /** Type of the return variable object */
    typedef typename VariableConstructor::type VariableType;
    /** typedef for output tokens on the stack: either a variable or a double */
    typedef std::variant<VariableType, double> OutputToken;

    /** Parse the formula and return a varible object of the correct type.
     * Throws  std::runtime_error if there is a problem with parsing.
     */
    VariableType parse(const std::string& formula)
    {
      try {
        // clear stacks
        std::stack<OutputToken>().swap(m_outputStack);
        // process
        processString(formula);
        // sanity checks
        if (m_outputStack.empty())
          throw std::runtime_error("empty string");
        if (m_outputStack.size() != 1)
          throw std::runtime_error("could not parse, stack size not 1, probably a bug, please report");
        // so return the object
        return std::visit(Utils::VisitOverload{
          [](const VariableType & v) -> VariableType {return v;},
          [](double d) -> VariableType { return VariableConstructor()(d); }
        }, m_outputStack.top());
      } catch (std::runtime_error& e) {
        // add context to error
        raiseError(e);
      }
    }
  protected:
    /** Execute the given operator by taking the operands from the stack and applying the operator to them */
    void executeOperator(EOperator op) override
    {
      assertOperatorUsable(m_outputStack.size());
      // so far all a binary operators
      // cppcheck-suppress unreadVariable; used in lambda below
      OutputToken op2 = m_outputStack.top(); m_outputStack.pop();
      // cppcheck-suppress unreadVariable; used in lambda below
      OutputToken op1 = m_outputStack.top(); m_outputStack.pop();
      // and apply ...
#ifndef __INTEL_COMPILER
      // FIXME: Remove the masking of this code for the intel compiler when there is a version that supports it.
      m_outputStack.push(std::visit(Utils::VisitOverload {
        // eagerly apply operations if both operands are numbers
        [op](double a, double b) -> OutputToken { return OutputToken(applyOperator(op, a, b)); },
        // otherwise defer to variable constructor
        [op](auto a, auto b) -> OutputToken { return VariableConstructor()(op, a, b); },
      }, op1, op2));
#else
      throw std::runtime_error("the formula parsing is not working with the intel compiler");
#endif
    }

    /** Add the variable to the output token stack, create it from a string or keep it as it is */
    void addVariable(const InputToken& var) override
    {
#ifndef __INTEL_COMPILER
      // FIXME: Remove the masking of this code for the intel compiler when there is a version that supports it.
      m_outputStack.push(std::visit(Utils::VisitOverload {
        // if the variable is a string its an identifier and we have to construct a variable from it
        [](const std::string & s) -> OutputToken { return VariableConstructor()(s); },
        // otherwise keep as is
        [](auto s) -> OutputToken { return s; }
      }, var));
#else
      throw std::runtime_error("the formula parsing is not working with the intel compiler");
#endif
    }

    /** Stack of output tokens in the reversh polish notation */
    std::stack<OutputToken> m_outputStack;
  };

  /** Example struct to be used with the FormulaParser to create a string
   * representation of the formula, mainly for testing and debugging.
   */
  struct StringFormulaConstructor {
    /** Create strings as output of FormulaParser */
    typedef std::string type;
    /** Shorthand for the operator enum */
    typedef FormulaParserBase::EOperator Op;

    /** Create output type from a string identifier: let's quote it */
    type operator()(const std::string& name) { return "'" + name + "'"; }
    /** Create output type from a double: just convert to string */
    type operator()(double value) { return std::to_string(value); }
    /** Apply operator to two output types: just add brackets and operator */
    type operator()(Op op, const type& a, const type& b)
    {
      return "(" + a + FormulaParserBase::operatorToChar(op) + b + ")";
    }
    /** Apply operator to double and output type */
    type operator()(Op op, double a, const type& b) { return (*this)(op, std::to_string(a), b); }
    /** Apply operator to output type and double */
    type operator()(Op op, const type& a, double b) { return (*this)(op, a, std::to_string(b)); }
  };

}
