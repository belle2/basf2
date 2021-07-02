/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * Generate VXD misalignment and store in database
   *
   * Generate VXD misalignment and store in database
   *
   */
  class AlignmentGeneratorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    AlignmentGeneratorModule();

    /** Generate misalignment and store in database */
    virtual void initialize() override;


  private:

    std::vector<int> m_payloadIov;  /**< IoV for the payload */
    bool m_createPayload; /**< Whether to create and store the payload in database */
    std::vector<std::string> m_data;  /**< Data for misalignment generation */
    std::string m_payloadName;  /**< Name of generated misalignment in database */
  };
}
