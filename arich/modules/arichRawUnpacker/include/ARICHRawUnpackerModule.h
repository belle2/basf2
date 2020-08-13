/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHRAWUNPACKERMODULE_H
#define ARICHRAWUNPACKERMODULE_H

#include <framework/core/HistoModule.h>
#include <TH1F.h>

namespace Belle2 {

  /**
   * Fill ARICHHit collection from ARICHDigits
   */
  class ARICHRawUnpackerModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    ARICHRawUnpackerModule();

    /**
     * Destructor
     */
    virtual ~ARICHRawUnpackerModule();

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
    bool m_debug;
    unsigned int m_ibyte = 0;
    TH1* h_rate_a_all = NULL;//yone
    TH1* h_rate_b_all  = NULL;//yone
    TH1* h_rate_c_all  = NULL;//yone
    TH1* h_rate_d_all  = NULL;//yone

  };

  inline unsigned int ARICHRawUnpackerModule::calbyte(const int* buf)
  {
    int shift = (3 - m_ibyte % 4) * 8;
    unsigned int val = 0xff & (buf[m_ibyte / 4] >> shift);
    m_ibyte++;
    return val;
  }

  inline unsigned int ARICHRawUnpackerModule::calword(const int* buf)
  {
    return (calbyte(buf) << 24) | (calbyte(buf) << 16)
           | (calbyte(buf) << 8) | calbyte(buf);
  }


} // Belle2 namespace

#endif
