/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <string>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

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
     * Set Run length (in sec) by DAQ
     */
    void setRunLength(unsigned int run_length) { m_runLength = run_length; }

    /**
     * Set Trigger rate (in Hz)
     */
    void setTriggerRate(double trigger_rate) { m_triggerRate = trigger_rate; }

    /**
     * Set DetectorSet based on which sub-detectors is included
     */

    void setBelle2Detector(unsigned int pxd, unsigned int svd,
                           unsigned int cdc, unsigned int top,
                           unsigned int arich, unsigned int ecl,
                           unsigned int klm)
    {
      if (pxd > 0)
        m_Belle2Detector = m_Belle2Detector + Const::PXD ;
      if (svd > 0)
        m_Belle2Detector = m_Belle2Detector + Const::SVD ;
      if (cdc > 0)
        m_Belle2Detector = m_Belle2Detector + Const::CDC ;
      if (top > 0)
        m_Belle2Detector = m_Belle2Detector + Const::TOP ;
      if (arich > 0)
        m_Belle2Detector = m_Belle2Detector + Const::ARICH ;
      if (ecl > 0)
        m_Belle2Detector = m_Belle2Detector + Const::ECL ;
      if (klm > 0)
        m_Belle2Detector = m_Belle2Detector + Const::KLM ;
    }



    /**
     * Set Bad run tag  (=0 means good run)
     */
    void setBadRun(unsigned int bad_run) {m_badRun = bad_run; }


    /**
     * Set L1 preScale Trigger factors
     */
    void setPreScaleTrigger(unsigned int prescaleTrigger, int bit)
    {
      if (bit > -1  && bit < 192) {
        m_prescaleTrigger[bit] = prescaleTrigger;
      } else {
        B2FATAL("setPreScaleTrigger Error : " << bit << " not a valid range. \n Terminate");
      }
    }



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
     * Get Run length (in sec) by DAQ
     */
    unsigned int getRunLength() const { return m_runLength; }

    /**
     * Get Trigger rate (Hz)
     */
    double getTriggerRate() const { return m_triggerRate; }


    /**
     * Get Bad run tag
     */
    unsigned int getBadRun() const { return m_badRun; }


    /**
     * Get DetectorSet for the subdetectors included in the set
     */
    Const::DetectorSet getBelle2Detector() const { return m_Belle2Detector;}


    /**
     * Get L1 preScale Trigger factors
     */
    unsigned int getPreScaleTrigger(int bit) const
    {
      if (bit > -1  && bit < 192) {
        return m_prescaleTrigger[bit];
      } else {
        B2FATAL("getPreScaleTrigger Error : " << bit << " not a valid range. \n Terminate");
      }
    }



    /**
     * Print the content value
     */
    virtual void Print(Option_t* option = "") const override;



  private:
    /** Experiment Number */
    unsigned int m_exp{0};

    /** Run Number */
    unsigned int m_run{0};

    /** Run type  */
    std::string m_runType{""};

    /** Run start time  (Epoch time) */
    unsigned long long int m_startTime{0};

    /** Run stop time  (Epoch time) */
    unsigned long long int m_stopTime{0};

    /** No. of events HLT processed   */
    unsigned int m_receivedNevent{0};

    /** Get No. of events triggered  */
    unsigned int m_acceptedNevent{0};

    /** No. of events recorded  */
    unsigned int m_sentNevent{0};

    /** Trigger rate (in Hz) */
    double m_triggerRate{0};

    /** Run length  (in sec) by DAQ. Start and Stop time are when shifter starts the run but in reality there is some difference as DAQ doesn't start immediately or can be paused during the run.*/
    unsigned int m_runLength{0};

    /** Bad run tag  (=0 means good run)*/
    unsigned int m_badRun{0};

    /** DetectorSet for the sub-detector used */
    Const::DetectorSet m_Belle2Detector;

    /** L1 Trigger Prescale factors */
    unsigned int m_prescaleTrigger[192] = {0};

    ClassDefOverride(RunInfo, 2); /**< ClassDef */
  };

} // end namespace Belle2
