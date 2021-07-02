/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHRATECALMODULE_H
#define ARICHRATECALMODULE_H

#include <framework/core/HistoModule.h>
#include <string>

#include <TH2.h>

namespace Belle2 {

  /**
   * Fill ARICHHit collection from ARICHDigits
   */
  class ARICHRateCalModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    ARICHRateCalModule();

    /**
     * Destructor
     */
    virtual ~ARICHRateCalModule();

    virtual void defineHisto() override;

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  protected:
    unsigned int calbyte(const int* buf);
    unsigned int calword(const int* buf);

  protected:
    TH2* h_rate2D[100] = {NULL};
    int m_nrun;
    int m_nevents;
    double m_dth;
    double m_th0;
    bool m_debugmode;
    bool m_internalmode;
    std::string m_daqdb;
    unsigned int m_ibyte = 0;
    int m_run_count = 0;
    int m_evt_count = 0;

  };

  inline unsigned int ARICHRateCalModule::calbyte(const int* buf)
  {
    int shift = (3 - m_ibyte % 4) * 8;
    unsigned int val = 0xff & (buf[m_ibyte / 4] >> shift);
    m_ibyte++;
    return val;
  }

  inline unsigned int ARICHRateCalModule::calword(const int* buf)
  {
    return (calbyte(buf) << 24) | (calbyte(buf) << 16)
           | (calbyte(buf) << 8) | calbyte(buf);
  }

} // Belle2 namespace

#endif
