/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <pxd/dataobjects/PXDErrorFlags.h>

#include <boost/endian/arithmetic.hpp>


///*********************************************************************************
///****************** DHC Data Frame structs starts here ***************************
///*********************************************************************************

namespace Belle2 {

  namespace PXD {

    using ubig16_t = boost::endian::big_uint16_t;
    using ubig32_t = boost::endian::big_uint32_t;
    using Belle2::PXD::PXDError::PXDErrorFlags;

    /** DHC frame header word data struct.
     * Encapsules the access for different bits within the header
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_frame_header_word0 {
      /// the data
      const ubig16_t data;

      /// get the data
      inline ubig16_t getData(void) const
      {
        return data;
      };
      /// get type of frame
      inline unsigned short getFrameType(void) const
      {
        return (data & 0x7800) >> 11;
      };
      /// get error flag
      inline unsigned short getErrorFlag(void) const
      {
        return (data & 0x8000) >> 15;
      };
      /// get misc
      inline unsigned short getMisc(void) const
      {
        return data & 0x7FF;
      };
      /// print
      void print(void) const;
    };

    /** DHC start frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_start_frame {
      const dhc_frame_header_word0 word0; ///< word0
      const ubig16_t trigger_nr_lo; ///< trigger_nr_lo
      const ubig16_t trigger_nr_hi; ///< trigger_nr_hi
      const ubig16_t time_tag_lo_and_type; ///< time_tag_lo_and_type
      const ubig16_t time_tag_mid; ///< time_tag_mid
      const ubig16_t time_tag_hi; ///< time_tag_hi
      const ubig16_t run_subrun; ///< run_subrun
      const ubig16_t exp_run; ///< exp_run
      const unsigned int crc32; ///< crc32
      // fixed length, only for reading

      /// get run_subrun
      inline unsigned short getRunSubrun(void) const
      {
        return run_subrun;
      };
      /// get exp_run
      inline unsigned short getExpRun(void) const
      {
        return exp_run;
      };
      /// get trigger_nr_lo
      inline unsigned short getEventNrLo(void) const
      {
        return trigger_nr_lo;
      };
      /// get trigger_nr_hi
      inline unsigned short getEventNrHi(void) const
      {
        return trigger_nr_hi;
      };

      /// isFakedData
      bool isFakedData(void) const;
      /// get fixed size (byte)
      inline unsigned int getFixedSize(void) const
      {
        return 20;
      };
      ///print
      void print(void) const;
      /// get active_dhe_mask (from word0)
      inline unsigned short get_active_dhe_mask(void) const
      {
        return word0.getMisc() & 0x1F;
      };
      /// get dhc_id (from word0)
      inline unsigned short get_dhc_id(void) const
      {
        return (word0.getMisc() >> 5) & 0xF;
      };
      /// get subrun (from run_subrun)
      inline unsigned short get_subrun(void) const
      {
        return run_subrun & 0x00FF;
      };
      /// get run (from run_subrun)
      inline unsigned short get_run(void) const
      {
        return (((run_subrun & 0xFF00) >> 8)  | ((exp_run & 0x003F) << 8));
      };
      /// get experiment (from exp_run)
      inline unsigned short get_experiment(void) const
      {
        return (exp_run & 0xFFC0) >> 6;
      };
      /// get gated_flag (from word0)
      inline unsigned short get_gated_flag(void) const
      {
        return (word0.getMisc() & 0x200) != 0;
      };
      /// get gated_isher (from word0)
      inline unsigned short get_gated_isher(void) const
      {
        return (word0.getMisc() & 0x400) != 0;
      };
    };

    /** DHH start frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_dhe_start_frame {
      const dhc_frame_header_word0 word0; ///< word0
      const ubig16_t trigger_nr_lo; ///< trigger_nr_lo
      const ubig16_t trigger_nr_hi; ///< trigger_nr_hi
      const ubig16_t dhe_time_tag_lo; ///< dhe_time_tag_lo
      const ubig16_t dhe_time_tag_hi; ///< dhe_time_tag_hi
      const ubig16_t sfnr_offset; ///< sfnr_offset
      const unsigned int crc32; ///< crc32
      // fixed length

      /// get trigger_nr_lo
      inline unsigned short getEventNrLo(void) const
      {
        return trigger_nr_lo;
      };
      /// get trigger_nr_hi
      inline unsigned short getEventNrHi(void) const
      {
        return trigger_nr_hi;
      };
      /// last DHP frame before trigger
      inline unsigned short getStartFrameNr(void) const
      {
        return (sfnr_offset & 0xFC00) >> 10;
      };
      /// trigger gate (updated to 8 bit, before 10!)
      inline unsigned short getTriggerGate(void) const
      {
        return sfnr_offset & 0xFF;
      };
      /// 8 words
      inline unsigned int getFixedSize(void) const
      {
        return 16;
      };
      /// print
      void print(void) const;
      /// get Active DHP Mask (from word0)
      inline unsigned int getActiveDHPMask(void) const
      {
        return word0.getMisc() & 0xF;
      };
      /// get DHE Id (from word0)
      inline unsigned int getDHEId(void) const
      {
        return (word0.getMisc() >> 4) & 0x3F;
      };
    };

    /** DHH common mode frame data struct.
     * Encapsules the access for different bits within the header
     * Not used in hardware yet, preliminary, data format might change
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_commode_frame {
      const dhc_frame_header_word0 word0; ///< word0
      const ubig16_t trigger_nr_lo; ///< trigger_nr_lo
      const ubig16_t data[96]; ///< data
      const unsigned int crc32; ///< crc32
      // fixed length

      /// get trigger_nr_lo
      inline unsigned short getEventNrLo(void) const
      {
        return trigger_nr_lo;
      };
      /// 100 words
      inline unsigned int getFixedSize(void) const
      {
        return (4 + 96) * 2;
      };
      /// get DHE Id (from word0)
      inline unsigned int getDHEId(void) const
      {
        return (word0.getMisc() >> 4) & 0x3F;
      };
    };

    /** DHC direct readout frame data struct.
     * Covers functions common for RAW and ZSD frames.
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_direct_readout_frame {
      const dhc_frame_header_word0 word0; ///< word0
      const ubig16_t trigger_nr_lo; ///< trigger_nr_lo
      // an unbelievable amount of words may follow
      // and finally a 32 bit checksum

      /// get trigger_nr_lo
      inline unsigned short getEventNrLo(void) const
      {
        return trigger_nr_lo;
      };
      /// print
      void print(void) const;
      /// get DHE Id (from word0)
      inline unsigned short getDHEId(void) const
      {
        return (word0.getMisc() >> 4) & 0x3F;
      };
      /// get DHP Port (from word0)
      inline unsigned short getDHPPort(void) const
      {
        return (word0.getMisc()) & 0x3;
      };
      /// get DataReformattedFlag (from word0)
      inline bool getDataReformattedFlag(void) const
      {
        return (word0.getMisc() >> 3) & 0x1;
      };
    };

    /** DHC RAW direct readout frame data struct.
     * Dummy.
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_direct_readout_frame_raw : public dhc_direct_readout_frame {
    };

    /** DHC Zero supressed direct readout frame data struct.
     * Dummy.
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_direct_readout_frame_zsd : public dhc_direct_readout_frame {
    };

    /** ONSEN Trigger frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_onsen_trigger_frame {
      const dhc_frame_header_word0 word0; ///< word0
      const ubig16_t trignr0; ///< trignr0
      const ubig32_t magic1; ///< CAFExxxx , redundant
      const ubig32_t trignr1; ///< HLT Trigger/Tag part 1
      const ubig32_t trigtag1; ///< HLT Trigger/Tag part 2
      const ubig32_t magic2; ///< CAFExxxx, redundant
      const ubig32_t trignr2; ///< redundant, DATCON Trigger/Tag part 1
      const ubig32_t trigtag2; ///< redundant, DATCON Trigger/Tag part 2
      const unsigned int crc32; ///< crc32

      /// 8*4 bytes might still be changed
      inline unsigned int getFixedSize(void) const
      {
        return 32;
      };
      /// get trignr0
      inline unsigned short get_trig_nr0(void) const
      {
        return trignr0;
      };
      /// get trignr1
      inline unsigned int get_trig_nr1(void) const
      {
        return trignr1;
      };
      /// get trignr2
      inline unsigned int get_trig_nr2(void) const
      {
        return trignr2;
      };
      /// get trigtag1
      inline unsigned int get_trig_tag1(void) const
      {
        return trigtag1;
      };
      /// get trigtag2
      inline unsigned int get_trig_tag2(void) const
      {
        return trigtag2;
      };
      /// get subrun1 (from trigtag1)
      inline unsigned short get_subrun1(void) const
      {
        return trigtag1 & 0xFF;
      };
      /// get run1 (from trigtag1)
      inline unsigned short get_run1(void) const
      {
        return ((trigtag1 & 0x003FFF00) >> 8);
      };
      /// get experiment1 (from trigtag1)
      inline unsigned short get_experiment1(void) const
      {
        return (trigtag1 & 0xFFC00000) >> 22;
      };
      /// get subrun2 (from trigtag2)
      inline unsigned short get_subrun2(void) const
      {
        return trigtag2 & 0xFF;
      };
      /// get run2 (from trigtag2)
      inline unsigned short get_run2(void) const
      {
        return ((trigtag2 & 0x003FFF00) >> 8);
      };
      /// get experiment2
      inline unsigned short get_experiment2(void) const
      {
        return (trigtag2 & 0xFFC00000) >> 22;
      };
      /// print
      void print(void) const;

      /// check error and return error mask
      PXDError::PXDErrorFlags check_error(
        bool ignore_datcon_flag = false,
        bool ignore_hltroi_magic_flag = false,
        bool ignore_merger_mm_flag = false
      ) const;

      /// is fake datcon
      inline bool is_fake_datcon(void) const
      {
        return (magic2 == 0xCAFE0000 && trignr2 == 0x00000000 && trigtag2 == 0x00000000);
      };
      /// is accepted
      inline bool is_Accepted(void) const
      {
        return (magic1 & 0x8000) != 0;
      };
      /// is sendROIs
      inline bool is_SendROIs(void) const
      {
        return (magic1 & 0x2000) != 0;
      };
      /// is sendUnfiltered
      inline bool is_SendUnfiltered(void) const
      {
        return (magic1 & 0x4000) != 0;
      };
    };

    /** ONSEN (debug) ROI frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_onsen_roi_frame {
      const dhc_frame_header_word0 word0; ///< mainly empty
      const ubig16_t trignr0; ///< trignr0
      // plus n* ROIs (64 bit)
      // plus inner checksum 32bit
      // plus checksum 32bit

      /// get trignr0
      inline unsigned short get_trig_nr0(void) const
      {
        return trignr0;
      };
      /// check error and return error mask
      PXDError::PXDErrorFlags check_error(
        int length,
        bool ignore_inv_size_flag = false
      ) const;
      /// print
      void print(void) const;
      /// 4 byte header, ROIS (n*8), 4 byte copy of inner CRC, 4 byte outer CRC
      inline int getMinSize(void) const {return 4 + 4 + 4;};
      /// check inner crc (currently not implemented/needed)
      unsigned int check_inner_crc(unsigned int /*length*/) const
      {
        // Parts of the data are now in the ONSEN Trigger frame,
        // therefore the inner CRC cannot be checked that easily!
        // TODO can be re-implemented if needed
        return 0;
      };
    };

    /** DHC Ghost frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_ghost_frame {
      const dhc_frame_header_word0 word0; ///< word0
      const ubig16_t trigger_nr_lo; ///< trigger_nr_lo
      const unsigned int crc32; ///< crc32
      /// fixed length

      /// get fixed size
      inline unsigned int getFixedSize(void) const
      {
        return 8;
      };
      ///print
      void print(void) const;
      /// get DHE Id (from word0)
      inline unsigned short getDHEId(void) const
      {
        return (word0.getMisc() >> 4) & 0x3F;
      };
      /// get DDHP port (from word0)
      inline unsigned short getDHPPort(void) const
      {
        return (word0.getMisc()) & 0x3;
      };
      /// get Error Bits
      unsigned short getErrorBits(void) const;
    };

    /** DHC End frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_end_frame {
      const dhc_frame_header_word0 word0; ///< word0
      const ubig16_t trigger_nr_lo; ///< trigger_nr_lo
      const ubig32_t wordsinevent; ///< wordsinevent
      const ubig32_t errorinfo; ///< errorinfo
      const unsigned int crc32; ///< crc32
      // fixed length

      /// get words
      unsigned int get_words(void) const
      {
        return wordsinevent;
      }
      /// get fixed size
      inline unsigned int getFixedSize(void) const
      {
        return 16;
      };
      /// is faked data
      bool isFakedData(void) const;
      /// print
      void print(void) const;
      /// get dhc id (from word0)
      inline unsigned int get_dhc_id(void) const
      {
        return (word0.getMisc() >> 5) & 0xF;
      };
      /// get error info
      inline unsigned int getErrorInfo(void) const
      {
        return errorinfo;
      };
    };

    /** DHE End frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_dhe_end_frame {
      const dhc_frame_header_word0 word0; ///< word0
      const ubig16_t trigger_nr_lo; ///< trigger_nr_lo
      const ubig16_t wordsineventlo; ///< words swapped... because of DHE 16 bit handling
      const ubig16_t wordsineventhi; ///< wordsineventhi
      const ubig32_t errorinfo; ///< not well defined yet
      const unsigned int crc32; ///< crc32
      // fixed length

      /// get words
      unsigned int get_words(void) const
      {
        return wordsineventlo | ((unsigned int)wordsineventhi << 16);
      }
      /// get fixed size
      inline unsigned int getFixedSize(void) const
      {
        return 16;
      };
      /// print
      void print(void) const;
      /// get DHE Id
      inline unsigned int getDHEId(void) const
      {
        return (word0.getMisc() >> 4) & 0x3F;
      };
      /// get error info
      inline unsigned int getErrorInfo(void) const
      {
        return errorinfo;
      };
      /// get error state machine DHP
      unsigned int getErrorStateMachineDHP(int dhpid) const;
      /// get error state machine start DHP
      unsigned int getErrorStateMachineStartDHP(int dhpid) const;
      /// get error state machine end DHP
      unsigned int getErrorStateMachineEndDHP(int dhpid) const;
    };


///*********************************************************************************
///****************** DHC Frame Wrapper Code starts here **************************
///*********************************************************************************

    /** DHC frame wrapper class.
     * Contains functions common for all type of frames (CRC etc)
     * Provides a union pointer to the different struct types
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    class dhc_frames {
    public:
      union {
        const void* data; ///< no type
        const dhc_onsen_trigger_frame* data_onsen_trigger_frame; ///< data_onsen_trigger_frame
        const dhc_start_frame* data_dhc_start_frame; ///< data_dhc_start_frame
        const dhc_end_frame* data_dhc_end_frame; ///< data_dhc_end_frame
        const dhc_dhe_start_frame* data_dhe_start_frame; ///< data_dhe_start_frame
        const dhc_dhe_end_frame* data_dhe_end_frame; ///< data_dhe_end_frame
        const dhc_commode_frame* data_commode_frame; ///< data_commode_frame
        const dhc_direct_readout_frame* data_direct_readout_frame; ///< data_direct_readout_frame
        const dhc_direct_readout_frame_raw* data_direct_readout_frame_raw; ///< data_direct_readout_frame_raw
        const dhc_direct_readout_frame_zsd* data_direct_readout_frame_zsd; ///< data_direct_readout_frame_zsd
        const dhc_ghost_frame* data_ghost_frame; ///< data_ghost_frame
        const dhc_onsen_roi_frame* data_onsen_roi_frame; ///< data_onsen_roi_frame
      };
      unsigned int datasize; ///< datasize
      int type; ///< type
      int length; ///< length

      /// set default values
      dhc_frames(void)
      {
        data = 0;
        datasize = 0;
        type = -1;
        length = 0;
      };
      /// get type of frame
      int getFrameType(void)
      {
        return type;
      };
      /** set data and type (and length to 0)
       * @param d data
       * @param t type
       */
      void set(const void* d, unsigned int t)
      {
        data = d;
        type = t;
        length = 0;
      };
      /** set data, type and length
       * @param d data
       * @param t type
       * @param l length
       */
      void set(const void* d, unsigned int t, unsigned int l)
      {
        data = d;
        type = t;
        length = l;
      };
      /** set data (get type from dhc_frame_header_word0 and set length to 0)
       * @param d data
       */
      void set(const void* d)
      {
        data = d;
        type = reinterpret_cast <const dhc_frame_header_word0*>(data)->getFrameType();
        length = 0;
      };
      /// get event nr lo (from data)
      inline unsigned int getEventNrLo(void) const
      {
        return ((ubig16_t*)data)[1];
      };
      /// check padding and return it
      PXDError::PXDErrorFlags check_padding();
      /// check crc and return it
      PXDError::PXDErrorFlags check_crc(bool ignore_crc_flag = false);

      /// get fixed size
      unsigned int getFixedSize(void);

    };
  };
};
