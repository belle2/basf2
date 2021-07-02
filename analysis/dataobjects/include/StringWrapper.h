/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/datastore/RelationsObject.h>
#include <string>

namespace Belle2 {

  /**
   * This class is a wrapper for strings, such as MCDecayStrings, to allow them to be associated
   * with particles via a relation.
   */
  class StringWrapper : public RelationsObject {

    // Note: Currently all functions are simple enough to be defined in this file,
    // so there is no separate .cc for implementations.

  public:

    /**
     * Default constructor.
     * All private members are set to 0 or empty strings.
     */
    StringWrapper() : m_string("") {};

    /**
     * Set string.
     *
     * @param inputstring the new string.
     */
    void setString(const std::string& inputstring)
    {
      m_string = inputstring;
    }

    /**
     * Get string.
     *
     * @return the string.
     */
    std::string getString() const
    {
      return m_string;
    }

  private:
    std::string m_string; /**< the contained string */
    ClassDef(StringWrapper, 1) /**< class definition */
  };
} //end namespace Belle2
