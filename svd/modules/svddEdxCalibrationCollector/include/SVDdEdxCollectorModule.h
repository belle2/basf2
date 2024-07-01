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
  class SVDdEdxCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    SVDdEdxCollectorModule();

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
    double m_protonMomentum;       /**<   momentum for the proton from the Lambda */
    double m_protonSVDdEdx;     /**<  SVD dE/dx response for the proton from the Lambda */
    double m_pionLambdap;    /**<   momentum for the pion from the Lambda */
    double m_pionLambdaSVDdEdx;    /**<  SVD dE/dx response for the pion from the Lambda */

    double m_InvMDstar;       /**<  Invariant mass of Dstar candidates */
    double m_InvMD0;       /**<  Invariant mass of D0 candidates */
    double m_DeltaM;       /**<  deltaM = m(Dstar)-m(D0) */
    double m_kaonMomentum;       /**<   momentum for the kaon from the D0 */
    double m_kaonSVDdEdx;     /**<  SVD dE/dx response for the kaon from the D0 */
    double m_pionDp;    /**<   momentum for the pion from the D0 */
    double m_pionDSVDdEdx;    /**<  SVD dE/dx response for the pion from the D0 */
    double m_slowPionMomentum;    /**<   momentum for the pion from the Dstar */
    double m_slowPionSVDdEdx;    /**<  SVD dE/dx response for the pion from the Dstar */

    double m_InvMGamma;       /**<  Invariant mass of converted photon candidates */
    double m_firstElectronMomentum;       /**<   momentum for the first electron */
    double m_firstElectronSVDdEdx;     /**<  SVD dE/dx response for the first electron */
    double m_secondElectronMomentum;    /**<   momentum for the second electron */
    double m_secondElectronSVDdEdx;    /**<  SVD dE/dx response for the second electron */


    std::string m_LambdaListName = "";    /**< Name of the Lambda particle list */
    std::string m_DstarListName = "";    /**< Name of the Dstar particle list */
    std::string m_GammaListName = "";    /**< Name of the Gamma particle list */


  };

} // end namespace Belle2
