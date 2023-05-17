/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TTree.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/TBranchLeafType.h>

namespace Belle2 {


  /** This class contains **/
  template <class Variable>
  class VariableTBranch {
    /// The value to be stored in tha branch
    typename Variable::variableType m_storedValue;
    /// Member for the pointer to the TBranch
    TBranch* m_branch;
  public:
    /** Add to the TTree
     * @param tree a branch whose name and type are inferred from parameter var
     */
    explicit VariableTBranch(TTree* tree);

    /** Set the value to be stored in the TTree
     * @param var : this value will be set
     */
    void setValue(const typename Variable::variableType& var)
    { m_storedValue = var;}

    /** Evaluate the variable for the given SpacePoints
     * @tparam SpacePoints : SpacePoints type(s)
     * @param sps : SpacePoints
     */
    template<class ... SpacePoints>
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
      m_branch = tree->Branch(Variable::name().c_str(), & m_storedValue,
                              TBranchLeafType(m_storedValue));
  }
}
