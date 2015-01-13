/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef BRANCHOF_H
#define BRANCHOF_H

#include <stddef.h>
#include <typeinfo>
#include <assert.h>

#include "StaticString.h"
#include <TTree.h>
#include <TBranch.h>
#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/tempro/TaggedType.h>

namespace Belle2 {
  namespace CDCLocalTracking {


    /// Template deriving a name from a tag. Default is the typeid.name()
    template<class Tag>
    struct GetName {
      /// Static function to return the derived name.
      static const char* c_str()
      { return typeid(Tag).name(); }
    };

    /// Template specialisation for StaticStrings. The name of a StaticString is made of their characters.
    template<char...  Chars>
    struct GetName< StaticString<Chars...> > {
      /// Static function to return the derived name.
      static const char* c_str()
      { return StaticString<Chars...>::c_str(); }
    };


    /// Statically typed branch object to wrap a TBranch and provides to memory object which used by the TBranch.
    /** BranchOf aims to provide and statically typed variant to ROOT's TBranch. It is tagged for tagged lookup semantics
     *  as implemented by TaggedTuple. The tag also determines the name of the TBranch written to disk. */
    template<class Type_, class Tag_ = Type_>
    class BranchOf : public TaggedType<BranchOf<Type_, Tag_>, Tag_> {
    public:
      /// Type of the values in the branch
      typedef Type_ value_type;

      /// Type of the values in the branch
      typedef BranchOf<Type_, Tag_> Type;

      /// Tag of the branch used for tagged lookup and the name of the branch.
      typedef Tag_ Tag;

    public:
      /// Default constructor
      BranchOf() : m_value(), m_ptrTBranch(nullptr) { assert(not m_value); }

      /// Getter for the current value in the branch
      const value_type& getValue() const
      { return m_value; }

      /// Setter for the current value in the branch
      void setValue(const value_type& value)
      {  m_value = value; }

      /// Getter for the branch name
      static const char* getName()
      { return GetName<Tag>::c_str(); }

      /// Creates a TBranch in the given TTree and locks onto the newly created branch.
      bool create(TTree& tree) {

        if (m_ptrTBranch) {
          B2WARNING("Branch " << getName() << " already created.");
          return false;
        }
        TBranch* ptrTBranch = tree.Branch(getName(), &m_value);

        if (ptrTBranch) {
          m_ptrTBranch = ptrTBranch;
        } else {
          B2WARNING("Creation of TBranch failed.");
        }

        return bool(ptrTBranch);
      }

    private:
      value_type m_value; ///< Memory for the current value in the branch
      TBranch* m_ptrTBranch; ///< Reference to the wrapped TBranch

    };

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // BRANCHOF_H



