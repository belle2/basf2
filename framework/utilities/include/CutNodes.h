/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <memory>
#include <iostream>
#include <functional>

#include <framework/utilities/AbstractNodes.h>
#include <framework/utilities/NodeFactory.h>
#include <framework/logging/Logger.h>

#include <boost/algorithm/string.hpp>
#include <boost/python/tuple.hpp>

namespace py = boost::python;
typedef const py::tuple& Nodetuple;

namespace Belle2 {
  /**
   * Forward declaration of function defined in framework/utilities/GeneralCut.cc
  **/
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
   * std::equal<bool>{}(1.2, true) ==> true; 1.2 is implicitly converted to true, because of std::equal<bool>
   * std::equal<double>{}(1.2, true) ==> false; true is implicity converted to 1.0, because of std::equal<double>
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
  * Nodeclass with a single AbstractBooleanNode as child.
  * check() returns the negated result of the child node if m_negation is true.
  */
  template<class AVariableManager>
  class UnaryBooleanNode : public AbstractBooleanNode<AVariableManager> {
    /**
    * Template argument dependent Particle type definition
    **/
    typedef typename AVariableManager::Object Object;
  public:
    /**
     * Check if Object passes this subexpression of the cut by calling check on the child node.
     * @param p pointer to the object, that should be checked.
     */
    bool check(const Object* p) const override
    {
      // Return negated result of child node evaluation if m_negation is true.
      if (m_negation) return !m_bnode->check(p);

      return m_bnode->check(p);
    }

    /**
     * Print node
     * brackets and negation keywords are added if m_bracketized, m_negation are set to true.
     */
    void print() const override
    {
      if (m_negation) std::cout << "not";
      if (m_bracketized) std::cout << "[";
      m_bnode->print();
      if (m_bracketized) std::cout << "]";
    }

    /** Decompile Node back to a string.
     * Decompile(compile) should give the same result.
     * brackets and negation keywords are added if m_bracketized, m_negation are set to true.
    **/
    std::string decompile() const override
    {
      std::stringstream stringstream;
      if (m_negation) stringstream << "not"; // Add negation keyword

      if (m_bracketized) stringstream << "["; // Add opening bracket
      stringstream << m_bnode->decompile();
      if (m_bracketized) stringstream << "]"; // Add closing bracket

      return stringstream.str();
    }

  private:
    friend class NodeFactory; // friend declaration so that NodeFactory can call the private constructor
    /**
     * @param node (const boost::python::tuple&) tuple containing a boolean node
     * @param negation (bool): flag if check output of m_bnode should be returned negated
     * @param bracketized (bool): flag if cut substring was in brackets, used for preserving brackets when decompling to a string.
    **/
    explicit UnaryBooleanNode(Nodetuple node, bool negation, bool bracketized)
      : m_bnode{NodeFactory::compile_boolean_node<AVariableManager>(node)}, m_negation{negation}, m_bracketized{bracketized}
    {
    }

    std::unique_ptr<const AbstractBooleanNode<AVariableManager>> m_bnode; /**< boolean subexpression of a cut */
    const bool m_negation; /**< if the evaluation of m_bnode->check(p) should be negated in check() */
    const bool
    m_bracketized; /**< if the boolean expression from which this node is compiled was in brackets, relevant in decompile to yield the original string */
  };


  /**
  * Nodeclass with a two AbstractBooleanNode as children and
  * a Boolean Operator (AND, OR)
  * Check() method evaluates the child nodes and returns
  * the result of the boolean operation.
  **/
  template<class AVariableManager>
  class BinaryBooleanNode : public AbstractBooleanNode<AVariableManager> {
    /**
     * Template argument dependent Particle type definition
    **/
    typedef typename AVariableManager::Object Object;
  public:
    /**
      * Check if object passes this subexpression of the cut by calling check on the children nodes.
      * Boolean results are evaluated with the BooleanOperator(AND/OR) the node object.
      * @param p pointer to the object, that should be checked.
      */
    bool check(const Object* p) const override
    {

      switch (m_boperator) {
        case BooleanOperator::AND:
          return m_left_bnode->check(p) && m_right_bnode->check(p);
          break;
        case BooleanOperator::OR:
          return m_left_bnode->check(p) || m_right_bnode->check(p);
          break;
        default:
          throw std::runtime_error("BinaryBooleanNode has an invalid BooleanOperator");
      }
      return false;
    }

    /**
     * Print node
     */
    void print() const override
    {
      m_left_bnode->print();
      injectBooleanOperatorToStream(std::cout, m_boperator);
      m_right_bnode->print();
    }

    /**
     * Decompile Node back to a string.
     * decompile(compile) should give the same result.
     */
    std::string decompile() const override
    {
      std::stringstream stringstream;
      stringstream << m_left_bnode->decompile(); // decompile left AbstractBooleanNode
      injectBooleanOperatorToStream(stringstream, m_boperator);
      stringstream << m_right_bnode->decompile(); // decompile right AbstractBooleanNode

      return stringstream.str();
    }

  private:
    friend class NodeFactory; // friend declaration so that NodeFactory can call the private constructor
    /**
     * @param left_node (const boost::python::tuple&): tuple from which the left boolean child node is constructed
     * @param right_node (const boost::python::tuple&): tuple from which the right boolean child node is constructed
     * @param boperator (BooleanOperator): determines the boolean operation in check(const Object* p)
    **/
    explicit BinaryBooleanNode(Nodetuple left_node, Nodetuple right_node, BooleanOperator boperator)
      : m_left_bnode{NodeFactory::compile_boolean_node<AVariableManager>(left_node)}, m_right_bnode{NodeFactory::compile_boolean_node<AVariableManager>(right_node)},
        m_boperator{boperator}
    {
    }
    std::unique_ptr<const AbstractBooleanNode<AVariableManager>> m_left_bnode; /**< boolean subexpression of a cut */
    std::unique_ptr<const AbstractBooleanNode<AVariableManager>> m_right_bnode; /**< boolean subexpression of a cut */
    const BooleanOperator m_boperator; /**< Boolean operation to be applied to the check() results of the child nodes. */
  };

  /**
   * Nodeclass with a single AbstractExpressioNode as child.
   * Allows subexpressions consisting of a single boolean valued expression
   */
  template<class AVariableManager>
  class UnaryRelationalNode : public AbstractBooleanNode<AVariableManager> {
    /**
    * Template argument dependent Particle type definition
    **/
    typedef typename AVariableManager::Object Object;
  public:
    /**
    * Evaluate the Expression Node child and assert that it is boolean.
    * @param p pointer to the object, that should be checked.
    */
    bool check(const Object* p) const override
    {
      std::variant<double, int, bool> ret = m_enode->evaluate(p);
      if (std::holds_alternative<bool>(ret)) {
        return std::get<bool>(ret);
      } else if (std::holds_alternative<int>(ret)) {
        B2WARNING("Static casting integer value to bool in cutstring evaluation." << LogVar("Cut substring",
                  m_enode->decompile()) << LogVar("Casted value", std::get<int>(ret)));
        return static_cast<bool>(std::get<int>(ret));
      } else if (std::holds_alternative<double>(ret)) {
        B2WARNING("Static casting double value to bool in cutstring evaluation." <<  LogVar("Cut substring",
                  m_enode->decompile()) << LogVar(" Casted value", std::get<double>(ret)));
        return static_cast<bool>(std::get<double>(ret));
      } else {
        throw std::runtime_error("UnaryRelationalNode should evaluate to bool.");
      }
    }

    /**
     * Print node
     */
    void print() const override
    {
      m_enode->print();
    }

    /**
    * Decompile Node back to a string.
    * decompile(compile) should give the same result.
    */
    std::string decompile() const override
    {
      return m_enode->decompile();
    }

  private:
    friend class NodeFactory; // friend declaration so that NodeFactory can call the private constructor
    /**
     * Constructor
     * @param node (const boost::python::tuple&) tuple containing a expression node
    **/
    explicit UnaryRelationalNode(Nodetuple node) : m_enode{NodeFactory::compile_expression_node<AVariableManager>(node)}
    {}
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_enode; /**< subexpression of a cut */
  };

  /** BooleanNode which has two AbstractExpressionNodes nodes
  * and a ComparisonOperator.
  * Check() evaluates the children nodes and compares the results
  * with the specified operator.
  **/
  template<class AVariableManager>
  class BinaryRelationalNode : public AbstractBooleanNode<AVariableManager> {
    /**
    * Template argument dependent Particle type definition
    **/
    typedef typename AVariableManager::Object Object;
  public:
    /**
    * Evaluate the Expression children nodes which yield std::variant values.
    * variant data types cannot be compared in relational expressions, every data type combination has to be considered.
    * @param p pointer to the object, that should be checked.
    * returns the boolean result of the relational expression.
    */
    bool check(const Object* p) const override
    {
      // Get std::variant values of children node
      std::variant<double, int, bool> left_eval = m_left_enode->evaluate(p);
      std::variant<double, int, bool> right_eval = m_right_enode->evaluate(p);
      switch (m_coperator) {
        case ComparisonOperator::EQUALEQUAL:
          return std::visit(Visitor<std::equal_to> {}, left_eval, right_eval);
          break;
        case ComparisonOperator::GREATEREQUAL:
          return std::visit(Visitor<std::greater_equal> {}, left_eval, right_eval);
          break;
        case ComparisonOperator::LESSEQUAL:
          return std::visit(Visitor<std::less_equal> {}, left_eval, right_eval);
          break;
        case ComparisonOperator::GREATER:
          return std::visit(Visitor<std::greater> {}, left_eval, right_eval);
          break;
        case ComparisonOperator::LESS:
          return std::visit(Visitor<std::less> {}, left_eval, right_eval);
          break;

        case ComparisonOperator::NOTEQUAL:
          return std::visit(Visitor<std::not_equal_to> {}, left_eval, right_eval);
          break;
        default:
          throw std::runtime_error("BinaryRelationalNode has an invalid ComparisonOperator.");
      }
      return false;
    }

    /**
     * Print node
     */
    void print() const override
    {
      m_left_enode->print();
      injectComparisonOperatorToStream(std::cout, m_coperator);
      m_right_enode->print();
    }
    /**
     * Decompile Node back to a string.
     * decompile(compile) should give the same result.
     */
    std::string decompile() const override
    {
      std::stringstream stringstream;

      stringstream << m_left_enode->decompile();
      injectComparisonOperatorToStream(stringstream, m_coperator);
      stringstream << m_right_enode->decompile();

      return stringstream.str();
    }

  private:
    friend class NodeFactory; // friend declaration so that NodeFactory can call the private constructor
    /**
     * Constructor
     * @param left_node (const boost::python::tuple&): tuple containing a expression node
     * @param right_node (const boost::python::tuple&): tuple containing a expression node
     * @param coperator (ComparisonOperator): comparison operator enum value
    **/
    explicit BinaryRelationalNode(Nodetuple left_node, Nodetuple right_node, ComparisonOperator coperator)
      : m_left_enode{NodeFactory::compile_expression_node<AVariableManager>(left_node)}, m_right_enode{NodeFactory::compile_expression_node<AVariableManager>(right_node)},
        m_coperator{coperator}
    {
    }
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_left_enode; /**< subexpression of a cut */
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_right_enode; /**< subexpression of a cut */
    const ComparisonOperator m_coperator; /**< comparison operator to be applied to the expression evaluations */
  };

  /** BooleanNode which has three AbstractExpressionNodes nodes
  * and two ComparisonOperator.
  * This allows Ranges in the Cutstring e.g 2.0 < var < 4.3
  * Check() evaluates the children nodes and compares both sides
  * with the specified operator.
  **/
  template<class AVariableManager>
  class TernaryRelationalNode : public AbstractBooleanNode<AVariableManager> {
    /**
    * Template argument dependent Particle type definition
    **/
    typedef typename AbstractBooleanNode<AVariableManager>::Object Object;
  public:
    /**
     * @param p pointer to the object, that should be checked.
     * Checking is done sequentially.
     * At first the left-center condition is checked. If it evaluates to false we can directly return false.
     * If it true we have to check to center-right condition.
     */
    bool check(const Object* p) const override
    {
      std::variant<double, int, bool> left_eval = m_left_enode->evaluate(p);
      std::variant<double, int, bool> center_eval = m_center_enode->evaluate(p);
      std::variant<double, int, bool> right_eval = m_right_enode->evaluate(p);

      switch (m_lc_coperator) {
        case ComparisonOperator::EQUALEQUAL:
          if (not std::visit(Visitor<std::equal_to> {}, left_eval, center_eval)) return false;
          break;
        case ComparisonOperator::GREATEREQUAL:
          if (not std::visit(Visitor<std::greater_equal> {}, left_eval, center_eval)) return false;
          break;
        case ComparisonOperator::LESSEQUAL:
          if (not std::visit(Visitor<std::less_equal> {}, left_eval, center_eval)) return false;

          break;
        case ComparisonOperator::GREATER:
          if (not std::visit(Visitor<std::greater> {}, left_eval, center_eval)) return false;
          break;
        case ComparisonOperator::LESS:
          if (not std::visit(Visitor<std::less> {}, left_eval, center_eval)) return false;
          break;
        case ComparisonOperator::NOTEQUAL:
          if (not std::visit(Visitor<std::not_equal_to> {}, left_eval, center_eval)) return false;
          break;
        default:
          throw std::runtime_error("TernaryRelational has an invalid m_lc_operator");
      }
      switch (m_cr_coperator) {
        case ComparisonOperator::EQUALEQUAL:
          if (not std::visit(Visitor<std::equal_to> {}, center_eval, right_eval)) return false;
          break;
        case ComparisonOperator::GREATEREQUAL:
          if (not std::visit(Visitor<std::greater_equal> {}, center_eval, right_eval)) return false;
          break;
        case ComparisonOperator::LESSEQUAL:
          if (not std::visit(Visitor<std::less_equal> {}, center_eval, right_eval)) return false;

          break;
        case ComparisonOperator::GREATER:
          if (not std::visit(Visitor<std::greater> {}, center_eval, right_eval)) return false;
          break;
        case ComparisonOperator::LESS:
          if (not std::visit(Visitor<std::less> {}, center_eval, right_eval)) return false;
          break;
        case ComparisonOperator::NOTEQUAL:
          if (not std::visit(Visitor<std::not_equal_to> {}, center_eval, right_eval)) return false;
          break;
        default:
          throw std::runtime_error("TernaryRelational has an invalid m_cr_operator");
      }
      return true;
    }
    /**
     * Print node
    **/
    void print() const override
    {
      m_left_enode->print();
      injectComparisonOperatorToStream(std::cout, m_cr_coperator);
      m_center_enode->print();
      injectComparisonOperatorToStream(std::cout, m_lc_coperator);
      m_left_enode->print();
    }

    /**
     * Decompile Node back to a string.
     * decompile(compile) should give the same result.
     */
    std::string decompile() const override
    {
      std::stringstream stringstream;
      stringstream << m_left_enode->decompile();
      injectComparisonOperatorToStream(stringstream, m_cr_coperator);
      stringstream << m_center_enode->decompile();
      injectComparisonOperatorToStream(stringstream, m_lc_coperator);
      stringstream << m_right_enode->decompile();
      return stringstream.str();
    }
  private:
    friend class NodeFactory;
    /**
    * Constructor
    * @param left_node (const boost::python::tuple&): tuple containing a expression node
    * @param center_node (const boost::python::tuple&): tuple containing a expression node
    * @param right_node (const boost::python::tuple&): tuple containing a expression node
    * @param lc_coperator (ComparisonOperator): comparison operator enum value specifying the comparison between left and center expression
    * @param cr_coperator (ComparisonOperator): comparison operator enum value specifiying the comparison between center and right expression
    **/
    explicit TernaryRelationalNode(Nodetuple left_node, Nodetuple center_node, Nodetuple right_node, ComparisonOperator lc_coperator,
                                   ComparisonOperator cr_coperator)
      : m_left_enode{NodeFactory::compile_expression_node<AVariableManager>(left_node)}, m_center_enode{NodeFactory::compile_expression_node<AVariableManager>(center_node)},
        m_right_enode{NodeFactory::compile_expression_node<AVariableManager>(right_node)},  m_lc_coperator{lc_coperator},
        m_cr_coperator{cr_coperator}
    {
    }
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>>
        m_left_enode; /**< left expression of the ternary relational expression */
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>>
        m_center_enode; /**< center expression of the ternary relational expression */
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>>
        m_right_enode; /**< right expression of the ternary relational expression */
    const ComparisonOperator
    m_lc_coperator; /**< comparison operator to be applied to the evaluations of the left and center expression */
    const ComparisonOperator
    m_cr_coperator; /**< comparison operator to be applied to the evaluations of the center and right expressions */
  };

  /**
   * UnaryExpressionNode
   * Node class with a single expression node as child.
   * Used for unary minus signs and parenthesis conservation on decompiling cuts.
  **/
  template<class AVariableManager>
  class UnaryExpressionNode : public AbstractExpressionNode<AVariableManager> {
    /**
    * Template argument dependent Particle type definition
    **/
    typedef typename AbstractExpressionNode<AVariableManager>::Object Object;
  public:
    /**
     * Evaluation of the child nodes and return result as a variant<double, int, bool>
     * return negative result if m_unary_minus true
     * @param p pointer to the object, that should be checked.
    **/
    std::variant<double, int, bool> evaluate(const Object* p) const override
    {
      std::variant<double, int, bool> ret = m_enode->evaluate(p);
      if (m_unary_minus) {
        if (std::holds_alternative<int>(ret)) return -1 * std::get<int>(ret);
        if (std::holds_alternative<double>(ret)) return -1.0 * std::get<double>(ret);
        throw std::runtime_error("Attemted unary sign with boolean type value.");
      }
      return ret;
    }
    /**
     * Print node
    **/
    void print() const override
    {
      if (m_unary_minus) std::cout << "-";
      m_enode->print();
    }

    /**
     * Decompile Node back to a string.
     * decompile(compile) should give the same result.
     */
    std::string decompile() const override
    {
      std::stringstream stringstream;
      if (m_unary_minus) stringstream << "-";
      stringstream << m_enode->decompile();
      return stringstream.str();
    }
  private:
    friend class NodeFactory;
    /**
     * Constructor
     * @param node (const boost::python::tuple&): tuple containing a expression node
     * @param unary_minus (bool): flag indicating if evaluate result should be mulitplied by -1
    **/
    explicit UnaryExpressionNode(Nodetuple node, bool unary_minus) : m_enode{NodeFactory::compile_expression_node<AVariableManager>(node)},
      m_unary_minus{unary_minus}
    {
    }
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_enode; /**< pointer to single expression node **/
    const bool m_unary_minus; /**< flag if expression evaluation should be returned times -1 **/
  };

  /**
   * BinaryExpressionNode
   * Node which connects two expression nodes with a arithemtic operation
   **/
  template<class AVariableManager>
  class BinaryExpressionNode : public AbstractExpressionNode<AVariableManager> {
    /**
    * Template argument dependent Particle type definition
    **/
    typedef typename AVariableManager::Object Object;
  public:
    /**
    * Evaluation of the child nodes and return result as a variant<double, int, bool>
    * apply arithmetic operation to the evaluations
    * @param p pointer to the object, that should be checked.
    **/
    std::variant<double, int, bool> evaluate(const Object* p) const override
    {
      std::variant<double, int, bool> l_val, r_val, ret;
      l_val = m_left_enode->evaluate(p);
      r_val = m_right_enode->evaluate(p);
      switch (m_aoperation) {
        case ArithmeticOperation::PLUS:
          if (std::holds_alternative<int>(l_val) && std::holds_alternative<int>(r_val)) {
            ret = std::get<int>(l_val) + std::get<int>(r_val);
            return ret;
          } else if (std::holds_alternative<double>(l_val) && std::holds_alternative<double>(r_val)) {
            ret = std::get<double>(l_val) + std::get<double>(r_val);
            return ret;
          } else {
            std::cout << typeid(l_val).name() << std::endl;
            std::cout << typeid(r_val).name() << std::endl;
            throw std::runtime_error("Invalid datatypes in plus operation.");
          }
          break;
        case ArithmeticOperation::MINUS:
          if (std::holds_alternative<int>(l_val) && std::holds_alternative<int>(r_val)) {
            ret = std::get<int>(l_val) - std::get<int>(r_val);
            return ret;
          } else if (std::holds_alternative<double>(l_val) && std::holds_alternative<double>(r_val)) {
            ret = std::get<double>(l_val) - std::get<double>(r_val);
            return ret;
          } else {
            std::cout << typeid(l_val).name() << std::endl;
            std::cout << typeid(r_val).name() << std::endl;
            throw std::runtime_error("Invalid datatypes in minus operation.");
          }
          break;
        case ArithmeticOperation::PRODUCT:
          if (std::holds_alternative<int>(l_val) && std::holds_alternative<int>(r_val)) {
            ret = std::get<int>(l_val) * std::get<int>(r_val);
            return ret;
          } else if (std::holds_alternative<double>(l_val) && std::holds_alternative<double>(r_val)) {
            ret = std::get<double>(l_val) * std::get<double>(r_val);
            return ret;
          } else {
            std::cout << typeid(l_val).name() << std::endl;
            std::cout << typeid(r_val).name() << std::endl;
            throw std::runtime_error("Invalid datatypes in product operation.");
          }
          break;
        case ArithmeticOperation::DIVISION:
          if (std::holds_alternative<int>(l_val) && std::holds_alternative<int>(r_val)) {
            ret = std::get<int>(l_val) / std::get<int>(r_val);
            return ret;
          } else if (std::holds_alternative<double>(l_val) && std::holds_alternative<double>(r_val)) {
            ret = std::get<double>(l_val) / std::get<double>(r_val);
            return ret;
          } else {
            throw std::runtime_error("Invalid datatypes in division operation.");
          }
          break;
        case ArithmeticOperation::POWER:
          if (std::holds_alternative<int>(l_val) && std::holds_alternative<int>(r_val)) {
            ret = std::pow(std::get<int>(l_val), std::get<int>(r_val));
            return ret;
          } else if (std::holds_alternative<double>(l_val) && std::holds_alternative<double>(r_val)) {
            ret = std::pow(std::get<double>(l_val), std::get<double>(r_val));
            return ret;
          } else {
            throw std::runtime_error("Invalid datatypes in power operation.");
          }
          break;
        default:
          throw std::runtime_error("Operation not valid");
      }
      ret = false;
      return ret;
    }
    /**
     * Print node
    **/
    void print() const override
    {
      m_left_enode->print();
      injectArithmeticOperatorToStream(std::cout, m_aoperation);
      m_right_enode->print();
    }

    /**
    * Decompile Node back to a string.
    * decompile(compile) should give the same result.
    */
    std::string decompile() const override
    {
      std::stringstream stringstream;
      stringstream << m_left_enode->decompile();
      injectArithmeticOperatorToStream(stringstream, m_aoperation);
      stringstream << m_right_enode->decompile();

      return stringstream.str();
    }

  private:
    friend class NodeFactory;
    /**
    * Constructor
    * @param left_node (const boost::python::tuple&): tuple containing a expression node
    * @param right_node (const boost::python::tuple&): tuple containing a expression node
    * @param aoperation (ArithmeticOperator): arithmetic operator enum value
    **/
    explicit BinaryExpressionNode(Nodetuple left_node, Nodetuple right_node, ArithmeticOperation aoperation) : m_left_enode{NodeFactory::compile_expression_node<AVariableManager>(left_node)},
      m_right_enode{NodeFactory::compile_expression_node<AVariableManager>(right_node)},  m_aoperation{aoperation}
    {
    }
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_left_enode; /**< left expression node */
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_right_enode; /**< right expression node */
    ArithmeticOperation m_aoperation; /**< arithmetic operation to be applied to the evaluations of left and right expressions */
  };

  /** Template class for storing the Constants (int, double, bool)
  * of the Cutstring.
  * evaluate(p) returns the value.
  **/
  template<class AVariableManager, typename T>
  class DataNode : public AbstractExpressionNode<AVariableManager> {
  private:
    friend class NodeFactory;
    /**
    * Template argument dependent Particle type definition
    **/
    typedef typename AbstractExpressionNode<AVariableManager>::Object Object;
    /**
     * Constructor
     * @param value (T): some constant value in cut may be of type double, int, bool
    **/
    explicit DataNode(T value) : m_value{value} {};
    const T m_value; /**< constant of type T which is always returned by evaluate(const Object* p) **/
  public:
    /**
    * return m_value as a variant<double, int, bool>
    * @param p pointer to the object, that should be checked. Not needed here since we always return the constant m_value.
    **/
    std::variant<double, int, bool> evaluate(const Object* p) const override
    {
      (void)p;
      std::variant<double, int, bool> ret{m_value};
      return ret;
    }
    /**
     * Print node
    **/
    void print() const override
    {
      std::cout << std::boolalpha;
      std::cout << m_value;
    }

    /**
    * Decompile Node back to a string.
    * decompile(compile) should give the same result.
    */
    std::string decompile() const override
    {
      std::stringstream stringstream;
      stringstream << std::boolalpha;
      stringstream << m_value;
      return stringstream.str();
    }
  };

  /** Class which stores the name of a variable.
  * Variable evaluation with the variable manager
  **/
  template<class AVariableManager>
  class IdentifierNode : public AbstractExpressionNode<AVariableManager> {
    /**
    * Template argument dependent Particle type definition
    **/
    typedef typename AVariableManager::Object Object;
    /**
    * Template argument dependent Variable type definition
    **/
    typedef typename AVariableManager::Var Var;
  public:
    /**
     * evaluate m_var with p and return the result
     * @param p pointer to the object, for which m_var should be evaluated.
    **/
    std::variant<double, int, bool> evaluate(const Object* p) const override
    {
      if (m_var != nullptr) {
        return m_var->function(p);
      } else {
        throw std::runtime_error("Cut string has an invalid format: Neither number nor variable name");
      }
    }
    /**
     * Print node
    **/
    void print() const override
    {
      std::cout << m_name;
    }

    /**
    * Decompile Node back to a string.
    * decompile(compile) should give the same result.
    */
    std::string decompile() const override
    {
      return m_name;
    }

    /**
     * Get variable from AVariableManger
    **/
    void processVariable()
    {
      AVariableManager& manager = AVariableManager::Instance();
      m_var = manager.getVariable(m_name);
      if (m_var == nullptr) {
        throw std::runtime_error(
          "Cut string has an invalid format: Variable not found: " + m_name);
      }
    }
  private:
    friend class NodeFactory;
    /**
    * Constructor
    * @param name (const std::string&): name of the identifier in cut
    * @param processVariableOnCreation (bool): flag indicating if the identifier in in the outer scope of the cut and therefore is considered as a variable which should be retrieved from AVariableManager. This is nessesary because MetaVariable arguments might not be valid variables. If every IdentiferNode tried to retrieve them at creation we get lookup errors.
    **/
    explicit IdentifierNode(const std::string& name, bool processVariableOnCreation) : m_name{name}, m_var{nullptr} {if (processVariableOnCreation) processVariable();}
    const std::string m_name; /**< name of the variable **/
    const Var* m_var; /**< set if there was a valid variable in this cut */
  };

  /**
   * FunctionNode
   * Node class for handling MetaVariables in cuts
  **/
  template<class AVariableManager>
  class FunctionNode: public AbstractExpressionNode<AVariableManager> {
    /**
    * Template argument dependent Particle type definition
    **/
    typedef typename AVariableManager::Object Object;
    /**
    * Template argument dependent Variable type definition
    **/
    typedef typename AVariableManager::Var Var;
  public:
    /**
    * evaluate m_var with p and return the result
    * @param p pointer to the object, for which m_var should be evaluated.
    **/
    std::variant<double, int, bool> evaluate(const Object* p) const override
    {
      return m_var->function(p);
    }
    /**
     * Print node
    **/
    void print() const override
    {
      std::string fullname = m_name + "(" + boost::algorithm::join(m_arguments, ", ") + ")";
      std::cout << fullname;
    }

    /**
    * Decompile Node back to a string.
    * decompile(compile) should give the same result.
    */
    std::string decompile() const override
    {
      std::string fullname = m_name + "(" + boost::algorithm::join(m_arguments, ", ") + ")";
      return fullname;
    }

    /**
     * Get MetaVariable from AVariableManager
    **/
    void processMetaVariable()
    {
      // Initialize Variable
      AVariableManager& manager = AVariableManager::Instance();
      m_var = manager.getVariable(m_name, m_arguments);
      if (m_var == nullptr) {
        auto fullname = m_name + "(" + boost::algorithm::join(m_arguments, ", ") + ")";

        throw std::runtime_error(
          "Cut string has an invalid format: Metavariable not found: " + fullname);
      }
    }
  private:
    friend class NodeFactory;
    /**
    * Constructor
    * @param functionName (const std::string&): function name of the MetaVariable
    * @param functionArguments (const std::vector<std::string>&): function arguments of the MetaVariable
    * @param processMetaVariableOnCreation (bool): flag indicating if the identifier in in the outer scope of the cut and therefore is considered as a variable which should be retrieved from AVariableManager. This is nessesary because MetaVariable arguments might not be valid variables. If every IdentiferNode tried to retrieve them at creation we get lookup errors.
    **/
    explicit FunctionNode(const std::string& functionName, const std::vector<std::string>& functionArguments,
                          bool processMetaVariableOnCreation): m_name{functionName},
      m_arguments{functionArguments}
    {if (processMetaVariableOnCreation) processMetaVariable();}
    const std::string m_name; /**< Function name of the MetaVariable **/
    const std::vector<std::string> m_arguments; /**< vector of string arguments of the MetaVariable **/
    const Var* m_var; /**< set if there was a valid variable in this cut */
  };


}
