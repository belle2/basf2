/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDRawAdc.h>
#include <pxd/dataobjects/PXDRawPedestal.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <pxd/dataobjects/PXDRawCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <pxd/dataobjects/PXDErrorFlags.h>
#include <pxd/dataobjects/PXDDAQStatus.h>

namespace Belle2 {

  namespace PXD {

    /** The PXDUnpacker module.
     *
     * This module is responsible for unpacking the Raw PXD data to Pixels in v_cellID and u_cellID (global tracking coordinates system)
     * Doing that, sophisticated error and consistency checking is done from the lowest data level on
     */

    class PXDUnpackerModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDUnpackerModule();

    private:

      /** Initialize the module */
      void initialize() override final;
      /** do the unpacking */
      void event() override final;
      /** Terminate the module */
      void terminate() override final;

      std::string m_RawPXDsName;  /**< The name of the StoreArray of processed RawPXDs */
      std::string m_PXDDAQEvtStatsName;  /**< The name of the StoreObjPtr of PXDDAQStatus to be generated */
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
      /** Check for susp. Padding/CRC, default off because of many false positive */
      bool m_checkPaddingCRC;
      /** Ignore Phase2 FW erro */
      bool m_ignoreDHPMask;
      /** Ignore Phase2 FW error */
      bool m_ignoreDHELength;
      /** Force Mapping even if DHH bit is not requesting it */
      bool m_forceMapping;
      /** Force No Mapping even if DHH bit is requesting it */
      bool m_forceNoMapping;
      /** Maximum DHP frame difference until error is reported */
      unsigned int m_maxDHPFrameDiff;

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
      unsigned int m_errorCounter[PXDError::ONSEN_MAX_TYPE_ERR];

      /** Input array for PXD Raw. */
      StoreArray<RawPXD> m_storeRawPXD;
      /** Input ptr for EventMetaData. */
      StoreObjPtr<EventMetaData> m_eventMetaData;
      /** Output array for Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;
      /** Output array for Raw ROIs. */
      StoreArray<PXDRawROIs> m_storeROIs;
      /** Output array for DAQ Status. */
      StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;
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
      void unpack_rawpxd(RawPXD& px, int inx);

      /** Unpack one frame (within an event).
       * @param data pointer to frame
       * @param len length of frame
       * @param Frame_Number current frame number
       * @param Frames_in_event number of frames in PxdRaw object (subevent)
       * @param daqpktstat Daq Packet Status Object
       * @param daqdhevect Daq DHE Status Object
       */
      void unpack_dhc_frame(void* data, const int len, const int Frame_Number, const int Frames_in_event, PXDDAQPacketStatus& daqpktstat);

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
                      unsigned dhe_reformat, unsigned short toffset, VxdID vxd_id, PXDDAQPacketStatus& daqpktstat);

      /** Unpack DHP RAW data within one DHE frame (pedestals, etc)
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhe_ID raw DHE ID from DHC frame
       * @param dhe_DHPport raw DHP port from DHC frame
       * @param vxd_id vertex Detector ID
       */
      void unpack_dhp_raw(void* data, unsigned int len, unsigned int dhe_ID, unsigned dhe_DHPport, VxdID vxd_id);

      /** Unpack DHP/FCE data within one DHE frame
       * Not fully implemented as cluster format not 100% fixed
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param vxd_id vertex Detector ID
       */
      void unpack_fce(unsigned short* data, unsigned int length, VxdID vxd_id);

      /** Error Mask set per packet / frame*/
      PXDError::PXDErrorFlags m_errorMask;
      /** Error Mask set per packet / DHE */
      PXDError::PXDErrorFlags m_errorMaskDHE;
      /** Error Mask set per packet / DHC */
      PXDError::PXDErrorFlags m_errorMaskDHC;
      /** Error Mask set per packet / packet */
      PXDError::PXDErrorFlags m_errorMaskPacket;
      /** Error Mask set per packet / event */
      PXDError::PXDErrorFlags m_errorMaskEvent;
      /** give verbose unpacking information -> TODO will be a parameter in next release */
      bool verbose = true;

      /** counter for not accepted events... should not happen TODO discussion ongoing with DAQ group */
      unsigned int m_notaccepted{0};
      /** counter for send debug rois */
      unsigned int m_sendrois{0};
      /** counter for send unfiltered */
      unsigned int m_sendunfiltered{0};
      /** flag send unfiltered */
      bool m_event_unfiltered{false};

      int last_dhp_readout_frame_lo[4];// signed because -1 means undefined

    public:
      /** helper function to "count" nr of set bits within lower 5 bits.
        * It does not need the class object, thus static.
        */
      int static nr5bits(int i);

      /** dump to a file, helper function for debugging.
        */
      void static dump_dhp(void* data, unsigned int frame_len);

      /** dump to a file, helper function for debugging.
        */
      void static dump_roi(void* data, unsigned int frame_len);

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
