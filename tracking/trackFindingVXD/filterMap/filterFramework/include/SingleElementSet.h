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

#include <typeinfo>

namespace Belle2 {

  /** Represents a set containing a single element;
   *
   * SingleElementSet is used in conjunction with the SelectionVariable to define
   * one of the building blocks of the Filters
   */
  template<typename Type>
  class SingleElementSet {
    /// Member variable for the single element of the set
    Type m_element;
  public:
    /// Constructor
    explicit SingleElementSet(Type element):  m_element(element) {};

    /** Method used by the filter tools to decide on the fate of the pair.
     *
     * @param x is the result of some SelectionVariable applied to a pair of objects.
     * The return value is true if x belongs to the open set ( -infinity, m_sup )
     */
    template< class VariableType >
    inline bool contains(const VariableType& x) const { return x == m_element;};

    /** Creates and sets the addresses of the leaves to store the min and max values.
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
      leafList += TBranchLeafType(m_element);

      TBranch* branch = new TBranch(t, branchName.c_str() , & m_element, leafList.c_str());
      t->GetListOfBranches()->Add(branch);
    }

    /** Accessor to the sup of the set */
    Type getElement(void) const { return m_element; } ;

    /** generates a "name" and fills the vector with the variable references
    @param filtername: optional name of the filter this range is attached to make the output look nicer
    @param references: pointer to vector which contains a pair of char which indicates the type object pointed to
      and the actual pointers to the bounds, if equal to nullptr it will not be filled
    **/
    std::string getNameAndReference(std::vector<std::pair<char, void*>>* pointers = nullptr, const std::string& varname = "x")
    {
      std::string val = std::to_string(m_element);
      // if pointer to vector is provided fill it
      if (pointers != nullptr) {
        // use the position in the vector as unique identifier
        val = "#" + std::to_string(pointers->size());
        (*pointers).push_back({TBranchLeafType(m_element), &m_element});
      }
      return ("(" + val + " == " + varname + ")");
    }
  };
}
