/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>

/* ROOT headers. */
#include <TFile.h>

namespace Belle2 {

  /**
   * Module EKLMADCModule.
   * @details
   * Module for standalone generation and studies of ADC output.
   */
  class EKLMADCModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMADCModule();

    /**
     * Destructor.
     */
    ~EKLMADCModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

  private:

    /**
     * Generate output histogram.
     * @param[in] name Name.
     * @param[in] l    Length of strip.
     * @param[in] d    Distance from SiPM.
     * @param[in] npe  Number of photoelctrons.
     */
    void generateHistogram(const char* name, double l, double d, int npe);

    /** Operation mode. */
    std::string m_mode;

    /** Name of output file. */
    std::string m_out;

    /** Output file. */
    TFile* m_fout;

    /** Scintillator simulation parameters. */
    DBObjPtr<KLMScintillatorDigitizationParameters> m_SciSimParDatabase;

    /** Scintillator simulation parameters. */
    KLMScintillatorDigitizationParameters* m_SciSimPar;

    /** Hardware clock settings. */
    DBObjPtr<HardwareClockSettings> m_HardwareClockSettings;

    /** Direct histogram. */
    float* m_hDir;

    /** Reflected histogram. */
    float* m_hRef;

  };

}
