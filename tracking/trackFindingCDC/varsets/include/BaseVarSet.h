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

#include <tracking/trackFindingCDC/utilities/Named.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <map>
#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Generic class that generates some named float values from a given object.
     *
     *  Base class defining the interface for various different implementation of sets of variables.
     **/
    template<class AObject>
    class BaseVarSet {

    public:
      /// Object type from which variables shall be extracted.
      using Object = AObject;

    public:
      /// Making destructor virtual
      virtual ~BaseVarSet() = default;

      /**
       *  Initialize the variable set before event processing.
       *  Can be specialised if the derived variable set has setup work to do.
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
       *  Terminate the variable set after event processing.
       *  Can be specialised if the derived variable set has to tear down aquired resources.
       */
      virtual void terminate()
      {}

      /**
       *  Main method that extracts the variable values from the complex object.
       *  @returns  Indication whether the extraction could be completed successfully.
       *            Base implementation returns always true.
       */
      virtual bool extract(const Object* /*obj*/)
      { return true; }

      /// Method for extraction from an object instead of a pointer.
      bool extract(const Object& obj)
      { return extract(&obj); }

      /**
       *  Getter for the named references to the individual variables
       *  Base implementaton returns empty vector
       */
      virtual std::vector<Named<Float_t*> > getNamedVariables(std::string /*prefix*/ = "")
      { return {}; }

      /**
       *  Getter for a map of names to float values
       */
      virtual std::map<std::string, Float_t> getNamedValues(std::string prefix = "")
      {
        std::map<std::string, Float_t> result;
        std::vector<Named<Float_t*> > namedVariables = getNamedVariables(prefix);
        for (const Named<Float_t* >& namedVariable : namedVariables) {
          Float_t* variable = namedVariable;
          result[namedVariable.getName()] = *variable;
        }
        return result;
      }

      /**
       *   Pointer to the variable with the given name.
       *   Returns nullptr if not found.
       */
      virtual MayBePtr<Float_t> find(std::string varName)
      {
        std::vector<Named<Float_t*> > namedVariables = getNamedVariables();
        for (const Named<Float_t* >& namedVariable : namedVariables) {
          if (namedVariable.getName() == varName) {
            Float_t* variable = namedVariable;
            return variable;
          }
        }
        return nullptr;
      }

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
