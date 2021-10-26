/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <variant>
#include <iostream>

namespace Belle2 {

  /**
   * A parsed cut-string naturally has a tree shape which incoorporated
   * the infomation of operator precedence and evaluation order
   * of the statements.
   * For this application it is sufficient to derive a syntax directed translation
   * of the abstract syntax tree.
   * One can divide the Nodes into categories: BooleanNodes and ExpressionNodes
   * BooleanNodes handle boolean statements and concatenations thereof.
   * Every subclass must implement the check(Particle* p) function.
  **/
  template<class AVariableManager>
  class AbstractBooleanNode {
  public:
    /**
     * Template argument dependent Particle type definition
    **/
    typedef typename AVariableManager::Object Object;
    /**
     * Template argument dependent Variable type definition
    **/
    typedef typename AVariableManager::Var Var;

    /**
     * pure virutal check function, has to be overridden in derived class
    **/
    virtual bool check(const Object* p) const = 0;
    /**
     * pure virutal print function, has to be overridden in derived class
    **/
    virtual void print() const = 0;
    /**
     * pure virutal decompile function, has to be overridden in derived class
    **/
    virtual std::string decompile() const = 0;
    /**
     * Virtual destructor
    **/
    virtual ~AbstractBooleanNode() {};
  };

  /**
   * AbstractExpressionNode
   * Superclass for all nodes which host expressions.
   * Every child class must override the evaluate member, which returns the value of
   * its expression as a variant<double, int, bool>.
   **/
  template<class AVariableManager>
  class AbstractExpressionNode {
  public:
    /**
     * Template argument dependent Particle type definition
    **/
    typedef typename AVariableManager::Object Object;
    /**
     * Template argument dependent Variable type definition
    **/
    typedef typename AVariableManager::Var Var;

    /**
     * pure virutal evaluate function, has to be overridden in derived class
    **/
    virtual std::variant<double, int, bool> evaluate(const Object* p) const = 0;
    /**
     * pure virutal print function, has to be overridden in derived class
    **/
    virtual void print() const = 0;
    /**
     * pure virutal decompile function, has to be overridden in derived class
    **/
    virtual std::string decompile() const = 0;
    /**
     * Virtual destructor
    **/
    virtual ~AbstractExpressionNode() {}

  };

  /**
  * Enum of possible Nodes in parsing tree. NodeTypes in python are encoded
  * with the corresponding dict @see node_types in yacc.py
  */
  enum class NodeType : int {
    UnaryBooleanNode,
    BinaryBooleanNode,
    UnaryRelationalNode,
    BinaryRelationalNode,
    TernaryRelationalNode,
    UnaryExpressionNode,
    BinaryExpressionNode,
    FunctionNode,
    IdentifierNode,
    DoubleNode,
    IntegerNode,
    BooleanNode
  };

  /**
  * Enum for decoding the boolean operator type.
  * For encoding scheme in python @see b_operator_types in yacc.py
  */
  enum class BooleanOperator : int {
    AND,
    OR
  };

  /**
  * Enum for decoding the comparison operator type.
  * For encoding scheme in python @see c_operator_types in yacc.py
  */
  enum class ComparisonOperator : int {
    EQUALEQUAL,
    GREATEREQUAL,
    LESSEQUAL,
    GREATER,
    LESS,
    NOTEQUAL
  };

  /**
  * Enum for decoding the comparison operator type.
  * For encoding scheme in python @see a_operator_types in yacc.py
  */
  enum class ArithmeticOperation : int {
    PLUS,
    MINUS,
    PRODUCT,
    DIVISION,
    POWER
  };

  /**
   * Helper functions for AbstractBooleanNode and AbstractExpressionNode print() and decompile() members
   * Shifts the correct characters into the ostream, depending on the BooleanOperator given.
   * For decompiling/printing we want the corresponding character of the operator
   * e.g " and " for BooleanOperator::AND
   * print() redirects to std::cout, decompile() should redirect to a std::stringstream
   * Both streams subclass std::ostream. This allows the use of these functions in both cases.
   */
  void injectBooleanOperatorToStream(std::ostream& stream, const BooleanOperator& boperator);

  /**
   * Helper functions for AbstractBooleanNode and AbstractExpressionNode print() and decompile() members
   * Shifts the correct characters into the ostream, depending on the ComparisonOperator given.
   * For decompiling/printing we want the corresponding character of the operator
   * e.g " == " for ComparisonOperator::EQUALEQUAL
   * print() redirects to std::cout, decompile() should redirect to a std::stringstream
   * Both streams subclass std::ostream. This allows the use of these functions in both cases.
   */
  void injectComparisonOperatorToStream(std::ostream& stream, const ComparisonOperator& coperator);

  /**
   * Helper functions for AbstractBooleanNode and AbstractExpressionNode print() and decompile() members
   * Shifts the correct characters into the ostream, depending on the ComparisonOperator given.
   * For decompiling/printing we want the corresponding character of the operator
   * e.g " + " for ArithmeticOperator::PLUS
   * print() redirects to std::cout, decompile() should redirect to a std::stringstream
   * Both streams subclass std::ostream. This allows the use of these functions in both cases.
   */
  void injectArithmeticOperatorToStream(std::ostream& stream, const ArithmeticOperation& aoperation);

}
