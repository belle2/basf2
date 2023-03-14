/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/TBranchLeafType.h>
#include <TBranch.h>
#include <TTree.h>

#include <framework/logging/Logger.h>

namespace Belle2 {

  /** Represents an upper bounded set of arithmetic types.
   *
   * UpperBoundedSet is used in conjunction with the SelectionVariable to define
   * one of the building blocks of the Filters
   */
  template< typename SupType >
  class UpperBoundedSet {

    /// Supremum of the set
    SupType m_sup;
  public:
    /// Constructor
    explicit UpperBoundedSet(SupType sup): m_sup(sup) {};

    /// Constructor without argument
    UpperBoundedSet(): m_sup(0) {};

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
    void persist(TTree* t, const std::string& branchName, const std::string& variableName)
    {

      std::string leafList;
      leafList += variableName;
      leafList += "_sup/";
      leafList += TBranchLeafType(m_sup);

      TBranch* branch = new TBranch(t, branchName.c_str(), & m_sup, leafList.c_str());
      t->GetListOfBranches()->Add(branch);
    }

    /** Setting the branch address for a filter in a TTree
     * @param t : the TTree in which the branch address shall be set
     * @param branchName : name of the branch
     */
    void setBranchAddress(TTree* t, const std::string& branchName,
                          const std::string& /*variableName*/)
    {
      if (t->SetBranchAddress(branchName, & m_sup) < 0) B2FATAL("UpperBoundedSet: branch address not valid!");
    }

    /** Accessor to the sup of the set */
    SupType getSup(void) const { return m_sup; } ;


    /** generates a "name" and fills the vector with the variable references
      @param pointers: pointer to vector which contains a pair of char which indicates the type object pointed to
        and the actual pointers to the bounds, if equal to nullptr it will not be filled
      @param varname: optional name of the filter this range is attached to make the output look nicer
    **/
    std::string getNameAndReference(std::vector<std::pair<char, void*>>* pointers = nullptr, const std::string& varname = "X")
    {
      std::string maxVal = std::to_string(m_sup);
      // if pointer to vector is provided fill it
      if (pointers != nullptr) {
        // use the position in the vector as unique identifier
        maxVal = "#" + std::to_string(pointers->size());
        (*pointers).push_back({TBranchLeafType(m_sup), &m_sup});
      }
      return ("(" + varname + " < " + maxVal + ")");
    }
  };
}
