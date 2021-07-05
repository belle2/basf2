/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Interface defining a mapping of objects to attribute values e.g. a color.
    template <class T>
    class Mapping {

    public:
      /// Destructor of interfaces should be virtual
      virtual ~Mapping() = default;

      /// Main function returning an attribute value for an object at the given index.
      virtual std::string map(int index __attribute__((unused)),
                              T& t __attribute__((unused)))
      {
        return "";
      }

      /// Informal string summarizing the translation from the object to the attribute value.
      virtual std::string info()
      {
        return "(no info available)\n";
      }
    };

    /// Realizing a mapping returning a constant attribute value regardless of object or position
    template <class T>
    class ConstantMapping : public Mapping<T> {

    public:
      /// Constructor receiving the constant value
      explicit ConstantMapping(const std::string& value)
        : m_value(value)
      {
      }

      /// Return the constant value
      std::string map(int index __attribute__((unused)), T& t __attribute__((unused))) override
      {
        return m_value;
      }

      /// Informal string summarizing the translation from the object to the attribute value.
      std::string info() override
      {
        return "always " + m_value + "\n";
      }

    private:
      /// Memory for the returned attribute value
      std::string m_value;
    };

    /// Realizing a mapping from a pool of values to be cycled from the index
    template <class T>
    class CycleMapping : public Mapping<T> {

    public:
      /// Constructor receiving the values to be cycled.
      explicit CycleMapping(const std::vector<std::string>& values)
        : m_values(values)
      {
      }

      /// Return the a value from the cycle value according to the given inded
      std::string map(int index, T& t __attribute__((unused))) override
      {
        assert(index >= 0);
        return m_values[index % m_values.size()];
      }

      /// Informal string summarizing the translation from the object to the attribute value.
      std::string info() override
      {
        return "Cycle through " + bracketed(join(", ", m_values)) + "\n";
      }

    private:
      /// Memory for the attribute values to be cycled
      std::vector<std::string> m_values;
    };
  }
}
