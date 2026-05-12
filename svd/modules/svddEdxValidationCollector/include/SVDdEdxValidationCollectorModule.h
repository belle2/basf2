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

    int m_evt = 0; /**<  event number */
    int m_exp = 0; /**<  experiment number */
    int m_run = 0; /**<  run number */

    double m_time = 0; /**<  event time */


    double m_InvMLambda = 0;      /**<  Invariant mass of Lambda candidates */
    double m_protonp = 0;      /**<   momentum for the proton from the Lambda */
    double m_protonSVDdEdx = 0;    /**<  SVD dE/dx response for the proton from the Lambda */

    double m_InvMDstar = 0;      /**<  Invariant mass of Dstar candidates */
    double m_InvMD0 = 0;      /**<  Invariant mass of D0 candidates */
    double m_DeltaM = 0;      /**<  deltaM = m(Dstar)-m(D0) */
    double m_kaonp = 0;      /**<   momentum for the kaon from the D0 */
    double m_kaonSVDdEdx = 0;    /**<  SVD dE/dx response for the kaon from the D0 */
    double m_pionDp = 0;   /**<   momentum for the pion from the D0 */
    double m_pionDSVDdEdx = 0;   /**<  SVD dE/dx response for the pion from the D0 */
    double m_slowPionp = 0;   /**<   momentum for the pion from the Dstar */
    double m_slowPionSVDdEdx = 0;   /**<  SVD dE/dx response for the pion from the Dstar */

    double m_InvMGamma = 0;      /**<  Invariant mass of converted photon candidates */
    double m_firstElectronp = 0;      /**<   momentum for the first electron */
    double m_firstElectronSVDdEdx = 0;    /**<  SVD dE/dx response for the first electron */
    double m_secondElectronp = 0;   /**<   momentum for the second electron */
    double m_secondElectronSVDdEdx = 0;   /**<  SVD dE/dx response for the second electron */

    double m_protonElectronIDALL = 0; /**<  electron ID value (all subdetectors) for the proton from Lambda */
    double m_protonPionIDALL = 0; /**<  pion ID value (all subdetectors) for the proton from Lambda */
    double m_protonKaonIDALL = 0; /**<  kaon ID value (all subdetectors) for the proton from Lambda */
    double m_protonProtonIDALL = 0; /**<  proton ID value (all subdetectors) for the proton from Lambda */

    double m_kaonElectronIDALL = 0;  /**<  electron ID value (all subdetectors) for the kaon from D */
    double m_kaonPionIDALL = 0;  /**<  pion ID value (all subdetectors) for the kaon from D */
    double m_kaonKaonIDALL = 0;  /**<  kaon ID value (all subdetectors) for the kaon from D */
    double m_kaonProtonIDALL = 0;  /**<  proton ID value (all subdetectors) for the kaon from D */

    double m_pionDElectronIDALL = 0;  /**<  electron ID value (all subdetectors) for the pion from D */
    double m_pionDPionIDALL = 0;  /**<  pion ID value (all subdetectors) for the pion from D */
    double m_pionDKaonIDALL = 0;  /**<  kaon ID value (all subdetectors) for the pion from D */
    double m_pionDProtonIDALL = 0;  /**<  proton ID value (all subdetectors) for the pion from D */

    double m_slowPionElectronIDALL = 0;  /**<  electron ID value (all subdetectors) for the pion from Dstar */
    double m_slowPionPionIDALL = 0;  /**<  pion ID value (all subdetectors) for the pion from Dstar */
    double m_slowPionKaonIDALL = 0;  /**<  kaon ID value (all subdetectors) for the pion from Dstar */
    double m_slowPionProtonIDALL = 0;  /**<  proton ID value (all subdetectors) for the pion from Dstar */

    double m_firstElectronElectronIDALL = 0;  /**<  electron ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronPionIDALL = 0;  /**<  pion ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronKaonIDALL = 0;  /**<  kaon ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronProtonIDALL = 0;  /**<  proton ID value (all subdetectors) for the e+ from gamma */

    double m_protonElectronIDSVDonly = 0; /**<  electron ID value (only SVD) for the proton from Lambda */
    double m_protonPionIDSVDonly = 0; /**<  pion ID value (only SVD) for the proton from Lambda */
    double m_protonKaonIDSVDonly = 0; /**<  kaon ID value (only SVD) for the proton from Lambda */
    double m_protonProtonIDSVDonly = 0; /**<  proton ID value (only SVD) for the proton from Lambda */

    double m_kaonElectronIDSVDonly = 0; /**<  electron ID value (only SVD) for the K from D */
    double m_kaonPionIDSVDonly = 0; /**<  pion ID value (only SVD) for the K from D */
    double m_kaonKaonIDSVDonly = 0; /**<  kaon ID value (only SVD) for the K from D */
    double m_kaonProtonIDSVDonly = 0; /**<  proton ID value (only SVD) for the K from D */

    double m_pionDElectronIDSVDonly = 0; /**<  electron ID value (only SVD) for the pi from D */
    double m_pionDPionIDSVDonly = 0; /**<  pion ID value (only SVD) for the pi from D */
    double m_pionDKaonIDSVDonly = 0; /**<  kaon ID value (only SVD) for the pi from D */
    double m_pionDProtonIDSVDonly = 0; /**<  proton ID value (only SVD) for the pi from D */

    double m_slowPionElectronIDSVDonly = 0; /**<  electron ID value (only SVD) for the pi from Dstar */
    double m_slowPionPionIDSVDonly = 0; /**<  pion ID value (only SVD) for the pi from Dstar */
    double m_slowPionKaonIDSVDonly = 0; /**<  kaon ID value (only SVD) for the pi from Dstar */
    double m_slowPionProtonIDSVDonly = 0; /**<  proton ID value (only SVD) for the pi from Dstar */

    double m_firstElectronElectronIDSVDonly = 0; /**<  electron ID value (only SVD) for the e+ from gamma */
    double m_firstElectronPionIDSVDonly = 0; /**<  pion ID value (only SVD) for the e+ from gamma */
    double m_firstElectronKaonIDSVDonly = 0; /**<  kaon ID value (only SVD) for the e+ from gamma */
    double m_firstElectronProtonIDSVDonly = 0; /**<  proton ID value (only SVD) for the e+ from gamma */

    double m_protonElectronLLSVDonly = 0; /**<  electron log-likelihood value (only SVD) for the proton from Lambda */
    double m_protonPionLLSVDonly = 0; /**<  pion log-likelihood value (only SVD) for the proton from Lambda */
    double m_protonKaonLLSVDonly = 0; /**<  kaon log-likelihood value (only SVD) for the proton from Lambda */
    double m_protonProtonLLSVDonly = 0; /**<  proton log-likelihood value (only SVD) for the proton from Lambda */

    double m_kaonElectronLLSVDonly = 0; /**<  electron log-likelihood value (only SVD) for the K from D */
    double m_kaonPionLLSVDonly = 0; /**<  pion log-likelihood value (only SVD) for the K from D */
    double m_kaonKaonLLSVDonly = 0; /**<  kaon log-likelihood value (only SVD) for the K from D */
    double m_kaonProtonLLSVDonly = 0; /**<  proton log-likelihood value (only SVD) for the K from D */

    double m_pionDElectronLLSVDonly = 0; /**<  electron log-likelihood value (only SVD) for the pi from D */
    double m_pionDPionLLSVDonly = 0; /**<  pion log-likelihood value (only SVD) for the pi from D */
    double m_pionDKaonLLSVDonly = 0; /**<  kaon log-likelihood value (only SVD) for the pi from D */
    double m_pionDProtonLLSVDonly = 0; /**<  proton log-likelihood value (only SVD) for the pi from D */

    double m_slowPionElectronLLSVDonly = 0; /**<  electron log-likelihood value (only SVD) for the pi from Dstar */
    double m_slowPionPionLLSVDonly = 0; /**<  pion log-likelihood value (only SVD) for the pi from Dstar */
    double m_slowPionKaonLLSVDonly = 0; /**<  kaon log-likelihood value (only SVD) for the pi from Dstar */
    double m_slowPionProtonLLSVDonly = 0; /**<  proton log-likelihood value (only SVD) for the pi from Dstar */

    double m_firstElectronElectronLLSVDonly = 0; /**<  electron log-likelihood value (only SVD) for the e+ from gamma */
    double m_firstElectronPionLLSVDonly = 0; /**<  pion log-likelihood value (only SVD) for the e+ from gamma */
    double m_firstElectronKaonLLSVDonly = 0; /**<  kaon log-likelihood value (only SVD) for the e+ from gamma */
    double m_firstElectronProtonLLSVDonly = 0; /**<  proton log-likelihood value (only SVD) for the e+ from gamma */

    double m_protonElectronIDnoSVD = 0; /**<  electron ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonPionIDnoSVD = 0; /**<  pion ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonKaonIDnoSVD = 0; /**<  kaon ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonProtonIDnoSVD = 0; /**<  proton ID value (all subdetectors except SVD) for the p from Lambda */

    double m_kaonElectronIDnoSVD = 0; /**<  electron ID value (all subdetectors except SVD) for the K from D */
    double m_kaonPionIDnoSVD = 0; /**<  pion ID value (all subdetectors except SVD) for the K from D */
    double m_kaonKaonIDnoSVD = 0; /**<  kaon ID value (all subdetectors except SVD) for the K from D */
    double m_kaonProtonIDnoSVD = 0; /**<  proton ID value (all subdetectors except SVD) for the K from D */

    double m_pionDElectronIDnoSVD = 0; /**<  electron ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDPionIDnoSVD = 0; /**<  pion ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDKaonIDnoSVD = 0; /**<  kaon ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDProtonIDnoSVD = 0; /**<  proton ID value (all subdetectors except SVD) for the pi from D */

    double m_slowPionElectronIDnoSVD = 0; /**<  electron ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionPionIDnoSVD = 0; /**<  pion ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionKaonIDnoSVD = 0; /**<  kaon ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionProtonIDnoSVD = 0; /**<  proton ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_firstElectronElectronIDnoSVD = 0; /**<  electron ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronPionIDnoSVD = 0; /**<  pion ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronKaonIDnoSVD = 0; /**<  kaon ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronProtonIDnoSVD = 0; /**<  proton ID value (all subdetectors except SVD) for the e+ from gamma */


    double m_protonBinaryProtonElectronIDALL = 0; /**<  binary p/e ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryProtonPionIDALL = 0; /**<  binary p/pi ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryProtonKaonIDALL = 0; /**<  binary p/K ID value (all subdetectors) for the p from Lambda */

    double m_protonBinaryKaonProtonIDALL = 0; /**<  binary K/p ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryPionProtonIDALL = 0; /**<  binary pi/p ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryElectronProtonIDALL = 0; /**<  binary e/p ID value (all subdetectors) for the p from Lambda */

    double m_kaonBinaryKaonElectronIDALL = 0; /**<  binary K/e ID value (all subdetectors) for the K from D */
    double m_kaonBinaryKaonPionIDALL = 0; /**<  binary K/pi ID value (all subdetectors) for the K from D */
    double m_kaonBinaryKaonProtonIDALL = 0; /**<  binary K/p ID value (all subdetectors) for the K from D */

    double m_kaonBinaryPionKaonIDALL = 0; /**<  binary pi/K ID value (all subdetectors) for the K from D */
    double m_kaonBinaryProtonKaonIDALL = 0; /**<  binary p/K ID value (all subdetectors) for the K from D */
    double m_kaonBinaryElectronKaonIDALL = 0; /**<  binary e/K ID value (all subdetectors) for the K from D */

    double m_pionDBinaryPionElectronIDALL = 0; /**<  binary pi/e ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryPionKaonIDALL = 0; /**<  binary pi/K ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryPionProtonIDALL = 0; /**<  binary pi/p ID value (all subdetectors) for the pi from D */

    double m_pionDBinaryElectronPionIDALL = 0; /**<  binary e/pi ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryKaonPionIDALL = 0; /**<  binary K/pi ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryProtonPionIDALL = 0; /**<  binary p/pi ID value (all subdetectors) for the pi from D */

    double m_slowPionBinaryPionElectronIDALL = 0; /**<  binary pi/e ID value (all subdetectors) for the pi from Dstar */
    double m_slowPionBinaryPionKaonIDALL = 0; /**<  binary pi/K ID value (all subdetectors) for the pi from Dstar */
    double m_slowPionBinaryPionProtonIDALL = 0; /**<  binary pi/p ID value (all subdetectors) for the pi from Dstar */

    double m_slowPionBinaryElectronPionIDALL = 0; /**<  binary e/pi ID value (all subdetectors) for the pi from Dstar */
    double m_slowPionBinaryKaonPionIDALL = 0; /**<  binary K/pi ID value (all subdetectors) for the pi from Dstar */
    double m_slowPionBinaryProtonPionIDALL = 0; /**<  binary p/pi ID value (all subdetectors) for the pi from Dstar */

    double m_firstElectronBinaryElectronPionIDALL = 0; /**<  binary e/pi ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryElectronKaonIDALL = 0; /**<  binary K/pi ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryElectronProtonIDALL = 0; /**<  binary p/pi ID value (all subdetectors) for the e+ from gamma */

    double m_firstElectronBinaryPionElectronIDALL = 0; /**<  binary pi/e ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryKaonElectronIDALL = 0; /**<  binary K/e ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryProtonElectronIDALL = 0; /**<  binary p/e ID value (all subdetectors) for the e+ from gamma */


    double m_protonBinaryProtonElectronIDnoSVD = 0; /**<  binary p/e ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryProtonPionIDnoSVD = 0; /**<  binary p/pi ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryProtonKaonIDnoSVD = 0; /**<  binary p/K ID value (all subdetectors except SVD) for the p from Lambda */

    double m_protonBinaryKaonProtonIDnoSVD = 0; /**<  binary K/p ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryPionProtonIDnoSVD = 0; /**<  binary pi/p ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryElectronProtonIDnoSVD = 0; /**<  binary e/p ID value (all subdetectors except SVD) for the p from Lambda */

    double m_kaonBinaryKaonElectronIDnoSVD = 0; /**<  binary K/e ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryKaonPionIDnoSVD = 0; /**<  binary K/pi ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryKaonProtonIDnoSVD = 0; /**<  binary K/p ID value (all subdetectors except SVD) for the K from D */

    double m_kaonBinaryPionKaonIDnoSVD = 0; /**<  binary pi/K ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryProtonKaonIDnoSVD = 0; /**<  binary p/K ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryElectronKaonIDnoSVD = 0; /**<  binary e/K ID value (all subdetectors except SVD) for the K from D */

    double m_pionDBinaryPionElectronIDnoSVD = 0; /**<  binary pi/e ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryPionKaonIDnoSVD = 0; /**<  binary pi/K ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryPionProtonIDnoSVD = 0; /**<  binary pi/p ID value (all subdetectors except SVD) for the pi from D */

    double m_pionDBinaryElectronPionIDnoSVD = 0; /**<  binary e/pi ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryKaonPionIDnoSVD = 0; /**<  binary K/pi ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryProtonPionIDnoSVD = 0; /**<  binary p/pi ID value (all subdetectors except SVD) for the pi from D */

    double m_slowPionBinaryPionElectronIDnoSVD = 0; /**<  binary pi/e ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionBinaryPionKaonIDnoSVD = 0; /**<  binary pi/K ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionBinaryPionProtonIDnoSVD = 0; /**<  binary pi/p ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_slowPionBinaryElectronPionIDnoSVD = 0; /**<  binary e/pi ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionBinaryKaonPionIDnoSVD = 0; /**<  binary K/pi ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionBinaryProtonPionIDnoSVD = 0; /**<  binary p/pi ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_firstElectronBinaryElectronPionIDnoSVD =
      0; /**<  binary e/pi ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronKaonIDnoSVD =
      0; /**<  binary e/K ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronProtonIDnoSVD =
      0; /**<  binary e/p ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_firstElectronBinaryPionElectronIDnoSVD =
      0; /**<  binary pi/e ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryKaonElectronIDnoSVD =
      0; /**<  binary K/e ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryProtonElectronIDnoSVD =
      0; /**<  binary p/e ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_protonBinaryProtonElectronIDSVDonly = 0; /**<  binary p/e ID value (only SVD) for the p from Lambda */
    double m_protonBinaryProtonPionIDSVDonly = 0; /**<  binary p/pi ID value (only SVD) for the p from Lambda */
    double m_protonBinaryProtonKaonIDSVDonly = 0; /**<  binary p/K ID value (only SVD) for the p from Lambda */

    double m_protonBinaryKaonProtonIDSVDonly = 0; /**<  binary K/p ID value (only SVD) for the p from Lambda */
    double m_protonBinaryPionProtonIDSVDonly = 0; /**<  binary pi/p ID value (only SVD) for the p from Lambda */
    double m_protonBinaryElectronProtonIDSVDonly = 0; /**<  binary e/p ID value (only SVD) for the p from Lambda */

    double m_kaonBinaryKaonElectronIDSVDonly = 0; /**<  binary K/e ID value (only SVD) for the K from D */
    double m_kaonBinaryKaonPionIDSVDonly = 0; /**<  binary K/pi ID value (only SVD) for the K from D */
    double m_kaonBinaryKaonProtonIDSVDonly = 0; /**<  binary K/ ID value (only SVD) for the K from D */

    double m_kaonBinaryPionKaonIDSVDonly = 0; /**<  binary pi/K ID value (only SVD) for the K from D */
    double m_kaonBinaryProtonKaonIDSVDonly = 0; /**<  binary p/K ID value (only SVD) for the K from D */
    double m_kaonBinaryElectronKaonIDSVDonly = 0; /**<  binary e/K ID value (only SVD) for the K from D */

    double m_pionDBinaryPionElectronIDSVDonly = 0; /**<  binary pi/e ID value (only SVD) for the pi from D */
    double m_pionDBinaryPionKaonIDSVDonly = 0; /**<  binary pi/K ID value (only SVD) for the pi from D */
    double m_pionDBinaryPionProtonIDSVDonly = 0; /**<  binary pi/p ID value (only SVD) for the pi from D */

    double m_pionDBinaryElectronPionIDSVDonly = 0; /**<  binary e/pi ID value (only SVD) for the pi from D */
    double m_pionDBinaryKaonPionIDSVDonly = 0; /**<  binary K/pi ID value (only SVD) for the pi from D */
    double m_pionDBinaryProtonPionIDSVDonly = 0; /**<  binary p/pi ID value (only SVD) for the pi from D */

    double m_slowPionBinaryPionElectronIDSVDonly = 0; /**<  binary pi/e ID value (only SVD) for the pi from Dstar */
    double m_slowPionBinaryPionKaonIDSVDonly = 0; /**<  binary pi/K ID value (only SVD) for the pi from Dstar */
    double m_slowPionBinaryPionProtonIDSVDonly = 0; /**<  binary pi/p ID value (only SVD) for the pi from Dstar */

    double m_slowPionBinaryElectronPionIDSVDonly = 0; /**<  binary e/pi ID value (only SVD) for the pi from Dstar */
    double m_slowPionBinaryKaonPionIDSVDonly = 0; /**<  binary K/pi ID value (only SVD) for the pi from Dstar */
    double m_slowPionBinaryProtonPionIDSVDonly = 0; /**<  binary p/pi ID value (only SVD) for the pi from Dstar */

    double m_firstElectronBinaryElectronPionIDSVDonly = 0; /**<  binary e/pi ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronKaonIDSVDonly = 0; /**<  binary e/K ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronProtonIDSVDonly = 0; /**<  binary e/p ID value (only SVD) for the e+ from gamma */

    double m_firstElectronBinaryPionElectronIDSVDonly = 0; /**<  binary pi/e ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryKaonElectronIDSVDonly = 0; /**<  binary K/e ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryProtonElectronIDSVDonly = 0; /**<  binary p/e ID value (only SVD) for the e+ from gamma */

    std::string m_LambdaListName = "";    /**< Name of the Lambda particle list */
    std::string m_DstarListName = "";    /**< Name of the Dstar particle list */
    std::string m_GammaListName = "";    /**< Name of the Gamma particle list */


  };

} // end namespace Belle2
