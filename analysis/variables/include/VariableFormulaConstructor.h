/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <analysis/VariableManager/Manager.h>
#include <framework/utilities/FormulaParser.h>

namespace Belle2 {

  /** Struct to construct new variable function objects from a name or a double value or to apply operations on
   * these variable function objects. Used for the `formula()` meta variable and passed to FormulaParser */
  struct VariableFormulaConstructor {
    /** Return value we want for the FormulaParser::parse */
    typedef Variable::Manager::FunctionPtr type;
    /** Shorthand for the operator enum */
    typedef FormulaParserBase::EOperator Op;
    /** Construct a variable from a given name */
    type operator()(const std::string& name)
    {
      auto var = Variable::Manager::Instance().getVariable(name);
      if (!var) throw std::runtime_error("Could not find " + name + " via the Variable::Manager. Check the name");
      return var->function;
    }

    /** Construct a variable from a double value */
    type operator()(double value)
    {
      return [value](const Particle*) { return value; };
    }

    /** Apply operator on two variables */
    type operator()(Op op, const type& a, const type& b)
    {
      return [op, a, b](const Particle * p) {
        switch (op) {
          case Op::c_plus: return a(p) + b(p);
          case Op::c_minus: return a(p) - b(p);
          case Op::c_multiply: return a(p) * b(p);
          case Op::c_divide: return a(p) / b(p);
          case Op::c_power: return std::pow(a(p), b(p));
          default: break;
        }
        throw std::runtime_error("Cannot handle operator " + std::to_string((int)op));
      };
    }

    /** Apply operator on a double and a variable */
    type operator()(Op op, double& a, const type& b)
    {
      return [op, a, b](const Particle * p) {
        switch (op) {
          case Op::c_plus: return a + b(p);
          case Op::c_minus: return a - b(p);
          case Op::c_multiply: return a * b(p);
          case Op::c_divide: return a / b(p);
          case Op::c_power: return std::pow(a, b(p));
          default: break;
        }
        throw std::runtime_error("Cannot handle operator " + std::to_string((int)op));
      };
    }

    /** Apply operator on a variable and a double */
    type operator()(Op op, const type& a, double b)
    {
      return [op, a, b](const Particle * p) {
        switch (op) {
          case Op::c_plus: return a(p) + b;
          case Op::c_minus: return a(p) - b;
          case Op::c_multiply: return a(p) * b;
          case Op::c_divide: return a(p) / b;
          case Op::c_power: return std::pow(a(p), b);
          default: break;
        }
        throw std::runtime_error("Cannot handle operator " + std::to_string((int)op));
      };
    }
  };
}
