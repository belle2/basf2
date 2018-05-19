/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <iostream>
#include <TObject.h>
#include <TTimeStamp.h>
#include <string>

namespace Belle2 {

  /**
   * Database object for Run Information
   */
  class RunInfo: public TObject {
  public:

    /**
     *  Default constructor
     */
    RunInfo() {}

    /**
     * Set Experimental Number
     */
    void setExp(unsigned int exp) { m_exp = exp; }

    /**
     * Set Run Number
     */
    void setRun(unsigned int run) { m_run = run; }

    /**
     * Set Run Type
     */
    void setRunType(const std::string& run_type) {m_runType = run_type;}

    /**
     * Set Run Start time (Epoch time)
     */
    void setStartTime(unsigned long long int start_time) { m_startTime = start_time; }

    /**
     * Set Run Stop time (Epoch time)
     */
    void setStopTime(unsigned long long int stop_time) { m_stopTime = stop_time; }

    /**
     * Set No. of events HLT processed
     */
    void setReceivedNevent(unsigned int received_nevent) {m_receivedNevent = received_nevent; }

    /**
     * Set No. of events triggered
     */
    void setAcceptedNevent(unsigned int accepted_nevent) {m_acceptedNevent = accepted_nevent; }

    /**
     * Set No. of events recorded
     */
    void setSentNevent(unsigned int sent_nevent) {m_sentNevent = sent_nevent; }

    /**
     * Set Run length (in sec)
     */
    void setRunLength(unsigned int run_length) { m_runLength = run_length; }

    /**
     * Set Trigger rate (in Hz)
     */
    void setTriggerRate(double trigger_rate) { m_triggerRate = trigger_rate; }

    /**
     * Set PXD status : 1(included)  0 (excluded)
     */
    void setPXD(unsigned int pxd) { m_pxd = pxd; }

    /**
     * Set SVD status : 1(included)  0 (excluded)
     */
    void setSVD(unsigned int svd) { m_svd = svd; }

    /**
     * Set CDC status : 1(included)  0 (excluded)
     */
    void setCDC(unsigned int cdc) { m_cdc = cdc; }

    /**
     * Set TOP status : 1(included)  0 (excluded)
     */
    void setTOP(unsigned int top) { m_top = top; }

    /**
     * Set ARICH status : 1(included)  0 (excluded)
     */
    void setARICH(unsigned int arich) { m_arich = arich; }

    /**
     * Set ECL status : 1(included)  0 (excluded)
     */
    void setECL(unsigned int ecl) { m_ecl = ecl; }

    /**
     * Set KLM status : 1(included)  0 (excluded)
     */
    void setKLM(unsigned int klm) { m_klm = klm; }

    /**
     * Set Bad run tag : 1(included)  0 (excluded)
     */
    void setBadRun(unsigned int bad_run) {m_badRun = bad_run; }


    /**
     * Get Experiment number
     */
    unsigned int getExp() const { return m_exp; }

    /**
     * Get Run number
     */
    unsigned int getRun() const { return m_run; }

    /**
     * Get Run type
     */
    const std::string& getRunType() const {return m_runType;}

    /**
     * Get Run start time (Epoch time)
     */
    unsigned long long int getStartTime() const { return m_startTime; }


    /**
     * Get Run stop time (Epoch time)
     */
    unsigned long long int getStopTime() const { return m_stopTime; }

    /**
     * Get No. of events HLT processed
     */
    unsigned int getReceivedNevent() const { return m_receivedNevent; }

    /**
     * Get No. of events triggered
     */
    unsigned int getAcceptedNevent() const { return m_acceptedNevent; }

    /**
     * Get No. of events recorded
     */
    unsigned int getSentNevent() const { return m_sentNevent; }

    /**
     * Get Run length (in sec)
     */
    unsigned int getRunLength() const { return m_runLength; }

    /**
     * Get Trigger rate (Hz)
     */
    double getTriggerRate() const { return m_triggerRate; }

    /**
     * Get PXD status : 1(included)  0 (excluded)
     */
    unsigned int getPXD() const { return m_pxd; }

    /**
     * Get SVD status :  1(included)  0 (excluded)
     */
    unsigned int getSVD() const { return m_svd; }

    /**
     * Get CDC status :  1(included)  0 (excluded)
     */
    unsigned int getCDC() const { return m_cdc; }

    /**
     * Get TOP status : 1(included)  0 (excluded)
     */
    unsigned int getTOP() const { return m_top; }

    /**
     * Get ARICH status : 1(included)  0 (excluded)
     */
    unsigned int getARICH() const { return m_arich; }

    /**
     * Get ECL status : 1(included)  0 (excluded)
     */
    unsigned int getECL() const { return m_ecl; }

    /**
     * Get KLM status  : 1(included)  0 (excluded)
     */
    unsigned int getKLM() const { return m_klm; }

    /**
     * Get Bad run tag
     */
    unsigned int getBadRun() const { return m_badRun; }

    /**
     * Print the content value
     */
    void Print();



  private:
    /** Experiment Number */
    unsigned int m_exp;

    /** Run Number */
    unsigned int m_run;

    /** Run type  */
    std::string m_runType;

    /** Run start time  (Epoch time) */
    unsigned long long int m_startTime;

    /** Run stop time  (Epoch time) */
    unsigned long long int m_stopTime;

    /** No. of events HLT processed   */
    unsigned int m_receivedNevent;

    /** Get No. of events triggered  */
    unsigned int m_acceptedNevent;

    /** No. of events recorded  */
    unsigned int m_sentNevent;

    /** Trigger rate (in Hz) */
    double m_triggerRate;

    /** Run length  (in sec)*/
    unsigned int m_runLength;

    /** Bad run tag */
    unsigned int m_badRun;

    /** PXD status : 1(included)  0 (excluded)  */
    unsigned int m_pxd;

    /** SVD status : 1(included)  0 (excluded)  */
    unsigned int m_svd;

    /** CDC status  : 1(included)  0 (excluded) */
    unsigned int m_cdc;

    /** TOP status : 1(included)  0 (excluded)  */
    unsigned int m_top;

    /** ARICH status : 1(included)  0 (excluded) */
    unsigned int m_arich;

    /** ECL status : 1(included)  0 (excluded) */
    unsigned int m_ecl;

    /** KLM status : 1(included)  0 (excluded) */
    unsigned int m_klm;
    ClassDef(RunInfo, 1); /**< ClassDef */
  };

} // end namespace Belle2
