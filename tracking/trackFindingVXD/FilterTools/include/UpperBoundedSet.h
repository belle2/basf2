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

#include "tracking/trackFindingVXD/FilterTools/TBranchLeafType.h"
#include <TBranch.h>
#include <TTree.h>

#include <typeinfo>

namespace Belle2 {

  /** Represents an upper bounded set of arithmetic types.
   *
   * UpperBoundedSet is used in conjunction with the SelectionVariable to define
   * one of the building blocks of the Filters
   */
  template< typename SupType >
  class UpperBoundedSet {
    SupType m_sup;
  public:
    UpperBoundedSet(SupType sup):  m_sup(sup) {};

    /** Method used by the filter tools to decide on the fate of the pair.
     *
     * @param x is the result of some SelectionVariable applied to a pair of objects.
     * The return value is true if x belongs to the open set ( -infinity, m_sup )
     */

    template< class VariableType >
    inline bool contains(const VariableType& x) const { return x < m_sup;};

    /** Creates and sets the addresses of the leaves to store sup value.
     *
     * @param t  the pointer to the TTree that will contain the TBranch of this range.
     * @param branchName the name of the TBranch that will host this range.
     * @param variableName the name of the selection variable this range is applied to.
     *
     * The leaves will be named as the selection variable name with the "_sup"
     * suffixes for the m_sup value.
     */

    void persist(TTree* t, const string& branchName, const string& variableName)
    {

      string leafList;
      leafList += variableName;
      leafList += "_sup/";
      leafList += TBranchLeafType(m_sup);

      TBranch* branch = new TBranch(t, branchName.c_str() , & m_sup, leafList.c_str());
      t->GetListOfBranches()->Add(branch);
    }

    /** Accessor to the sup of the set */
    SupType getSup(void) const { return m_sup; } ;

  };

}
