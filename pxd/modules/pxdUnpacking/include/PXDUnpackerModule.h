/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDUnpackerModule_H
#define PXDUnpackerModule_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDRawAdc.h>
#include <pxd/dataobjects/PXDRawPedestal.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <pxd/dataobjects/PXDRawCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/datastore/StoreArray.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>

namespace Belle2 {

  namespace PXD {

#define ONSEN_MAX_TYPE_ERR  64


    /** The PXDUnpacker module.
     *
     * This module is responsible for unpacking the Raw PXD data to Pixels in v_cellID and u_cellID (global tracking coordinates system)
     */
    class PXDUnpackerModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDUnpackerModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the unpacking */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

      int get_errors(int block_number);

      int get_Nrerrors(void);

      std::string get_errTypes(int nr);

      std::string get_errWeight(int nr);

    private:

      std::string m_RawPXDsName;  /**< The name of the StoreArray of processed RawPXDs */
      std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits to be generated */
      std::string m_PXDRawAdcsName;  /**< The name of the StoreArray of PXDRawAdcs to be generated */
      std::string m_PXDRawPedestalsName;  /**< The name of the StoreArray of PXDRawPedestals to be generated */
      std::string m_PXDRawROIsName;  /**< The name of the StoreArray of PXDRawROIs to be generated */
      std::string m_RawClusterName;  /**< The name of the StoreArray of PXDRawROIs to be generated */
      std::string m_RemapLUTifob;  /**< Name of the LUT which remaps DHP data for inner forward and outer backward modules */
      std::string m_RemapLUTibof;  /**< Name of the LUT which remaps DHP data for inner backward and outer forward modules */
      bool m_ignore_headernrframes;

      /**  Swap the endianess of the ONSEN header yes/no */
      bool m_headerEndianSwap;
      /**  ignore missing DATCON */
      bool m_ignoreDATCON;
      /** Only unpack, but Do Not Store anything to file */
      bool m_doNotStore;
      /**< Set to one if data should be remapped according to Belle II node 10 */
      bool m_RemapFlag;
      /** Event Number from MetaInfo */
      unsigned long m_meta_event_nr;
      /** Run Number from MetaInfo */
      unsigned long m_meta_run_nr;
      /** Subrun Number from MetaInfo */
      unsigned long m_meta_subrun_nr;
      /** Experiment from MetaInfo */
      unsigned long m_meta_experiment;
      /** For DESY TB data */
      int m_DESY16_FixTrigOffset;
      /**< Set to one if Start of row flag should be ignored - HOT FIX for gradient */
      bool m_ignoreSorFlag;

      /** Event counter */
      unsigned int m_unpackedEventsCount;
      /** Error counters */
      unsigned int m_errorCounter[ONSEN_MAX_TYPE_ERR];

      /** Input array for PXD Raw. */
      StoreArray<RawPXD> m_storeRawPXD;
      /** Output array for Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;
      /** Output array for Raw Hits. */
      StoreArray<PXDRawROIs> m_storeROIs;
      /** Output array for Raw Adcs. */
      StoreArray<PXDRawAdc> m_storeRawAdc;
      /** Output array for Raw Adc:Pedestals. */
      StoreArray<PXDRawPedestal> m_storeRawPedestal;
      /** Output array for Clusters. */
      StoreArray<PXDRawCluster> m_storeRawCluster;


      /** Unpack one event (several frames) stored in RawPXD object./ Unpack one cluster stored in Cluster object.
       * @param px RawPXD data object
       * @param cl Cluster data object
       */
      void unpack_event(RawPXD& px);

      /** Unpack one frame (within an event).
       * @param data pointer to frame
       * @param len length of frame
       * @param Frame_Number current frame number
       * @param Frames_in_event number of frames in PxdRaw object (subevent)
       */
      void unpack_dhc_frame(void* data, const int len, const int Frame_Number, const int Frames_in_event);

      /** Unpack DHP data within one DHE frame
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhe_first_readout_frame_lo 16 bit of the first readout frame from DHE Start
       * @param dhe_ID raw DHE ID from DHC frame
       * @param dhe_DHPport raw DHP port from DHC frame
       * @param dhe_reformat flag if DHE did reformatting
       * @param toffset triggered row (offset)
       * @param vxd_id vertex Detector ID
       */
      void unpack_dhp(void* data, unsigned int len, unsigned int dhe_first_readout_frame_lo, unsigned int dhe_ID, unsigned dhe_DHPport,
                      unsigned dhe_reformat, unsigned short toffset, VxdID vxd_id);

      /** Unpack DHP RAW data within one DHE frame (pedestals, etc)
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhe_ID raw DHE ID from DHC frame
       * @param dhe_DHPport raw DHP port from DHC frame
       * @param vxd_id vertex Detector ID
       */
      void unpack_dhp_raw(void* data, unsigned int len, unsigned int dhe_ID, unsigned dhe_DHPport, VxdID vxd_id);

      /** Unpack DHP/FCE data within one DHE frame
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhe_first_readout_frame_lo 16 bit of the first readout frame from DHE Start
       * @param dhe_ID raw DHE ID from DHC frame
       * @param dhe_DHPport raw DHP port from DHC frame
       * @param dhe_reformat flag if DHE did reformatting
       * @param toffset triggered row (offset)
       * @param vxd_id vertex Detector ID
       */


      void unpack_fce(unsigned short* data, unsigned int length, VxdID vxd_id);

      /**
       * @param length length of cluster data
       * @param data pointer to cluster data
       * @param vxd_id vertex Detector ID
       */

      int nr5bits(int i) const;/// helper function to "count" nr of set bits within lower 5 bits

      //Remaps rows of inner forward (IF) and outer backward (OB) modules of the PXD
      unsigned int remap_row_IF_OB(unsigned int row_false, unsigned int col_false, unsigned int dhp_id, unsigned int dhe_ID);

      //Remaps cols of inner forward (IF) and outer backward (OB) modules of the PXD
      unsigned int remap_col_IF_OB(unsigned int row_false, unsigned int col_false, unsigned int dhp_id);

      //Remaps rows of inner backward (IB) and outer forward (OF) modules of the PXD
      unsigned int remap_row_IB_OF(unsigned int row_false, unsigned int col_false, unsigned int dhp_id, unsigned int dhe_ID);

      //Remaps cols of inner backward (IB) and outer forward (OF) modules of the PXD
      unsigned int remap_col_IB_OF(unsigned int row_false, unsigned int col_false, unsigned int dhp_id);

      //Lookup tables for DHP remapping
      int LUT_IF_OB[1030];
      int LUT_IB_OF[1030];

      void set_errors(int block_number, std::string errorWeight);

      /** Error Mask set per packet / event*/
      uint64_t m_errorMask;
      /** give verbose unpacking information -> will eb a parameter in next release */
      bool verbose = true;
      /** ignore missing datcon (dont show error) */
      bool ignore_datcon_flag = true;

      unsigned int dheID_order_error = 0;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDUnpackerModule_H
