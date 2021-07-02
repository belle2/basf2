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
#include <pxd/dataobjects/PXDInjectionBGTiming.h>

namespace Belle2 {

  /**
   * The module produces a StoreObjPtr of PXDInjectionBGTiming containing PXD timing
   * for gated mode operation.
   */
  class PXDInjectionVetoEmulatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDInjectionVetoEmulatorModule();

  private:

    /** Initialize */
    void initialize() override final;

    /**  Event */
    void event() override final;

    /** The name of the StoreObjPtr of PXDInjectionBGTiming to be generated */
    std::string m_PXDIBTimingName;

    /** Fraction of time in the PXD Gated Mode for the PXD readout */
    double m_pxdGatedModeLumiFraction;

    /** Minimal global time for injection veto for PXD */
    double m_minTimePXD;

    /** Maximal global time for injection veto for PXD */
    double m_maxTimePXD;

    /** Revolution time of noise bunches */
    double m_revolutionTime;

    /** Output object for injection Bkg timing for PXD. */
    StoreObjPtr<PXDInjectionBGTiming> m_storePXDIBTiming;

    /** Number of readout gates (or total number of Switcher channels) */
    int m_nGates;
  };
}
