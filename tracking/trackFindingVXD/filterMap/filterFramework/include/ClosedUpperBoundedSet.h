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

#include <framework/logging/Logger.h>

#include <TBranch.h>
#include <TTree.h>

#include <typeinfo>

namespace Belle2 {

  /** Represents an upper bounded set of arithmetic types.
   *
   * UpperBoundedSet is used in conjunction with the SelectionVariable to define
   * one of the building blocks of the Filters
   */
  template< typename MaxType >
  class ClosedUpperBoundedSet {
    MaxType m_max;
  public:
    ClosedUpperBoundedSet(MaxType max):  m_max(max) {};
    ClosedUpperBoundedSet():  m_max(0) {};

    /** Method used by the filter tools to decide on the fate of the pair.
     *
     * @param x is the result of some SelectionVariable applied to a pair of objects.
     * The return value is true if x belongs to the closed set [ -infinity, m_max ]
     */

    template< class VariableType >
    inline bool contains(const VariableType& x) const { return x <= m_max;};

    /** Creates and sets the addresses of the leaves to store the min and max values.
     *
     * @param t  the pointer to the TTree that will contain the TBranch of this range.
     * @param branchName the name of the TBranch that will host this range.
     * @param variableName the name of the selection variable this range is applied to.
     *
     * The leaves will be named as the selection variable name with the "_max"
     * suffixes for the m_max value.
     */

    void persist(TTree* t, const std::string& branchName, const std::string& variableName)
    {

      std::string leafList;
      leafList += variableName;
      leafList += "_max/";
      leafList += TBranchLeafType(m_max);

      TBranch* branch = new TBranch(t, branchName.c_str() , & m_max, leafList.c_str());
      t->GetListOfBranches()->Add(branch);
    }

    void setBranchAddress(TTree* t, const std::string& branchName,
                          const std::string& /*variableName*/)
    {
      if (t->SetBranchAddress(branchName.c_str(), & m_max) < 0) B2FATAL("ClosedUpperBoundedSet: branch address not valid!");
    }

    /** Accessor to the sup of the set (which is also the max) */
    MaxType getSup(void) const { return m_max; } ;


    /** generates a "name" and fills the vector with the variable references
    @param filtername: optional name of the filter this range is attached to make the output look nicer
    @param references: pointer to vector which contains a pair of char which indicates the type object pointed to
      and the actual pointers to the bounds, if equal to nullptr it will not be filled
    **/
    std::string getNameAndReference(std::vector< std::pair<char, void*> >* pointers = nullptr, std::string varname = "X")
    {
      std::string maxVal = std::to_string(m_max);
      // if pointer to vector is provided fill it
      if (pointers != nullptr) {
        // use the position in the vector as unique identifier
        maxVal = "#" + std::to_string(pointers->size());
        (*pointers).push_back({TBranchLeafType(m_max), &m_max});
      }
      return ("(" + varname + " <= " + maxVal + ")");
    }

  };

}
