/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_DATABASE_WEIGHTFILE_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_DATABASE_WEIGHTFILE_HEADER

#include <TObject.h>
#include <TGenericClassInfo.h>

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
    DatabaseRepresentationOfWeightfile(const std::string& data) : m_data(data) { }

    std::string m_data; /**< Serialized weightfile */

    ClassDef(DatabaseRepresentationOfWeightfile, 2); /**< Class to store weightfile in datastore */
  };

}

#endif
