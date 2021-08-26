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
  /*
   * Forward declaration of function defined in framework/utilities/GeneralCut.cc
   */
  bool almostEqualDouble(const double& a, const double& b);

  /**
   * This is a class template which takes a template class operation as template argument.
   * This allows passing the functional std::greater<T> which are templates themselves.
   * We can pass std::greater without template argument and later specify T depending on which data types we have in the variants.
   *
   * Depending on the variant data types we might want to evaluate std::greater<int> or std::greater<double>.
   * A big advantage is, that we don't have to know the variant data types of Rval and Lval
   *  when instanciating the Visitor with the variants e.g std::visit(Visitor<std::greater_equal>{}, Lval, Rval)
   * The operation data type is decided in the corresponding visitor operator()(const T1&, const T2&)
   *
   * When comparing double/int and a bool the double/int overload of the functionals are used to disable implicit conversion to bool:
   * std::equal<bool>{}(1.2, true) ==> true; 1.2 is implicitly converted to true
   * std::equal<double>{}(1.2, true) ==> false; true is implicity converted to 1.0
   */
  template <template <typename type> class operation>
  struct Visitor {
    bool operator()(const double& val0, const double& val1)
    {
      return operation<double> {}(val0, val1);
    }
    bool operator()(const double& val0, const int& val1)
    {
      return operation<double> {}(val0, val1);
    }
    bool operator()(const double& val0, const bool& val1)
    {
      return operation<double> {}(val0, val1);
    }
    bool operator()(const int& val0, const int& val1)
    {
      return operation<int> {}(val0, val1);
    }
    bool operator()(const int& val0, const bool& val1)
    {
      return operation<int> {}(val0, val1);
    }
    bool operator()(const int& val0, const double& val1)
    {
      return operation<double> {}(val0, val1);
    }
    bool operator()(const bool& val0, const bool& val1)
    {
      return operation<bool> {}(val0, val1);
    }
    bool operator()(const bool& val0, const double& val1)
    {
      return operation<double> {}(val0, val1);
    }
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
     * Print Node
     * brackets and negation keywords are added if m_bracketized, m_negation are set to true.
     */
    void print() const override
    {
      if (m_negation) std::cout << "not";
      if (m_bracketized) std::cout << "[";
      m_bnode->print();
      if (m_bracketized) std::cout << "]";
    }

    /* Decompile Node back to a string.
     * Decompile(compile) should give the same result.
     * brackets and negation keywords are added if m_bracketized, m_negation are set to true.
     */
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
  * a Boolean Operator (AND, OR, XOR, NAND)
  * Check() method evaluates the child nodes and returns
  * the result of the boolean operation.
  **/
  template<class AVariableManager>
  class BinaryBooleanNode : public AbstractBooleanNode<AVariableManager> {
    typedef typename AVariableManager::Object Object;
  public:
    /**
      * Check if object passes this subexpression of the cut by calling check on the children nodes.
      * Boolean results are evaluated with the BooleanOperator(AND/OR/NAND/XOR) the node object.
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
        case BooleanOperator::NAND:
          return !(m_left_bnode->check(p) && m_right_bnode->check(p));
          break;
        case BooleanOperator::XOR:
          return !(m_left_bnode->check(p) == m_right_bnode->check(p));
          break;
        default:
          throw std::runtime_error("BinaryBooleanNode has an invalid BooleanOperator");
      }
      return false;
    }

    /**
     * Print Node
     */
    void print() const override
    {
      m_left_bnode->print();
      injectBooleanOperatorToStream(std::cout, m_boperator);
      m_right_bnode->print();
    }

    /* Decompile Node back to a string.
     * Decompile(compile) should give the same result.
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
     * Print Node
     */
    void print() const override
    {
      m_enode->print();
    }

    /* Decompile Node back to a string.
     * Decompile(compile) should give the same result.
     */
    std::string decompile() const override
    {
      return m_enode->decompile();
    }

  private:
    friend class NodeFactory; // friend declaration so that NodeFactory can call the private constructor
    explicit UnaryRelationalNode(Nodetuple node) : m_enode{NodeFactory::compile_expression_node<AVariableManager>(node)}
    {
    }
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_enode; /**< subexpression of a cut */
  };

  /** BooleanNode which has two AbstractExpressionNodes nodes
  * and a ComparisonOperator.
  * Check() evaluates the children nodes and compares the results
  * with the specified operator.
  **/
  template<class AVariableManager>
  class BinaryRelationalNode : public AbstractBooleanNode<AVariableManager> {
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
     * Print Node
     */
    void print() const override
    {
      m_left_enode->print();
      injectComparisonOperatorToStream(std::cout, m_coperator);
      m_right_enode->print();
    }

    /* Decompile Node back to a string.
     * Decompile(compile) should give the same result.
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
    explicit BinaryRelationalNode(Nodetuple left_node, Nodetuple right_node, ComparisonOperator coperator)
      : m_left_enode{NodeFactory::compile_expression_node<AVariableManager>(left_node)}, m_right_enode{NodeFactory::compile_expression_node<AVariableManager>(right_node)},
        m_coperator{coperator}
    {
    }
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_left_enode; /**< subexpression of a cut */
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_right_enode; /**< subexpression of a cut */
    const ComparisonOperator m_coperator;
  };

  /** BooleanNode which has three AbstractExpressionNodes nodes
  * and two ComparisonOperator.
  * This allows Ranges in the Cutstring e.g 2.0 < var < 4.3
  * Check() evaluates the children nodes and compares both sides
  * with the specified operator.
  **/
  template<class AVariableManager>
  class TernaryRelationalNode : public AbstractBooleanNode<AVariableManager> {
    typedef typename AbstractBooleanNode<AVariableManager>::Object Object;
  public:
    /*
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

    void print() const override
    {
      m_left_enode->print();
      injectComparisonOperatorToStream(std::cout, m_cr_coperator);
      m_center_enode->print();
      injectComparisonOperatorToStream(std::cout, m_lc_coperator);
      m_left_enode->print();
    }

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
    explicit TernaryRelationalNode(Nodetuple left_node, Nodetuple center_node, Nodetuple right_node, ComparisonOperator lc_coperator,
                                   ComparisonOperator cr_coperator)
      : m_left_enode{NodeFactory::compile_expression_node<AVariableManager>(left_node)}, m_center_enode{NodeFactory::compile_expression_node<AVariableManager>(center_node)},
        m_right_enode{NodeFactory::compile_expression_node<AVariableManager>(right_node)},  m_lc_coperator{lc_coperator},
        m_cr_coperator{cr_coperator}
    {
    }
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_left_enode;
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_center_enode;
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_right_enode;
    const ComparisonOperator m_lc_coperator;
    const ComparisonOperator m_cr_coperator;
  };

  template<class AVariableManager>
  class UnaryExpressionNode : public AbstractExpressionNode<AVariableManager> {
    typedef typename AbstractExpressionNode<AVariableManager>::Object Object;
  public:
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
    void print() const override
    {
      if (m_unary_minus) std::cout << "-";
      m_enode->print();
    }

    std::string decompile() const override
    {
      std::stringstream stringstream;
      if (m_unary_minus) stringstream << "-";
      stringstream << m_enode->decompile();
      return stringstream.str();
    }
  private:
    friend class NodeFactory;
    explicit UnaryExpressionNode(Nodetuple node, bool unary_minus) : m_enode{NodeFactory::compile_expression_node<AVariableManager>(node)},
      m_unary_minus{unary_minus}
    {
    }
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_enode;
    const bool m_unary_minus;
  };

  template<class AVariableManager>
  class BinaryExpressionNode : public AbstractExpressionNode<AVariableManager> {
    typedef typename AVariableManager::Object Object;
  public:
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

    void print() const override
    {
      m_left_enode->print();
      injectArithmeticOperatorToStream(std::cout, m_aoperation);
      m_right_enode->print();
    }

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
    explicit BinaryExpressionNode(Nodetuple left_node, Nodetuple right_node, ArithmeticOperation aoperation) : m_left_enode{NodeFactory::compile_expression_node<AVariableManager>(left_node)},
      m_right_enode{NodeFactory::compile_expression_node<AVariableManager>(right_node)},  m_aoperation{aoperation}
    {
    }
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_left_enode;
    std::unique_ptr<const AbstractExpressionNode<AVariableManager>> m_right_enode;
    ArithmeticOperation m_aoperation;
  };

  /** Template class for storing the Constants (int, double, bool)
  * of the Cutstring.
  * evaluate(p) returns the value.
  **/
  template<class AVariableManager, typename T>
  class DataNode : public AbstractExpressionNode<AVariableManager> {
  private:
    friend class NodeFactory;
    typedef typename AbstractExpressionNode<AVariableManager>::Object Object;
    explicit DataNode(T value) : m_value{value} {};
    const T m_value;
  public:
    std::variant<double, int, bool> evaluate(const Object* p) const override
    {
      (void)p;
      std::variant<double, int, bool> ret{m_value};
      return ret;
    }
    void print() const override
    {
      std::cout << std::boolalpha;
      std::cout << m_value;
    }
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
    typedef typename AVariableManager::Object Object;
    typedef typename AVariableManager::Var Var;
  public:
    std::variant<double, int, bool> evaluate(const Object* p) const override
    {
      if (m_var != nullptr) {
        return m_var->function(p);
      } else {
        throw std::runtime_error("Cut string has an invalid format: Neither number nor variable name");
      }
    }
    void print() const override
    {
      std::cout << m_name;
    }
    std::string decompile() const override
    {
      return m_name;
    }
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
    explicit IdentifierNode(const std::string& name, bool processVariableOnCreation) : m_name{name}, m_var{nullptr} {if (processVariableOnCreation) processVariable();}
    const std::string m_name;
    const Var* m_var; /**< set if there was a valid variable in this cut */
  };

  /* Function class

  */
  template<class AVariableManager>
  class FunctionNode: public AbstractExpressionNode<AVariableManager> {
    typedef typename AVariableManager::Object Object;
    typedef typename AVariableManager::Var Var;
  public:
    std::variant<double, int, bool> evaluate(const Object* p) const override
    {
      return m_var->function(p);
    }
    void print() const override
    {
      std::string fullname = m_name + "(" + boost::algorithm::join(m_arguments, ", ") + ")";
      std::cout << fullname;
    }
    std::string decompile() const override
    {
      std::string fullname = m_name + "(" + boost::algorithm::join(m_arguments, ", ") + ")";
      return fullname;
    }
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
    explicit FunctionNode(const std::string& functionName, const std::vector<std::string>& functionArguments,
                          bool processMetaVariableOnCreation): m_name{functionName},
      m_arguments{functionArguments}
    {if (processMetaVariableOnCreation) processMetaVariable();}
    const std::string m_name;
    const std::vector<std::string> m_arguments;
    const Var* m_var; /**< set if there was a valid variable in this cut */
  };


}
