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


#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>

#include <map>
#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
    Generic class that generates some named float values from a given object.

    Base class defining the interface for various different implementation of sets of variables.
     **/
    template<class AObject>
    class BaseVarSet {

    public:
      /// Object type from which variables shall be extracted.
      typedef AObject Object;

    public:
      /// Making destructor virtual
      virtual ~BaseVarSet() {}

      /**
      Main method that extracts the variable values from the complex object.
      @returns  Indication whether the extraction could be completed successfully.
             Base implementation returns always true.
      */
      virtual bool extract(const Object* /*obj*/)
      { return true; }

      /// Method for extraction from an object instead of a pointer.
      bool extract(const Object& obj)
      { return extract(&obj); }

      /**
      Initialize the variable set before event processing.
         Can be specialised if the derived variable set has setup work to do.
       */
      virtual void initialize()
      {}

      /// Allow setup work to take place at beginning of new run
      virtual void beginRun()
      {}

      /// Allow setup work to take place at beginning of new event
      virtual void beginEvent()
      {}

      /// Allow clean up to take place at end of run
      virtual void endRun()
      {}

      /**
      Terminate the variable set after event processing.
      Can be specialised if the derived variable set has to tear down aquired resources.
       */
      virtual void terminate()
      {}

      /**
      Getter for the named tuples storing the values of all the (possibly nested) VarSets
      Base implementation returns empty vector.
       */
      virtual std::vector<Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables()
      { return std::vector<NamedFloatTuple*>(); }

      /**
      Const getter for the named tuples storing the values of all the (possibly nested)
      variable sets. Base implementation returns an empty vector.
       */
      virtual std::vector<const Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables() const
      { return std::vector<const NamedFloatTuple*>(); }

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

    }; //end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
