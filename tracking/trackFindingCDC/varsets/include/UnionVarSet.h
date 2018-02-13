/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <tracking/trackFindingCDC/utilities/Named.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Class that accomodates many variable sets and presents them as on set of variables
    template <class AObject>
    class UnionVarSet : public BaseVarSet<AObject> {

    private:
      /// Type of the super class
      using Super = BaseVarSet<AObject>;

    public:
      /// Object type from which variables shall be extracted.
      using Object = AObject;

      /// Type of the contained variable sets
      using ContainedVarSet = BaseVarSet<Object>;

    public:
      /// Initialize all contained variable set before event processing.
      void initialize() final {
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets)
        {
          this->addProcessingSignalListener(varSet.get());
        }
        Super::initialize();
      }

      /// Allowing the other variant of the extract method
      using Super::extract;

      /**
       *  Main method that extracts the variable values from the complex object.
       *
       *  @returns  Indication whether the extraction could be completed successfully.
       */
      bool extract(const Object* obj) final {
        bool result = true;
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets)
        {
          result &= varSet->extract(obj);
        }
        return result;
      }

      /**
       *  Getter for the named references to the individual variables
       *  Base implementaton returns empty vector
       */
      std::vector<Named<Float_t*>> getNamedVariables(const std::string& prefix) override
      {
        std::vector<Named<Float_t*> > result;
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets) {
          std::vector<Named<Float_t*> > extend = varSet->getNamedVariables(prefix);
          result.insert(result.end(), extend.begin(), extend.end());
        }
        return result;
      }

      /**
       *   Pointer to the variable with the given name.
       *   Returns nullptr if not found.
       */
      MayBePtr<Float_t> find(const std::string& varName) override
      {
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets) {
          MayBePtr<Float_t> found = varSet->find(varName);
          if (found) return found;
        }
        return nullptr;
      }

      /// Add a variable set to the contained variable sets.
      void push_back(std::unique_ptr<ContainedVarSet> varSet)
      {
        if (varSet) {
          m_varSets.push_back(std::move(varSet));
        }
      }

      /// Remove all contained variable sets.
      void clear()
      {
        m_varSets.clear();
      }

      /// Return the number of currently contained variable sets
      size_t size() const
      {
        return m_varSets.size();
      }

    private:
      /// Collection of contained variables sets.
      std::vector<std::unique_ptr<ContainedVarSet>> m_varSets;
    };
  }
}
