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

#include <tracking/trackFindingCDC/varsets/FixedSizeNamedFloatTuple.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /** Generic class that generates the same variables from a each of a pair of instances.
     **/
    template<class BaseVarSet>
    class PairVarSet {

    public:
      /// Object type from which the variables shall be extracted
      typedef typename BaseVarSet::Object BaseObject;

      /// Object type from which variables shall be extracted.
      typedef typename std::pair<const BaseObject*, const BaseObject*> Object;

    public:
      /// Constructure taking a optional prefix that can be attached to the names if request.
      explicit PairVarSet(const std::string& prefix = "") :
        m_firstVarSet(prefix + "first_"),
        m_secondVarSet(prefix + "second_")
      {;}

      /// Virtual destructor
      virtual ~PairVarSet() {;}

      /// Main method that extracts the variable values from the complex object.
      virtual bool extract(const std::pair<const BaseObject*, const BaseObject*>* obj)
      {
        assert(obj);
        bool first_extracted = m_firstVarSet.extract(obj->first);
        bool second_extracted = m_secondVarSet.extract(obj->second);
        return first_extracted and second_extracted;
      }

      /// Method for extraction from an object instead of a pointer.
      bool extract(const std::pair<const BaseObject*, const BaseObject*>& obj)
      {
        return extract(&obj);
      }


      /** Initialize the variable set before event processing.
       *  Can be specialised if the derived variable set has setup work to do.
       */
      virtual void initialize()
      {
        m_firstVarSet.initialize();
        m_secondVarSet.initialize();
      }

      /** Terminate the variable set after event processing.
       *  Can be specialised if the derived variable set has to tear down aquired resources.
       */
      virtual void terminate()
      {
        m_secondVarSet.terminate();
        m_firstVarSet.terminate();
      }

      /** Getter for the named tuples storing the values of all the (possibly nested) VarSets
       *  Base implementation returns the  named tuple of this variable set.
       */
      virtual std::vector<Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables()
      {
        std::vector<NamedFloatTuple*> firstResult = m_firstVarSet.getAllVariables();
        std::vector<NamedFloatTuple*> secondResult = m_secondVarSet.getAllVariables();
        firstResult.insert(firstResult.end(), secondResult.begin(), secondResult.end());
        return firstResult;
      }

      /** Const getter for the named tuples storing the values of all the (possibly nested)
       *  variable sets. Base implementation returns the named tuple of this variable set.
       */
      virtual std::vector<const Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables() const
      {
        std::vector<const NamedFloatTuple*> firstResult = m_firstVarSet.getAllVariables();
        std::vector<const NamedFloatTuple*> secondResult = m_secondVarSet.getAllVariables();
        firstResult.insert(firstResult.end(), secondResult.begin(), secondResult.end());
        return firstResult;
      }

      /// Getter for a map of all name including the optional prefix and value pairs
      std::map<std::string, Float_t> getNamedValuesWithPrefix() const
      {
        std::map<std::string, Float_t> namedValues;
        std::vector<const NamedFloatTuple*> allVariables = getAllVariables();
        for (const NamedFloatTuple* variables : allVariables) {
          size_t nVariables = variables->size();
          for (size_t iVariable = 0; iVariable < nVariables; ++iVariable) {
            std::string name = variables->getNameWithPrefix(iVariable);
            Float_t value = variables->get(iVariable);
            namedValues[name] = value;
          }
        }
        return namedValues;
      }

    private:
      /// VarSet for the first element of the set.
      BaseVarSet m_firstVarSet;

      /// VarSet for the second element of the set.
      BaseVarSet m_secondVarSet;

    }; //end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
