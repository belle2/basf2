/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/HistoModule.h>
#include <TH1F.h>

namespace Belle2 {

  /**
   * Fill ARICHHit collection from ARICHDigits.
   */
  class ARICHRawUnpackerModule : public HistoModule {

  public:

    /**
     * Constructor.
     */
    ARICHRawUnpackerModule();

    /**
     * Destructor.
     */
    virtual ~ARICHRawUnpackerModule();

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
     * Event processor.
     */
    virtual void event() override;

  protected:

    /**
     * Read byte with number m_ibyte from the buffer and increase
     * the number by 1.
     * @param[in] buf Buffer.
     */
    unsigned int calbyte(const int* buf);

    /**
     * Read word (4 bytes) from the buffer and increase the byte number
     * m_ibyte by 4.
     * @param[in] buf Buffer.
     */
    unsigned int calword(const int* buf);

  protected:

    /** Debug mode. */
    bool m_debug;

    /** Current byte number. */
    unsigned int m_ibyte = 0;

    /** Rate histogram (unused). */
    TH1* h_rate_a_all = NULL;

    /** Rate histogram (unused). */
    TH1* h_rate_b_all  = NULL;

    /** Rate histogram (unused). */
    TH1* h_rate_c_all  = NULL;

    /** Rate histogram (unused). */
    TH1* h_rate_d_all  = NULL;

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
