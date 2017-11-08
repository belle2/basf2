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

#include <tracking/trackFindingVXD/filterMap/filterFramework/TBranchLeafType.h>
#include <TBranch.h>
#include <TTree.h>

#include <framework/logging/Logger.h>

#include <typeinfo>

namespace Belle2 {

  /** Represents a range of arithmetic types.
   *
   * Range is used in conjunction with the SelectionVariable to define
   * one of the building blocks of the Filters
   */

  template< typename InfType, typename SupType>
  class Range {
    const char* c_infSuffix = "_inf";
    const char* c_supSuffix = "_sup";

    InfType m_inf;
    SupType m_sup;
  public:

    /** Constructor */
    Range(InfType inf, SupType sup): m_inf(inf), m_sup(sup) {};

    Range(): m_inf(0.), m_sup(0.) {};
    /** Method used by the filter tools to decide on the fate of the pair.
     *
     * @param x is the result of some SelectionVariable applied to a pair of objects.
     * The return value is true if x belongs to the open set ( m_min, m_sup )
     */
    template< class VariableType >
    inline bool contains(const VariableType& x) const { return m_inf < x && x < m_sup ;};

    /** Creates and sets the addresses of the leaves to store the inf and sup values.
     *
     * @param t  the pointer to the TTree that will contain the TBranch of this range.
     * @param branchName the name of the TBranch that will host this range.
     * @param variableName the name of the selection variable this range is applied to.
     *
     * The leaves will be named as the selection variable name with the "_inf"/"_sup"
     * suffixes for the inf/sup value.
     */
    void persist(TTree* t, const std::string& branchName, const std::string& variableName)
    {

      std::string leafList;
      leafList += variableName;
      leafList += c_infSuffix;
      leafList += "/";
      leafList += TBranchLeafType(m_inf);
      leafList += ":";
      leafList += variableName;
      leafList += c_supSuffix;
      leafList += "/";
      leafList += TBranchLeafType(m_sup);
      TBranch* branch = new TBranch(t, branchName.c_str() , & m_inf, leafList.c_str());
      t->GetListOfBranches()->Add(branch);
    }

    void setBranchAddress(TTree* t, const std::string& branchName,
                          const std::string& /*variableName*/)
    {
      if (t->SetBranchAddress(branchName.c_str(), & m_inf) < 0) B2FATAL("Range: branch address not valid");
    }

    /** Accessor to the inf of the set */
    InfType getInf(void) const { return m_inf; } ;

    /** Accessor to the sup of the set */
    SupType getSup(void) const { return m_sup; } ;

  };


}
