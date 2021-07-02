/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_DATABASE_WEIGHTFILE_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_DATABASE_WEIGHTFILE_HEADER

#include <TObject.h>

#include <string>

namespace Belle2 {

  /**
   * Database representation of a Weightfile object.
   * This class inherits from TObject and contains only one simple string,
   * hence it can easily be saved in the database
   */
  class DatabaseRepresentationOfWeightfile : public TObject {
  public:
    /**
     * Default constructor, necessary for ROOT to stream the object
     */
    DatabaseRepresentationOfWeightfile() { }

    /**
     * Constructor from weight file name.
     */
    explicit DatabaseRepresentationOfWeightfile(const std::string& data) : m_data(data) { }

    std::string m_data; /**< Serialized weightfile */

    ClassDef(DatabaseRepresentationOfWeightfile, 2); /**< Class to store weightfile in datastore */
  };

}

#endif
