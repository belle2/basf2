/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// Basf2 headers
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <analysis/dataobjects/ParticleList.h>

// frugally-deep header
#define FDEEP_FLOAT_TYPE double
#include <fdeep/fdeep.hpp>

namespace Belle2 {
  /**
   * Apply nbarMVA for Belle I
   */
  class BelleNbarMVAModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    BelleNbarMVAModule();

    /** Load the weight file */
    void initialize() override;

    /** Perform the calculation here */
    void event() override;

  private:

    std::string m_identifier;  /**< Identifier of the MVA */
    std::string m_particleList;  /**< Name of the ParticleList to apply the MVA */
    StoreObjPtr<ParticleList> m_plist;  /**< ParticeList to apply the MVA */
    std::unique_ptr<fdeep::model> m_model;  /**< Model of the MVA */
  };
}
