/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <functional>
#include <iostream>
#include <cmath>
#include <boost/python.hpp>
#include <framework/utilities/CutNodes.h>
#include <framework/utilities/TestHelpers.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/utilities/Conversion.h>
#include <gtest/gtest.h>


using namespace Belle2;
namespace {
  namespace py = boost::python;
  using VarVariant = Belle2::Variable::Manager::VarVariant;
/// Class to mock objects for our variable manager.
  struct MockObjectType {
    /// Stupid singlevalued object.
    explicit MockObjectType(const double& d) : m_value{d} {}
    explicit MockObjectType(const int& i) : m_value{i} {}
    explicit MockObjectType(const bool& b) : m_value{b} {}
    VarVariant m_value;
  };
  /// Function forward declaration for mocking a MetaVariable
  std::function<VarVariant(const MockObjectType*)> mockMetaVariable(const std::vector<std::string>& arguments);

  /**
   *  Class to mock variables for our variable manager.
   *  This is also the minimal interface a variable must have,
   *  to be used in the GeneralCut.
   */
  class MockVariableType {
  public:
    /// Function of the variable which always returns the value of the object.
    VarVariant function(const MockObjectType* object) const
    {
      return m_function(object);
    }
    explicit MockVariableType(const std::string& name) : m_name{name}
    {
      m_function = [](const MockObjectType * object) -> VarVariant {
        if (object != nullptr)
        {
          return object->m_value;
        } else
        {
          return std::numeric_limits<double>::quiet_NaN();
        }
      };
    }

    MockVariableType(const std::string& name, std::function<VarVariant(const MockObjectType*)> function) : m_name{name}, m_function{function}
    {}

    /// Name of the variable.
    std::string m_name;
    std::function<VarVariant(const MockObjectType*)> m_function;
  };

  /**
   *  Mock a variable manager for tests. This is also the minimal interface a
   *  class must have to be used as a VariableManager in the GeneralCut template.
   */
  class MockVariableManager {
  public:
    /// Use MockObjectType as Objects.
    using Object = MockObjectType;
    /// Use MockvariableType as Variables.
    using Var = MockVariableType;
    /// Define Node evaluation type
    using VarVariant = Belle2::Variable::Manager::VarVariant;

    /// Singleton.
    static MockVariableManager& Instance()
    {
      static MockVariableManager instance;
      return instance;
    }

    /// Return the single mocking variable we have in cases the correct name is given.
    Var* getVariable(const std::string& name)
    {
      if (name == "mocking_variable") {
        return &m_mocking_variable;
      } else {
        return nullptr;
      }
    }

    Var* getVariable(const std::string& functionName, const std::vector<std::string>& functionArguments)
    {
      auto function = mockMetaVariable(functionArguments);
      m_mocking_metavariable = MockVariableType(functionName, function);
      return &m_mocking_metavariable;
    }

    /// The only variable we have in this test.
    Var m_mocking_variable{"mocking_variable"};
    Var m_mocking_metavariable{""};
  };


// Mock sum metavariable
  std::function<VarVariant(const MockObjectType*)> mockMetaVariable(const std::vector<std::string>& arguments)
  {
    const MockVariableType* var = MockVariableManager::Instance().getVariable(arguments[0]);
    if (var == nullptr) {
      throw std::runtime_error("Variable could not be found.");
    }
    auto func =  [var, arguments](const MockObjectType * object) -> VarVariant {
      double sum = 0.0;
      if (std::holds_alternative<int>(var->function(object)))
      {
        sum += std::get<int>(var->function(object));
      } else if (std::holds_alternative<double>(var->function(object)))
      {
        sum += std::get<double>(var->function(object));
      } else
      {
        sum += std::get<bool>(var->function(object));
      }
      for (size_t i = 1; i < arguments.size(); i++)
      {
        sum += Belle2::convertString<double>(arguments[i]);
      }
      return sum;
    };
    return func;
  }

  /// Each boost::python::tuple has to be of the correct length for every CutNode.
  /// Otherwise, we get an index error when we try to extract the values.
  /// These simple test cases check that exceptions are thrown if the lengths are incorrect.
  TEST(CutNodesTest, TupleLength)
  {
    Py_Initialize();
    py::tuple tuple = py::tuple();
    EXPECT_B2FATAL(NodeFactory::compile_boolean_node<MockVariableManager>(tuple));
    EXPECT_B2FATAL(NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::UnaryBooleanNode));
    EXPECT_B2FATAL(NodeFactory::compile_boolean_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryBooleanNode));
    EXPECT_B2FATAL(NodeFactory::compile_boolean_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode));
    EXPECT_B2FATAL(NodeFactory::compile_boolean_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode));
    EXPECT_B2FATAL(NodeFactory::compile_boolean_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode));
    EXPECT_B2FATAL(NodeFactory::compile_boolean_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::UnaryExpressionNode));
    EXPECT_B2FATAL(NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode));
    EXPECT_B2FATAL(NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::BooleanNode));
    EXPECT_B2FATAL(NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::IntegerNode));
    EXPECT_B2FATAL(NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode));
    EXPECT_B2FATAL(NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::IdentifierNode));
    EXPECT_B2FATAL(NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    tuple = py::make_tuple(static_cast<int>(NodeType::FunctionNode));
    EXPECT_B2FATAL(NodeFactory::compile_expression_node<MockVariableManager>(tuple));

  }


  /// Test for the UnaryBooleanNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, unaryBooleanNode)
  {
    Py_Initialize();
    // Create two BooleanNode tuples with values true and false.
    const py::tuple booleanTuple_true = py::make_tuple(static_cast<int>(NodeType::BooleanNode), true);
    const py::tuple booleanTuple_false = py::make_tuple(static_cast<int>(NodeType::BooleanNode), false);
    // Compile BooleanNodes and check evaluation
    auto booleanNode_true = NodeFactory::compile_expression_node<MockVariableManager>(booleanTuple_true);
    auto booleanNode_false = NodeFactory::compile_expression_node<MockVariableManager>(booleanTuple_false);
    EXPECT_TRUE(std::get<bool>(booleanNode_true->evaluate(nullptr)));
    EXPECT_FALSE(std::get<bool>(booleanNode_false->evaluate(nullptr)));

    // Create two UnaryRelationalNodes tuples with each BooleanNode tuple as child.
    const py::tuple uRelTuple_true = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), booleanTuple_true);
    const py::tuple uRelTuple_false = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), booleanTuple_false);
    // Compile UnaryExpressionNodes and check evaluation
    auto uRelNode_true = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple_true);
    auto uRelNode_false = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple_false);
    EXPECT_TRUE(uRelNode_true->check(nullptr));
    EXPECT_FALSE(uRelNode_false->check(nullptr));

    // Create all tuple combinations of true/false children and flags
    // True child
    const py::tuple uBoolTuple_true = py::make_tuple(static_cast<int>(NodeType::UnaryBooleanNode), uRelTuple_true, false, false);
    const py::tuple uBoolTuple_true_brack = py::make_tuple(static_cast<int>(NodeType::UnaryBooleanNode), uRelTuple_true, false, true);
    const py::tuple uBoolTuple_true_neg = py::make_tuple(static_cast<int>(NodeType::UnaryBooleanNode), uRelTuple_true, true, false);
    const py::tuple uBoolTuple_true_neg_brack = py::make_tuple(static_cast<int>(NodeType::UnaryBooleanNode), uRelTuple_true, true,
                                                               true);

    auto uBoolNode_true = NodeFactory::compile_boolean_node<MockVariableManager>(uBoolTuple_true);
    EXPECT_TRUE(uBoolNode_true->check(nullptr));
    EXPECT_EQ(uBoolNode_true->decompile(), "true");

    auto uBoolNode_true_brack = NodeFactory::compile_boolean_node<MockVariableManager>(uBoolTuple_true_brack);
    EXPECT_TRUE(uBoolNode_true_brack->check(nullptr));
    EXPECT_EQ(uBoolNode_true_brack->decompile(), "[true]");

    auto uBoolNode_true_neg = NodeFactory::compile_boolean_node<MockVariableManager>(uBoolTuple_true_neg);
    EXPECT_FALSE(uBoolNode_true_neg->check(nullptr));
    EXPECT_EQ(uBoolNode_true_neg->decompile(), "not true");

    auto uBoolNode_true_neg_brack = NodeFactory::compile_boolean_node<MockVariableManager>(uBoolTuple_true_neg_brack);
    EXPECT_FALSE(uBoolNode_true_neg_brack->check(nullptr));
    EXPECT_EQ(uBoolNode_true_neg_brack->decompile(), "not [true]");

    // False child
    const py::tuple uBoolTuple_false = py::make_tuple(static_cast<int>(NodeType::UnaryBooleanNode), uRelTuple_false, false, false);
    const py::tuple uBoolTuple_false_brack = py::make_tuple(static_cast<int>(NodeType::UnaryBooleanNode), uRelTuple_false, false, true);
    const py::tuple uBoolTuple_false_neg = py::make_tuple(static_cast<int>(NodeType::UnaryBooleanNode), uRelTuple_false, true, false);
    const py::tuple uBoolTuple_false_neg_brack = py::make_tuple(static_cast<int>(NodeType::UnaryBooleanNode), uRelTuple_false, true,
                                                                true);

    auto uBoolNode_false = NodeFactory::compile_boolean_node<MockVariableManager>(uBoolTuple_false);
    EXPECT_FALSE(uBoolNode_false->check(nullptr));
    EXPECT_EQ(uBoolNode_false->decompile(), "false");

    auto uBoolNode_false_brack = NodeFactory::compile_boolean_node<MockVariableManager>(uBoolTuple_false_brack);
    EXPECT_FALSE(uBoolNode_false_brack->check(nullptr));
    EXPECT_EQ(uBoolNode_false_brack->decompile(), "[false]");

    auto uBoolNode_false_neg = NodeFactory::compile_boolean_node<MockVariableManager>(uBoolTuple_false_neg);
    EXPECT_TRUE(uBoolNode_false_neg->check(nullptr));
    EXPECT_EQ(uBoolNode_false_neg->decompile(), "not false");

    auto uBoolNode_false_neg_brack = NodeFactory::compile_boolean_node<MockVariableManager>(uBoolTuple_false_neg_brack);
    EXPECT_TRUE(uBoolNode_false_neg_brack->check(nullptr));
    EXPECT_EQ(uBoolNode_false_neg_brack->decompile(), "not [false]");

  }
  /// Test for the BinaryBooleannode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, binaryBooleanNode)
  {
    // Create two BooleanNode tuples with values true and false.
    const py::tuple booleanTuple_true = py::make_tuple(static_cast<int>(NodeType::BooleanNode), true);
    const py::tuple booleanTuple_false = py::make_tuple(static_cast<int>(NodeType::BooleanNode), false);

    // Create two UnaryRelationalNodes tuples with each BooleanNode tuple as child.
    const py::tuple uRelTuple_true = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), booleanTuple_true);
    const py::tuple uRelTuple_false = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), booleanTuple_false);

    // Create all tuple combinations of children and BooleanOperators
    const py::tuple binBoolTuple_AND_true_true = py::make_tuple(static_cast<int>(NodeType::BinaryBooleanNode), uRelTuple_true,
                                                                uRelTuple_true, static_cast<int>(BooleanOperator::AND));
    const py::tuple binBoolTuple_AND_false_true = py::make_tuple(static_cast<int>(NodeType::BinaryBooleanNode), uRelTuple_false,
                                                  uRelTuple_true, static_cast<int>(BooleanOperator::AND));
    const py::tuple binBoolTuple_AND_true_false = py::make_tuple(static_cast<int>(NodeType::BinaryBooleanNode), uRelTuple_true,
                                                  uRelTuple_false, static_cast<int>(BooleanOperator::AND));
    const py::tuple binBoolTuple_AND_false_false = py::make_tuple(static_cast<int>(NodeType::BinaryBooleanNode), uRelTuple_false,
                                                   uRelTuple_false, static_cast<int>(BooleanOperator::AND));

    const py::tuple binBoolTuple_OR_true_true = py::make_tuple(static_cast<int>(NodeType::BinaryBooleanNode), uRelTuple_true,
                                                               uRelTuple_true, static_cast<int>(BooleanOperator::OR));
    const py::tuple binBoolTuple_OR_false_true = py::make_tuple(static_cast<int>(NodeType::BinaryBooleanNode), uRelTuple_false,
                                                                uRelTuple_true, static_cast<int>(BooleanOperator::OR));
    const py::tuple binBoolTuple_OR_true_false = py::make_tuple(static_cast<int>(NodeType::BinaryBooleanNode), uRelTuple_true,
                                                                uRelTuple_false, static_cast<int>(BooleanOperator::OR));
    const py::tuple binBoolTuple_OR_false_false = py::make_tuple(static_cast<int>(NodeType::BinaryBooleanNode), uRelTuple_false,
                                                  uRelTuple_false, static_cast<int>(BooleanOperator::OR));


    auto binBoolNode_AND_true_true = NodeFactory::compile_boolean_node<MockVariableManager>(binBoolTuple_AND_true_true);
    auto binBoolNode_AND_false_true = NodeFactory::compile_boolean_node<MockVariableManager>(binBoolTuple_AND_false_true);
    auto binBoolNode_AND_true_false = NodeFactory::compile_boolean_node<MockVariableManager>(binBoolTuple_AND_true_false);
    auto binBoolNode_AND_false_false = NodeFactory::compile_boolean_node<MockVariableManager>(binBoolTuple_AND_false_false);

    auto binBoolNode_OR_true_true = NodeFactory::compile_boolean_node<MockVariableManager>(binBoolTuple_OR_true_true);
    auto binBoolNode_OR_false_true = NodeFactory::compile_boolean_node<MockVariableManager>(binBoolTuple_OR_false_true);
    auto binBoolNode_OR_true_false = NodeFactory::compile_boolean_node<MockVariableManager>(binBoolTuple_OR_true_false);
    auto binBoolNode_OR_false_false = NodeFactory::compile_boolean_node<MockVariableManager>(binBoolTuple_OR_false_false);

    EXPECT_TRUE(binBoolNode_AND_true_true->check(nullptr));
    EXPECT_FALSE(binBoolNode_AND_false_true->check(nullptr));
    EXPECT_FALSE(binBoolNode_AND_true_false->check(nullptr));
    EXPECT_FALSE(binBoolNode_AND_false_false->check(nullptr));

    EXPECT_TRUE(binBoolNode_OR_true_true->check(nullptr));
    EXPECT_TRUE(binBoolNode_OR_false_true->check(nullptr));
    EXPECT_TRUE(binBoolNode_OR_true_false->check(nullptr));
    EXPECT_FALSE(binBoolNode_OR_false_false->check(nullptr));

  }
  /// Test for the UnaryRelationalnNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, unaryRelationalNode)
  {
    Py_Initialize();
    // Get main module
    py::object main = py::import("__main__");
    py::object pyfloat = main.attr("__builtins__").attr("float");

    // Test IntegerNode
    py::tuple tuple = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 1337);
    py::tuple uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    auto node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 0);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::IntegerNode), -0);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 1);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::IntegerNode), -1);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    // Test DoubleNode
    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 0.0);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode), -0.0);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 1.0);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode), -1.0);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 1.1);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode), -1.1);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("nan"));
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("inf"));
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_TRUE(node->check(nullptr));


    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("-inf"));
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    // Test BooleanNodes
    tuple = py::make_tuple(static_cast<int>(NodeType::BooleanNode), true);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    tuple = py::make_tuple(static_cast<int>(NodeType::BooleanNode), false);
    uRelTuple = py::make_tuple(static_cast<int>(NodeType::UnaryRelationalNode), tuple);
    node = NodeFactory::compile_boolean_node<MockVariableManager>(uRelTuple);
    EXPECT_FALSE(node->check(nullptr));

  }
  /// Test for the BinaryRelationalNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, binaryRelationalNode)
  {
    Py_Initialize();
    // Get main module
    py::object main = py::import("__main__");
    py::object pyfloat = main.attr("__builtins__").attr("float");

    py::tuple child1 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 1337);
    py::tuple child2 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 1337);
    py::tuple bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                                         static_cast<int>(ComparisonOperator::EQUALEQUAL));
    auto node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 1337.0);
    child2 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 1337);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child2, child1,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 1337.0);
    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 1337.0);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    // NaN with NaN comparisons
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("nan"));
    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("nan"));
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    // NaN != NaN yields true
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    // Compare nan with a numerical value
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("nan"));
    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 0.0);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    // Compare nan with -inf
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("nan"));
    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("inf"));
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    // Compare nan with -inf
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("nan"));
    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("-inf"));
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    // Compare inf with numerical value
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("inf"));
    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 0.0);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    // Compare inf with inf
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("inf"));
    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("inf"));
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    // Compare inf with -inf
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("inf"));
    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("-inf"));
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    // Check double precision
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 3.141592653589793);
    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 3.141592653589792);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    // Check almost equal doubles
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 3.1415926535897931234567890);
    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 3.1415926535897931234567891);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    // Check almost equal comparison for doubles and integers
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 3.0000000000000000000000001);
    child2 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 3);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    // Note: almostEqualDouble does not work if one input is exactely 0.0
    //       and the other input is almost equal to 0.0
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 0.0);
    child2 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 0);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    // Check double and int that are not almost equal but notequal 0.0
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 3.00000000000001);
    child2 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 3);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    // Check almost equal comparison for double and bool
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 1.0000000000000001);
    child2 = py::make_tuple(static_cast<int>(NodeType::BooleanNode), true);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    // Uses almostEqualDouble and should evaluate to true
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    // Uses almostEqualDouble and should evaluate to false
    EXPECT_FALSE(node->check(nullptr));

    // Note: almostEqualDouble does not work if one input is exactely 0.0
    //       and the other input is almost equal to 0.0
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 0.0);
    child2 = py::make_tuple(static_cast<int>(NodeType::BooleanNode), false);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));


    // Check double and bool that are not almost equal
    child1 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 1.00000000000001);
    child2 = py::make_tuple(static_cast<int>(NodeType::BooleanNode), true);
    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_FALSE(node->check(nullptr));

    bRelTuple = py::make_tuple(static_cast<int>(NodeType::BinaryRelationalNode), child1, child2,
                               static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(bRelTuple);
    EXPECT_TRUE(node->check(nullptr));

  }
  /// Test for the TernaryRelationalNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, ternaryRelationalNode)
  {
    Py_Initialize();
    // Get main module
    py::object main = py::import("__main__");
    py::object pyfloat = main.attr("__builtins__").attr("float");

    auto child1 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 1);
    auto child2 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 2);
    auto child3 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 3);
    auto tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child2, child3,
                                static_cast<int>(ComparisonOperator::LESS), static_cast<int>(ComparisonOperator::LESS));

    auto node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 < 2 < 3");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child1, child1,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 1 == 1");


    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child1, child1,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 1 <= 1");


    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child1, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 1 <= 2");


    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child1, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 1 < 2");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child1, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 1 != 2");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child1, child1,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 1 >= 1");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child1, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 1 > 2");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child1, child1,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 1 != 1");


    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child1, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 1 <= 2");

    // Create new double tuple
    child3 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 3.141);

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child1, child3,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 1 <= 3.141");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child1, child3, child3,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "1 == 3.141 <= 3.141");


    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child3,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "3.141 == 3.141 <= 3.141");


    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child3,
                           static_cast<int>(ComparisonOperator::NOTEQUAL), static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "3.141 != 3.141 <= 3.141");

    child3 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("nan"));

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child3,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "nan == nan == nan");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "nan == nan == 2");


    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "nan == nan != 2");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child3,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "nan == nan != nan");

    child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), pyfloat("inf"));

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::NOTEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_TRUE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "nan == nan != inf");


    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::EQUALEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "nan == nan == inf");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::LESS));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "nan == nan < inf");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::LESSEQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "nan == nan <= inf");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::GREATER));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "nan == nan > inf");

    tuple = py::make_tuple(static_cast<int>(NodeType::TernaryRelationalNode), child3, child3, child2,
                           static_cast<int>(ComparisonOperator::EQUALEQUAL), static_cast<int>(ComparisonOperator::GREATEREQUAL));
    node = NodeFactory::compile_boolean_node<MockVariableManager>(tuple);
    EXPECT_FALSE(node->check(nullptr));
    EXPECT_EQ(node->decompile(), "nan == nan >= inf");

  }
  /// Test for the UnaryExpressionNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, unaryExpressionNode)
  {
    Py_Initialize();
    auto child1 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 1);
    auto tuple = py::make_tuple(static_cast<int>(NodeType::UnaryExpressionNode), child1, false, false);
    auto node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<int>(node->evaluate(nullptr)), 1);

    // Check docompiling
    EXPECT_EQ(node->decompile(), "1");

    tuple = py::make_tuple(static_cast<int>(NodeType::UnaryExpressionNode), child1, true, false);
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<int>(node->evaluate(nullptr)), -1);

    // Check docompiling
    EXPECT_EQ(node->decompile(), "-1");

    tuple = py::make_tuple(static_cast<int>(NodeType::UnaryExpressionNode), child1, true, true);
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<int>(node->evaluate(nullptr)), -1);

    // Check docompiling
    EXPECT_EQ(node->decompile(), "-( 1 )");

    tuple = py::make_tuple(static_cast<int>(NodeType::UnaryExpressionNode), child1, false, true);
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<int>(node->evaluate(nullptr)), 1);

    // Check docompiling
    EXPECT_EQ(node->decompile(), "( 1 )");

  }
  /// Test for the BinaryExpressionNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, binaryExpressionNode)
  {
    Py_Initialize();
    auto child1 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 1);
    auto child2 = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 2.5);
    auto tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child1, child1,
                                static_cast<int>(ArithmeticOperation::PLUS));
    auto node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<int>(node->evaluate(nullptr)), 2);

    // Check docompiling
    EXPECT_EQ(node->decompile(), "1 + 1");

    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child1, child2,
                           static_cast<int>(ArithmeticOperation::PLUS));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(nullptr)), 3.5);
    EXPECT_EQ(node->decompile(), "1 + 2.5");

    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child1, child2,
                           static_cast<int>(ArithmeticOperation::MINUS));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(nullptr)), -1.5);
    EXPECT_EQ(node->decompile(), "1 - 2.5");


    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child1, child2,
                           static_cast<int>(ArithmeticOperation::PRODUCT));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(nullptr)), 2.5);
    EXPECT_EQ(node->decompile(), "1 * 2.5");


    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child1, child2,
                           static_cast<int>(ArithmeticOperation::DIVISION));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(nullptr)), 0.4);
    EXPECT_EQ(node->decompile(), "1 / 2.5");


    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child1, child1,
                           static_cast<int>(ArithmeticOperation::POWER));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(nullptr)), 1.0);
    EXPECT_EQ(node->decompile(), "1 ** 1");

    child1 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 2);
    child2 = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 3);

    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child1, child2,
                           static_cast<int>(ArithmeticOperation::POWER));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(nullptr)), 8.0);
    EXPECT_EQ(node->decompile(), "2 ** 3");

    child2 = py::make_tuple(static_cast<int>(NodeType::IdentifierNode), "mocking_variable");
    MockObjectType part{10};
    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child1, child2,
                           static_cast<int>(ArithmeticOperation::POWER));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(&part)), 1024);
    EXPECT_EQ(node->decompile(), "2 ** mocking_variable");


    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child2, child2,
                           static_cast<int>(ArithmeticOperation::PLUS));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<int>(node->evaluate(&part)), 20);
    EXPECT_EQ(node->decompile(), "mocking_variable + mocking_variable");

    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child2, child2,
                           static_cast<int>(ArithmeticOperation::MINUS));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<int>(node->evaluate(&part)), 0);
    EXPECT_EQ(node->decompile(), "mocking_variable - mocking_variable");

    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child2, child2,
                           static_cast<int>(ArithmeticOperation::PRODUCT));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<int>(node->evaluate(&part)), 100);
    EXPECT_EQ(node->decompile(), "mocking_variable * mocking_variable");


    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child2, child2,
                           static_cast<int>(ArithmeticOperation::DIVISION));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(&part)), 1);
    EXPECT_EQ(node->decompile(), "mocking_variable / mocking_variable");


    part = MockObjectType{20};
    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child1, child2,
                           static_cast<int>(ArithmeticOperation::DIVISION));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(&part)), 0.1);
    EXPECT_EQ(node->decompile(), "2 / mocking_variable");

    part = MockObjectType{0.1};
    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode), child2, child1,
                           static_cast<int>(ArithmeticOperation::DIVISION));
    node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(&part)), 0.05);
    EXPECT_EQ(node->decompile(), "mocking_variable / 2");


  }
  /// Test for the IdentifierNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, identifierNode)
  {
    Py_Initialize();
    MockObjectType part{4.2};
    auto tuple = py::make_tuple(static_cast<int>(NodeType::IdentifierNode), "mocking_variable");
    auto node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(&part)), 4.2);

    tuple = py::make_tuple(static_cast<int>(NodeType::IdentifierNode), "THISDOESNOTEXIST");
    EXPECT_THROW(NodeFactory::compile_expression_node<MockVariableManager>(tuple), std::runtime_error);

    // Check nested runtime_error
    tuple = py::make_tuple(static_cast<int>(NodeType::UnaryExpressionNode), tuple, false, false);
    EXPECT_THROW(NodeFactory::compile_expression_node<MockVariableManager>(tuple), std::runtime_error);

  }
  /// Test for the FunctionNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, functionNode)
  {
    Py_Initialize();
    MockObjectType part{4};
    auto tuple = py::make_tuple(
                   static_cast<int>(NodeType::FunctionNode),
                   "sum",
                   "mocking_variable, 2, 1"
                 );
    auto node = NodeFactory::compile_expression_node<MockVariableManager>(tuple);
    EXPECT_EQ(std::get<double>(node->evaluate(&part)), 7);
    EXPECT_EQ(node->decompile(), "sum(mocking_variable, 2, 1)");
    part = MockObjectType{4.2};
    EXPECT_EQ(std::get<double>(node->evaluate(&part)), 7.2);
    part = MockObjectType{false};
    EXPECT_EQ(std::get<double>(node->evaluate(&part)), 3.0);
    part = MockObjectType{true};
    EXPECT_EQ(std::get<double>(node->evaluate(&part)), 4.0);

  }
  /// Test for the DataNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, integerNode)
  {
    Py_Initialize();
    // Get main module
    py::object main = py::import("__main__");

    // Create IntegerNode tuples.
    const py::tuple iTuple_val = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 1337);
    const py::tuple iTuple_val_neg = py::make_tuple(static_cast<int>(NodeType::IntegerNode), -1337);
    const py::tuple iTuple_val_zero = py::make_tuple(static_cast<int>(NodeType::IntegerNode), 0);

    // Check overflow management
    const py::tuple iTuple_max = py::make_tuple(static_cast<int>(NodeType::IntegerNode),
                                                main.attr("__builtins__").attr("int")("2147483647"));
    const py::tuple iTuple_min = py::make_tuple(static_cast<int>(NodeType::IntegerNode),
                                                main.attr("__builtins__").attr("int")("-2147483648"));
    const py::tuple iTuple_pos_overflow = py::make_tuple(static_cast<int>(NodeType::IntegerNode),
                                                         main.attr("__builtins__").attr("int")("2147483648"));
    const py::tuple iTuple_neg_overflow = py::make_tuple(static_cast<int>(NodeType::IntegerNode),
                                                         main.attr("__builtins__").attr("int")("-2147483649"));

    // Compile IntegerNodes and check evaluation
    auto iNode_val = NodeFactory::compile_expression_node<MockVariableManager>(iTuple_val);
    auto iNode_val_neg = NodeFactory::compile_expression_node<MockVariableManager>(iTuple_val_neg);
    auto iNode_val_zero = NodeFactory::compile_expression_node<MockVariableManager>(iTuple_val_zero);
    // Compile overflow management
    auto iNode_max = NodeFactory::compile_expression_node<MockVariableManager>(iTuple_max);
    auto iNode_min = NodeFactory::compile_expression_node<MockVariableManager>(iTuple_min);
    auto iNode_pos_overflow = NodeFactory::compile_expression_node<MockVariableManager>(iTuple_pos_overflow);
    auto iNode_neg_overflow = NodeFactory::compile_expression_node<MockVariableManager>(iTuple_neg_overflow);

    // test evaluation
    EXPECT_EQ(std::get<int>(iNode_val->evaluate(nullptr)), 1337);
    EXPECT_EQ(std::get<int>(iNode_val_neg->evaluate(nullptr)), -1337);
    EXPECT_EQ(std::get<int>(iNode_val_zero->evaluate(nullptr)), 0);
    EXPECT_EQ(std::get<int>(iNode_max->evaluate(nullptr)), 2147483647);
    EXPECT_EQ(std::get<int>(iNode_min->evaluate(nullptr)), -2147483648);
    // test overflowing node evaluation (returned as double)
    EXPECT_EQ(std::get<double>(iNode_pos_overflow->evaluate(nullptr)), 2147483648.);
    EXPECT_EQ(std::get<double>(iNode_neg_overflow->evaluate(nullptr)), -2147483649.);

    // test decompiling
    EXPECT_EQ(iNode_val->decompile(), "1337");
    EXPECT_EQ(iNode_val_neg->decompile(), "-1337");
    EXPECT_EQ(iNode_val_zero->decompile(), "0");
    EXPECT_EQ(iNode_max->decompile(), "2147483647");
    EXPECT_EQ(iNode_min->decompile(), "-2147483648");
    // test overflowing node decompiling
    EXPECT_EQ(iNode_pos_overflow->decompile(), "2.14748e+09");
    EXPECT_EQ(iNode_neg_overflow->decompile(), "-2.14748e+09");
  }
  /// Test for the DataNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, booleanNode)
  {
    Py_Initialize();
    // Create two BooleanNode tuples with values true and false.
    const py::tuple booleanTuple_true = py::make_tuple(static_cast<int>(NodeType::BooleanNode), true);
    const py::tuple booleanTuple_false = py::make_tuple(static_cast<int>(NodeType::BooleanNode), false);
    // Compile BooleanNodes and check evaluation
    auto booleanNode_true = NodeFactory::compile_expression_node<MockVariableManager>(booleanTuple_true);
    auto booleanNode_false = NodeFactory::compile_expression_node<MockVariableManager>(booleanTuple_false);
    // test evaluation
    EXPECT_TRUE(std::get<bool>(booleanNode_true->evaluate(nullptr)));
    EXPECT_FALSE(std::get<bool>(booleanNode_false->evaluate(nullptr)));
    // test decompiling
    EXPECT_EQ(booleanNode_true->decompile(), "true");
    EXPECT_EQ(booleanNode_false->decompile(), "false");
  }
  /// Test for the DataNode: Create Nodes from boost::python::tuples and check functionality.
  TEST(CutNodesTest, doubleNode)
  {
    Py_Initialize();
    // Get main module
    py::object main = py::import("__main__");

    // Value tuples
    const py::tuple tuple_val = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 3.141);
    const py::tuple tuple_val_neg = py::make_tuple(static_cast<int>(NodeType::DoubleNode), -3.141);
    const py::tuple tuple_val_zero = py::make_tuple(static_cast<int>(NodeType::DoubleNode), 0.0);
    // Infinity tuples
    const py::tuple tuple_inf = py::make_tuple(static_cast<int>(NodeType::DoubleNode), main.attr("__builtins__").attr("float")("inf"));
    const py::tuple tuple_neg_inf = py::make_tuple(static_cast<int>(NodeType::DoubleNode),
                                                   main.attr("__builtins__").attr("float")("-inf"));
    // nan tuple
    const py::tuple tuple_nan = py::make_tuple(static_cast<int>(NodeType::DoubleNode), main.attr("__builtins__").attr("float")("nan"));

    // Value nodes
    auto doubleNode_val = NodeFactory::compile_expression_node<MockVariableManager>(tuple_val);
    auto doubleNode_val_neg = NodeFactory::compile_expression_node<MockVariableManager>(tuple_val_neg);
    auto doubleNode_val_zero = NodeFactory::compile_expression_node<MockVariableManager>(tuple_val_zero);

    // Infinity nodes
    auto doubleNode_inf = NodeFactory::compile_expression_node<MockVariableManager>(tuple_inf);
    auto doubleNode_neg_inf = NodeFactory::compile_expression_node<MockVariableManager>(tuple_neg_inf);
    // NaN nodes
    auto doubleNode_nan = NodeFactory::compile_expression_node<MockVariableManager>(tuple_nan);

    // test value evaluation
    EXPECT_EQ(std::get<double>(doubleNode_val->evaluate(nullptr)), 3.141);
    EXPECT_EQ(std::get<double>(doubleNode_val_neg->evaluate(nullptr)), -3.141);
    EXPECT_EQ(std::get<double>(doubleNode_val_zero->evaluate(nullptr)), 0);

    // test infinity evaluation
    EXPECT_EQ(std::get<double>(doubleNode_inf->evaluate(nullptr)), std::numeric_limits<double>::infinity());
    EXPECT_EQ(std::get<double>(doubleNode_neg_inf->evaluate(nullptr)), -1 * std::numeric_limits<double>::infinity());

    // test NaN evaluation
    EXPECT_TRUE(std::isnan(std::get<double>(doubleNode_nan->evaluate(nullptr))));

    // test value decompiling
    EXPECT_EQ(doubleNode_val->decompile(), "3.141");
    EXPECT_EQ(doubleNode_val_neg->decompile(), "-3.141");
    EXPECT_EQ(doubleNode_val_zero->decompile(), "0");
    // test infinity decompiling
    EXPECT_EQ(doubleNode_inf->decompile(), "inf");
    EXPECT_EQ(doubleNode_neg_inf->decompile(), "-inf");
    // test nan decompiling
    EXPECT_EQ(doubleNode_nan->decompile(), "nan");
  }
}  // namespace
