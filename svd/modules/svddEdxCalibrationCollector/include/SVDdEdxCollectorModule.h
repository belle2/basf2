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
    double m_diraLambda;        /**<  direction angle of Lambda candidates */
    double m_protonMomentum;       /**<   momentum for the proton from the Lambda */
    double m_protonSVDdEdx;     /**<  SVD dE/dx response for the proton from the Lambda */
    double m_protonSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the proton from the Lambda */
    // double m_protonSVDdEdxMean;     /**<  SVD dE/dx response (non-truncated) for the proton from the Lambda */
    double m_protondEdxTrackMomentum;       /**<   momentum for the proton from the Lambda, taken from the VXDDedxTrack */
    int m_protondEdxTrackNHits;       /**<   number of hits for proton from the Lambda, taken from the VXDDedxTrack */
    // int m_protondEdxTrackNHitsUsed;       /**<   number of hits used for dEdx truncated mean for proton from the Lambda, taken from the VXDDedxTrack */
    double m_protondEdxTrackCosTheta;       /**<   cosTheta for the proton from the Lambda, taken from the VXDDedxTrack */
    // std::vector<double> m_protonSVDdEdxList{};       /**<   SVD dE/dx response for the proton from the Lambda, per hit */
    double m_protonnSVDHits;    /**<  nSVDHits for the proton from the Lambda */

    double m_pionLambdap;    /**<   momentum for the pion from the Lambda */
    double m_pionLambdaSVDdEdx;    /**<  SVD dE/dx response for the pion from the Lambda */
    double m_pionLambdaSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the pion from the Lambda */
    // double m_pionLambdaSVDdEdxMean;     /**<  SVD dE/dx response (non-truncated) for the pion from the Lambda */
    double m_pionLambdadEdxTrackMomentum;       /**<   momentum for the pion from the Lambda, taken from the VXDDedxTrack */
    int m_pionLambdadEdxTrackNHits;       /**<   number of hits for pion from the Lambda, taken from the VXDDedxTrack */
    // int m_pionLambdadEdxTrackNHitsUsed;       /**<   number of hits used for dEdx truncated mean for pion from the Lambda, taken from the VXDDedxTrack */
    double m_pionLambdadEdxTrackCosTheta;       /**<   cosTheta for the pion from the Lambda, taken from the VXDDedxTrack */
    // std::vector<double> m_pionLambdaSVDdEdxList;       /**<   SVD dE/dx response for the pion from the Lambda, per hit */
    double m_pionLambdanSVDHits;    /**<  nSVDHits for the pion from the Lambda */

    double m_InvMDstar;       /**<  Invariant mass of Dstar candidates */
    double m_InvMD0;       /**<  Invariant mass of D0 candidates */
    // double m_D0Momentum;       /**<  Momentum of D0 candidates */
    double m_DeltaM;       /**<  deltaM = m(Dstar)-m(D0) */
    double m_diraD0;        /**<  direction angle of D0 candidates */
    double m_kaonMomentum;       /**<   momentum for the kaon from the D0 */
    // double m_kaonMomentumX;       /**<   momentum for the kaon from the D0 */
    // double m_kaonMomentumY;       /**<   momentum for the kaon from the D0 */
    // double m_kaonMomentumZ;       /**<   momentum for the kaon from the D0 */
    double m_kaonSVDdEdx;     /**<  SVD dE/dx response for the kaon from the D0 */
    double m_kaonSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the kaon from the D0 */
    // double m_kaonSVDdEdxMean;     /**<  SVD dE/dx response (non-truncated) for the kaon from the D0 */
    double m_kaondEdxTrackMomentum;       /**<   momentum for the kaon from the D0, taken from the VXDDedxTrack */
    int m_kaondEdxTrackNHits;       /**<   number of hits for kaon from the D0, taken from the VXDDedxTrack */
    // int m_kaondEdxTrackNHitsUsed;       /**<   number of hits used for dEdx truncated mean for kaon from the D0, taken from the VXDDedxTrack */
    double m_kaondEdxTrackCosTheta;       /**<   cosTheta for the kaon from the D0, taken from the VXDDedxTrack */
    // std::vector<double> m_kaonSVDdEdxList;       /**<   SVD dE/dx response for the kaon from the D0, per hit */
    double m_kaonnSVDHits;    /**<  nSVDHits for the kaon from the D0 */

    double m_pionDp;    /**<   momentum for the pion from the D0 */
    // double m_pionDMomentumX;       /**<   momentum for the pion from the D0 */
    // double m_pionDMomentumY;       /**<   momentum for the pion from the D0 */
    // double m_pionDMomentumZ;       /**<   momentum for the pion from the D0 */
    double m_pionDSVDdEdx;    /**<  SVD dE/dx response for the pion from the D0 */
    double m_pionDSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the pion from the D0 */
    // double m_pionDSVDdEdxMean;     /**<  SVD dE/dx response (non-truncated) for the pion from the D0 */
    double m_pionDdEdxTrackMomentum;       /**<   momentum for the pion from the D0, taken from the VXDDedxTrack */
    int m_pionDdEdxTrackNHits;       /**<   number of hits for pion from the D0, taken from the VXDDedxTrack */
    // int m_pionDdEdxTrackNHitsUsed;       /**<   number of hits used for dEdx truncated mean for pion from the D0, taken from the VXDDedxTrack */
    double m_pionDdEdxTrackCosTheta;       /**<   cosTheta for the pion from the D0, taken from the VXDDedxTrack */
    // std::vector<double> m_pionDSVDdEdxList;       /**<   SVD dE/dx response for the pion from the D0, per hit */
    double m_pionDnSVDHits;    /**<  nSVDHits for the pion from the D0 */

    double m_slowPionMomentum;    /**<   momentum for the pion from the Dstar */
    // double m_slowPionMomentumX;       /**<   momentum for the pion from the Dstar */
    // double m_slowPionMomentumY;       /**<   momentum for the pion from the Dstar */
    // double m_slowPionMomentumZ;       /**<   momentum for the pion from the Dstar */
    double m_slowPionSVDdEdx;    /**<  SVD dE/dx response for the pion from the Dstar */
    double m_slowPionSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the pion from the Dstar */
    // double m_slowPionSVDdEdxMean;     /**<  SVD dE/dx response (non-truncated) for the pion from the Dstar */
    double m_slowPiondEdxTrackMomentum;       /**<   momentum for the pion from the Dstar, taken from the VXDDedxTrack */
    int m_slowPiondEdxTrackNHits;       /**<   number of hits for pion from the Dstar, taken from the VXDDedxTrack */
    // int m_slowPiondEdxTrackNHitsUsed;       /**<   number of hits used for dEdx truncated mean for pion from the Dstar, taken from the VXDDedxTrack */
    double m_slowPiondEdxTrackCosTheta;       /**<   cosTheta for the pion from the Dstar, taken from the VXDDedxTrack */
    // std::vector<double> m_slowPionSVDdEdxList;       /**<   SVD dE/dx response for the pion from the Dstar, per hit */
    double m_slowPionnSVDHits;    /**<  nSVDHits for the pion from the Dstar */

    double m_InvMGamma;       /**<  Invariant mass of converted photon candidates */
    double m_drGamma;        /**<  dr of converted photon candidates */
    double m_diraGamma;        /**<  direction angle of converted photon candidates */
    double m_firstElectronMomentum;       /**<   momentum for the first electron */
    double m_firstElectronSVDdEdx;     /**<  SVD dE/dx response for the first electron */
    double m_firstElectronSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the first electron */
    // double m_firstElectronSVDdEdxMean;     /**<  SVD dE/dx response (non-truncated) for the first electron */
    double m_firstElectrondEdxTrackMomentum;       /**<   momentum for the first electron, taken from the VXDDedxTrack */
    int m_firstElectrondEdxTrackNHits;       /**<   number of hits for first electron, taken from the VXDDedxTrack */
    // int m_firstElectrondEdxTrackNHitsUsed;       /**<   number of hits used for dEdx truncated mean for first electron, taken from the VXDDedxTrack */
    double m_firstElectrondEdxTrackCosTheta;       /**<   cosTheta for the first electron, taken from the VXDDedxTrack */
    // std::vector<double> m_firstElectronSVDdEdxList;       /**<   SVD dE/dx response for the first electron, per hit */
    double m_firstElectronnSVDHits;    /**<  nSVDHits for the first electron */

    double m_secondElectronMomentum;    /**<   momentum for the second electron */
    double m_secondElectronSVDdEdx;    /**<  SVD dE/dx response for the second electron */
    double m_secondElectronSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the second electron */
    // double m_secondElectronSVDdEdxMean;     /**<  SVD dE/dx response (non-truncated) for the second electron */
    double m_secondElectrondEdxTrackMomentum;       /**<   momentum for the second electron, taken from the VXDDedxTrack */
    int m_secondElectrondEdxTrackNHits;       /**<   number of hits for second electron, taken from the VXDDedxTrack */
    // int m_secondElectrondEdxTrackNHitsUsed;       /**<   number of hits used for dEdx truncated mean for second electron, taken from the VXDDedxTrack */
    double m_secondElectrondEdxTrackCosTheta;       /**<   cosTheta for the second electron, taken from the VXDDedxTrack */
    // std::vector<double> m_secondElectronSVDdEdxList;       /**<   SVD dE/dx response for the second electron, per hit */
    double m_secondElectronnSVDHits;    /**<  nSVDHits for the second electron */

    double m_genericp;    /**<   momentum for the generic track */
    double m_genericSVDdEdx;    /**<  SVD dE/dx for the generic track */
    double m_genericnSVDHits;    /**<  nSVDHits for the generic track */
    double m_genericSVDdEdxErr;     /**<  SVD dE/dx uncertainty for the generic track */
    // double m_genericSVDdEdxMean;     /**<  SVD dE/dx response (non-truncated) for the generic track */
    double m_genericdEdxTrackMomentum;       /**<   momentum for the generic track, taken from the VXDDedxTrack */
    int m_genericdEdxTrackNHits;       /**<   number of hits for generic track, taken from the VXDDedxTrack */
    // int m_genericdEdxTrackNHitsUsed;       /**<   number of hits used for dEdx truncated mean for generic track, taken from the VXDDedxTrack */
    double m_genericdEdxTrackCosTheta;       /**<   cosTheta for the generic track, taken from the VXDDedxTrack */
    // std::vector<double> m_genericSVDdEdxList;       /**<   SVD dE/dx response for the generic track, per hit */

    std::string m_LambdaListName = "";    /**< Name of the Lambda particle list */
    std::string m_DstarListName = "";    /**< Name of the Dstar particle list */
    std::string m_GammaListName = "";    /**< Name of the Gamma particle list */
    std::string m_GenericListName = "";    /**< Name of the pion particle list */

  };

} // end namespace Belle2
