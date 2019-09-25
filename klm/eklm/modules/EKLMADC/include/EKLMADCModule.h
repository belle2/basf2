/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <TFile.h>

/* Belle2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>

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

    /** Direct histogram. */
    float* m_hDir;

    /** Reflected histogram. */
    float* m_hRef;

  };

}
