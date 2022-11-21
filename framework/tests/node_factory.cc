/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <functional>
#include <variant>
#include <boost/python.hpp>

#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/utilities/CutNodes.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>


using namespace Belle2;
namespace {
  namespace py = boost::python;
  using VarVariant = Variable::Manager::VarVariant;
  using VariableManager = Variable::Manager;
  /// Class to mock objects for out variable manager.
  struct MockObjectType {
    /// Stupid singlevalued object.
    explicit MockObjectType(const double& d) : m_value{d} {}
    explicit MockObjectType(const int& i) : m_value{i} {}
    explicit MockObjectType(const bool& b) : m_value{b} {}
    VarVariant m_value;
  };

  /**
   *  Class to mock variables for out variable manager.
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

    /** Typedef for variable return type, can either be double, int or bool in std::variant */
    typedef std::variant<double, int, bool> VarVariant;


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
      (void) functionName;
      (void) functionArguments;
      return &m_mocking_variable;
    }

    /// The only variable we have in this test.
    Var m_mocking_variable{"mocking_variable"};
  };


  TEST(NodeFactory, FactoryDeathTest)
  {
    Py_Initialize();
    // Make empty tuple
    py::tuple tuple = py::tuple();
    EXPECT_B2FATAL(Belle2::NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    // Make UnaryExpressionNode which doesn't have the correct length
    tuple = py::make_tuple(static_cast<int>(NodeType::UnaryExpressionNode));
    EXPECT_B2FATAL(Belle2::NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    // Make BinaryExpressionNode which doesn't have the correct length
    tuple = py::make_tuple(static_cast<int>(NodeType::BinaryExpressionNode));
    EXPECT_B2FATAL(Belle2::NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    // Make IntegerNode which doesn't have the correct length
    tuple = py::make_tuple(static_cast<int>(NodeType::IntegerNode));
    EXPECT_B2FATAL(Belle2::NodeFactory::compile_expression_node<MockVariableManager>(tuple));

// Make DoubleNode which doesn't have the correct length
    tuple = py::make_tuple(static_cast<int>(NodeType::DoubleNode));
    EXPECT_B2FATAL(Belle2::NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    // Make BooleanNode which doesn't have the correct length
    tuple = py::make_tuple(static_cast<int>(NodeType::BooleanNode));
    EXPECT_B2FATAL(Belle2::NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    // Make IdentifierNode which doesn't have the correct length
    tuple = py::make_tuple(static_cast<int>(NodeType::IdentifierNode));
    EXPECT_B2FATAL(Belle2::NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    // Make FunctionNode which doesn't have the correct length
    tuple = py::make_tuple(static_cast<int>(NodeType::FunctionNode));
    EXPECT_B2FATAL(Belle2::NodeFactory::compile_expression_node<MockVariableManager>(tuple));

    // Make Identifier Tuple which should trigger a runtime error
    tuple = py::make_tuple(static_cast<int>(NodeType::IdentifierNode), "THISDOESNOTEXIST");
    EXPECT_THROW(Belle2::NodeFactory::compile_expression_node<MockVariableManager>(tuple), std::runtime_error);

  }
}
