/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/HistoModule.h>
#include <string>

#include <TH2.h>

namespace Belle2 {

  /**
   * Fill ARICHHit collection from ARICHDigits.
   */
  class ARICHRateCalModule : public HistoModule {

  public:

    /**
     * Constructor.
     */
    ARICHRateCalModule();

    /**
     * Destructor.
     */
    virtual ~ARICHRateCalModule();

    /**
     * Definition of the histograms.
     */
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

  protected:
    /**
     * Get calculated byte.
     */
    unsigned int calbyte(const int* buf);

    /**
     * Get calculated word.
     */
    unsigned int calword(const int* buf);

  protected:
    TH2* h_rate2D[100] = {NULL}; /**< 2D histogram */
    int m_nrun; /**< number of scan runs */
    int m_nevents; /**< number of events per run */
    double m_dth; /**< dth */
    double m_th0; /**< th0 */
    bool m_debugmode; /**< whether debug mode is requested */
    bool m_internalmode; /**< whether internal thscan mode is requested */
    std::string m_daqdb; /**< daqdb config name */
    unsigned int m_ibyte = 0; /**< byte */
    int m_run_count = 0; /**< run counter */
    int m_evt_count = 0; /**< event counter */

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
