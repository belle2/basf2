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

    L1EmulationInformation(): m_eclBhabha(0), m_BhabhaVeto(0), m_SBhabhaVeto(0), m_ggVeto(0), m_ntrg(0), m_weight(0.)
    {
      for (int i = 0; i < 50; i++) {
        m_subweight[i] = 0;
        m_summary[i] = 0;
      }
    }

    ~L1EmulationInformation() {}

//setters
    /**set trigger results*/
    void setSummary(int i, double Summary) {m_summary[i] = Summary;}
    /**set total weight*/
    void settotWeight(double w) {m_weight = w;}

    /**set total weight of sub trigger*/
    void setsubWeight(int i, double w) { m_subweight[i] = w;}

    /**set the number of triggers*/
    void setnTrg(int n = 1) {m_ntrg += n;}

    /**set ecl Bhabha veto*/
    void setECLBhabha(int eclBhabha) {m_eclBhabha = eclBhabha;}

    /**set Bhabha veto*/
    void setBhabhaVeto(int BhabhaVeto) {m_BhabhaVeto = BhabhaVeto;}

    /**set Bhabha veto*/
    void setSBhabhaVeto(int SBhabhaVeto) {m_SBhabhaVeto = SBhabhaVeto;}

    /**set diphoton veto*/
    void setggVeto(int ggVeto) {m_ggVeto = ggVeto;}

    /**get trigger results*/
    int getSummary(int i) const {return m_summary[i];}

    /**get ecl Bhabha veto*/
    int getECLBhabha() const {return m_eclBhabha;}

    /**get Bhabha veto*/
    int getBhabhaVeto() const {return m_BhabhaVeto;}

    /**get SBhabha veto*/
    int getSBhabhaVeto() const {return m_SBhabhaVeto;}

    /**get diphoton veto*/
    int getggVeto() const {return m_ggVeto;}

    /**get the numbre of triggers*/
    int getnTrg() const {return m_ntrg;}

    /**get total weight*/
    double gettotWeight() const {return m_weight;}

    /**get total weight of sub trigger*/
    double getsubWeight(int i) const {return m_subweight[i];}

  private:

    /**Bhabha veot by using  ECL information only*/
    int m_eclBhabha;

    /**Bhabha veto by using CDC and ECL information*/
    int m_BhabhaVeto;

    /**single track Bhabha veto by using CDC and ECL information*/
    int m_SBhabhaVeto;

    /**gamma gamma veto by using CDC and ECL information*/
    int m_ggVeto;

    /**the trigger results*/
    double m_summary[50];

    /**the number of triggers*/
    int m_ntrg;

    /**the total weight*/
    double m_weight;

    /**the sub weight*/
    double m_subweight[50];

    ClassDef(L1EmulationInformation, 2)
  };

}
