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

#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <vector>
#include <string>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Generic class that generates the same variables from a each of a pair of instances.
    template <class ABaseVarSet>
    class RelationVarSet : public BaseVarSet<Relation<const typename ABaseVarSet::Object> > {

      /// Type of the base class
      using Super = BaseVarSet<Relation<const typename ABaseVarSet::Object> >;

    public:
      /// Object type from which the variables shall be extracted
      using BaseObject = typename ABaseVarSet::Object;

    public:
      /**
       *  Initialize the variable set before event processing.
       *  Can be specialised if the derived variable set has setup work to do.
       */
      void initialize() override
      {
        this->addProcessingSignalListener(&m_firstVarSet);
        this->addProcessingSignalListener(&m_secondVarSet);
        Super::initialize();
      }

      /// Main method that extracts the variable values from the complex object.
      bool extract(const Relation<const BaseObject>* obj) override
      {
        assert(obj);
        bool firstExtracted = m_firstVarSet.extract(obj->first);
        bool secondExtracted = m_secondVarSet.extract(obj->second);
        return firstExtracted and secondExtracted;
      }

      /// Method for extraction from an object instead of a pointer.
      bool extract(const Relation<const BaseObject>& obj)
      {
        return extract(&obj);
      }

      /**
       *  Getter for the named references to the individual variables
       *  Base implementaton returns empty vector
       */
      std::vector<Named<Float_t*>> getNamedVariables(const std::string& prefix) override
      {
        std::vector<Named<Float_t*> > result = m_firstVarSet.getNamedVariables(prefix + m_firstPrefix);
        std::vector<Named<Float_t*> > extend = m_secondVarSet.getNamedVariables(prefix + m_secondPrefix);
        result.insert(result.end(), extend.begin(), extend.end());
        return result;
      }

      /**
       *   Pointer to the variable with the given name.
       *   Returns nullptr if not found.
       */
      MayBePtr<Float_t> find(const std::string& varName) override
      {
        if (0 == varName.find(m_firstPrefix)) {
          std::string varNameWithoutPrefix = varName.substr(m_firstPrefix.size());
          MayBePtr<Float_t> found = m_firstVarSet.find(varNameWithoutPrefix);
          if (found) return found;
        }

        if (0 == varName.find(m_secondPrefix)) {
          std::string varNameWithoutPrefix = varName.substr(m_secondPrefix.size());
          MayBePtr<Float_t> found = m_secondVarSet.find(varNameWithoutPrefix);
          if (found) return found;
        }

        return nullptr;
      }

    private:
      /// Prefix for all variable in the variable set of the first element of the pair
      std::string m_firstPrefix = "first_";

      /// VarSet for the first element of the set.
      ABaseVarSet m_firstVarSet;

      /// Prefix for all variable in the variable set of the second element of the pair
      std::string m_secondPrefix = "second_";

      /// VarSet for the second element of the set.
      ABaseVarSet m_secondVarSet;

    };
  }
}
