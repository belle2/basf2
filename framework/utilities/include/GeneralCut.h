/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <boost/python.hpp>
#include <framework/utilities/CutNodes.h>
#include <framework/utilities/NodeFactory.h>

#include <string>
#include <memory>

#include <sstream>

#include <iostream>
#include <stdexcept>

#include <variant>

namespace py = boost::python;
typedef const py::tuple& Nodetuple;

namespace Belle2 {

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
   * 3. Square brackets [] nesting boolean statements
   * 4. Parenthesis () for grouping expressions
   * 5. Floats
   * 6. Variables registered in the general "Variable Manager" which are used as a template argument to this class.
   *
   * For example when using the analysis VariableManager for particles, valid cuts are:
   * 1.2 < M < 1.5
   * daughter(0, M) < daughter(1, M)
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
      // Here we parse
      Py_Initialize();
      try {
        py::object b2parser_namespace = py::import("b2parser");
        py::tuple tuple = py::extract<py::tuple>(b2parser_namespace.attr("parse")(cut));
        return std::unique_ptr<GeneralCut>(new GeneralCut(tuple));
      } catch (py::error_already_set&) {
        PyErr_Print();
        B2FATAL("Parsing error on cutstring:\n" + cut);
      }
    }
    /**
     * Check if the current cuts are passed by the given object
     * @param p pointer to the object, that should be checked. All formerly received variables from the variable manager
     * (from the type Var), are asked for their value using var->function(p).
     */
    bool check(const Object* p) const
    {
      if (m_root != nullptr) return m_root->check(p);
      throw std::runtime_error("GeneralCut m_root is not initialized.");
    }

    /**
     * Print cut tree
     */
    void print() const
    {
      m_root->print();
    }

    /**
     * Do the compilation from a string in return. In principle, compile(decompile()) should give the same result again.
     */
    std::string decompile() const
    {
      std::stringstream stringstream;
      stringstream << m_root->decompile();
      return stringstream.str();
    }


  private:
    /**
     * Constructor of the cut. Call init with given Nodetuple
     * @param tuple (const boost::python::tuple&) constructed by the python parser from cut.
     */
    explicit GeneralCut(Nodetuple tuple) : m_root{NodeFactory::compile_boolean_node<AVariableManager>(tuple)} {}

    /**
     * Delete Copy constructor
     */
    GeneralCut(const GeneralCut&) = delete;

    /**
     * Delete assign operator
     */
    GeneralCut& operator=(const GeneralCut&) = delete;

    std::unique_ptr<const AbstractBooleanNode<AVariableManager>> m_root; /**< cut root node */
  };
}
