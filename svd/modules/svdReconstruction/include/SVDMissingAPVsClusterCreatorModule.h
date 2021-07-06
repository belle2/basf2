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

#include <svd/dataobjects/SVDCluster.h>

#include <svd/online/SVDOnlineToOfflineMap.h>
#include <framework/database/PayloadFile.h>

namespace Belle2 {

  namespace SVD {

    /** SVDMissingAPVsClusterCreatorModule: The SVD MissingAPVsClusterCreator.
     *
     * This module produces clusters in the middle of a region read by a disabled APV
     */
    class SVDMissingAPVsClusterCreatorModule : public Module {

    public:

      /** Constructor defining the parameters */
      SVDMissingAPVsClusterCreatorModule();

      /** Initialize the module */
      virtual void initialize() override;

      /** check if channel mapping is changed */
      virtual void beginRun() override;

      /** do the clustering */
      virtual void event() override;

    protected:


      // Data members
      //1. Collections and relations Names
      std::string m_storeClustersName = ""; /**< name of the collection to use for the SVDClusters */
      float m_time = 0; /**< time of the cluster, in ns*/
      float m_timeError = 10; /**< time error of the cluster, in ns*/
      float m_seedCharge = 10000; /**< seed charge of the cluster, in e-*/
      float m_charge = 20000; /**< total charge of the cluster, in e-*/
      float m_SNR = 15; /**< SNR of the cluster*/
      int m_size = 128; /**< size of the cluster*/
      int m_firstFrame = 0; /**< first frame*/
      int m_nFakeClusters = 4; /**< number of fake clusters*/

      /** Collection of SVDClusters */
      StoreArray<SVDCluster> m_storeClusters;

      //channel mapping stuff
      static std::string m_xmlFileName /**< channel mapping xml filename*/;
      DBObjPtr<PayloadFile> m_mapping; /**<channel mapping payload*/
      std::unique_ptr<SVDOnlineToOfflineMap> m_map; /**<channel mapping map*/


    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2
