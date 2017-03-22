/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDUnpackerDESY0416Module_H
#define PXDUnpackerDESY0416Module_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDRawAdc.h>
#include <pxd/dataobjects/PXDRawPedestal.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <pxd/dataobjects/PXDRawCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  namespace PXD {

#define ONSEN_MAX_TYPE_ERR  64


    /** The PXDUnpackerDESY0416 module.
     *
     * This module is responsible for unpacking the Raw PXD data to Pixels in v_cellID and u_cellID (global tracking coordinates system)
     */
    class PXDUnpackerDESY0416Module : public Module {

    public:
      /** Constructor defining the parameters */
      PXDUnpackerDESY0416Module();

      /** Initialize the module */
      virtual void initialize();
      /** do the unpacking */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    private:

      std::string m_RawPXDsName;  /**< The name of the StoreArray of processed RawPXDs */
      std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits to be generated */
      std::string m_PXDRawAdcsName;  /**< The name of the StoreArray of PXDRawAdcs to be generated */
      std::string m_PXDRawPedestalsName;  /**< The name of the StoreArray of PXDRawPedestals to be generated */
      std::string m_PXDRawROIsName;  /**< The name of the StoreArray of PXDRawROIs to be generated */
      std::string m_RawClusterName;  /**< The name of the StoreArray of PXDRawROIs to be generated */

      /**  Swap the endianess of the ONSEN header yes/no */
      bool m_headerEndianSwap;
      /**  ignore missing DATCON */
      bool m_ignoreDATCON;
      /** Only unpack, but Do Not Store anything to file */
      bool m_doNotStore;
      //* Fix EventMeta - HLT Trigger Offset for DESY TB 2016, only for error reporting, no data is modified */
      int m_DESY16_FixTrigOffset;
      //* Fix Row Offset for DESY TB 2016 */
      int m_DESY16_FixRowOffset;
      /** Critical error mask which defines return value of task */
      uint64_t m_criticalErrorMask;

      /** Event Number from MetaInfo */
      unsigned long m_meta_event_nr;
      /** Run Number from MetaInfo */
      unsigned long m_meta_run_nr;
      /** Subrun Number from MetaInfo */
      unsigned long m_meta_subrun_nr;
      /** Experiment from MetaInfo */
      unsigned long m_meta_experiment;
      /** Time(Tag) from MetaInfo */
      unsigned long long int m_meta_time;

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

      //Remaps rows of inner forward (IF) and outer backward (OB) modules of the PXD
      void remap_IF_OB(unsigned int& row, unsigned int& col, unsigned int dhp_id, unsigned int dhe_ID);

      //Remaps cols of inner backward (IB) and outer forward (OF) modules of the PXD
      void remap_IB_OF(unsigned int& row, unsigned int& col, unsigned int dhp_id, unsigned int dhe_ID);

      void test_mapping(void);

      /** Error Mask set per packet / event*/
      uint64_t m_errorMask;
      /** give verbose unpacking information -> will eb a parameter in next release */
      bool verbose = true;
      /** ignore missing datcon (dont show error) */
      bool ignore_datcon_flag = true;

      /** counter for not accepted events... should not happen */
      unsigned int m_notaccepted{0};
      /** counter for send debug rois */
      unsigned int m_sendrois{0};
      /** counter for send unfiltered */
      unsigned int m_sendunfiltered{0};
      /** flag send unfiltered */
      // Commented out - not used! bool m_event_unfiltered{false};

    public:
      // The following helper functions do not need the class object
      int static nr5bits(int i);/// helper function to "count" nr of set bits within lower 5 bits

      void static dump_dhp(void* data, unsigned int frame_len);
      void static dump_roi(void* data, unsigned int frame_len);
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDUnpackerDESY0416Module_H
