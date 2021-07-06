/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <string>
#include <map>

#include <TH2.h>

namespace Belle2 {
  /**
   * Generate PDFs for assigning probability that cluster generated
   * from signal particle.
   */
  class SVDClusterQualityEstimatorCalibrationModule : public Module {

  public:
    /** Constructor */
    SVDClusterQualityEstimatorCalibrationModule();



    /** Init the module.
    *
    * prepares histograms.
    */
    virtual void initialize() override;


    /** eventWise jobs */
    virtual void event() override;


    /** final output  */
    virtual void terminate() override;



  protected:

    void calculateProb(TH2F* signal, TH2F* background, TH2F* probability); /**< compute probvability*/
    void calculateError(TH2F* signal, TH2F* background, TH2F* error); /**<compute error*/

    // Data members
    std::string m_svdClustersName; /**< SVDCluster collection name */
    std::string m_recoTracksName; /**< RecoTrack collection name */

    StoreArray<SVDCluster> m_svdClusters; /**< the storeArray for svdClusters  */

    StoreArray<RecoTrack> m_recoTracks; /**< StoreArray for recoTracks */

    int m_binSizeCharge; /**< Number of bins in charge dimension */

    int m_binSizeTime; /**< Number of bins in time dimension */

    int m_maxClusterSize; /**< Maximum cluster size the PDFs will be distributed over */

    bool m_useLegacyNaming; /**< Choice between PDF naming conventions */

    std::string m_outputFileName; /**< output filename*/

    std::string
    m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */

    //for setting up histograms

    std::map<std::string, TH2F* > signalHistMap; /**< map to store signal histograms */
    std::map<std::string, TH2F*> backgroundHistMap; /**< map to store background histograms */

  };
} // end namespace Belle2
