/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Analysis headers. */
#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>

/* Basf2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

/* Python headers. */
#include <Python.h>

/* C++ headers. */
#include <map>
#include <string>

namespace Belle2 {

  /**
   * Post-processing particle weighting.
   */
  class PostProcessingParticleWeighting {

  public:

    /**
     * Constructor.
     * @param[in] lookupTableName Lookup-table name.
     */
    PostProcessingParticleWeighting(const char* lookupTableName);

    /**
     * Destructor.
     */
    ~PostProcessingParticleWeighting();

    /**
     * Set value.
     * @param[in] variableName Variable name.
     * @param[in] value        Value.
     */
    void setValue(std::string variableName, double value);

    /**
     * Get weight information.
     */
    WeightInfo getInfo(int experiment, int run);

    /**
     * Get weight information (Python).
     */
    PyObject* getInfoPython(int experiment, int run);

  protected:

    /** Values. */
    std::map<std::string, double> m_Values;

    /** Event metadata. */
    EventMetaData m_EventMetaData;

    /** Lookup table. */
    DBObjPtr<ParticleWeightingLookUpTable> m_LookupTable;

  };

}
