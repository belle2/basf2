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
    double m_softPionp;    /**<   momentum for the pion from the Dstar */
    double m_softPionSVDdEdx;    /**<  SVD dE/dx response for the pion from the Dstar */

    double m_InvMGamma;       /**<  Invariant mass of converted photon candidates */
    double m_firstElectronp;       /**<   momentum for the first electron */
    double m_firstElectronSVDdEdx;     /**<  SVD dE/dx response for the first electron */
    double m_secondElectronp;    /**<   momentum for the second electron */
    double m_secondElectronSVDdEdx;    /**<  SVD dE/dx response for the second electron */

    double m_protonElectronID_ALL;  /**<  electron ID value (all subdetectors) for the proton from Lambda */
    double m_protonPionID_ALL;  /**<  pion ID value (all subdetectors) for the proton from Lambda */
    double m_protonKaonID_ALL;  /**<  kaon ID value (all subdetectors) for the proton from Lambda */
    double m_protonProtonID_ALL;  /**<  proton ID value (all subdetectors) for the proton from Lambda */

    double m_kaonElectronID_ALL;   /**<  electron ID value (all subdetectors) for the kaon from D */
    double m_kaonPionID_ALL;   /**<  pion ID value (all subdetectors) for the kaon from D */
    double m_kaonKaonID_ALL;   /**<  kaon ID value (all subdetectors) for the kaon from D */
    double m_kaonProtonID_ALL;   /**<  proton ID value (all subdetectors) for the kaon from D */

    double m_pionDElectronID_ALL;   /**<  electron ID value (all subdetectors) for the pion from D */
    double m_pionDPionID_ALL;   /**<  pion ID value (all subdetectors) for the pion from D */
    double m_pionDKaonID_ALL;   /**<  kaon ID value (all subdetectors) for the pion from D */
    double m_pionDProtonID_ALL;   /**<  proton ID value (all subdetectors) for the pion from D */

    double m_softPionElectronID_ALL;   /**<  electron ID value (all subdetectors) for the pion from Dstar */
    double m_softPionPionID_ALL;   /**<  pion ID value (all subdetectors) for the pion from Dstar */
    double m_softPionKaonID_ALL;   /**<  kaon ID value (all subdetectors) for the pion from Dstar */
    double m_softPionProtonID_ALL;   /**<  proton ID value (all subdetectors) for the pion from Dstar */

    double m_firstElectronElectronID_ALL;   /**<  electron ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronPionID_ALL;   /**<  pion ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronKaonID_ALL;   /**<  kaon ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronProtonID_ALL;   /**<  proton ID value (all subdetectors) for the e+ from gamma */

    double m_secondElectronElectronID_ALL;   /**<  electron ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronPionID_ALL;   /**<  pion ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronKaonID_ALL;   /**<  kaon ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronProtonID_ALL;   /**<  proton ID value (all subdetectors) for the e- from gamma */


    double m_protonElectronID_SVDonly;  /**<  electron ID value (only SVD) for the proton from Lambda */
    double m_protonPionID_SVDonly;  /**<  pion ID value (only SVD) for the proton from Lambda */
    double m_protonKaonID_SVDonly;  /**<  kaon ID value (only SVD) for the proton from Lambda */
    double m_protonProtonID_SVDonly;  /**<  proton ID value (only SVD) for the proton from Lambda */

    double m_kaonElectronID_SVDonly;  /**<  electron ID value (only SVD) for the K from D */
    double m_kaonPionID_SVDonly;  /**<  pion ID value (only SVD) for the K from D */
    double m_kaonKaonID_SVDonly;  /**<  kaon ID value (only SVD) for the K from D */
    double m_kaonProtonID_SVDonly;  /**<  proton ID value (only SVD) for the K from D */

    double m_pionDElectronID_SVDonly;  /**<  electron ID value (only SVD) for the pi from D */
    double m_pionDPionID_SVDonly;  /**<  pion ID value (only SVD) for the pi from D */
    double m_pionDKaonID_SVDonly;  /**<  kaon ID value (only SVD) for the pi from D */
    double m_pionDProtonID_SVDonly;  /**<  proton ID value (only SVD) for the pi from D */

    double m_softPionElectronID_SVDonly;  /**<  electron ID value (only SVD) for the pi from Dstar */
    double m_softPionPionID_SVDonly;  /**<  pion ID value (only SVD) for the pi from Dstar */
    double m_softPionKaonID_SVDonly;  /**<  kaon ID value (only SVD) for the pi from Dstar */
    double m_softPionProtonID_SVDonly;  /**<  proton ID value (only SVD) for the pi from Dstar */

    double m_firstElectronElectronID_SVDonly;  /**<  electron ID value (only SVD) for the e+ from gamma */
    double m_firstElectronPionID_SVDonly;  /**<  pion ID value (only SVD) for the e+ from gamma */
    double m_firstElectronKaonID_SVDonly;  /**<  kaon ID value (only SVD) for the e+ from gamma */
    double m_firstElectronProtonID_SVDonly;  /**<  proton ID value (only SVD) for the e+ from gamma */

    double m_secondElectronElectronID_SVDonly;  /**<  electron ID value (only SVD) for the e- from gamma */
    double m_secondElectronPionID_SVDonly;  /**<  pion ID value (only SVD) for the e- from gamma */
    double m_secondElectronKaonID_SVDonly;  /**<  kaon ID value (only SVD) for the e- from gamma */
    double m_secondElectronProtonID_SVDonly;  /**<  proton ID value (only SVD) for the e- from gamma */


    double m_protonElectronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonPionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonKaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonProtonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the p from Lambda */

    double m_kaonElectronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the K from D */
    double m_kaonPionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the K from D */
    double m_kaonKaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the K from D */
    double m_kaonProtonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the K from D */

    double m_pionDElectronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDPionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDKaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDProtonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the pi from D */

    double m_softPionElectronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_softPionPionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_softPionKaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_softPionProtonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_firstElectronElectronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronPionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronKaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronProtonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_secondElectronElectronID_noSVD;  /**<  electron ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronPionID_noSVD;  /**<  pion ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronKaonID_noSVD;  /**<  kaon ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronProtonID_noSVD;  /**<  proton ID value (all subdetectors except SVD) for the e- from gamma */


    double m_protonBinaryProtonElectronID_ALL;  /**<  binary p/e ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryProtonPionID_ALL;  /**<  binary p/pi ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryProtonKaonID_ALL;  /**<  binary p/K ID value (all subdetectors) for the p from Lambda */

    double m_protonBinaryKaonProtonID_ALL;  /**<  binary K/p ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryPionProtonID_ALL;  /**<  binary pi/p ID value (all subdetectors) for the p from Lambda */
    double m_protonBinaryElectronProtonID_ALL;  /**<  binary e/p ID value (all subdetectors) for the p from Lambda */

    double m_kaonBinaryKaonElectronID_ALL;  /**<  binary K/e ID value (all subdetectors) for the K from D */
    double m_kaonBinaryKaonPionID_ALL;  /**<  binary K/pi ID value (all subdetectors) for the K from D */
    double m_kaonBinaryKaonProtonID_ALL;  /**<  binary K/p ID value (all subdetectors) for the K from D */

    double m_kaonBinaryPionKaonID_ALL;  /**<  binary pi/K ID value (all subdetectors) for the K from D */
    double m_kaonBinaryProtonKaonID_ALL;  /**<  binary p/K ID value (all subdetectors) for the K from D */
    double m_kaonBinaryElectronKaonID_ALL;  /**<  binary e/K ID value (all subdetectors) for the K from D */

    double m_pionDBinaryPionElectronID_ALL;  /**<  binary pi/e ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryPionKaonID_ALL;  /**<  binary pi/K ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryPionProtonID_ALL;  /**<  binary pi/p ID value (all subdetectors) for the pi from D */

    double m_pionDBinaryElectronPionID_ALL;  /**<  binary e/pi ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryKaonPionID_ALL;  /**<  binary K/pi ID value (all subdetectors) for the pi from D */
    double m_pionDBinaryProtonPionID_ALL;  /**<  binary p/pi ID value (all subdetectors) for the pi from D */

    double m_softPionBinaryPionElectronID_ALL;  /**<  binary pi/e ID value (all subdetectors) for the pi from Dstar */
    double m_softPionBinaryPionKaonID_ALL;  /**<  binary pi/K ID value (all subdetectors) for the pi from Dstar */
    double m_softPionBinaryPionProtonID_ALL;  /**<  binary pi/p ID value (all subdetectors) for the pi from Dstar */

    double m_softPionBinaryElectronPionID_ALL;  /**<  binary e/pi ID value (all subdetectors) for the pi from Dstar */
    double m_softPionBinaryKaonPionID_ALL;  /**<  binary K/pi ID value (all subdetectors) for the pi from Dstar */
    double m_softPionBinaryProtonPionID_ALL;  /**<  binary p/pi ID value (all subdetectors) for the pi from Dstar */

    double m_firstElectronBinaryElectronPionID_ALL;  /**<  binary e/pi ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryElectronKaonID_ALL;  /**<  binary K/pi ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryElectronProtonID_ALL;  /**<  binary p/pi ID value (all subdetectors) for the e+ from gamma */

    double m_firstElectronBinaryPionElectronID_ALL;  /**<  binary pi/e ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryKaonElectronID_ALL;  /**<  binary K/e ID value (all subdetectors) for the e+ from gamma */
    double m_firstElectronBinaryProtonElectronID_ALL;  /**<  binary p/e ID value (all subdetectors) for the e+ from gamma */

    double m_secondElectronBinaryElectronPionID_ALL;  /**<  binary e/pi ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronBinaryElectronKaonID_ALL;  /**<  binary e/K ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronBinaryElectronProtonID_ALL;  /**<  binary e/p ID value (all subdetectors) for the e- from gamma */

    double m_secondElectronBinaryPionElectronID_ALL;  /**<  binary pi/e ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronBinaryKaonElectronID_ALL;  /**<  binary K/e ID value (all subdetectors) for the e- from gamma */
    double m_secondElectronBinaryProtonElectronID_ALL;  /**<  binary p/e ID value (all subdetectors) for the e- from gamma */



    double m_protonBinaryProtonElectronID_noSVD;  /**<  binary p/e ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryProtonPionID_noSVD;  /**<  binary p/pi ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryProtonKaonID_noSVD;  /**<  binary p/K ID value (all subdetectors except SVD) for the p from Lambda */

    double m_protonBinaryKaonProtonID_noSVD;  /**<  binary K/p ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryPionProtonID_noSVD;  /**<  binary pi/p ID value (all subdetectors except SVD) for the p from Lambda */
    double m_protonBinaryElectronProtonID_noSVD;  /**<  binary e/p ID value (all subdetectors except SVD) for the p from Lambda */

    double m_kaonBinaryKaonElectronID_noSVD;  /**<  binary K/e ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryKaonPionID_noSVD;  /**<  binary K/pi ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryKaonProtonID_noSVD;  /**<  binary K/p ID value (all subdetectors except SVD) for the K from D */

    double m_kaonBinaryPionKaonID_noSVD;  /**<  binary pi/K ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryProtonKaonID_noSVD;  /**<  binary p/K ID value (all subdetectors except SVD) for the K from D */
    double m_kaonBinaryElectronKaonID_noSVD;  /**<  binary e/K ID value (all subdetectors except SVD) for the K from D */

    double m_pionDBinaryPionElectronID_noSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryPionKaonID_noSVD;  /**<  binary pi/K ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryPionProtonID_noSVD;  /**<  binary pi/p ID value (all subdetectors except SVD) for the pi from D */

    double m_pionDBinaryElectronPionID_noSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryKaonPionID_noSVD;  /**<  binary K/pi ID value (all subdetectors except SVD) for the pi from D */
    double m_pionDBinaryProtonPionID_noSVD;  /**<  binary p/pi ID value (all subdetectors except SVD) for the pi from D */

    double m_softPionBinaryPionElectronID_noSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_softPionBinaryPionKaonID_noSVD;  /**<  binary pi/K ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_softPionBinaryPionProtonID_noSVD;  /**<  binary pi/p ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_softPionBinaryElectronPionID_noSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_softPionBinaryKaonPionID_noSVD;  /**<  binary K/pi ID value (all subdetectors except SVD) for the pi from Dstar */
    double m_softPionBinaryProtonPionID_noSVD;  /**<  binary p/pi ID value (all subdetectors except SVD) for the pi from Dstar */

    double m_firstElectronBinaryElectronPionID_noSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronKaonID_noSVD;  /**<  binary e/K ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronProtonID_noSVD;  /**<  binary e/p ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_firstElectronBinaryPionElectronID_noSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryKaonElectronID_noSVD;  /**<  binary K/e ID value (all subdetectors except SVD) for the e+ from gamma */
    double m_firstElectronBinaryProtonElectronID_noSVD;  /**<  binary p/e ID value (all subdetectors except SVD) for the e+ from gamma */

    double m_secondElectronBinaryElectronPionID_noSVD;  /**<  binary e/pi ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronBinaryElectronKaonID_noSVD;  /**<  binary e/K ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronBinaryElectronProtonID_noSVD;  /**<  binary e/p ID value (all subdetectors except SVD) for the e- from gamma */

    double m_secondElectronBinaryPionElectronID_noSVD;  /**<  binary pi/e ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronBinaryKaonElectronID_noSVD;  /**<  binary K/e ID value (all subdetectors except SVD) for the e- from gamma */
    double m_secondElectronBinaryProtonElectronID_noSVD;  /**<  binary p/e ID value (all subdetectors except SVD) for the e- from gamma */


    double m_protonBinaryProtonElectronID_SVDonly;  /**<  binary p/e ID value (only SVD) for the p from Lambda */
    double m_protonBinaryProtonPionID_SVDonly;  /**<  binary p/pi ID value (only SVD) for the p from Lambda */
    double m_protonBinaryProtonKaonID_SVDonly;  /**<  binary p/K ID value (only SVD) for the p from Lambda */

    double m_protonBinaryKaonProtonID_SVDonly;  /**<  binary K/p ID value (only SVD) for the p from Lambda */
    double m_protonBinaryPionProtonID_SVDonly;  /**<  binary pi/p ID value (only SVD) for the p from Lambda */
    double m_protonBinaryElectronProtonID_SVDonly;  /**<  binary e/p ID value (only SVD) for the p from Lambda */

    double m_kaonBinaryKaonElectronID_SVDonly;  /**<  binary K/e ID value (only SVD) for the K from D */
    double m_kaonBinaryKaonPionID_SVDonly;  /**<  binary K/pi ID value (only SVD) for the K from D */
    double m_kaonBinaryKaonProtonID_SVDonly;  /**<  binary K/ ID value (only SVD) for the K from D */

    double m_kaonBinaryPionKaonID_SVDonly;  /**<  binary pi/K ID value (only SVD) for the K from D */
    double m_kaonBinaryProtonKaonID_SVDonly;  /**<  binary p/K ID value (only SVD) for the K from D */
    double m_kaonBinaryElectronKaonID_SVDonly;  /**<  binary e/K ID value (only SVD) for the K from D */

    double m_pionDBinaryPionElectronID_SVDonly;  /**<  binary pi/e ID value (only SVD) for the pi from D */
    double m_pionDBinaryPionKaonID_SVDonly;  /**<  binary pi/K ID value (only SVD) for the pi from D */
    double m_pionDBinaryPionProtonID_SVDonly;  /**<  binary pi/p ID value (only SVD) for the pi from D */

    double m_pionDBinaryElectronPionID_SVDonly;  /**<  binary e/pi ID value (only SVD) for the pi from D */
    double m_pionDBinaryKaonPionID_SVDonly;  /**<  binary K/pi ID value (only SVD) for the pi from D */
    double m_pionDBinaryProtonPionID_SVDonly;  /**<  binary p/pi ID value (only SVD) for the pi from D */

    double m_softPionBinaryPionElectronID_SVDonly;  /**<  binary pi/e ID value (only SVD) for the pi from Dstar */
    double m_softPionBinaryPionKaonID_SVDonly;  /**<  binary pi/K ID value (only SVD) for the pi from Dstar */
    double m_softPionBinaryPionProtonID_SVDonly;  /**<  binary pi/p ID value (only SVD) for the pi from Dstar */

    double m_softPionBinaryElectronPionID_SVDonly;  /**<  binary e/pi ID value (only SVD) for the pi from Dstar */
    double m_softPionBinaryKaonPionID_SVDonly;  /**<  binary K/pi ID value (only SVD) for the pi from Dstar */
    double m_softPionBinaryProtonPionID_SVDonly;  /**<  binary p/pi ID value (only SVD) for the pi from Dstar */

    double m_firstElectronBinaryElectronPionID_SVDonly;  /**<  binary e/pi ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronKaonID_SVDonly;  /**<  binary e/K ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryElectronProtonID_SVDonly;  /**<  binary e/p ID value (only SVD) for the e+ from gamma */

    double m_firstElectronBinaryPionElectronID_SVDonly;  /**<  binary pi/e ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryKaonElectronID_SVDonly;  /**<  binary K/e ID value (only SVD) for the e+ from gamma */
    double m_firstElectronBinaryProtonElectronID_SVDonly;  /**<  binary p/e ID value (only SVD) for the e+ from gamma */

    double m_secondElectronBinaryElectronPionID_SVDonly;  /**<  binary e/pi ID value (only SVD) for the e- from gamma */
    double m_secondElectronBinaryElectronKaonID_SVDonly;  /**<  binary e/K ID value (only SVD) for the e- from gamma */
    double m_secondElectronBinaryElectronProtonID_SVDonly;  /**<  binary e/p ID value (only SVD) for the e- from gamma */

    double m_secondElectronBinaryPionElectronID_SVDonly;  /**<  binary pi/e ID value (only SVD) for the e- from gamma */
    double m_secondElectronBinaryKaonElectronID_SVDonly;  /**<  binary K/e ID value (only SVD) for the e- from gamma */
    double m_secondElectronBinaryProtonElectronID_SVDonly;  /**<  binary p/e ID value (only SVD) for the e- from gamma */

    std::string m_LambdaListName = "";    /**< Name of the Lambda particle list */
    std::string m_DstarListName = "";    /**< Name of the Dstar particle list */
    std::string m_GammaListName = "";    /**< Name of the Gamma particle list */


  };

} // end namespace Belle2
