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

#include <RtypesCore.h>

#include <tracking/trackFindingCDC/utilities/Named.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <vector>
#include <map>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// An abstract tuple of float value where each value has an associated name.
    class NamedFloatTuple {

    public:
      /// Marking the destructor virtual since we are using virtual functions.
      virtual ~NamedFloatTuple();

      /// Getter for the number of parts
      virtual size_t size() const = 0;

      /**
       *  Getter for the index from a name.
       *  Looks through the associated names and returns the right index if found.
       *  Returns size() (one after the last element) if not found.
       *
       *  @param name       Name of the sought part
       *  @return           Index of the name, nParts if not found.
       */
      virtual int getNameIndex(const char* name) const = 0;

      /// Getter for the ith name.
      virtual std::string getName(int iValue) const = 0;

      /// Setter for the value of the ith part.
      virtual void set(int iValue, Float_t value) = 0;

      /// Setter for the value with the given name.
      void set(const char* const name, Float_t value)
      {
        set(getNameIndex(name), value);
      }

      /// Getter for the value of the ith part.
      virtual Float_t get(int iValue) const = 0;

      /// Getter for the value with the given name.
      Float_t get(const char* const name) const
      {
        return get(getNameIndex(name));
      }

      /**
       *  Getter for a pointer to the value with the given name.
       *  Return nullptr if not found.
       */
      virtual MayBePtr<Float_t> find(std::string name)
      {
        size_t index = getNameIndex(name.c_str());
        return (index < size()) ? &(operator[](index)) : nullptr;
      }

      /// Reference getter for the value of the ith part.
      virtual Float_t& operator[](int iValue) = 0;

      /// Reference getter for the value with the given name.
      Float_t& operator[](const char* const name)
      {
        return operator[](getNameIndex(name));
      }

      /// Getter for a map of all name and value pairs in this tuple
      std::map<std::string, Float_t> getNamedValues(std::string prefix = "") const;

      /// Getter for named references to the variables in this tuple
      std::vector<Named<Float_t*>> getNamedVariables(std::string prefix = "");
    };
  }
}
