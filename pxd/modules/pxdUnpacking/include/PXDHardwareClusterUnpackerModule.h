/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDHardwareClusterUnpackerModule_H
#define PXDHardwareClusterUnpackerModule_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDRawCluster.h>
#include <pxd/dataobjects/PXDHardwareCluster.h>
#include <framework/datastore/StoreArray.h>
#include <vxd/dataobjects/VxdID.h>
#include <boost/foreach.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <boost/spirit/home/support/detail/endian.hpp>


namespace Belle2 {

  namespace PXD {

    using namespace boost::spirit::endian;
#define ONSEN_MAX_TYPE_ERR  32

    /** The PXDClusterMerger module.
     * This module is responsible for transfering the PXDRawCluster from DHE Hardware into PXDHardwareCluster
     * where information which is needed by the clusterizer is stored
     */
    class PXDHardwareClusterUnpackerModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDHardwareClusterUnpackerModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the unpacking */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    private:

      std::string m_RawClustersName;  /**< The name of the StoreArray of processed RawClusters */
      std::string m_HardwareClusterName;  /**< The name of the StoreArray of processed Hardware Clusters */

      /**  Swap the endianess of the ONSEN header yes/no */
      bool m_headerEndianSwap;
      /**  ignore missing DATCON */
      bool m_ignoreDATCON;
      /** Only unpack, but Do Not Store anything to file */
      bool m_doNotStore;
      /** Event Number and compare mask grabbed from FTSW for now */
      unsigned int ftsw_evt_nr, ftsw_evt_mask;
      /** Event Number from MetaInfo */
      unsigned long m_meta_event_nr;
      /** Run Number from MetaInfo */
      unsigned long m_meta_run_nr;
      /** Subrun Number from MetaInfo */
      unsigned long m_meta_subrun_nr;
      /** Experiment from MetaInfo */
      unsigned long m_meta_experiment;

      /** Event counter */
      unsigned int m_unpackedEventsCount;

      /** structure for seed pixels */
      struct seed_pixel {
        unsigned int row;
        unsigned int col;
        unsigned char charge;
      };

      /** Output array for Clusters. */
      StoreArray<PXDHardwareCluster> m_storeHardwareCluster;

      /** input array. */
      StoreArray<PXDRawCluster> m_rawClusters;

      /** Unpack dhp (several cluster frames) stored in RawCluster object.
       * @param cl Cluster data object
       */
      void unpack_event(const PXDRawCluster& cl);

      /** Unpack one frame (within an event).
       * @param data pointer to frame
       * @param len length of frame
       * @param Frame_Number current frame number
       * @param Frames_in_event number of frames in PXDRawCluster object (subevent)
       */

      /** Unpack one cluster  */
      void unpack_fce(const void* data, unsigned int length, VxdID vxdID);

      /**
       * @param dhe_id Pixel row coordinate.
       * @param row_address
       * @param column_address
       * @param adc_value
       * @param vxdID
       */

      /** Calculates the total cluster charge of one cluster */
      unsigned int calc_cluster_charge(const unsigned short* data, unsigned int nr_pixel_words);

      /** Finds the seed pixel and its charge */
      seed_pixel find_seed_pixel(const unsigned short* data, unsigned int nr_pixel_words, unsigned int dhp_id);

      /** give verbose unpacking information -> will eb a parameter in next release */
      bool verbose = true;
      /** ignore missing datcon (dont show error) */
      bool ignore_datcon_flag = true;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDHardwareClusterUnpackerModule_H
