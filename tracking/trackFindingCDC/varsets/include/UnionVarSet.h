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


#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
       Class that accomodates many variable sets and presents them as on set of variables
    */
    template<class AObject>
    class UnionVarSet : public BaseVarSet<AObject> {

    private:
      /// Type of the super class
      typedef BaseVarSet<AObject> Super;

    public:
      /// Object type from which variables shall be extracted.
      typedef AObject Object;

      /// Type of the contained variable sets
      typedef BaseVarSet<Object> ContainedVarSet;

    public:
      using Super::extract;

      /**
      Main method that extracts the variable values from the complex object.

      @returns  Indication whether the extraction could be completed successfully.
      */
      virtual bool extract(const Object* obj) override final
      {
        bool result = true;
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets) {
          result &= varSet->extract(obj);
        }
        return result;
      }

      /**
      Initialize all contained variable set before event processing.
       */
      virtual void initialize() override final
      {
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets) {
          varSet->initialize();
        }
      }

      /// Signal the beginning of a new run
      virtual void beginRun() override
      {
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets) {
          varSet->beginRun();
        }
      }

      /// Signal the beginning of a new event
      virtual void beginEvent() override
      {
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets) {
          varSet->beginEvent();
        }
      }

      /// Signal the end of a run
      virtual void endRun() override
      {
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets) {
          varSet->endRun();
        }
      }

      /**
      Terminate all contained variable set after event processing.
      */
      virtual void terminate() override final
      {
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets) {
          varSet->terminate();
        }
      }

      /**
      Getter for the named tuples storing the values of all the (possibly nested) VarSets
      Base implementation returns empty vector.
       */
      virtual
      std::vector<Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables() override final
      {
        std::vector<NamedFloatTuple*> allVariables;
        for (std::unique_ptr<ContainedVarSet>& varSet : m_varSets) {
          std::vector<NamedFloatTuple*> variablesOfVarSet = varSet->getAllVariables();
          allVariables.insert(allVariables.end(),
                              variablesOfVarSet.begin(),
                              variablesOfVarSet.end());
        }
        return allVariables;
      }

      /**
         Const getter for the named tuples storing the values of all the (possibly nested)
         variable sets. Base implementation returns an empty vector.
       */
      virtual
      std::vector<const Belle2::TrackFindingCDC::NamedFloatTuple*>
      getAllVariables() const override final
      {
        std::vector<const NamedFloatTuple*> allVariables;
        for (const std::unique_ptr<ContainedVarSet>& varSet : m_varSets) {
          const ContainedVarSet* constVarSet = varSet.get();
          std::vector<const NamedFloatTuple*> variablesOfVarSet = constVarSet->getAllVariables();
          allVariables.insert(allVariables.end(),
                              variablesOfVarSet.begin(),
                              variablesOfVarSet.end());
        }
        return allVariables;
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
      size_t size()
      {
        return m_varSets.size();
      }

    private:
      /// Collection of contained variables sets.
      std::vector<std::unique_ptr<ContainedVarSet>> m_varSets;

    }; //end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
