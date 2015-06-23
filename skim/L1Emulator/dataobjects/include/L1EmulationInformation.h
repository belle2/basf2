/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

//#include <skim/hlt/dataobjects/L1EmulationInformation.h>
#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {

  /**Class to store the variables for event selection in physics trigger*/
  class L1EmulationInformation: public RelationsObject {

  public:

    L1EmulationInformation(): m_eclBhabha(0), m_BhabhaVeto(0), m_ggVeto(0)
    {
      m_summary.clear();
    }

    ~L1EmulationInformation() {}

//setters
    /**set trigger results*/
    void setSummary(int Summary) {m_summary.push_back(Summary);}

    /**set ecl Bhabha veto*/
    void setECLBhabha(int eclBhabha) {m_eclBhabha = eclBhabha;}

    /**set Bhabha veto*/
    void setBhabhaVeto(int BhabhaVeto) {m_BhabhaVeto = BhabhaVeto;}

    /**set diphoton veto*/
    void setggVeto(int ggVeto) {m_ggVeto = ggVeto;}

    /**get trigger results*/
    std::vector<int> getSummary() const {return m_summary;}

    /**get ecl Bhabha veto*/
    int getECLBhabha() const {return m_eclBhabha;}

    /**get Bhabha veto*/
    int getBhabhaVeto() const {return m_BhabhaVeto;}

    /**get diphoton veto*/
    int getggVeto() const {return m_ggVeto;}

  private:

    /**Bhabha veot by using  ECL information only*/
    int m_eclBhabha;

    /**Bhabha veto by using CDC and ECL information*/
    int m_BhabhaVeto;

    /**diphoton veto by using CDC and ECL information*/
    int m_ggVeto;

    /**the trigger results*/
    std::vector<int> m_summary;

    ClassDef(L1EmulationInformation, 1)
  };

}
