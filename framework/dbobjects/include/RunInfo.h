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

    /* Constant */

    /* Default constructor */
    RunInfo() {}

    /* Set ... */
    void setExp(unsigned int exp) { m_exp = exp; }
    void setRun(unsigned int run) { m_run = run; }
    void setRunType(const std::string& run_type) {m_run_type = run_type;}
    void setStartTime(const std::string& start_time) { m_start_time = start_time; }
    void setStopTime(const std::string& stop_time) { m_stop_time = stop_time; }
    void setReceivedNevent(unsigned int received_nevent) {m_received_nevent = received_nevent; }
    void setAcceptedNevent(unsigned int accepted_nevent) {m_accepted_nevent = accepted_nevent; }
    void setSentNevent(unsigned int sent_nevent) {m_sent_nevent = sent_nevent; }
    void setRunLength(unsigned int run_length) { m_run_length = run_length; }
    void setTriggerRate(double trigger_rate) { m_trigger_rate = trigger_rate; }
    void setPXD(unsigned int pxd) { m_pxd = pxd; }
    void setSVD(unsigned int svd) { m_svd = svd; }
    void setCDC(unsigned int cdc) { m_cdc = cdc; }
    void setTOP(unsigned int top) { m_top = top; }
    void setARICH(unsigned int arich) { m_arich = arich; }
    void setECL(unsigned int ecl) { m_ecl = ecl; }
    void setKLM(unsigned int klm) { m_klm = klm; }
    void setBadRun(unsigned int bad_run) {m_bad_run = bad_run; }

    /* Get ... */
    unsigned int getExp() const { return m_exp; }
    unsigned int getRun() const { return m_run; }
    const std::string& getRunType() const {return m_run_type;}
    const std::string& getStartTime() const { return m_start_time; }
    const std::string& getStopTime() const { return m_stop_time; }
    unsigned int getReceivedNevent() const { return m_received_nevent; }
    unsigned int getAcceptedNevent() const { return m_accepted_nevent; }
    unsigned int getSentNevent() const { return m_sent_nevent; }
    unsigned int getRunLength() const { return m_run_length; }
    double getTriggerRate() const { return m_trigger_rate; }
    unsigned int getPXD() const { return m_pxd; }
    unsigned int getSVD() const { return m_svd; }
    unsigned int getCDC() const { return m_cdc; }
    unsigned int getTOP() const { return m_top; }
    unsigned int getARICH() const { return m_arich; }
    unsigned int getECL() const { return m_ecl; }
    unsigned int getKLM() const { return m_klm; }
    unsigned int getBadRun() const { return m_bad_run; }

    /* Print out contents */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Run Summary" << std::endl;
      std::cout << " Exp " << m_exp << ", Run " << m_run << std::endl;
      std::cout << " Run Type " << m_run_type << std::endl;
      std::cout << " Start time " << m_start_time << ", Stop time " << m_stop_time
                << ", Run Length " << m_run_length << ", Trigger Rate " << m_trigger_rate << std::endl;
      std::cout << " Received nevents " << m_received_nevent << " Accepted nevents " << m_accepted_nevent
                << ", Sent nevents " << m_sent_nevent << std::endl;
      std::cout << " Status: PXD " << m_pxd << ", SVD " << m_svd << ", CDC " << m_cdc
                << ", TOP " << m_top << ", ARICH " << m_arich << ", ECL " << m_ecl << ", KLM " << m_klm
                << std::endl;
    }

  private:
    unsigned int m_exp;
    unsigned int m_run;
    std::string m_run_type;
    //TTimeStamp m_start_time;
    //TTimeStamp m_stop_time;
    std::string m_start_time;
    std::string m_stop_time;
    unsigned int m_received_nevent;
    unsigned int m_accepted_nevent;
    unsigned int m_sent_nevent;
    double m_trigger_rate;
    unsigned int m_run_length;
    unsigned int m_bad_run;
    unsigned int m_pxd;
    unsigned int m_svd;
    unsigned int m_cdc;
    unsigned int m_top;
    unsigned int m_arich;
    unsigned int m_ecl;
    unsigned int m_klm;
    ClassDef(RunInfo, 1); /**< ClassDef */
  };

} // end namespace Belle2
