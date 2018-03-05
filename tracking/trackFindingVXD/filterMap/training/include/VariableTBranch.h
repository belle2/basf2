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

#include <TTree.h>
#include <TObject.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/TBranchLeafType.h>
#include <type_traits>

namespace Belle2 {


  /** This class contains **/
  template <class Variable>
  class VariableTBranch {
    typename Variable::variableType m_storedValue;
    TBranch* m_branch;
  public:
    /** Add to the TTree  @param tree a branch whose name and type are
    inferred from @param var **/
    explicit VariableTBranch(TTree* tree);

    /** Standard destructor */
    ~VariableTBranch()
    { /* nothing to delete, the m_branch belongs to the tree */};

    /* Set the value to be stored in the TTree */
    void setValue(const typename Variable::variableType& var)
    { m_storedValue = var;}

    template< class ... SpacePoints  >
    void evaluateOn(const SpacePoints& ... sps)
    {
      m_storedValue = Variable::value(sps ...);
    }
  };

  /** Constructor specialized for arithmetic types **/
  template <class Variable>
  VariableTBranch<Variable>::VariableTBranch(TTree* tree)
  {
    if (tree != nullptr && tree -> GetBranch(Variable::name().c_str()) == nullptr)
      m_branch = tree->Branch(Variable::name().c_str() , & m_storedValue,
                              TBranchLeafType(m_storedValue));
  }
}
