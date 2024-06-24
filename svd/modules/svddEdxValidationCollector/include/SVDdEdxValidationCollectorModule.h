/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <string>




namespace Belle2 {
  /**
   * Collector module used to create the histograms needed for the
   * SVD dE/dx calibration
   */
  class SVDdEdxValidationCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    SVDdEdxValidationCollectorModule();

    /**
     * Initialize the module
     */
    void prepare() override final;

    /**
     * Event processor
     */
    void collect() override final;

  private:

    int m_evt; /**<  event number */
    int m_exp; /**<  experiment number */
    int m_run; /**<  run number */

    double m_time; /**<  event time */


    double m_InvMLambda;       /**<  Invariant mass of Lambda candidates */
    double m_p_p;       /**<   momentum for the proton from the Lambda */
    double m_p_SVDdEdx;     /**<  SVD dE/dx response for the proton from the Lambda */
    double m_pi_p;    /**<   momentum for the pion from the Lambda */
    double m_pi_SVDdEdx;    /**<  SVD dE/dx response for the pion from the Lambda */

    double m_InvMDstar;       /**<  Invariant mass of Dstar candidates */
    double m_InvMD0;       /**<  Invariant mass of D0 candidates */
    double m_DeltaM;       /**<  deltaM = m(Dstar)-m(D0) */
    double m_K_p;       /**<   momentum for the kaon from the D0 */
    double m_K_SVDdEdx;     /**<  SVD dE/dx response for the kaon from the D0 */
    double m_piD_p;    /**<   momentum for the pion from the D0 */
    double m_piD_SVDdEdx;    /**<  SVD dE/dx response for the pion from the D0 */
    double m_piS_p;    /**<   momentum for the pion from the Dstar */
    double m_piS_SVDdEdx;    /**<  SVD dE/dx response for the pion from the Dstar */

    double m_InvMGamma;       /**<  Invariant mass of converted photon candidates */
    double m_e_1_p;       /**<   momentum for the first electron */
    double m_e_1_SVDdEdx;     /**<  SVD dE/dx response for the first electron */
    double m_e_2_p;    /**<   momentum for the second electron */
    double m_e_2_SVDdEdx;    /**<  SVD dE/dx response for the second electron */

    double m_p_electronID_ALL;  /**<  electron ID value (all subdetectors) for the proton from Lambda */
    double m_p_pionID_ALL;  /**<  pion ID value (all subdetectors) for the proton from Lambda */
    double m_p_kaonID_ALL;  /**<  kaon ID value (all subdetectors) for the proton from Lambda */
    double m_p_protonID_ALL;  /**<  proton ID value (all subdetectors) for the proton from Lambda */

    double m_K_electronID_ALL;   /**<  electron ID value (all subdetectors) for the kaon from D */
    double m_K_pionID_ALL;   /**<  pion ID value (all subdetectors) for the kaon from D */
    double m_K_kaonID_ALL;   /**<  kaon ID value (all subdetectors) for the kaon from D */
    double m_K_protonID_ALL;   /**<  proton ID value (all subdetectors) for the kaon from D */

    double m_piD_electronID_ALL;   /**<  electron ID value (all subdetectors) for the pion from D */
    double m_piD_pionID_ALL;   /**<  pion ID value (all subdetectors) for the pion from D */
    double m_piD_kaonID_ALL;   /**<  kaon ID value (all subdetectors) for the pion from D */
    double m_piD_protonID_ALL;   /**<  proton ID value (all subdetectors) for the pion from D */

    double m_piS_electronID_ALL;   /**<  electron ID value (all subdetectors) for the pion from Dstar */
    double m_piS_pionID_ALL;   /**<  pion ID value (all subdetectors) for the pion from Dstar */
    double m_piS_kaonID_ALL;   /**<  kaon ID value (all subdetectors) for the pion from Dstar */
    double m_piS_protonID_ALL;   /**<  proton ID value (all subdetectors) for the pion from Dstar */

    double m_e_1_electronID_ALL;   /**<  electron ID value (all subdetectors) for the e+ from gamma */
    double m_e_1_pionID_ALL;   /**<  pion ID value (all subdetectors) for the e+ from gamma */
    double m_e_1_kaonID_ALL;   /**<  kaon ID value (all subdetectors) for the e+ from gamma */
    double m_e_1_protonID_ALL;   /**<  proton ID value (all subdetectors) for the e+ from gamma */

    double m_e_2_electronID_ALL;   /**<  electron ID value (all subdetectors) for the e- from gamma */
    double m_e_2_pionID_ALL;   /**<  pion ID value (all subdetectors) for the e- from gamma */
    double m_e_2_kaonID_ALL;   /**<  kaon ID value (all subdetectors) for the e- from gamma */
    double m_e_2_protonID_ALL;   /**<  proton ID value (all subdetectors) for the e- from gamma */


    double m_p_electronID_SVDonly;  /**<  electron ID value (only SVD) for the proton from Lambda */
    double m_p_pionID_SVDonly;  /**<  pion ID value (only SVD) for the proton from Lambda */
    double m_p_kaonID_SVDonly;  /**<  kaon ID value (only SVD) for the proton from Lambda */
    double m_p_protonID_SVDonly;  /**<  proton ID value (only SVD) for the proton from Lambda */

    double m_K_electronID_SVDonly;  /**<  electron ID value (only SVD) for the K from D */
    double m_K_pionID_SVDonly;  /**<  pion ID value (only SVD) for the K from D */
    double m_K_kaonID_SVDonly;  /**<  kaon ID value (only SVD) for the K from D */
    double m_K_protonID_SVDonly;  /**<  proton ID value (only SVD) for the K from D */

    double m_piD_electronID_SVDonly;  /**<  electron ID value (only SVD) for the pi from D */
    double m_piD_pionID_SVDonly;  /**<  pion ID value (only SVD) for the pi from D */
    double m_piD_kaonID_SVDonly;  /**<  kaon ID value (only SVD) for the pi from D */
    double m_piD_protonID_SVDonly;  /**<  proton ID value (only SVD) for the pi from D */

    double m_piS_electronID_SVDonly;  /**<  electron ID value (only SVD) for the pi from Dstar */
    double m_piS_pionID_SVDonly;  /**<  pion ID value (only SVD) for the pi from Dstar */
    double m_piS_kaonID_SVDonly;  /**<  kaon ID value (only SVD) for the pi from Dstar */
    double m_piS_protonID_SVDonly;  /**<  proton ID value (only SVD) for the pi from Dstar */

    double m_e_1_electronID_SVDonly;  /**<  electron ID value (only SVD) for the e+ from gamma */
    double m_e_1_pionID_SVDonly;  /**<  pion ID value (only SVD) for the e+ from gamma */
    double m_e_1_kaonID_SVDonly;  /**<  kaon ID value (only SVD) for the e+ from gamma */
    double m_e_1_protonID_SVDonly;  /**<  proton ID value (only SVD) for the e+ from gamma */

    double m_e_2_electronID_SVDonly;  /**<  electron ID value (only SVD) for the e- from gamma */
    double m_e_2_pionID_SVDonly;  /**<  pion ID value (only SVD) for the e- from gamma */
    double m_e_2_kaonID_SVDonly;  /**<  kaon ID value (only SVD) for the e- from gamma */
    double m_e_2_protonID_SVDonly;  /**<  proton ID value (only SVD) for the e- from gamma */


    double m_p_electronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the p from Lambda */
    double m_p_pionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the p from Lambda */
    double m_p_kaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the p from Lambda */
    double m_p_protonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the p from Lambda */

    double m_K_electronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the K from D */
    double m_K_pionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the K from D */
    double m_K_kaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the K from D */
    double m_K_protonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the K from D */

    double m_piD_electronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the pi from D */
    double m_piD_pionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the pi from D */
    double m_piD_kaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the pi from D */
    double m_piD_protonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the pi from D */

    double m_piS_electronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_piS_pionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_piS_kaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_piS_protonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_e_1_electronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_e_1_pionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_e_1_kaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_e_1_protonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_e_2_electronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the e- from gamma */
    double m_e_2_pionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the e- from gamma */
    double m_e_2_kaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the e- from gamma */
    double m_e_2_protonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the e- from gamma */


    double m_p_proton_electronID_ALL;  /**<  binary p/e ID value (all subdetectors) for the p from Lambda */
    double m_p_proton_pionID_ALL;  /**<  binary p/pi ID value (all subdetectors) for the p from Lambda */
    double m_p_proton_kaonID_ALL;  /**<  binary p/K ID value (all subdetectors) for the p from Lambda */

    double m_p_kaon_protonID_ALL;  /**<  binary K/p ID value (all subdetectors) for the p from Lambda */
    double m_p_pion_protonID_ALL;  /**<  binary pi/p ID value (all subdetectors) for the p from Lambda */
    double m_p_electron_protonID_ALL;  /**<  binary e/p ID value (all subdetectors) for the p from Lambda */

    double m_K_kaon_electronID_ALL;  /**<  binary K/e ID value (all subdetectors) for the K from D */
    double m_K_kaon_pionID_ALL;  /**<  binary K/pi ID value (all subdetectors) for the K from D */
    double m_K_kaon_protonID_ALL;  /**<  binary K/p ID value (all subdetectors) for the K from D */

    double m_K_pion_kaonID_ALL;  /**<  binary pi/K ID value (all subdetectors) for the K from D */
    double m_K_proton_kaonID_ALL;  /**<  binary p/K ID value (all subdetectors) for the K from D */
    double m_K_electron_kaonID_ALL;  /**<  binary e/K ID value (all subdetectors) for the K from D */

    double m_piD_pion_electronID_ALL;  /**<  binary pi/e ID value (all subdetectors) for the pi from D */
    double m_piD_pion_kaonID_ALL;  /**<  binary pi/K ID value (all subdetectors) for the pi from D */
    double m_piD_pion_protonID_ALL;  /**<  binary pi/p ID value (all subdetectors) for the pi from D */

    double m_piD_electron_pionID_ALL;  /**<  binary e/pi ID value (all subdetectors) for the pi from D */
    double m_piD_kaon_pionID_ALL;  /**<  binary K/pi ID value (all subdetectors) for the pi from D */
    double m_piD_proton_pionID_ALL;  /**<  binary p/pi ID value (all subdetectors) for the pi from D */

    double m_piS_pion_electronID_ALL;  /**<  binary pi/e ID value (all subdetectors) for the pi from Dstar */
    double m_piS_pion_kaonID_ALL;  /**<  binary pi/K ID value (all subdetectors) for the pi from Dstar */
    double m_piS_pion_protonID_ALL;  /**<  binary pi/p ID value (all subdetectors) for the pi from Dstar */

    double m_piS_electron_pionID_ALL;  /**<  binary e/pi ID value (all subdetectors) for the pi from Dstar */
    double m_piS_kaon_pionID_ALL;  /**<  binary K/pi ID value (all subdetectors) for the pi from Dstar */
    double m_piS_proton_pionID_ALL;  /**<  binary p/pi ID value (all subdetectors) for the pi from Dstar */

    double m_e_1_electron_pionID_ALL;  /**<  binary e/pi ID value (all subdetectors) for the e+ from gamma */
    double m_e_1_electron_kaonID_ALL;  /**<  binary K/pi ID value (all subdetectors) for the e+ from gamma */
    double m_e_1_electron_protonID_ALL;  /**<  binary p/pi ID value (all subdetectors) for the e+ from gamma */

    double m_e_1_pion_electronID_ALL;  /**<  binary pi/e ID value (all subdetectors) for the e+ from gamma */
    double m_e_1_kaon_electronID_ALL;  /**<  binary K/e ID value (all subdetectors) for the e+ from gamma */
    double m_e_1_proton_electronID_ALL;  /**<  binary p/e ID value (all subdetectors) for the e+ from gamma */

    double m_e_2_electron_pionID_ALL;  /**<  binary e/pi ID value (all subdetectors) for the e- from gamma */
    double m_e_2_electron_kaonID_ALL;  /**<  binary e/K ID value (all subdetectors) for the e- from gamma */
    double m_e_2_electron_protonID_ALL;  /**<  binary e/p ID value (all subdetectors) for the e- from gamma */

    double m_e_2_pion_electronID_ALL;  /**<  binary pi/e ID value (all subdetectors) for the e- from gamma */
    double m_e_2_kaon_electronID_ALL;  /**<  binary K/e ID value (all subdetectors) for the e- from gamma */
    double m_e_2_proton_electronID_ALL;  /**<  binary p/e ID value (all subdetectors) for the e- from gamma */



    double m_p_proton_electronID_noSVD;  /**<  binary p/e ID value (all subdetectors except SVD) for the p from Lambda */
    double m_p_proton_pionID_noSVD;  /**<  binary p/pi ID value (all subdetectors except SVD) for the p from Lambda */
    double m_p_proton_kaonID_noSVD;  /**<  binary p/K ID value (all subdetectors except SVD) for the p from Lambda */

    double m_p_kaon_protonID_noSVD;  /**<  binary K/p ID value (all subdetectors except SVD) for the p from Lambda */
    double m_p_pion_protonID_noSVD;  /**<  binary pi/p ID value (all subdetectors except SVD) for the p from Lambda */
    double m_p_electron_protonID_noSVD;  /**<  binary e/p ID value (all subdetectors except SVD) for the p from Lambda */

    double m_K_kaon_electronID_noSVD;  /**<  binary K/e ID value (all subdetectors except SVD) for the K from D */
    double m_K_kaon_pionID_noSVD;  /**<  binary K/pi ID value (all subdetectors except SVD) for the K from D */
    double m_K_kaon_protonID_noSVD;  /**<  binary K/p ID value (all subdetectors except SVD) for the K from D */

    double m_K_pion_kaonID_noSVD;  /**<  binary pi/K ID value (all subdetectors except SVD) for the K from D */
    double m_K_proton_kaonID_noSVD;  /**<  binary p/K ID value (all subdetectors except SVD) for the K from D */
    double m_K_electron_kaonID_noSVD;  /**<  binary e/K ID value (all subdetectors except SVD) for the K from D */

    double m_piD_pion_electronID_noSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the pi from D */
    double m_piD_pion_kaonID_noSVD;  /**<  binary pi/K ID value (all subdetectors except SVD) for the pi from D */
    double m_piD_pion_protonID_noSVD;  /**<  binary pi/p ID value (all subdetectors except SVD) for the pi from D */

    double m_piD_electron_pionID_noSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the pi from D */
    double m_piD_kaon_pionID_noSVD;  /**<  binary K/pi ID value (all subdetectors except SVD) for the pi from D */
    double m_piD_proton_pionID_noSVD;  /**<  binary p/pi ID value (all subdetectors except SVD) for the pi from D */

    double m_piS_pion_electronID_noSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_piS_pion_kaonID_noSVD;  /**<  binary pi/K ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_piS_pion_protonID_noSVD;  /**<  binary pi/p ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_piS_electron_pionID_noSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_piS_kaon_pionID_noSVD;  /**<  binary K/pi ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_piS_proton_pionID_noSVD;  /**<  binary p/pi ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_e_1_electron_pionID_noSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_e_1_electron_kaonID_noSVD;  /**<  binary e/K ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_e_1_electron_protonID_noSVD;  /**<  binary e/p ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_e_1_pion_electronID_noSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_e_1_kaon_electronID_noSVD;  /**<  binary K/e ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_e_1_proton_electronID_noSVD;  /**<  binary p/e ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_e_2_electron_pionID_noSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the e- from gamma */
    double m_e_2_electron_kaonID_noSVD;  /**<  binary e/K ID value (all subdetectors except SVD) for the e- from gamma */
    double m_e_2_electron_protonID_noSVD;  /**<  binary e/p ID value (all subdetectors except SVD) for the e- from gamma */

    double m_e_2_pion_electronID_noSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the e- from gamma */
    double m_e_2_kaon_electronID_noSVD;  /**<  binary K/e ID value (all subdetectors except SVD) for the e- from gamma */
    double m_e_2_proton_electronID_noSVD;  /**<  binary p/e ID value (all subdetectors except SVD) for the e- from gamma */


    double m_p_proton_electronID_SVDonly;  /**<  binary p/e ID value (only SVD) for the p from Lambda */
    double m_p_proton_pionID_SVDonly;  /**<  binary p/pi ID value (only SVD) for the p from Lambda */
    double m_p_proton_kaonID_SVDonly;  /**<  binary p/K ID value (only SVD) for the p from Lambda */

    double m_p_kaon_protonID_SVDonly;  /**<  binary K/p ID value (only SVD) for the p from Lambda */
    double m_p_pion_protonID_SVDonly;  /**<  binary pi/p ID value (only SVD) for the p from Lambda */
    double m_p_electron_protonID_SVDonly;  /**<  binary e/p ID value (only SVD) for the p from Lambda */

    double m_K_kaon_electronID_SVDonly;  /**<  binary K/e ID value (only SVD) for the K from D */
    double m_K_kaon_pionID_SVDonly;  /**<  binary K/pi ID value (only SVD) for the K from D */
    double m_K_kaon_protonID_SVDonly;  /**<  binary K/ ID value (only SVD) for the K from D */

    double m_K_pion_kaonID_SVDonly;  /**<  binary pi/K ID value (only SVD) for the K from D */
    double m_K_proton_kaonID_SVDonly;  /**<  binary p/K ID value (only SVD) for the K from D */
    double m_K_electron_kaonID_SVDonly;  /**<  binary e/K ID value (only SVD) for the K from D */

    double m_piD_pion_electronID_SVDonly;  /**<  binary pi/e ID value (only SVD) for the pi from D */
    double m_piD_pion_kaonID_SVDonly;  /**<  binary pi/K ID value (only SVD) for the pi from D */
    double m_piD_pion_protonID_SVDonly;  /**<  binary pi/p ID value (only SVD) for the pi from D */

    double m_piD_electron_pionID_SVDonly;  /**<  binary e/pi ID value (only SVD) for the pi from D */
    double m_piD_kaon_pionID_SVDonly;  /**<  binary K/pi ID value (only SVD) for the pi from D */
    double m_piD_proton_pionID_SVDonly;  /**<  binary p/pi ID value (only SVD) for the pi from D */

    double m_piS_pion_electronID_SVDonly;  /**<  binary pi/e ID value (only SVD) for the pi from Dstar */
    double m_piS_pion_kaonID_SVDonly;  /**<  binary pi/K ID value (only SVD) for the pi from Dstar */
    double m_piS_pion_protonID_SVDonly;  /**<  binary pi/p ID value (only SVD) for the pi from Dstar */

    double m_piS_electron_pionID_SVDonly;  /**<  binary e/pi ID value (only SVD) for the pi from Dstar */
    double m_piS_kaon_pionID_SVDonly;  /**<  binary K/pi ID value (only SVD) for the pi from Dstar */
    double m_piS_proton_pionID_SVDonly;  /**<  binary p/pi ID value (only SVD) for the pi from Dstar */

    double m_e_1_electron_pionID_SVDonly;  /**<  binary e/pi ID value (only SVD) for the e+ from gamma */
    double m_e_1_electron_kaonID_SVDonly;  /**<  binary e/K ID value (only SVD) for the e+ from gamma */
    double m_e_1_electron_protonID_SVDonly;  /**<  binary e/p ID value (only SVD) for the e+ from gamma */

    double m_e_1_pion_electronID_SVDonly;  /**<  binary pi/e ID value (only SVD) for the e+ from gamma */
    double m_e_1_kaon_electronID_SVDonly;  /**<  binary K/e ID value (only SVD) for the e+ from gamma */
    double m_e_1_proton_electronID_SVDonly;  /**<  binary p/e ID value (only SVD) for the e+ from gamma */

    double m_e_2_electron_pionID_SVDonly;  /**<  binary e/pi ID value (only SVD) for the e- from gamma */
    double m_e_2_electron_kaonID_SVDonly;  /**<  binary e/K ID value (only SVD) for the e- from gamma */
    double m_e_2_electron_protonID_SVDonly;  /**<  binary e/p ID value (only SVD) for the e- from gamma */

    double m_e_2_pion_electronID_SVDonly;  /**<  binary pi/e ID value (only SVD) for the e- from gamma */
    double m_e_2_kaon_electronID_SVDonly;  /**<  binary K/e ID value (only SVD) for the e- from gamma */
    double m_e_2_proton_electronID_SVDonly;  /**<  binary p/e ID value (only SVD) for the e- from gamma */

    std::string m_LambdaListName = "";    /**< Name of the Lambda particle list */
    std::string m_DstarListName = "";    /**< Name of the Dstar particle list */
    std::string m_GammaListName = "";    /**< Name of the Gamma particle list */


  };

} // end namespace Belle2
