/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLRECPi0MODULE_H_
#define ECLRECPi0MODULE_H_

#include <framework/core/Module.h>
#include <vector>
#include <TRandom3.h>
#include  "CLHEP/Vector/LorentzVector.h"


namespace Belle2 {
  namespace ECL {

    /** Module for making ECLRecoHits of CDCHits.
     *
     */
    class ECLRecPi0Module : public Module {

    public:

      /** Constructor.
       */
      ECLRecPi0Module();

      /** Destructor.
       */
      ~ECLRecPi0Module();


      /** Initialize variables, print info, and start CPU clock. */
      virtual void initialize();

      /** Nothing so far.*/
      virtual void beginRun();

      /** Actual digitization of all hits in the ECL.
       *
       *  The digitized hits are written into the DataStore.
       */
      virtual void event();

      /** Nothing so far. */
      virtual void endRun();

      /** Stopping of CPU clock.*/
      virtual void terminate();

      /** mass constraint fit of  pi0      */
      void fit(CLHEP::HepLorentzVector m_shower1, CLHEP::HepLorentzVector m_shower2);
      /** calculate error of Energy      */
      double errorE(CLHEP::HepLorentzVector m_shower);
      /** calculate error of Theta      */
      double errorTheta(CLHEP::HepLorentzVector m_shower);
      /** calculate error of phi     */
      double errorPhi(CLHEP::HepLorentzVector m_shower);
      /** calculate error of R of cell       */
      double cellR(CLHEP::HepLorentzVector m_shower);


      /** operator of square      */
      double squ(double x) { return x * x; }
      /** get Energy of Pi0      */
      double Get_E() { return m_pi0E; }
      /** get px of Pi0      */
      double Get_px() { return m_pi0px; }
      /** get py of Pi0      */
      double Get_py() { return m_pi0py; }
      /** get pz of Pi0      */
      double Get_pz() { return m_pi0pz; }
      /** get combined mass of Pi0      */
      double Get_mass() { return m_pi0mass; }
      /** get chi2 of  Pi0      */
      double Get_chi2() { return m_pi0chi2; }

      /** Require: E_gamma > gamma_energy_threshold;*/
      float gamma_energy_threshold;

      /** Require: theta_open > theta_min - opening_angle_cut_margi.n */
      float opening_angle_cut_margin;

      /** pi0 mass region (without fitting). */
      float pi0_mass_min;

      /** pi0 mass region (without fitting). */
      float pi0_mass_max;

      /** apply mass constraint fit */
      int fit_flag;

      /** chi2 threshold of fitting result */
      float chi2_max;

      /** define enum region_type */
      enum region_type {REGION_SIGMA, REGION_GEV};




    protected:



    private:

      /**  Name of collection of ECLGamma.*/
      std::string m_ECLGammaName ;
      /**  Name of collection of ECLPi0.*/
      std::string m_ECLPi0Name ;
      /** CPU time     */
      double m_timeCPU;
      /** Run number   */
      int    m_nRun;
      /** Event number */
      int    m_nEvent;
      /** ECL Gamma index */
      int    m_GNum;
      /** ECL Gamma index */
      int    m_Pi0Num;
      /** gamma1 information    */
      int    m_showerId1 ;
      /** gamma2 information    */
      int    m_showerId2 ;
      /** gamma1 information    */
      double m_px1;
      /** gamma1 information    */
      double m_py1;
      /** gamma1 information    */
      double m_pz1;
      /** gamma2 information    */
      double m_px2;
      /** gamma2 information    */
      double m_py2;
      /** gamma2 information    */
      double m_pz2;

      /** pi0 information    */
      double m_pi0E;
      /** pi0 information    */
      double m_pi0px;
      /** pi0 information    */
      double m_pi0py;
      /** pi0 information    */
      double m_pi0pz;
      /** pi0 information    */
      double m_pi0mass;
      /** pi0 information    */
      double m_pi0chi2;

    };
  }//ECL
}//Belle2

#endif /* EVTMETAINFO_H_ */
