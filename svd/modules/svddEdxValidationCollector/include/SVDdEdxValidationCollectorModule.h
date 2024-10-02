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
    double m_protonp;       /**<   momentum for the proton from the Lambda */
    double m_protonSVDdEdx;     /**<  SVD dE/dx response for the proton from the Lambda */
    double m_pionLambdap;    /**<   momentum for the pion from the Lambda */
    double m_pionLambdaSVDdEdx;    /**<  SVD dE/dx response for the pion from the Lambda */

    double m_InvMDstar;       /**<  Invariant mass of Dstar candidates */
    double m_InvMD0;       /**<  Invariant mass of D0 candidates */
    double m_DeltaM;       /**<  deltaM = m(Dstar)-m(D0) */
    double m_kaonp;       /**<   momentum for the kaon from the D0 */
    double m_kaonSVDdEdx;     /**<  SVD dE/dx response for the kaon from the D0 */
    double m_pionDp;    /**<   momentum for the pion from the D0 */
    double m_pionDSVDdEdx;    /**<  SVD dE/dx response for the pion from the D0 */
    double m_slowPionp;    /**<   momentum for the pion from the Dstar */
    double m_slowPionSVDdEdx;    /**<  SVD dE/dx response for the pion from the Dstar */

    double m_InvMGamma;       /**<  Invariant mass of converted photon candidates */
    double m_firstElectronp;       /**<   momentum for the first electron */
    double m_firstElectronSVDdEdx;     /**<  SVD dE/dx response for the first electron */
    double m_secondElectronp;    /**<   momentum for the second electron */
    double m_secondElectronSVDdEdx;    /**<  SVD dE/dx response for the second electron */

    double m_protonElectronIDALL;  /**<  electron ID value (all subdetectors) for the proton from Lambda */
    double m_protonPionIDALL;  /**<  pion ID value (all subdetectors) for the proton from Lambda */
    double m_protonKaonIDALL;  /**<  kaon ID value (all subdetectors) for the proton from Lambda */
    double m_protonProtonIDALL;  /**<  proton ID value (all subdetectors) for the proton from Lambda */

    double m_kaonElectronIDALL;   /**<  electron ID value (all subdetectors) for the kaon from D */
    double m_kaonPionIDALL;   /**<  pion ID value (all subdetectors) for the kaon from D */
    double m_kaonKaonIDALL;   /**<  kaon ID value (all subdetectors) for the kaon from D */
    double m_kaonProtonIDALL;   /**<  proton ID value (all subdetectors) for the kaon from D */

    double m_pionDElectronIDALL;   /**<  electron ID value (all subdetectors) for the pion from D */
    double m_pionDPionIDALL;   /**<  pion ID value (all subdetectors) for the pion from D */
    double m_pionDKaonIDALL;   /**<  kaon ID value (all subdetectors) for the pion from D */
    double m_pionDProtonIDALL;   /**<  proton ID value (all subdetectors) for the pion from D */

    double m_slowPionElectronIDALL;   /**<  electron ID value (all subdetectors) for the pion from Dstar */
    double m_slowPionPionIDALL;   /**<  pion ID value (all subdetectors) for the pion from Dstar */
    double m_slowPionKaonIDALL;   /**<  kaon ID value (all subdetectors) for the pion from Dstar */
    double m_slowPionProtonIDALL;   /**<  proton ID value (all subdetectors) for the pion from Dstar */

    double m_firstElectronElectronIDALL;   /**<  electron ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronPionIDALL;   /**<  pion ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronKaonIDALL;   /**<  kaon ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronProtonIDALL;   /**<  proton ID value (all subdetectors) for the e+ from gamma */

    double m_secondElectronElectronIDALL;   /**<  electron ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronPionIDALL;   /**<  pion ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronKaonIDALL;   /**<  kaon ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronProtonIDALL;   /**<  proton ID value (all subdetectors) for the e- from gamma */


    double m_protonElectronIDSVDonly;  /**<  electron ID value (only SVD) for the proton from Lambda */
    double m_protonPionIDSVDonly;  /**<  pion ID value (only SVD) for the proton from Lambda */
    double m_protonKaonIDSVDonly;  /**<  kaon ID value (only SVD) for the proton from Lambda */
    double m_protonProtonIDSVDonly;  /**<  proton ID value (only SVD) for the proton from Lambda */

    double m_kaonElectronIDSVDonly;  /**<  electron ID value (only SVD) for the K from D */
    double m_kaonPionIDSVDonly;  /**<  pion ID value (only SVD) for the K from D */
    double m_kaonKaonIDSVDonly;  /**<  kaon ID value (only SVD) for the K from D */
    double m_kaonProtonIDSVDonly;  /**<  proton ID value (only SVD) for the K from D */

    double m_pionDElectronIDSVDonly;  /**<  electron ID value (only SVD) for the pi from D */
    double m_pionDPionIDSVDonly;  /**<  pion ID value (only SVD) for the pi from D */
    double m_pionDKaonIDSVDonly;  /**<  kaon ID value (only SVD) for the pi from D */
    double m_pionDProtonIDSVDonly;  /**<  proton ID value (only SVD) for the pi from D */

    double m_slowPionElectronIDSVDonly;  /**<  electron ID value (only SVD) for the pi from Dstar */
    double m_slowPionPionIDSVDonly;  /**<  pion ID value (only SVD) for the pi from Dstar */
    double m_slowPionKaonIDSVDonly;  /**<  kaon ID value (only SVD) for the pi from Dstar */
    double m_slowPionProtonIDSVDonly;  /**<  proton ID value (only SVD) for the pi from Dstar */

    double m_firstElectronElectronIDSVDonly;  /**<  electron ID value (only SVD) for the e+ from gamma */
    double m_firstElectronPionIDSVDonly;  /**<  pion ID value (only SVD) for the e+ from gamma */
    double m_firstElectronKaonIDSVDonly;  /**<  kaon ID value (only SVD) for the e+ from gamma */
    double m_firstElectronProtonIDSVDonly;  /**<  proton ID value (only SVD) for the e+ from gamma */

    double m_secondElectronElectronIDSVDonly;  /**<  electron ID value (only SVD) for the e- from gamma */
    double m_secondElectronPionIDSVDonly;  /**<  pion ID value (only SVD) for the e- from gamma */
    double m_secondElectronKaonIDSVDonly;  /**<  kaon ID value (only SVD) for the e- from gamma */
    double m_secondElectronProtonIDSVDonly;  /**<  proton ID value (only SVD) for the e- from gamma */

    double m_protonElectronLLSVDonly;  /**<  electron log-likelihood value (only SVD) for the proton from Lambda */
    double m_protonPionLLSVDonly;  /**<  pion log-likelihood value (only SVD) for the proton from Lambda */
    double m_protonKaonLLSVDonly;  /**<  kaon log-likelihood value (only SVD) for the proton from Lambda */
    double m_protonProtonLLSVDonly;  /**<  proton log-likelihood value (only SVD) for the proton from Lambda */

    double m_kaonElectronLLSVDonly;  /**<  electron log-likelihood value (only SVD) for the K from D */
    double m_kaonPionLLSVDonly;  /**<  pion log-likelihood value (only SVD) for the K from D */
    double m_kaonKaonLLSVDonly;  /**<  kaon log-likelihood value (only SVD) for the K from D */
    double m_kaonProtonLLSVDonly;  /**<  proton log-likelihood value (only SVD) for the K from D */

    double m_pionDElectronLLSVDonly;  /**<  electron log-likelihood value (only SVD) for the pi from D */
    double m_pionDPionLLSVDonly;  /**<  pion log-likelihood value (only SVD) for the pi from D */
    double m_pionDKaonLLSVDonly;  /**<  kaon log-likelihood value (only SVD) for the pi from D */
    double m_pionDProtonLLSVDonly;  /**<  proton log-likelihood value (only SVD) for the pi from D */

    double m_slowPionElectronLLSVDonly;  /**<  electron log-likelihood value (only SVD) for the pi from Dstar */
    double m_slowPionPionLLSVDonly;  /**<  pion log-likelihood value (only SVD) for the pi from Dstar */
    double m_slowPionKaonLLSVDonly;  /**<  kaon log-likelihood value (only SVD) for the pi from Dstar */
    double m_slowPionProtonLLSVDonly;  /**<  proton log-likelihood value (only SVD) for the pi from Dstar */

    double m_firstElectronElectronLLSVDonly;  /**<  electron log-likelihood value (only SVD) for the e+ from gamma */
    double m_firstElectronPionLLSVDonly;  /**<  pion log-likelihood value (only SVD) for the e+ from gamma */
    double m_firstElectronKaonLLSVDonly;  /**<  kaon log-likelihood value (only SVD) for the e+ from gamma */
    double m_firstElectronProtonLLSVDonly;  /**<  proton log-likelihood value (only SVD) for the e+ from gamma */

    double m_secondElectronElectronLLSVDonly;  /**<  electron log-likelihood value (only SVD) for the e- from gamma */
    double m_secondElectronPionLLSVDonly;  /**<  pion log-likelihood value (only SVD) for the e- from gamma */
    double m_secondElectronKaonLLSVDonly;  /**<  kaon log-likelihood value (only SVD) for the e- from gamma */
    double m_secondElectronProtonLLSVDonly;  /**<  proton log-likelihood value (only SVD) for the e- from gamma */

    double m_protonElectronIDnoSVD;  /**<  electron ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonPionIDnoSVD;  /**<  pion ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonKaonIDnoSVD;  /**<  kaon ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonProtonIDnoSVD;  /**<  proton ID value (all subdetectors except SVD) for the p from Lambda */

    double m_kaonElectronIDnoSVD;  /**<  electron ID value (all subdetectors except SVD) for the K from D */
    double m_kaonPionIDnoSVD;  /**<  pion ID value (all subdetectors except SVD) for the K from D */
    double m_kaonKaonIDnoSVD;  /**<  kaon ID value (all subdetectors except SVD) for the K from D */
    double m_kaonProtonIDnoSVD;  /**<  proton ID value (all subdetectors except SVD) for the K from D */

    double m_pionDElectronIDnoSVD;  /**<  electron ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDPionIDnoSVD;  /**<  pion ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDKaonIDnoSVD;  /**<  kaon ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDProtonIDnoSVD;  /**<  proton ID value (all subdetectors except SVD) for the pi from D */

    double m_slowPionElectronIDnoSVD;  /**<  electron ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionPionIDnoSVD;  /**<  pion ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionKaonIDnoSVD;  /**<  kaon ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionProtonIDnoSVD;  /**<  proton ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_firstElectronElectronIDnoSVD;  /**<  electron ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronPionIDnoSVD;  /**<  pion ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronKaonIDnoSVD;  /**<  kaon ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronProtonIDnoSVD;  /**<  proton ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_secondElectronElectronIDnoSVD;  /**<  electron ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronPionIDnoSVD;  /**<  pion ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronKaonIDnoSVD;  /**<  kaon ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronProtonIDnoSVD;  /**<  proton ID value (all subdetectors except SVD) for the e- from gamma */


    double m_protonBinaryProtonElectronIDALL;  /**<  binary p/e ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryProtonPionIDALL;  /**<  binary p/pi ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryProtonKaonIDALL;  /**<  binary p/K ID value (all subdetectors) for the p from Lambda */

    double m_protonBinaryKaonProtonIDALL;  /**<  binary K/p ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryPionProtonIDALL;  /**<  binary pi/p ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryElectronProtonIDALL;  /**<  binary e/p ID value (all subdetectors) for the p from Lambda */

    double m_kaonBinaryKaonElectronIDALL;  /**<  binary K/e ID value (all subdetectors) for the K from D */
    double m_kaonBinaryKaonPionIDALL;  /**<  binary K/pi ID value (all subdetectors) for the K from D */
    double m_kaonBinaryKaonProtonIDALL;  /**<  binary K/p ID value (all subdetectors) for the K from D */

    double m_kaonBinaryPionKaonIDALL;  /**<  binary pi/K ID value (all subdetectors) for the K from D */
    double m_kaonBinaryProtonKaonIDALL;  /**<  binary p/K ID value (all subdetectors) for the K from D */
    double m_kaonBinaryElectronKaonIDALL;  /**<  binary e/K ID value (all subdetectors) for the K from D */

    double m_pionDBinaryPionElectronIDALL;  /**<  binary pi/e ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryPionKaonIDALL;  /**<  binary pi/K ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryPionProtonIDALL;  /**<  binary pi/p ID value (all subdetectors) for the pi from D */

    double m_pionDBinaryElectronPionIDALL;  /**<  binary e/pi ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryKaonPionIDALL;  /**<  binary K/pi ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryProtonPionIDALL;  /**<  binary p/pi ID value (all subdetectors) for the pi from D */

    double m_slowPionBinaryPionElectronIDALL;  /**<  binary pi/e ID value (all subdetectors) for the pi from Dstar */
    double m_slowPionBinaryPionKaonIDALL;  /**<  binary pi/K ID value (all subdetectors) for the pi from Dstar */
    double m_slowPionBinaryPionProtonIDALL;  /**<  binary pi/p ID value (all subdetectors) for the pi from Dstar */

    double m_slowPionBinaryElectronPionIDALL;  /**<  binary e/pi ID value (all subdetectors) for the pi from Dstar */
    double m_slowPionBinaryKaonPionIDALL;  /**<  binary K/pi ID value (all subdetectors) for the pi from Dstar */
    double m_slowPionBinaryProtonPionIDALL;  /**<  binary p/pi ID value (all subdetectors) for the pi from Dstar */

    double m_firstElectronBinaryElectronPionIDALL;  /**<  binary e/pi ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryElectronKaonIDALL;  /**<  binary K/pi ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryElectronProtonIDALL;  /**<  binary p/pi ID value (all subdetectors) for the e+ from gamma */

    double m_firstElectronBinaryPionElectronIDALL;  /**<  binary pi/e ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryKaonElectronIDALL;  /**<  binary K/e ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryProtonElectronIDALL;  /**<  binary p/e ID value (all subdetectors) for the e+ from gamma */

    double m_secondElectronBinaryElectronPionIDALL;  /**<  binary e/pi ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronBinaryElectronKaonIDALL;  /**<  binary e/K ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronBinaryElectronProtonIDALL;  /**<  binary e/p ID value (all subdetectors) for the e- from gamma */

    double m_secondElectronBinaryPionElectronIDALL;  /**<  binary pi/e ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronBinaryKaonElectronIDALL;  /**<  binary K/e ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronBinaryProtonElectronIDALL;  /**<  binary p/e ID value (all subdetectors) for the e- from gamma */



    double m_protonBinaryProtonElectronIDnoSVD;  /**<  binary p/e ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryProtonPionIDnoSVD;  /**<  binary p/pi ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryProtonKaonIDnoSVD;  /**<  binary p/K ID value (all subdetectors except SVD) for the p from Lambda */

    double m_protonBinaryKaonProtonIDnoSVD;  /**<  binary K/p ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryPionProtonIDnoSVD;  /**<  binary pi/p ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryElectronProtonIDnoSVD;  /**<  binary e/p ID value (all subdetectors except SVD) for the p from Lambda */

    double m_kaonBinaryKaonElectronIDnoSVD;  /**<  binary K/e ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryKaonPionIDnoSVD;  /**<  binary K/pi ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryKaonProtonIDnoSVD;  /**<  binary K/p ID value (all subdetectors except SVD) for the K from D */

    double m_kaonBinaryPionKaonIDnoSVD;  /**<  binary pi/K ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryProtonKaonIDnoSVD;  /**<  binary p/K ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryElectronKaonIDnoSVD;  /**<  binary e/K ID value (all subdetectors except SVD) for the K from D */

    double m_pionDBinaryPionElectronIDnoSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryPionKaonIDnoSVD;  /**<  binary pi/K ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryPionProtonIDnoSVD;  /**<  binary pi/p ID value (all subdetectors except SVD) for the pi from D */

    double m_pionDBinaryElectronPionIDnoSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryKaonPionIDnoSVD;  /**<  binary K/pi ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryProtonPionIDnoSVD;  /**<  binary p/pi ID value (all subdetectors except SVD) for the pi from D */

    double m_slowPionBinaryPionElectronIDnoSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionBinaryPionKaonIDnoSVD;  /**<  binary pi/K ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionBinaryPionProtonIDnoSVD;  /**<  binary pi/p ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_slowPionBinaryElectronPionIDnoSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionBinaryKaonPionIDnoSVD;  /**<  binary K/pi ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_slowPionBinaryProtonPionIDnoSVD;  /**<  binary p/pi ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_firstElectronBinaryElectronPionIDnoSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronKaonIDnoSVD;  /**<  binary e/K ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronProtonIDnoSVD;  /**<  binary e/p ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_firstElectronBinaryPionElectronIDnoSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryKaonElectronIDnoSVD;  /**<  binary K/e ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryProtonElectronIDnoSVD;  /**<  binary p/e ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_secondElectronBinaryElectronPionIDnoSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronBinaryElectronKaonIDnoSVD;  /**<  binary e/K ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronBinaryElectronProtonIDnoSVD;  /**<  binary e/p ID value (all subdetectors except SVD) for the e- from gamma */

    double m_secondElectronBinaryPionElectronIDnoSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronBinaryKaonElectronIDnoSVD;  /**<  binary K/e ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronBinaryProtonElectronIDnoSVD;  /**<  binary p/e ID value (all subdetectors except SVD) for the e- from gamma */


    double m_protonBinaryProtonElectronIDSVDonly;  /**<  binary p/e ID value (only SVD) for the p from Lambda */
    double m_protonBinaryProtonPionIDSVDonly;  /**<  binary p/pi ID value (only SVD) for the p from Lambda */
    double m_protonBinaryProtonKaonIDSVDonly;  /**<  binary p/K ID value (only SVD) for the p from Lambda */

    double m_protonBinaryKaonProtonIDSVDonly;  /**<  binary K/p ID value (only SVD) for the p from Lambda */
    double m_protonBinaryPionProtonIDSVDonly;  /**<  binary pi/p ID value (only SVD) for the p from Lambda */
    double m_protonBinaryElectronProtonIDSVDonly;  /**<  binary e/p ID value (only SVD) for the p from Lambda */

    double m_kaonBinaryKaonElectronIDSVDonly;  /**<  binary K/e ID value (only SVD) for the K from D */
    double m_kaonBinaryKaonPionIDSVDonly;  /**<  binary K/pi ID value (only SVD) for the K from D */
    double m_kaonBinaryKaonProtonIDSVDonly;  /**<  binary K/ ID value (only SVD) for the K from D */

    double m_kaonBinaryPionKaonIDSVDonly;  /**<  binary pi/K ID value (only SVD) for the K from D */
    double m_kaonBinaryProtonKaonIDSVDonly;  /**<  binary p/K ID value (only SVD) for the K from D */
    double m_kaonBinaryElectronKaonIDSVDonly;  /**<  binary e/K ID value (only SVD) for the K from D */

    double m_pionDBinaryPionElectronIDSVDonly;  /**<  binary pi/e ID value (only SVD) for the pi from D */
    double m_pionDBinaryPionKaonIDSVDonly;  /**<  binary pi/K ID value (only SVD) for the pi from D */
    double m_pionDBinaryPionProtonIDSVDonly;  /**<  binary pi/p ID value (only SVD) for the pi from D */

    double m_pionDBinaryElectronPionIDSVDonly;  /**<  binary e/pi ID value (only SVD) for the pi from D */
    double m_pionDBinaryKaonPionIDSVDonly;  /**<  binary K/pi ID value (only SVD) for the pi from D */
    double m_pionDBinaryProtonPionIDSVDonly;  /**<  binary p/pi ID value (only SVD) for the pi from D */

    double m_slowPionBinaryPionElectronIDSVDonly;  /**<  binary pi/e ID value (only SVD) for the pi from Dstar */
    double m_slowPionBinaryPionKaonIDSVDonly;  /**<  binary pi/K ID value (only SVD) for the pi from Dstar */
    double m_slowPionBinaryPionProtonIDSVDonly;  /**<  binary pi/p ID value (only SVD) for the pi from Dstar */

    double m_slowPionBinaryElectronPionIDSVDonly;  /**<  binary e/pi ID value (only SVD) for the pi from Dstar */
    double m_slowPionBinaryKaonPionIDSVDonly;  /**<  binary K/pi ID value (only SVD) for the pi from Dstar */
    double m_slowPionBinaryProtonPionIDSVDonly;  /**<  binary p/pi ID value (only SVD) for the pi from Dstar */

    double m_firstElectronBinaryElectronPionIDSVDonly;  /**<  binary e/pi ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronKaonIDSVDonly;  /**<  binary e/K ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronProtonIDSVDonly;  /**<  binary e/p ID value (only SVD) for the e+ from gamma */

    double m_firstElectronBinaryPionElectronIDSVDonly;  /**<  binary pi/e ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryKaonElectronIDSVDonly;  /**<  binary K/e ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryProtonElectronIDSVDonly;  /**<  binary p/e ID value (only SVD) for the e+ from gamma */

    double m_secondElectronBinaryElectronPionIDSVDonly;  /**<  binary e/pi ID value (only SVD) for the e- from gamma */
    double m_secondElectronBinaryElectronKaonIDSVDonly;  /**<  binary e/K ID value (only SVD) for the e- from gamma */
    double m_secondElectronBinaryElectronProtonIDSVDonly;  /**<  binary e/p ID value (only SVD) for the e- from gamma */

    double m_secondElectronBinaryPionElectronIDSVDonly;  /**<  binary pi/e ID value (only SVD) for the e- from gamma */
    double m_secondElectronBinaryKaonElectronIDSVDonly;  /**<  binary K/e ID value (only SVD) for the e- from gamma */
    double m_secondElectronBinaryProtonElectronIDSVDonly;  /**<  binary p/e ID value (only SVD) for the e- from gamma */

    std::string m_LambdaListName = "";    /**< Name of the Lambda particle list */
    std::string m_DstarListName = "";    /**< Name of the Dstar particle list */
    std::string m_GammaListName = "";    /**< Name of the Gamma particle list */


  };

} // end namespace Belle2
