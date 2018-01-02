/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDUnpackerBSModule_H
#define PXDUnpackerBSModule_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDRawAdc.h>
#include <pxd/dataobjects/PXDRawPedestal.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <pxd/dataobjects/PXDRawCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/datastore/StoreArray.h>

#include <pxd/dataobjects/PXDErrorFlags.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
// #include <pxd/dataobjects/PXDDAQPacketStatus.h>
// #include <pxd/dataobjects/PXDDAQDHCStatus.h>
// #include <pxd/dataobjects/PXDDAQDHEStatus.h>

using namespace Belle2::PXD::PXDError;

namespace Belle2 {

  namespace PXD {

    /** The PXDUnpackerBS module.
     *
     * This module is responsible for unpacking the Raw PXD data to Pixels in v_cellID and u_cellID (global tracking coordinates system)
     */
    class PXDUnpackerBSModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDUnpackerBSModule();

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
      /**  ignore some not set Meta Flags */
      bool m_ignoreMetaFlags;
      /** Only unpack, but Do Not Store anything to file */
      bool m_doNotStore;
      //* Fix EventMeta - HLT Trigger Offset for DESY TB 2016, only for error reporting, no data is modified */
      int m_DESY16_FixTrigOffset;
      //* Fix Row Offset for DESY TB 2016 */
      int m_DESY16_FixRowOffset;
      /** Critical error mask which defines return value of task */
      uint64_t m_criticalErrorMask; // TODO this should be type PXDErrorFlag .. but that does not work with addParam()

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
      /** Output array for Raw ROIs. */
      StoreArray<PXDRawROIs> m_storeROIs;
//       /** Output array for DAQ DHE Status. */
//       StoreArray<PXDDAQDHEStatus> m_storeDHEStats;
//       /** Output array for DAQ DHC Status. */
//       StoreArray<PXDDAQDHCStatus> m_storeDHCStats;
//       /** Output array for DAQ Status/Packet. */
//       StoreArray<PXDDAQPacketStatus> m_storeDAQPktStats;
      /** Output array for DAQ Status. */ // TODO Why Array, can be object?
      StoreArray<PXDDAQStatus> m_storeDAQEvtStats;
      /** Output array for Raw Adcs. */
      StoreArray<PXDRawAdc> m_storeRawAdc;
      /** Output array for Raw Adc:Pedestals. */
      StoreArray<PXDRawPedestal> m_storeRawPedestal;
      /** Output array for Clusters. */
      StoreArray<PXDRawCluster> m_storeRawCluster;

      /** Unpack one event (several frames) stored in RawPXD object
       * @param px RawPXD data object
       * @param inx Index of RawPXD packet
       */
      void unpack_rawpxd(RawPXD& px, int inx, PXDDAQStatus& daqevtstat);

      /** Unpack one frame (within an event).
       * @param data pointer to frame
       * @param len length of frame
       * @param Frame_Number current frame number
       * @param Frames_in_event number of frames in PxdRaw object (subevent)
       */
      void unpack_dhc_frame(void* data, const int len, const int Frame_Number, const int Frames_in_event, PXDDAQPacketStatus& daqpktstat,
                            std::vector <PXDDAQDHEStatus>& daqdhevect);

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

      /** Error Mask set per packet / frame*/
      PXDErrorFlags m_errorMask;
      /** Error Mask set per packet / DHE */
      PXDErrorFlags m_errorMaskDHE;
      /** Error Mask set per packet / DHC */
      PXDErrorFlags m_errorMaskDHC;
      /** Error Mask set per packet / packet */
      PXDErrorFlags m_errorMaskPacket;
      /** Error Mask set per packet / event */
      PXDErrorFlags m_errorMaskEvent;
      /** give verbose unpacking information -> will ba a parameter in next release */
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
      bool m_event_unfiltered{false};

      int last_dhp_readout_frame_lo[4];// signed because -1 means undefined

    public:
      // The following helper functions do not need the class object
      int static nr5bits(int i);/// helper function to "count" nr of set bits within lower 5 bits

      void static dump_dhp(void* data, unsigned int frame_len);
      void static dump_roi(void* data, unsigned int frame_len);
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDUnpackerBSModule_H
