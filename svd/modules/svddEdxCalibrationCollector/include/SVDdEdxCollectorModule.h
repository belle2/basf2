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
    double m_CosDirAngleLambda;        /**<  Cosine of the angle between momentum and vertex vectors of Lambda candidates */
    double m_protonMomentum;       /**<   momentum for the proton from the Lambda */
    double m_protonSVDdEdx;     /**<  SVD dE/dx response for the proton from the Lambda */
    double m_protonSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the proton from the Lambda */
    double m_protondEdxTrackMomentum;       /**<   momentum for the proton from the Lambda, taken from the VXDDedxTrack */
    int m_protondEdxTrackNHits;       /**<   number of hits for proton from the Lambda, taken from the VXDDedxTrack */
    double m_protondEdxTrackCosTheta;       /**<   cosTheta for the proton from the Lambda, taken from the VXDDedxTrack */
    double m_protonnSVDHits;    /**<  nSVDHits for the proton from the Lambda */

    double m_pionLambdap;    /**<   momentum for the pion from the Lambda */
    double m_pionLambdaSVDdEdx;    /**<  SVD dE/dx response for the pion from the Lambda */
    double m_pionLambdaSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the pion from the Lambda */
    double m_pionLambdadEdxTrackMomentum;       /**<   momentum for the pion from the Lambda, taken from the VXDDedxTrack */
    int m_pionLambdadEdxTrackNHits;       /**<   number of hits for pion from the Lambda, taken from the VXDDedxTrack */
    double m_pionLambdadEdxTrackCosTheta;       /**<   cosTheta for the pion from the Lambda, taken from the VXDDedxTrack */
    double m_pionLambdanSVDHits;    /**<  nSVDHits for the pion from the Lambda */

    double m_InvMDstar;       /**<  Invariant mass of Dstar candidates */
    double m_InvMD0;       /**<  Invariant mass of D0 candidates */
    double m_DeltaM;       /**<  deltaM = m(Dstar)-m(D0) */
    double m_CosDirAngleD0;        /**<  Cosine of the angle between momentum and vertex vectors of D0 candidates */
    double m_kaonMomentum;       /**<   momentum for the kaon from the D0 */
    double m_kaonSVDdEdx;     /**<  SVD dE/dx response for the kaon from the D0 */
    double m_kaonSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the kaon from the D0 */
    double m_kaondEdxTrackMomentum;       /**<   momentum for the kaon from the D0, taken from the VXDDedxTrack */
    int m_kaondEdxTrackNHits;       /**<   number of hits for kaon from the D0, taken from the VXDDedxTrack */
    double m_kaondEdxTrackCosTheta;       /**<   cosTheta for the kaon from the D0, taken from the VXDDedxTrack */
    double m_kaonnSVDHits;    /**<  nSVDHits for the kaon from the D0 */

    double m_pionDMomentum;    /**<   momentum for the pion from the D0 */
    double m_pionDSVDdEdx;    /**<  SVD dE/dx response for the pion from the D0 */
    double m_pionDSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the pion from the D0 */
    double m_pionDdEdxTrackMomentum;       /**<   momentum for the pion from the D0, taken from the VXDDedxTrack */
    int m_pionDdEdxTrackNHits;       /**<   number of hits for pion from the D0, taken from the VXDDedxTrack */
    double m_pionDdEdxTrackCosTheta;       /**<   cosTheta for the pion from the D0, taken from the VXDDedxTrack */
    double m_pionDnSVDHits;    /**<  nSVDHits for the pion from the D0 */

    double m_slowPionMomentum;    /**<   momentum for the pion from the Dstar */
    double m_slowPionSVDdEdx;    /**<  SVD dE/dx response for the pion from the Dstar */
    double m_slowPionSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the pion from the Dstar */
    double m_slowPiondEdxTrackMomentum;       /**<   momentum for the pion from the Dstar, taken from the VXDDedxTrack */
    int m_slowPiondEdxTrackNHits;       /**<   number of hits for pion from the Dstar, taken from the VXDDedxTrack */
    double m_slowPiondEdxTrackCosTheta;       /**<   cosTheta for the pion from the Dstar, taken from the VXDDedxTrack */
    double m_slowPionnSVDHits;    /**<  nSVDHits for the pion from the Dstar */

    double m_InvMGamma;       /**<  Invariant mass of converted photon candidates */
    double m_drGamma;        /**<  dr of converted photon candidates */
    double m_CosDirAngleGamma;        /**<  Cosine of the angle between momentum and vertex vectors of converted photon candidates */
    double m_firstElectronMomentum;       /**<   momentum for the first electron */
    double m_firstElectronSVDdEdx;     /**<  SVD dE/dx response for the first electron */
    double m_firstElectronSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the first electron */
    double m_firstElectrondEdxTrackMomentum;       /**<   momentum for the first electron, taken from the VXDDedxTrack */
    int m_firstElectrondEdxTrackNHits;       /**<   number of hits for first electron, taken from the VXDDedxTrack */
    double m_firstElectrondEdxTrackCosTheta;       /**<   cosTheta for the first electron, taken from the VXDDedxTrack */
    double m_firstElectronnSVDHits;    /**<  nSVDHits for the first electron */

    double m_secondElectronMomentum;    /**<   momentum for the second electron */
    double m_secondElectronSVDdEdx;    /**<  SVD dE/dx response for the second electron */
    double m_secondElectronSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the second electron */
    double m_secondElectrondEdxTrackMomentum;       /**<   momentum for the second electron, taken from the VXDDedxTrack */
    int m_secondElectrondEdxTrackNHits;       /**<   number of hits for second electron, taken from the VXDDedxTrack */
    double m_secondElectrondEdxTrackCosTheta;       /**<   cosTheta for the second electron, taken from the VXDDedxTrack */
    double m_secondElectronnSVDHits;    /**<  nSVDHits for the second electron */

    double m_genericTrackMomentum;    /**<   momentum for the generic track */
    double m_genericTrackSVDdEdx;    /**<  SVD dE/dx for the generic track */
    double m_genericTracknSVDHits;    /**<  nSVDHits for the generic track */
    double m_genericTrackSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the generic track */
    double m_genericTrackdEdxTrackMomentum;       /**<   momentum for the generic track, taken from the VXDDedxTrack */
    int m_genericTrackdEdxTrackNHits;       /**<   number of hits for generic track, taken from the VXDDedxTrack */
    double m_genericTrackdEdxTrackCosTheta;       /**<   cosTheta for the generic track, taken from the VXDDedxTrack */

    std::string m_LambdaListName = "";    /**< Name of the Lambda particle list */
    std::string m_DstarListName = "";    /**< Name of the Dstar particle list */
    std::string m_GammaListName = "";    /**< Name of the Gamma particle list */
    std::string m_GenericListName = "";    /**< Name of the generic track particle list */

  };

} // end namespace Belle2
