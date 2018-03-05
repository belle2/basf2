/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/training/VariableTBranch.h>
#include <TTree.h>
namespace Belle2 {

  /** Dump on a TTree the values of all the variables in a filter **/
  template< typename ... filterLeaves >
  class VariablesTTree {
    /** Empty, just specialized templates are interesting */
  };


  /** Defines the interface using an empty template pack **/
  template<>
  class VariablesTTree<> {
    TTree* m_tree;  // Not owned
  public:

    /** All the variables will be written in @param tree */
    VariablesTTree(TTree* tree): m_tree(tree) {};

    /* Destructor*/
    ~VariablesTTree() {/* nothing to delete */};

    /* Accessor to the TTree pointer */
    TTree* getTTree(void) { return m_tree; };

    /** Handy function **/
    template< class Filter >
    static auto build(const Filter&, TTree* tree) ->
    VariablesTTree<Filter>
    { return VariablesTTree<Filter>(tree); };
  };


  /** Specialization for a simple filter  **/
  template< class Variable , class ... other>
  class VariablesTTree< Filter< Variable, other ...> >:
    public VariablesTTree<> {
    VariableTBranch< Variable > m_varTBranch;
  public:
    explicit VariablesTTree(TTree* tree): VariablesTTree<>(tree),
      m_varTBranch(tree) {};

    template< class ... SpacePoints >
    void evaluateOn(const SpacePoints& ... sps)
    {
      m_varTBranch.evaluateOn(sps...);
    }
  };


  /** Specialization for unary operators acting on a filter  **/
  template< class unaryOperator , class ... args, class ... other>
  class VariablesTTree< Filter< unaryOperator, Filter< args ...>,
          other ...> >:
          public VariablesTTree<> {
    VariablesTTree< Filter< args ...> > m_node;
  public:
    explicit VariablesTTree(TTree* tree): VariablesTTree<>(tree),
      m_node(tree) {};

    template< class ... SpacePoints >
    void evaluateOn(const SpacePoints& ... sps)
    {
      m_node.evaluateOn(sps...);
    }
  };


  /** Specialization for binary operators acting on a filter  **/
  template< class binaryOperator , class ... argsA, class ... argsB,
            class ... other>
  class VariablesTTree< Filter< binaryOperator,
          Filter< argsA ...>, Filter< argsB ... >,
          other ...> >:
          public VariablesTTree<> {
    VariablesTTree< Filter< argsA ... > > m_nodeA;
    VariablesTTree< Filter< argsB ... > > m_nodeB;
  public:
    explicit VariablesTTree(TTree* tree): VariablesTTree<>(tree),
      m_nodeA(tree), m_nodeB(tree)
    {};

    template< class ... SpacePoints >
    void evaluateOn(const SpacePoints& ... sps)
    {
      m_nodeA.evaluateOn(sps...);
      m_nodeB.evaluateOn(sps...);
    }
  };
}
