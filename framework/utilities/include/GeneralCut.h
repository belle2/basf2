/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/utilities/Conversion.h>

#include <string>
#include <vector>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <sstream>

#include <iostream>
#include <stdexcept>
#include <cmath>
#include <variant>

namespace Belle2 {

  /**
   * Returns position of the matched closing parenthesis if the first character in the given
   * string contains an opening parenthesis. Otherwise return 0.
   */
  unsigned long int findMatchedParenthesis(std::string str, char open = '[', char close = ']');

  /**
   * Split into std::vector on delimiter ignoring delimiters between parenthesis
   */
  std::vector<std::string> splitOnDelimiterAndConserveParenthesis(std::string str, char delimiter, char open, char close);

  /**
   * Returns the position of a pattern in a string ignoring everything that is in parenthesis.
   */
  unsigned long int findIgnoringParenthesis(std::string str, std::string pattern, unsigned int begin = 0);

  /**
   * Helper function to test if two floats are almost equal.
   */
  bool almostEqualFloat(const float& a, const float& b);

  /**
   * Helper function to test if two doubles are almost equal.
   */
  bool almostEqualDouble(const double& a, const double& b);

  /**
     * This is a class template which takes a template class operation as template argument.
     * This allows passing the functional class templates e.g std::greater<T>,  which are templates themselves.
     *
     * In the Nodes we often have to compare two node evaluation results with each other.
     * They are of type type `variant<double, int, bool>`. Variants cannot be compared to each other directly, you have to extract the values and compare them.
     * This gives nine different combinations for two variants.
     * C++ introduced the std::visit concept for this purpose of variant evaluation.
     * std::visit takes a Visitor class and the variants as parameters.
     * One way to write a Visitor is the following way, where a operator() overload is supplied for every data type combination the variants can have.
     * The visitor has to be exhaustive (every data type combination must be covered), and every operator() overload has to have the same return type.
     *
     * We have to do this comparisons for all comparison operators e.g ==, !=, > ...
     * We can do this by passing the corresponding functional class template e.g std::equal_to<T>, std::not_equal_to<T>, std::greater<T>
     * The datatype T is substituted in the operator() overload depending on the data type combination.
     *
     * When comparing double/int and a bool the double/int overload of the functionals are used to disable implicit conversion to bool:
     * std::equal_to<bool>{}(1.2, true) ==> true; 1.2 is implicitly converted to true, because of std::equal<bool>
     * std::equal_to<double>{}(1.2, true) ==> false; true is implicity converted to 1.0, because of std::equal<double>
     */
  template <template <typename type> class operation>
  struct Visitor {
    /**
     * double double overload with double comparison.
     **/
    bool operator()(const double& val0, const double& val1)
    {
      return operation<double> {}(val0, val1);
    }
    /**
     * double int overload with double comparison.
     **/
    bool operator()(const double& val0, const int& val1)
    {
      return operation<double> {}(val0, val1);
    }
    /**
     * double bool  overload with double comparison.
     **/
    bool operator()(const double& val0, const bool& val1)
    {
      return operation<double> {}(val0, val1);
    }
    /**
     * int int overload with int comparison.
     **/
    bool operator()(const int& val0, const int& val1)
    {
      return operation<int> {}(val0, val1);
    }
    /**
     * int bool overload with int comparison.
     **/
    bool operator()(const int& val0, const bool& val1)
    {
      return operation<int> {}(val0, val1);
    }
    /**
     * int double overload with double comparison.
     **/
    bool operator()(const int& val0, const double& val1)
    {
      return operation<double> {}(val0, val1);
    }
    /**
     * bool bool overload with bool comparison.
     **/
    bool operator()(const bool& val0, const bool& val1)
    {
      return operation<bool> {}(val0, val1);
    }
    /**
     * bool double overload with double comparison.
     **/
    bool operator()(const bool& val0, const double& val1)
    {
      return operation<double> {}(val0, val1);
    }
    /**
     * bool int overload with int comparison.
     **/
    bool operator()(const bool& val0, const int& val1)
    {
      return operation<int> {}(val0, val1);
    }
  };

  /**
   * Seperate Visitor struct for equal_to comparison of variant<double, int bool>.
   * Uses almostEqualDouble if one argument is double.
  **/
  struct EqualVisitor {
    /**
     * double double overload with double comparison.
     **/
    bool operator()(const double& val0, const double& val1)
    {
      return almostEqualDouble(val0, val1);
    }
    /**
     * double int overload with double comparison.
     **/
    bool operator()(const double& val0, const int& val1)
    {
      return almostEqualDouble(val0, val1);
    }
    /**
     * double bool  overload with double comparison.
     **/
    bool operator()(const double& val0, const bool& val1)
    {
      return almostEqualDouble(val0, val1);
    }
    /**
     * int int overload with int comparison.
     **/
    bool operator()(const int& val0, const int& val1)
    {
      return std::equal_to<int> {}(val0, val1);
    }
    /**
     * int bool overload with int comparison.
     **/
    bool operator()(const int& val0, const bool& val1)
    {
      return std::equal_to<int> {}(val0, val1);
    }
    /**
     * int double overload with double comparison.
     **/
    bool operator()(const int& val0, const double& val1)
    {
      return almostEqualDouble(val0, val1);
    }
    /**
     * bool bool overload with bool comparison.
     **/
    bool operator()(const bool& val0, const bool& val1)
    {
      return std::equal_to<bool> {}(val0, val1);
    }
    /**
     * bool double overload with double comparison.
     **/
    bool operator()(const bool& val0, const double& val1)
    {
      return almostEqualDouble(val0, val1);
    }
    /**
     * bool int overload with int comparison.
     **/
    bool operator()(const bool& val0, const int& val1)
    {
      return std::equal_to<int> {}(val0, val1);
    }

  };



  /**
   * This class implements a common way to implement cut/selection functionality for arbitrary objects.
   * Every module which wants to perform cuts should use this object.
   * As a parameter the module requires a std::string with the written cut.
   * This std::string has to be passed as an argument to the static Compile method of the Cut class, which returns a unique_ptr to the Cut object.
   * Cuts can be performed via the check method.
   *
   * Valid cuts can contain:
   * 1. Logic conditions: and, or
   * 2. Numeric conditions: <, <=, >, >=, ==, !=
   * 3. Square brackets []
   * 4. Floats
   * 5. Variables registered in the general "Variable Manager" which are used as a template argument to this class.
   *
   * For example when using the analysis VariableManager for particles, valid cuts are:
   * 1.2 < M < 1.5
   * daughter0(M) < daughter1(M)
   * [M > 1.5 or M < 0.5] and 0.2 < getExtraInfo(SignalProbability) < 0.7
   *
   * == and != conditions are evaluated not exactly because we deal with floating point values
   * instead two floating point number are equal if their distance in their integral ordering is less than 3.
   *
   * The general "Variable Manager" passed as a template argument to this class has to have some properties:
   *  * public typedef Object: Which objects can be handled by the variable manager - a pointer on this type ob objects will
   *    be required by the check method of the cut.
   *  * public typedef Var: The type of objects, that are returned by the variable manager, when you ask it for a variable (by giving a name to getVariable)
   *  * public static function getInstance: so the variable manager has to be a singleton.
   *  * public function getVariable(const std::string& name): which should return a pointer to an object of type AVariableManager::Var which are used to get
   *    the value corresponding to this name. Whenever this value is needed, the function called "function" is called with a pointer to a Object, that is
   *    given in the check function of this cut.
   *
   *  The best example for a VariableManager, that has all these parameters, is probably the analysis VariableManager with VariableManager::var equals
   *  to the analysis variable and the VariableManager::Object equal to a Particle.
   *  For a more slim example of a valid variable manager, see the generalCut.cc test, where a mock variable manager is created.
   */
  template <class AVariableManager>
  class GeneralCut {
    /// Object, that can be checked. This depends on the VariableManager, as the returned variables from the manager must calculate their values on pointers of these objects.
    typedef typename AVariableManager::Object Object;
    /// Variable returned by the variable manager.
    typedef typename AVariableManager::Var Var;

  public:
    /**
     * Creates an instance of a cut and returns a unique_ptr to it, if you need a copy-able object instead
     * you can cast it to a shared_ptr using std::shared_ptr<Variable::Cut>(Cut::compile(cutString))
     * @param cut the string defining the cut
     * @return std::unique_ptr<Cut>
     */
    static std::unique_ptr<GeneralCut> compile(const std::string& cut)
    {
      return std::unique_ptr<GeneralCut>(new GeneralCut(cut));
    }
    /**
     * Check if the current cuts are passed by the given object
     * @param p pointer to the object, that should be checked. All formerly received variables from the variable manager
     * (from the type Var), are asked for their value using var->function(p).
     */
    bool check(const Object* p) const
    {
      switch (m_operation) {
        case EMPTY:
          return true;
        case NONE:
          if (std::holds_alternative<double>(this->get(p))) {
            return std::isnan(std::get<double>(this->get(p))) ? false : std::get<double>(this->get(p));
          } else if (std::holds_alternative<int>(this->get(p))) {
            return std::get<int>(this->get(p));
          } else return std::get<bool>(this->get(p));
        case AND:
          return m_left->check(p) and m_right->check(p);
        case OR:
          return m_left->check(p) or m_right->check(p);
        case LT:
          return std::visit(Visitor<std::less> {}, m_left->get(p), m_right->get(p));
        case LE:
          return std::visit(Visitor<std::less_equal> {}, m_left->get(p), m_right->get(p));
        case GT:
          return std::visit(Visitor<std::greater> {}, m_left->get(p), m_right->get(p));
        case GE:
          return std::visit(Visitor<std::greater_equal> {}, m_left->get(p), m_right->get(p));
        case EQ:
          return std::visit(EqualVisitor {}, m_left->get(p), m_right->get(p));
        case NE:
          return !std::visit(EqualVisitor {}, m_left->get(p), m_right->get(p));
      }

      throw std::runtime_error("Cut string has an invalid format: Invalid operation");
      return false;
    }

    /**
     * Print cut tree
     */
    void print() const
    {
      switch (m_operation) {
        case EMPTY: std::cout << "EMPTY" << std::endl; break;
        case NONE: std::cout << "NONE" << std::endl; break;
        case AND: std::cout << "AND" << std::endl; break;
        case OR: std::cout << "OR" << std::endl; break;
        case LT: std::cout << "LT" << std::endl; break;
        case LE: std::cout << "LE" << std::endl; break;
        case GT: std::cout << "GT" << std::endl; break;
        case GE: std::cout << "GE" << std::endl; break;
        case EQ: std::cout << "EQ" << std::endl; break;
        case NE: std::cout << "NE" << std::endl; break;
      }
      if (m_left != nullptr) {
        std::cout << "Left " << std::endl;
        m_left->print();
        std::cout << "End Left" << std::endl;
      }
      if (m_right != nullptr) {
        std::cout << "Right " << std::endl;
        m_right->print();
        std::cout << "End Right" << std::endl;
      }
    }

    /**
     * Do the compilation from a string in return. In principle, compile(decompile()) should give the same result again.
     */
    std::string decompile() const
    {
      std::stringstream stringstream;
      if (m_operation == EMPTY) {
        return "";
      } else if (m_left != nullptr and m_right != nullptr) {

        stringstream << "[";
        stringstream << m_left->decompile();

        switch (m_operation) {
          case AND: stringstream << " and "; break;
          case OR: stringstream << " or "; break;
          case LT: stringstream << " < "; break;
          case LE: stringstream << " <= "; break;
          case GT: stringstream << " > "; break;
          case GE: stringstream << " >= "; break;
          case EQ: stringstream << " == "; break;
          case NE: stringstream << " != "; break;
          default: throw std::runtime_error("Cut string has an invalid format: Operator does not support left and right!"); break;
        }

        stringstream << m_right->decompile();
        stringstream << "]";

      } else if (m_left == nullptr and m_right == nullptr) {
        switch (m_operation) {
          case NONE:
            if (m_isNumeric) {
              stringstream << m_number;
            } else if (m_var != nullptr) {
              stringstream << m_var->name;
            } else {
              throw std::runtime_error("Cut string has an invalid format: Variable is empty!");
            }
            break;
          default: throw std::runtime_error("Cut string has an invalid format: Invalid operator without left and right!"); break;
        }
      } else {
        throw std::runtime_error("Cut string has an invalid format: invalid combination of left and right!");
      }

      return stringstream.str();
    }


  private:
    /**
     * Constructor of the cut. Call init with given string
     * @param str Cut is initalized with the specified cuts. Default are no cuts
     */
    explicit GeneralCut(std::string str)
    {
      str = preprocess(str);
      if (str.empty()) {
        m_operation = EMPTY;
        return;
      }

      if (not processLogicConditions(str)) {
        if (not processTernaryNumericConditions(str)) {
          if (not processBinaryNumericConditions(str)) {
            m_operation = NONE;
            try {
              m_number = Belle2::convertString<double>(str);
              m_isNumeric = true;
            } catch (std::invalid_argument&) {
              m_isNumeric = false;
              processVariable(str);
            }
          }
        }
      }
    }

    /**
     * Delete Copy constructor
     */
    GeneralCut(const GeneralCut&) = delete;

    /**
     * Delete assign operator
     */
    GeneralCut& operator=(const GeneralCut&) = delete;

    /**
     * Preprocess cut string. Trim string and delete global parenthesis
     */
    std::string preprocess(std::string str) const
    {
      boost::algorithm::trim(str);

      while (str.size() > 1 and findMatchedParenthesis(str) == str.size() - 1) {
        str = str.substr(1, str.size() - 2);
        boost::algorithm::trim(str);
      }

      return str;
    }

    /**
     * Look for logical conditions in the given cut string
     */
    bool processLogicConditions(std::string str)
    {
      unsigned long int begin = findMatchedParenthesis(str);
      unsigned long int pos = 0;

      if ((pos =  findIgnoringParenthesis(str, " or ", begin)) != std::string::npos) {
        m_operation = OR;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 4)));
        return true;
      }

      if ((pos =  findIgnoringParenthesis(str, " and ", begin)) != std::string::npos) {
        m_operation = AND;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 5)));
        return true;
      }

      return false;
    }

    /**
     * Look for numeric binary conditions (e.g. 1.2 < M ) in the given cut string
     */
    bool processBinaryNumericConditions(std::string str)
    {
      unsigned long int pos = 0;
      if ((pos =  findIgnoringParenthesis(str, "<=")) != std::string::npos) {
        m_operation = LE;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 2)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, "<")) != std::string::npos) {
        m_operation = LT;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 1)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, ">=")) != std::string::npos) {
        m_operation = GE;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 2)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, ">")) != std::string::npos) {
        m_operation = GT;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 1)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, "==")) != std::string::npos) {
        m_operation = EQ;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 2)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, "!=")) != std::string::npos) {
        m_operation = NE;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 2)));
        return true;
      }

      return false;
    }

    /**
     * Look for numeric ternary conditions (e.g. 1.2 < M < 1.5 ) in the given cut string
     */
    bool processTernaryNumericConditions(std::string str)
    {
      for (auto& c : {"<", ">", "!", "="}) {

        unsigned long int pos1 = 0;
        unsigned long int pos2 = 0;

        if (((pos1 =  findIgnoringParenthesis(str, c)) != std::string::npos) and
            ((pos2 =  findIgnoringParenthesis(str, "<", pos1 + 2)) != std::string::npos
             or (pos2 =  findIgnoringParenthesis(str, ">", pos1 + 2)) != std::string::npos
             or (pos2 =  findIgnoringParenthesis(str, "!", pos1 + 2)) != std::string::npos
             or (pos2 =  findIgnoringParenthesis(str, "=", pos1 + 2)) != std::string::npos)) {
          m_operation = AND;
          m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos2)));
          if (str[pos1 + 1] == '=')
            pos1++;
          m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos1 + 1)));
          return true;
        }
      }

      return false;
    }

    /**
     * Get a variable with the given name from the variable manager using
     * its getVariable(name) function.
     */
    void processVariable(const std::string& str)
    {
      AVariableManager& manager = AVariableManager::Instance();
      m_var = manager.getVariable(str);
      if (m_var == nullptr) {
        throw std::runtime_error(
          "Cut string has an invalid format: Variable not found: " + str);
      }
    }

    /**
     * Returns stored number or Variable value for the given object.
     */
    std::variant<double, int, bool> get(const Object* p) const
    {
      if (m_isNumeric) {
        return m_number;
      } else if (m_var != nullptr) {
        return m_var->function(p);
      } else {
        throw std::runtime_error("Cut string has an invalid format: Neither number nor variable name");
      }
    }

    /**
     * Enum with the allowed operations of the Cut Tree
     */
    enum Operation {
      EMPTY = 0,
      NONE,
      AND,
      OR,
      LT,
      LE,
      GT,
      GE,
      EQ,
      NE,
    } m_operation; /**< Operation which connects left and right cut */
    const Var* m_var; /**< set if there was a valid variable in this cut */
    double m_number; /**< literal number contained in the cut */
    bool m_isNumeric; /**< if there was a literal number in this cut */
    std::unique_ptr<GeneralCut> m_left; /**< Left-side cut */
    std::unique_ptr<GeneralCut> m_right; /**< Right-side cut */
  };
}
