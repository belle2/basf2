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

#include "tracking/trackFindingVXD/filterTools/TBranchLeafType.h"
#include <TBranch.h>
#include <TTree.h>

#include <typeinfo>

namespace Belle2 {

  /** Represents a closed lower bounded set of arithmetic types.
   *
   * ClosedLowerBoundedSet is used in conjunction with the SelectionVariable to define
   * one of the building blocks of the Filters
   */

  template< typename MinType >
  class ClosedLowerBoundedSet {
    MinType m_min;
  public:

    /** Constructor */
    ClosedLowerBoundedSet(MinType min): m_min(min) {};
    ClosedLowerBoundedSet(): m_min(0) {};

    /** Method used by the filter tools to decide on the fate of the pair.
     *
     * @param x is the result of some SelectionVariable applied to a pair of objects.
     * The return value is true if x belongs to the closed set [ m_min, infinity ]
     */
    template< class VariableType >
    inline bool contains(const VariableType& x) const { return  m_min <= x ;};

    /** Creates and sets the addresses of the leaves to store the min value.
     *
     * @param t  the pointer to the TTree that will contain the TBranch of this range.
     * @param branchName the name of the TBranch that will host this range.
     * @param variableName the name of the selection variable this range is applied to.
     *
     * The leaves will be named as the selection variable name with the "_min"
     * suffix for the min value.
     */
    void persist(TTree* t, const string& branchName, const string& variableName)
    {

      string leafList;
      leafList += variableName;
      leafList += "_min/";
      leafList += TBranchLeafType(m_min);
      TBranch* branch = new TBranch(t, branchName.c_str() , & m_min, leafList.c_str());
      t->GetListOfBranches()->Add(branch);
    }

    /** Accessor to the inf of the set (which coincides with the min)*/
    MinType getInf(void) const { return m_min; } ;

  };


}
