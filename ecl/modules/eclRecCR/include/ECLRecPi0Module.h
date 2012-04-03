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

    void fit(CLHEP::HepLorentzVector m_shower1, CLHEP::HepLorentzVector m_shower2);
    double errorE(CLHEP::HepLorentzVector m_shower);
    double errorTheta(CLHEP::HepLorentzVector m_shower);
    double errorPhi(CLHEP::HepLorentzVector m_shower);
    double cellR(CLHEP::HepLorentzVector m_shower);



    double squ(double x) { return x * x; }
    double Get_E() { return m_pi0E; }
    double Get_px() { return m_pi0px; }
    double Get_py() { return m_pi0py; }
    double Get_pz() { return m_pi0pz; }
    double Get_mass() { return m_pi0mass; }
    double Get_chi2() { return m_pi0chi2; }

    // Require: E_gamma > gamma_energy_threshold;
    float gamma_energy_threshold;

    // Require: theta_open > theta_min - opening_angle_cut_margin
    float opening_angle_cut_margin;

    // pi0 mass region (without fitting)
    float pi0_mass_min, pi0_mass_max;

    // apply mass constraint fit
    int fit_flag;

    // chi2 threshold of fitting result
    float chi2_max;

    // define   enum region_type
    enum region_type {REGION_SIGMA, REGION_GEV};




  protected:



  private:
    std::string m_MdstGammaName ;     /**  Name of collection of MdstGamma.*/
    std::string m_MdstPi0Name ;     /**  Name of collection of MdstPi0.*/

    double m_timeCPU;                /*!< CPU time     */
    int    m_nRun;                   /*!< Run number   */
    int    m_nEvent;                 /*!< Event number */
    int    m_GNum;                   /*!< Mdst Gamma index */
    int    m_Pi0Num;                   /*!< Mdst Gamma index */

    int    m_showerId1 ;               /*!gamma1 information    */
    int    m_showerId2 ;               /*!gamma2 information    */
    double m_px1;                     /*!gamma1 information    */
    double m_py1;                     /*!gamma1 information    */
    double m_pz1;                     /*!gamma1 information    */

    double m_px2;                     /*!gamma2 information    */
    double m_py2;                     /*!gamma2 information    */
    double m_pz2;                     /*!gamma2 information    */


    double m_pi0E;                      /*!pi0 information    */
    double m_pi0px;                     /*!pi0 information    */
    double m_pi0py;                     /*!pi0 information    */
    double m_pi0pz;                     /*!pi0 information    */
    double m_pi0mass;                   /*!pi0 information    */
    double m_pi0chi2;                 /*!pi0 information    */

  };
}

#endif /* EVTMETAINFO_H_ */
