/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>

#include <string>

namespace Belle2 {

  /**
   * Module for adding to MC samples the information about the TRG active veto from beam background overlay files.
   */
  class TRGGRLInjectionVetoFromOverlayModule : public Module {

  public:

  public:

    /** Constructor. */
    TRGGRLInjectionVetoFromOverlayModule();

    /** Initialize. */
    void initialize() override;

    /** Begin run. */
    void beginRun() override;

    /** Event. */
    void event() override;

  private:

    /** TRGGDLInputBits database object */
    DBObjPtr<TRGGDLDBInputBits> m_TRGInputBits;

    /** TRGGRLInfo object from MC simulation */
    StoreObjPtr<TRGGRLInfo> m_TRGGRLInfoFromSimulation;

    /** TRGSummary object from beam background overlay */
    StoreObjPtr<TRGSummary> m_TRGSummaryFromOverlay;

    /** Extension name for branches from beam background overlay */
    std::string m_extensionName;

  };

}
