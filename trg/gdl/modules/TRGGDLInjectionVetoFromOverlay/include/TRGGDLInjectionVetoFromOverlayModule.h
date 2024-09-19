/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>

#include <string>

namespace Belle2 {

  /**
   * Module for adding to MC samples the information about the TRG active veto from beam background overlay files.
   */
  class TRGGDLInjectionVetoFromOverlayModule : public Module {

  public:

  public:

    /** Constructor. */
    TRGGDLInjectionVetoFromOverlayModule();

    /** Initialize the module. */
    void initialize() override;

    /** Event processor. */
    void event() override;

  private:

    /** TRGSummary object from MC simulation */
    StoreObjPtr<TRGSummary> m_TRGSummaryFromSimulation;

    /** TRGSummary object from beam background overlay */
    StoreObjPtr<TRGSummary> m_TRGSummaryFromOverlay;

    /** Extension name for branches from beam background overlay */
    std::string m_extensionName;

  };

}
