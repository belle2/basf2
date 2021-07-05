/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/CompositeProcessingSignalListener.h>

#include <tracking/trackFindingCDC/utilities/Named.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <RtypesCore.h>

#include <map>
#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Generic class that generates some named float values from a given object.
     *
     *  Base class defining the interface for various different implementation of sets of variables.
     */
    template<class AObject>
    class BaseVarSet : public CompositeProcessingSignalListener {

    public:
      /// Object type from which variables shall be extracted.
      using Object = AObject;

    public:
      /// Making destructor virtual
      virtual ~BaseVarSet() = default;

      /**
       *  Main method that extracts the variable values from the complex object.
       *  Returns indication whether the extraction could be completed successfully.
       *  Base implementation returns always true.
       *  @param obj: dummy object not used
       *  @return: always true for the base implementation
       */
      virtual bool extract(const Object* obj __attribute__((unused)))
      {
        return true;
      }

      /// Method for extraction from an object instead of a pointer.
      bool extract(const Object& obj)
      {
        return extract(&obj);
      }

      /**
       *  Getter for the named references to the individual variables
       *  Base implementaton returns empty vector
       *  @param prefix: Name prefix to apply to all variable names.
       *  @return: in base implementation always an empty vector
       */
      virtual std::vector<Named<Float_t*> > getNamedVariables(const std::string& prefix __attribute__((unused)))
      {
        return {};
      }

      /// Getter for the named references to the individual variables
      std::vector<Named<Float_t*> > getNamedVariables()
      {
        const std::string prefix = "";
        return this->getNamedVariables(prefix);
      }

      /**
       *  Getter for a map of names to float values
       *  @param prefix Name prefix to apply to all variable names.
       */
      std::map<std::string, Float_t> getNamedValues(const std::string& prefix) const
      {
        std::map<std::string, Float_t> result;
        std::vector<Named<Float_t*> > namedVariables = this->getNamedVariables(prefix);
        for (const Named<Float_t*>& namedVariable : namedVariables) {
          Float_t* variable = namedVariable;
          result[namedVariable.getName()] = *variable;
        }
        return result;
      }

      /// Getter for a map of names to float values
      std::map<std::string, Float_t> getNamedValues() const
      {
        const std::string prefix = "";
        return this->getNamedValues(prefix);
      }

      /**
       *   Pointer to the variable with the given name.
       *   Returns nullptr if not found.
       */
      virtual MayBePtr<Float_t> find(const std::string& varName)
      {
        std::vector<Named<Float_t*> > namedVariables = this->getNamedVariables();
        for (const Named<Float_t* >& namedVariable : namedVariables) {
          if (namedVariable.getName() == varName) {
            Float_t* variable = namedVariable;
            return variable;
          }
        }
        return nullptr;
      }
    };
  }
}
