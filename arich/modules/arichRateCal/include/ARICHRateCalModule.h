/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHRATECALMODULE_H
#define ARICHRATECALMODULE_H

#include <framework/core/HistoModule.h>
#include <arich/geometry/ARICHGeometryPar.h>
#include <string>

#include <TH1.h>
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

    virtual void defineHisto();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  protected:
    unsigned int calbyte(const int* buf);
    unsigned int calword(const int* buf);

  protected:
    TH2* h_rate2D[100];
    int m_nrun;
    int m_nevents;
    double m_dth;
    double m_th0;
    bool m_debugmode;
    std::string m_daqdb;
    unsigned int m_ibyte;

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
