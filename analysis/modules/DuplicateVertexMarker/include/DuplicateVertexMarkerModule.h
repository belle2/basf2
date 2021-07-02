/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/datastore/StoreObjPtr.h>
#include <string>

namespace Belle2 {

  /**
   * Identify duplicate vertices (distinct particles, but built from the same
   * daughters) and mark the one with best chi2. Only works if the particle has
   * exactly two daughters. Mainly used to deal when merging V0 vertices with
   * hand-built ones.
   */
  class DuplicateVertexMarkerModule : public Module {

  public:

    /**
     * Constructor
     */
    DuplicateVertexMarkerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    StoreObjPtr<ParticleList> m_inPList; /**< input particle list */
    std::string m_particleList;              /**< input ParticleList name */
    std::string m_extraInfoName;             /**< output extra-info name */
    bool m_prioritiseV0;                    /**< if one of the decay is a V0, prioritise that before checking vertex quality */

    const Variable::Manager::Var* m_targetVar; /**< Pointer to target variable stored in the variable manager */

  };

} // Belle2 namespace
