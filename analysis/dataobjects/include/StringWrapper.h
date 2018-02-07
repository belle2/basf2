/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matt Barrett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
     * @param the new string.
     */
    void setString(std::string string)
    {
      m_string = string;
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
