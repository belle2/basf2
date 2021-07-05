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

#include <string>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Const expression that can compare two string at compile time
     *
     *  @param s1    One string
     *  @param s2    Other string
     *  @return      True for equal strings.
     */
    constexpr
    bool strequal(char const* s1, char const* s2)
    {
      return (not * s1 and not * s2) or (*s1 and * s2 and * s1 == *s2 and strequal(s1 + 1, s2 + 1));
    }

    /**
     *  Const expression that can search through an array of string at compile time and find the
     *  index of the string.
     *
     *  @param names   An array of cstrings
     *  @param name    The sought string
     *  @param iName   Optional index at which the search should begin.
     *  @return        The index at which the string was found.
     *                 nVars if not found, which points to one after the array.
     */
    template <size_t nVars>
    constexpr int index(const char* const(&names)[nVars], const char* name, size_t iName = 0)
    {
      return ((nVars == iName) ?
              iName :
              strequal(names[iName], name) ? iName : index(names, name, iName + 1));
    }

    /**
     *  Const expression that can search through an array of string at compile time and find the
     *  index of the string.
     *
     *  @param getName An array of cstrings
     *  @param name    The sought string
     *  @param iName   Optional index at which the search should begin.
     *  @return        The index at which the string was found.
     *                 nVars if not found, which points to one after the array.
     */
    template<size_t nVars>
    constexpr
    int index(const char* (getName(int)),
              const char* const name,
              const size_t iName = 0)
    {
      return ((nVars == iName) ?
              iName :
              strequal(getName(iName), name) ? iName : index<nVars>(getName, name, iName + 1));
    }

    // *INDENT-OFF*
    /**
     *  Const expression that gives the length of an array.
     *  @tparam T        Type contained in the  array
     *  @tparam N        Length of the array
     *  @param array    Array from which the length should be taken.
     *  @return          The length of the array
     */
    template <class T, size_t N>
    constexpr size_t size(T (&array)[N] __attribute__((unused)))
    {
      return N;
    }
    // *INDENT-ON*

    /**
     *  Generic class that contains a fixed number of named float values.
     *  This object template provides the memory and the names of the float values.
     *
     *  @tparam ANames Class with two contained parameters:
     *  @args@c nVars Number of parts that will be peeled from the complex object.
     *  @args@c names  Array of names which contain the nVars names of the float values.
     */
    template<class ANames>
    class FixedSizeNamedFloatTuple : public NamedFloatTuple {

    private:
      /// Number of floating point values represented by this class.
      static const size_t nVars = ANames::nVars;

    protected:
      /**
       *  Static getter for the index from the name.
       *  Looks through the associated names and returns the right index if found
       *  Returns nVars (one after the last element) if not found.
       *
       *  Short hand named spells nice in implementation code.
       *
       *  @param name   The sough name.
       *  @return       Index of the name, nVars if not found.
       */
      constexpr static int named(const char* name)
      {
        return index<nVars>(ANames::getName, name);
      }

    public:
      /// Getter for number of floating point values represented by this class.
      size_t size() const final
      {
        return ANames::nVars;
      }

      /**
       *  Getter for the index from a name.
       *  Looks through the associated names and returns the right index if found.
       *  Returns size() (one after the last element) if not found.
       *
       *  @param name       Name of the sought part
       *  @return           Index of the name, nParts if not found.
       */
      int getNameIndex(const char* name) const final
      {
        return named(name);
      }

      /// Getter for the ith name.
      std::string getName(int iValue) const final
      {
        assert(iValue < (int)nVars);
        assert(iValue >= 0);
        return ANames::getName(iValue);
      }

    public:
      /// Setter for the ith values. Static index version.
      template<int I>
      void set(Float_t value)
      {
        static_assert(nVars != I, "Requested name not found in names.");
        assert(I < (int)nVars);
        assert(I >= 0);
        m_values[I] = value;
      }

      /// Setter for the ith value.
      void set(int iValue, Float_t value) final {
        assert(iValue < (int)nVars);
        assert(iValue >= 0);
        m_values[iValue] = value;
      }

      /// Setter for the value with the given name.
      void set(const char* name, Float_t value)
      {
        set(named(name), value);
      }

      /// Getter for the ith value. Static index version.
      template<int I>
      Float_t get() const
      {
        static_assert(nVars != I, "Requested name not found in names.");
        assert(I < (int)nVars);
        assert(I >= 0);
        return m_values[I];
      }

      /// Getter for the ith value.
      Float_t get(int iValue) const final
      {
        assert(iValue < (int)nVars);
        assert(iValue >= 0);
        return m_values[iValue];
      }

      /// Getter for the value with the given name.
      Float_t get(const char* name) const
      {
        return get(named(name));
      }

      /// Reference getter for the ith value. Static index version.
      template<int I>
      Float_t& var()
      {
        static_assert(nVars != I, "Requested name not found in names.");
        assert(I < (int)nVars);
        assert(I >= 0);
        return m_values[I];
      }

      /// Reference getter for the ith value.
      Float_t& operator[](int iValue) final {
        assert(iValue < (int)nVars);
        assert(iValue >= 0);
        return m_values[iValue];
      }

      /// Reference getter for the value with the given name.
      Float_t& operator[](const char* name)
      {
        return this->var(named(name));
      }

    public:
      /// Memory for nVars floating point values.
      Float_t m_values[nVars] = {};

    };
  }
}
