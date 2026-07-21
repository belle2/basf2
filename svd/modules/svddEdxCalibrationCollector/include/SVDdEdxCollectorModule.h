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

    int m_evt = 0; /**<  event number */
    int m_exp = 0; /**<  experiment number */
    int m_run = 0; /**<  run number */

    double m_time = 0; /**<  event time */


    double m_InvMLambda = 0;       /**<  Invariant mass of Lambda candidates */
    double m_CosDirAngleLambda = 0;        /**<  Cosine of the angle between momentum and vertex vectors of Lambda candidates */
    double m_protonMomentum = 0;       /**<   momentum for the proton from the Lambda */
    double m_protonSVDdEdx = 0;     /**<  SVD dE/dx response for the proton from the Lambda */
    double m_protonSVDdEdxErr = 0;     /**<  SVD dE/dx uncertainty for the proton from the Lambda */
    double m_protondEdxTrackMomentum = 0;      /**<   momentum for the proton from the Lambda, taken from the VXDDedxTrack */
    int m_protondEdxTrackNHits = 0;      /**<   number of hits for proton from the Lambda, taken from the VXDDedxTrack */
    double m_protondEdxTrackCosTheta = 0;      /**<   cosTheta for the proton from the Lambda, taken from the VXDDedxTrack */
    double m_protonnSVDHits = 0;   /**<  nSVDHits for the proton from the Lambda */

    double m_pionLambdap = 0;   /**<   momentum for the pion from the Lambda */
    double m_pionLambdaSVDdEdx = 0;   /**<  SVD dE/dx response for the pion from the Lambda */
    double m_pionLambdaSVDdEdxErr = 0;    /**<  SVD dE/dx uncertainty for the pion from the Lambda */
    double m_pionLambdadEdxTrackMomentum = 0;      /**<   momentum for the pion from the Lambda, taken from the VXDDedxTrack */
    int m_pionLambdadEdxTrackNHits = 0;      /**<   number of hits for pion from the Lambda, taken from the VXDDedxTrack */
    double m_pionLambdadEdxTrackCosTheta = 0;      /**<   cosTheta for the pion from the Lambda, taken from the VXDDedxTrack */
    double m_pionLambdanSVDHits = 0;   /**<  nSVDHits for the pion from the Lambda */

    double m_InvMDstar = 0;      /**<  Invariant mass of Dstar candidates */
    double m_InvMD0 = 0;      /**<  Invariant mass of D0 candidates */
    double m_DeltaM = 0;      /**<  deltaM = m(Dstar)-m(D0) */
    double m_CosDirAngleD0 = 0;       /**<  Cosine of the angle between momentum and vertex vectors of D0 candidates */
    double m_kaonMomentum = 0;      /**<   momentum for the kaon from the D0 */
    double m_kaonSVDdEdx = 0;    /**<  SVD dE/dx response for the kaon from the D0 */
    double m_kaonSVDdEdxErr = 0;    /**<  SVD dE/dx uncertainty for the kaon from the D0 */
    double m_kaondEdxTrackMomentum = 0;      /**<   momentum for the kaon from the D0, taken from the VXDDedxTrack */
    int m_kaondEdxTrackNHits = 0;      /**<   number of hits for kaon from the D0, taken from the VXDDedxTrack */
    double m_kaondEdxTrackCosTheta = 0;      /**<   cosTheta for the kaon from the D0, taken from the VXDDedxTrack */
    double m_kaonnSVDHits = 0;   /**<  nSVDHits for the kaon from the D0 */

    double m_pionDMomentum = 0;   /**<   momentum for the pion from the D0 */
    double m_pionDSVDdEdx = 0;   /**<  SVD dE/dx response for the pion from the D0 */
    double m_pionDSVDdEdxErr = 0;    /**<  SVD dE/dx uncertainty for the pion from the D0 */
    double m_pionDdEdxTrackMomentum = 0;      /**<   momentum for the pion from the D0, taken from the VXDDedxTrack */
    int m_pionDdEdxTrackNHits = 0;      /**<   number of hits for pion from the D0, taken from the VXDDedxTrack */
    double m_pionDdEdxTrackCosTheta = 0;      /**<   cosTheta for the pion from the D0, taken from the VXDDedxTrack */
    double m_pionDnSVDHits = 0;   /**<  nSVDHits for the pion from the D0 */

    double m_slowPionMomentum = 0;   /**<   momentum for the pion from the Dstar */
    double m_slowPionSVDdEdx = 0;   /**<  SVD dE/dx response for the pion from the Dstar */
    double m_slowPionSVDdEdxErr = 0;    /**<  SVD dE/dx uncertainty for the pion from the Dstar */
    double m_slowPiondEdxTrackMomentum = 0;      /**<   momentum for the pion from the Dstar, taken from the VXDDedxTrack */
    int m_slowPiondEdxTrackNHits = 0;      /**<   number of hits for pion from the Dstar, taken from the VXDDedxTrack */
    double m_slowPiondEdxTrackCosTheta = 0;      /**<   cosTheta for the pion from the Dstar, taken from the VXDDedxTrack */
    double m_slowPionnSVDHits = 0;   /**<  nSVDHits for the pion from the Dstar */

    double m_InvMGamma = 0;      /**<  Invariant mass of converted photon candidates */
    double m_drGamma = 0;       /**<  dr of converted photon candidates */
    double m_CosDirAngleGamma = 0;       /**<  Cosine of the angle between momentum and vertex vectors of converted photon candidates */
    double m_firstElectronMomentum = 0;      /**<   momentum for the first electron */
    double m_firstElectronSVDdEdx = 0;    /**<  SVD dE/dx response for the first electron */
    double m_firstElectronSVDdEdxErr = 0;    /**<  SVD dE/dx uncertainty for the first electron */
    double m_firstElectrondEdxTrackMomentum = 0;      /**<   momentum for the first electron, taken from the VXDDedxTrack */
    int m_firstElectrondEdxTrackNHits = 0;      /**<   number of hits for first electron, taken from the VXDDedxTrack */
    double m_firstElectrondEdxTrackCosTheta = 0;      /**<   cosTheta for the first electron, taken from the VXDDedxTrack */
    double m_firstElectronnSVDHits = 0;   /**<  nSVDHits for the first electron */

    double m_secondElectronMomentum = 0;   /**<   momentum for the second electron */
    double m_secondElectronSVDdEdx = 0;   /**<  SVD dE/dx response for the second electron */
    double m_secondElectronSVDdEdxErr = 0;    /**<  SVD dE/dx uncertainty for the second electron */
    double m_secondElectrondEdxTrackMomentum = 0;      /**<   momentum for the second electron, taken from the VXDDedxTrack */
    int m_secondElectrondEdxTrackNHits = 0;      /**<   number of hits for second electron, taken from the VXDDedxTrack */
    double m_secondElectrondEdxTrackCosTheta = 0;      /**<   cosTheta for the second electron, taken from the VXDDedxTrack */
    double m_secondElectronnSVDHits = 0;   /**<  nSVDHits for the second electron */

    double m_genericTrackMomentum = 0;   /**<   momentum for the generic track */
    double m_genericTrackSVDdEdx = 0;   /**<  SVD dE/dx for the generic track */
    double m_genericTracknSVDHits = 0;   /**<  nSVDHits for the generic track */
    double m_genericTrackSVDdEdxErr = 0;    /**<  SVD dE/dx uncertainty for the generic track */
    double m_genericTrackdEdxTrackMomentum = 0;      /**<   momentum for the generic track, taken from the VXDDedxTrack */
    int m_genericTrackdEdxTrackNHits = 0;      /**<   number of hits for generic track, taken from the VXDDedxTrack */
    double m_genericTrackdEdxTrackCosTheta = 0;      /**<   cosTheta for the generic track, taken from the VXDDedxTrack */

    std::string m_LambdaListName = "";    /**< Name of the Lambda particle list */
    std::string m_DstarListName = "";    /**< Name of the Dstar particle list */
    std::string m_GammaListName = "";    /**< Name of the Gamma particle list */
    std::string m_GenericListName = "";    /**< Name of the generic track particle list */

  };

} // end namespace Belle2
