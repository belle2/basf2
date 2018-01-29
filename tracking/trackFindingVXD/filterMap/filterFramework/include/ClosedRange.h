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

#include <string>

namespace Belle2 {

  /** Represents a closed set of arithmetic types.
   *
   * Range is used in conjunction with the SelectionVariable to define
   * one of the building blocks of the Filters
   */

  template< typename MinType, typename MaxType>
  class ClosedRange {
  public:

    /** Constructor */
    ClosedRange(MinType min, MaxType max): m_min(min), m_max(max) {};
    ClosedRange(): m_min(0), m_max(0) {};

    /** Method used by the filter tools to decide if accept a combination.
     *
     * @param x is the result of some SelectionVariable
     * It returns  true if x belongs to the closed set [ m_min, m_max ]
     * It does not throw.
     */
    template< class VariableType >
    inline bool contains(const VariableType& x) const { return m_min <= x && x <= m_max ;};

    /** Creates and sets the addresses of the leaves to store the min and max values.
     *
     * @param t  the pointer to the TTree that will contain the TBranch of this range.
     * @param branchName the name of the TBranch that will host this range.
     * @param variableName the name of the selection variable this range is applied to.
     *
     * The leaves will be named as the selection variable name with the "_min"/"_max"
     * suffixes for the min/max value.
     */
    void persist(TTree* t, const std::string& branchName, const std::string& variableName)
    {

      std::string leafList;
      leafList += variableName;
      leafList += "_min/";
      leafList += TBranchLeafType(m_min);
      leafList += ":";
      leafList += variableName;
      leafList += "_max/";
      leafList += TBranchLeafType(m_max);
      TBranch* branch = new TBranch(t, branchName.c_str() , & m_min, leafList.c_str());
      t->GetListOfBranches()->Add(branch);
    }

    /** sets branch addresses of the given tree to the m_min and m_msx. Note: it assumes a certain oder for
     * @param t: the tree of the which the branch addresses need to be set
     * @param variableName: specifier for the leafs
    */
    void setBranchAddress(TTree* t, const std::string& branchName,
                          const std::string& /*variableName*/)
    {
      // sets branch address and checks for validity
      if (t->SetBranchAddress(branchName.c_str(), & m_min) < 0) B2FATAL("ClosedRange: branch address not valid");
    }

    /** Accessor to the inf of the set (which is also the min) */
    MinType getInf(void) const { return m_min; } ;

    /** Accessor to the sup of the set (which is alsto the max) */
    MaxType getSup(void) const { return m_max; } ;


    /** generates a "name" and fills the vector with the variable references
    @param filtername: optional name of the filter this range is attached to make the output look nicer
    @param references: pointer to vector which contains a pair of char which indicates the type object pointed to
      and the actual pointers to the bounds, if equal to nullptr it will not be filled
    **/
    std::string getNameAndReference(std::vector< std::pair<char, void*> >* pointers = nullptr, std::string varname = "X")
    {
      std::string minVal = std::to_string(m_min);
      std::string maxVal = std::to_string(m_max);
      // if pointer to vector is provided fill it
      if (pointers != nullptr) {
        // use the position in the vector as unique identifier
        minVal = "#" + std::to_string(pointers->size());
        (*pointers).push_back({TBranchLeafType(m_min), &m_min});
        maxVal = "#" + std::to_string(pointers->size());
        (*pointers).push_back({TBranchLeafType(m_max), &m_max});
      }
      return ("(" + minVal + " <= " + varname + " <= " + maxVal + ")");
    }

  private:

    // NOTE: the persit function assumes that these data members (m_min, m_max) are in exactly that
    // order defined! So DO NOT move these declarations!!!!
    /// the minimum of this range
    MinType m_min;
    /// the maximum of this range
    MaxType m_max;

  };


}
