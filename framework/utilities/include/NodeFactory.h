/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <memory>
#include <framework/utilities/CutNodes.h>

#include <boost/python/tuple.hpp>
#include <boost/python/extract.hpp>

namespace py = boost::python;
typedef const py::tuple& Nodetuple;

namespace Belle2 {
  /** Class for constructing Node objects of boost::python::tuple
  * NodeFactory is a friend to all Node classes, in order to use
  * their private Constructor.
  **/
  template<class AVariableManager>
  class UnaryBooleanNode;

  template<class AVariableManager>
  class BinaryBooleanNode;

  template<class AVariableManager>
  class UnaryRelationalNode;

  template<class AVariableManager>
  class BinaryRelationalNode;

  template<class AVariableManager>
  class TernaryRelationalNode;

  template<class AVariableManager>
  class UnaryExpressionNode;

  template<class AVariableManager>
  class BinaryExpressionNode;

  template<class AVariableManager, typename T>
  class DataNode;

  template<class AVariableManager>
  class IdentifierNode;

  template<class AVariableManager>
  class FunctionNode;

  /**
   * Wrapper class for static node compile functions
  **/
  class NodeFactory {
  public:
    /**
     * This template function creates a boolean node from a boost::python::tuple
     * The Python Parser encodes the node type as int as the first argument of each tuple.
     * Node types are decoded via the NodeType enum defined in framework/utilities/include/AbstractNodes.h
     * e.g (2, (11, True)) is a UnaryRelationalNode (nodetype value 2), which has a BooleanNode(nodetype value 11) as a child.
     * List of Nodes:
     * - UnaryBooleanNode
     * - BinaryBooleanNode
     * - UnaryRelationalNode
     * - BinaryRelationalNode
     * - TernaryRelationalNode
     * @param tuple (const boost::python::tuple&): python tuple generated from the parser
     */
    template<class AVariableManager>
    static std::unique_ptr<const AbstractBooleanNode<AVariableManager>> compile_boolean_node(Nodetuple tuple)
    {
      // First argument of every NodeTuple is an integer indicating the NodeType
      NodeType node_type = static_cast<NodeType>(static_cast<int>(py::extract<int>(tuple[0])));

      if (node_type == NodeType::UnaryBooleanNode) {
        // Extract first child tuple
        Nodetuple child = static_cast<const py::tuple>(tuple[1]);
        bool negation = py::extract<bool>(tuple[2]);
        bool bracketized = py::extract<bool>(tuple[3]);
        return std::unique_ptr<const AbstractBooleanNode<AVariableManager>>(new UnaryBooleanNode<AVariableManager>(child, negation,
               bracketized));

      } else if (node_type == NodeType::BinaryBooleanNode) {
        Nodetuple left_node = static_cast<const py::tuple>(tuple[1]);
        Nodetuple right_node = static_cast<const py::tuple>(tuple[2]);
        BooleanOperator boperator = static_cast<BooleanOperator>(static_cast<int>(py::extract<int>(tuple[3])));
        return std::unique_ptr<const AbstractBooleanNode<AVariableManager>>(new BinaryBooleanNode<AVariableManager>(left_node, right_node,
               boperator));
      } else if (node_type == NodeType::UnaryRelationalNode) {
        Nodetuple node = static_cast<const py::tuple>(tuple[1]);
        return std::unique_ptr<const AbstractBooleanNode<AVariableManager>>(new UnaryRelationalNode<AVariableManager>(node));
      } else if (node_type == NodeType::BinaryRelationalNode) {
        Nodetuple left_node = static_cast<const py::tuple>(tuple[1]);
        Nodetuple right_node = static_cast<const py::tuple>(tuple[2]);
        ComparisonOperator coperator = static_cast<ComparisonOperator>(static_cast<int>(py::extract<int>(tuple[3])));
        return std::unique_ptr<const AbstractBooleanNode<AVariableManager>>(new BinaryRelationalNode<AVariableManager>(left_node,
               right_node , coperator));
      } else if (node_type == NodeType::TernaryRelationalNode) {
        Nodetuple left_node = static_cast<const py::tuple>(tuple[1]);
        Nodetuple center_node = static_cast<const py::tuple>(tuple[2]);
        Nodetuple right_node = static_cast<const py::tuple>(tuple[3]);
        ComparisonOperator lc_coperator = static_cast<ComparisonOperator>(static_cast<int>(py::extract<int>(tuple[4])));
        ComparisonOperator cr_coperator = static_cast<ComparisonOperator>(static_cast<int>(py::extract<int>(tuple[5])));

        return std::unique_ptr<const AbstractBooleanNode<AVariableManager>>(new TernaryRelationalNode<AVariableManager>(left_node,
               center_node, right_node,
               lc_coperator, cr_coperator));
      } else {
        throw std::runtime_error("error NodeFactory::compile_boolean_node: got invalid boolean NodeType.");
      }
    }
    /**
     * This template function creates a ExpressionNode from a boost::python::tuple
     * The Python Parser encodes the node type as int in the first argument of each tuple.
     * Node types are decoded via the NodeType enum defined in framework/utilities/include/AbstractNodes.h
     * e.g (9, 1.2) is a DataNode<double> with value 1.2
     * List of Nodes:
     * - UnaryExpressionNode
     * - BinaryExpressionNode
     * - DataNode<double>
     * - DataNode<int>
     * - DataNode<bool>
     */
    template<class AVariableManager>
    static std::unique_ptr<const AbstractExpressionNode<AVariableManager>> compile_expression_node(Nodetuple tuple)
    {
      // int is extracted from the py::tuple and cast as NodeTuple enum
      NodeType node_type = static_cast<NodeType>(static_cast<int>(py::extract<int>(tuple[0])));
      if (node_type == NodeType::UnaryExpressionNode) {
        Nodetuple node = static_cast<const py::tuple>(tuple[1]);
        bool unary_minus = py::extract<bool>(tuple[2]);
        return std::unique_ptr<const AbstractExpressionNode<AVariableManager>>(new UnaryExpressionNode<AVariableManager>(node,
               unary_minus));
      } else if (node_type == NodeType::BinaryExpressionNode) {
        Nodetuple left_node = static_cast<const py::tuple>(tuple[1]);
        Nodetuple right_node = static_cast<const py::tuple>(tuple[2]);
        ArithmeticOperation aoperation = static_cast<ArithmeticOperation>(static_cast<int>(py::extract<int>(tuple[3])));
        return std::unique_ptr<const AbstractExpressionNode<AVariableManager>>(new BinaryExpressionNode<AVariableManager>(left_node,
               right_node,
               aoperation));
      } else if (node_type == NodeType::IntegerNode) {
        int i = py::extract<int>(tuple[1]);
        return std::unique_ptr<const AbstractExpressionNode<AVariableManager>>(new DataNode<AVariableManager, int>(i));
      } else if (node_type == NodeType::DoubleNode) {
        // We have to check for inf and nan values.
        // use python math functions to check
        py::object math = py::import("math");
        // Get tuple item as object
        py::object data = static_cast<py::object>(tuple[1]);
        if (py::extract<bool>(math.attr("isinf")(data))) {
          double inf = std::numeric_limits<double>::infinity();
          return std::unique_ptr<const AbstractExpressionNode<AVariableManager>>(new DataNode<AVariableManager, double>(inf));
        }
        if (py::extract<bool>(math.attr("isnan")(data))) {
          double nan = std::nan("");
          return std::unique_ptr<const AbstractExpressionNode<AVariableManager>>(new DataNode<AVariableManager, double>(nan));
        }
        double d = py::extract<double>(tuple[1]);
        return std::unique_ptr<const AbstractExpressionNode<AVariableManager>>(new DataNode<AVariableManager, double>(d));
      } else if (node_type == NodeType::BooleanNode) {
        bool b = py::extract<bool>(tuple[1]);
        return std::unique_ptr<const AbstractExpressionNode<AVariableManager>>(new DataNode<AVariableManager, bool>(b));
      } else if (node_type == NodeType::IdentifierNode) {
        std::string identifier = py::extract<std::string>(tuple[1]);
        bool processVariableOnCreation = py::extract<bool>(tuple[2]);
        return std::unique_ptr<const AbstractExpressionNode<AVariableManager>>(new IdentifierNode<AVariableManager>(identifier,
               processVariableOnCreation));
      } else if (node_type == NodeType::FunctionNode) {

        // Extract functionName as second argument of the tuple
        std::string functionName = py::extract<std::string>(tuple[1]);

        // Extract number of function arguents
        int numArguments = py::extract<int>(tuple[2]);

        // Extract argument tuple
        Nodetuple argumentTuple = static_cast<const py::tuple>(tuple[3]);

        // Create vector of strings by compiling each argument tuple and immediately call decompile.
        std::vector<std::string> functionArguments;
        for (int i = 0; i < numArguments; i++) {
          Nodetuple argument = static_cast<const py::tuple>(argumentTuple[i]);

          // Compile argument
          std::unique_ptr<const AbstractBooleanNode<AVariableManager>> argNode = compile_boolean_node<AVariableManager>(argument);

          // Decompile argument node into a string
          std::string stringArgument = argNode->decompile();
          if (stringArgument.size() == 0) throw std::runtime_error("Argument empty");
          functionArguments.push_back(stringArgument);
        }
        bool processMetaVariableOnCreation = py::extract<bool>(tuple[4]);

        return std::unique_ptr<const AbstractExpressionNode<AVariableManager>>(new FunctionNode<AVariableManager>(functionName,
               functionArguments, processMetaVariableOnCreation));
      } else {
        throw std::runtime_error("error NodeFactory::compile_expression_node: got invalid expression NodeType.");
      }
    }
  };
}

