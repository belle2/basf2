/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef STATICTYPEDTREE_H
#define STATICTYPEDTREE_H

#include <stddef.h>
#include <typeinfo>
#include <iostream>

#include "StaticString.h"
#include "TaggedTuple.h"
#include "EvalVariadic.h"

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

#include <framework/logging/Logger.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Statically typed wrapper for ROOT's TTree.
    /** Statically typed Tree variant taking the branches as template arguments.
     *  The branches and current branch values can be looked up by tagged lookup,
     *  because a TaggedTuple is used to store the branches.*/
    template<class ... Branches>
    class StaticTypedTree {

    public:
      /// Tagged tuple type the store the heterogenious branch types
      typedef TaggedTuple<Branches...> BranchTuple ;

      /// Metafunction to lookup the branch type from the given tag
      template<class Tag>
      using GetBranchTypeAtTag = typename BranchTuple::template GetTypeAtTag<Tag>;

      /// Metafunction to lookup the value type of the branch for the given tag
      template<class Tag>
      using GetValueTypeAtTag = typename GetBranchTypeAtTag<Tag>::value_type;

    public:
      /// Constructor setting up the name and title of the Tree but not actually creating the underlying TTree.
      StaticTypedTree(const char* name, const char* title = "") :
        m_name(name), m_title(title), m_ptrTTree(nullptr)
      {;}


      /// Getter for the branch by the given Tag
      template<class Tag>
      GetBranchTypeAtTag<Tag>& getBranch()
      { return branchTuple.template get<Tag>(); }

      /// Constant getter for the branch by the given Tag
      template<class Tag>
      const GetBranchTypeAtTag<Tag>& getBranch() const
      { return branchTuple.template get<Tag>(); }


      /// Constant getter for the current value of the branch indicated by the given Tag
      template<class Tag>
      const GetValueTypeAtTag<Tag>& getValue() const
      { return getBranch<Tag>().getValue(); }

      /// Setter for the current value of the branch indicated by the given Tag
      template<class Tag>
      void setValue(const GetValueTypeAtTag<Tag>& value)
      { return getBranch<Tag>().setValue(value); }


    public:
      /// Getter for the name of the tree
      const std::string& getName() const
      { return m_name; }

      /// Getter for the title of the tree
      const std::string& getTitle() const
      { return m_title; }

      /// Setter for the name of the tree
      void setName(const std::string& name)
      { m_name = name; }

      /// Setter for the title of the tree
      void setTitle(const std::string& title)
      { m_title = title; }

      /// Create the TTree into the given fill from the blueprint coded in the template arguments in this class
      void create(TFile& tFile) {
        TDirectory* ptrSavedCurrentTDirectory = gDirectory;

        tFile.cd();
        m_ptrTTree = new TTree(getName().c_str(), getTitle().c_str());

        createBranches();

        if (ptrSavedCurrentTDirectory) {
          ptrSavedCurrentTDirectory->cd();
        } else {
          gROOT->cd();
        }

      }

      /// Fill the TTree from the current branch values
      void fill()
      { m_ptrTTree->Fill(); }

      /// Save the tree to file
      void save() const {
        if (m_ptrTTree) {
          // Due to some weird we have to cd to the file of the TTree
          // before saving in order to have the tree in the correct file.
          TDirectory* ptrSavedCurrentTDirectory = gDirectory;

          m_ptrTTree->GetCurrentFile()->cd();
          m_ptrTTree->Write();

          if (ptrSavedCurrentTDirectory) {
            ptrSavedCurrentTDirectory->cd();
          } else {
            gROOT->cd();
          }

        }
      }

      /// Get the current number of stored entries.
      size_t size() const
      { return m_ptrTTree ? m_ptrTTree->GetEntries() : 0; }

    private:
      /// Helper function to create the branches of the wrapped TTree from the template parameters.
      void createBranches()
      { EvalVariadic { createBranch< typename BranchTuple::template GetTag<Branches> >()... };  }

    private:
      /// Helper function to create a single branch of the wrapped TTree for the given tag.
      template<class Tag>
      bool createBranch() {
        if (m_ptrTTree) {
          TTree& tTree = *m_ptrTTree;
          return getBranch<Tag>().create(tTree);
        } else {
          B2WARNING("Cannot create branch without a TTree.");
          return false;
        }
      }

    private:
      std::string m_name; ///< Memory for the name of the underlying TTree
      std::string m_title; ///< Memory for the title of the underlying TTree

      TTree* m_ptrTTree; ///< Reference to the wrapped TTree

      BranchTuple branchTuple; ///< Memory for the statically defined branches

    };

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif //  STATICTYPEDTREE_H


