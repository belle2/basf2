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
#include <pxd/unpacking/PXDRawDataDefinitions.h>

#include <boost/cstdint.hpp>
#include <boost/spirit/home/support/detail/endian.hpp>


///*********************************************************************************
///****************** DHC Data Frame structs starts here ***************************
///*********************************************************************************

namespace Belle2 {

  namespace PXD {

    /** DHC frame header word data struct.
     * Encapsules the access for different bits within the header
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */

    using boost::spirit::endian::ubig16_t;
    using boost::spirit::endian::ubig32_t;
    using Belle2::PXD::PXDError::PXDErrorFlags;

    struct dhc_frame_header_word0 {
      const ubig16_t data;
      /// fixed length
      inline ubig16_t getData(void) const
      {
        return data;
      };
      inline unsigned short getFrameType(void) const
      {
        return (data & 0x7800) >> 11;
      };
      inline unsigned short getErrorFlag(void) const
      {
        return (data & 0x8000) >> 15;
      };
      inline unsigned short getMisc(void) const
      {
        return data & 0x3FF;
      };
      void print(void) const;
    };

    /** DHC start frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_start_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const ubig16_t trigger_nr_hi;
      const ubig16_t time_tag_lo_and_type;
      const ubig16_t time_tag_mid;
      const ubig16_t time_tag_hi;
      const ubig16_t run_subrun;
      const ubig16_t exp_run;
      const unsigned int crc32;
      /// fixed length, only for reading

      inline unsigned short getRunSubrun(void) const { return run_subrun; };
      inline unsigned short getExpRun(void) const { return exp_run;  };
      inline unsigned short getEventNrLo(void) const { return trigger_nr_lo; };
      inline unsigned short getEventNrHi(void) const {    return trigger_nr_hi;  };

      bool isFakedData(void) const;
      inline unsigned int getFixedSize(void) const
      {
        return 20;// bytes
      };
      void print(void) const;
      inline unsigned short get_active_dhe_mask(void) const {return word0.getMisc() & 0x1F;};
      inline unsigned short get_dhc_id(void) const {return (word0.getMisc() >> 5) & 0xF;};
      inline unsigned short get_subrun(void) const {return run_subrun & 0x00FF;};
      inline unsigned short get_run(void) const {return (((run_subrun & 0xFF00) >> 8)  | ((exp_run & 0x003F) << 8));};
      inline unsigned short get_experiment(void) const {return (exp_run & 0xFFC0) >> 6 ;};
    };

    /** DHH start frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_dhe_start_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const ubig16_t trigger_nr_hi;
      const ubig16_t dhe_time_tag_lo;
      const ubig16_t dhe_time_tag_hi;
      const ubig16_t sfnr_offset;
      const unsigned int crc32;
      /// fixed length

      inline unsigned short getEventNrLo(void) const   {    return trigger_nr_lo;  };
      inline unsigned short getEventNrHi(void) const  {    return trigger_nr_hi;  };
      inline unsigned short getStartFrameNr(void) const  {    return (sfnr_offset & 0xFC00) >> 10;  };  // last DHP frame before trigger
      inline unsigned short getTriggerOffsetRow(void) const   {    return sfnr_offset & 0x03FF;  }; // and trigger row offset
      inline unsigned int getFixedSize(void) const  {    return 16; };// 8 words

      void print(void) const;
      inline unsigned int getActiveDHPMask(void) const {return word0.getMisc() & 0xF;};
      inline unsigned int getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
    };

    /** DHH common mode frame data struct.
     * Encapsules the access for different bits within the header
     * Not used in hardware yet, preliminary, data format might change
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_commode_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const ubig16_t data[96];
      const unsigned int crc32;
      /// fixed length

      inline unsigned short getEventNrLo(void) const  {    return trigger_nr_lo;  };
      inline unsigned int getFixedSize(void) const  {    return (4 + 96) * 2; };// 100 words
      inline unsigned int getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
    };

    /** DHC direct readout frame data struct.
     * Covers functions common for RAW and ZSD frames.
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_direct_readout_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      /// an unbelievable amount of words may follow
      /// and finally a 32 bit checksum

      inline unsigned short getEventNrLo(void) const   {    return trigger_nr_lo;  };
      void print(void) const;
      inline unsigned short getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
      inline unsigned short getDHPPort(void) const {return (word0.getMisc()) & 0x3;};
      inline bool getDataReformattedFlag(void) const {return (word0.getMisc() >> 3) & 0x1;};
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
      const dhc_frame_header_word0 word0;
      const ubig16_t trignr0;
      const ubig32_t magic1;//! CAFExxxx , redundant
      const ubig32_t trignr1;//! HLT Trigger/Tag part 1
      const ubig32_t trigtag1;//! HLT Trigger/Tag part 2
      const ubig32_t magic2;/// CAFExxxx, redundant
      const ubig32_t trignr2;/// redundant, DATCON Trigger/Tag part 1
      const ubig32_t trigtag2;/// redundant, DATCON Trigger/Tag part 2
      const unsigned int crc32;

      inline unsigned int getFixedSize(void) const  {    return 32;  }; //  8*4 bytes might still be changed
      inline unsigned short get_trig_nr0(void) const   {    return trignr0;  };
      inline unsigned int get_trig_nr1(void) const  {    return trignr1;  };
      inline unsigned int get_trig_nr2(void) const  {    return trignr2;  };
      inline unsigned int get_trig_tag1(void) const  {    return trigtag1;  };
      inline unsigned int get_trig_tag2(void) const  {    return trigtag2;  };
      inline unsigned short get_subrun1(void) const {return trigtag1 & 0xFF;};
      inline unsigned short get_run1(void) const {return ((trigtag1 & 0x003FFF00) >> 8);};
      inline unsigned short get_experiment1(void) const {return (trigtag1 & 0xFFC00000) >> 22 ;};
      inline unsigned short get_subrun2(void) const {return trigtag2 & 0xFF;};
      inline unsigned short get_run2(void) const {return ((trigtag2 & 0x003FFF00) >> 8);};
      inline unsigned short get_experiment2(void) const {return (trigtag2 & 0xFFC00000) >> 22 ;};
      void print(void) const;
      PXDError::PXDErrorFlags check_error(bool ignore_datcon_flag = false) const;

      inline bool is_fake_datcon(void) const { return (magic2 == 0xCAFE0000 && trignr2 == 0x00000000 && trigtag2 == 0x00000000);};
      inline bool is_Accepted(void) const  {    return (magic1 & 0x8000) != 0;  };
      inline bool is_SendROIs(void) const  {    return (magic1 & 0x2000) != 0;  }
      inline bool is_SendUnfiltered(void) const  {    return (magic1 & 0x4000) != 0;  };
    };

    /** ONSEN (debug) ROI frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_onsen_roi_frame {
      const dhc_frame_header_word0 word0;/// mainly empty
      const ubig16_t trignr0;
      /// plus n* ROIs (64 bit)
      /// plus inner checksum 32bit
      /// plus checksum 32bit

      inline unsigned short get_trig_nr0(void) const   {    return trignr0;  };
      PXDError::PXDErrorFlags check_error(int length) const;
      void print(void) const;
      // 4 byte header, ROIS (n*8), 4 byte copy of inner CRC, 4 byte outer CRC
      inline int getMinSize(void) const {return 4 + 4 + 4;};
      unsigned int check_inner_crc(unsigned int /*length*/) const
      {
        /// Parts of the data are now in the ONSEN Trigger frame, therefore the inner CRC cannot be checked that easily!
        // TODO can be re-implemented if needed
        return 0;
      };

    };

    /** DHC Ghost frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_ghost_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const unsigned int crc32;
      /// fixed length

      inline unsigned int getFixedSize(void) const  {    return 8;  };
      void print(void) const;
      inline unsigned short getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
      inline unsigned short getDHPPort(void) const {return (word0.getMisc()) & 0x3;};
    };

    /** DHC End frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_end_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const ubig32_t wordsinevent;
      const unsigned int errorinfo;
      const unsigned int crc32;
      /// fixed length

      unsigned int get_words(void) const  {    return wordsinevent;  }
      inline unsigned int getFixedSize(void) const  {    return 16;  };
      bool isFakedData(void) const;
      void print(void) const;
      inline unsigned int get_dhc_id(void) const {return (word0.getMisc() >> 5) & 0xF;};
    };

    /** DHE End frame data struct.
     * Encapsules the access for different bits and data words
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    struct dhc_dhe_end_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const ubig16_t wordsineventlo; // words swapped... because of DHE 16 bit handling
      const ubig16_t wordsineventhi;
      const unsigned int errorinfo;// not well defined yet
      const unsigned int crc32;
      /// fixed length

      unsigned int get_words(void) const  {    return wordsineventlo | ((unsigned int)wordsineventhi << 16);  }
      inline unsigned int getFixedSize(void) const  {    return 16;  };
      void print(void) const;
      inline unsigned int getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
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
        const void* data;/// no type
        const dhc_onsen_trigger_frame* data_onsen_trigger_frame;
        const dhc_start_frame* data_dhc_start_frame;
        const dhc_end_frame* data_dhc_end_frame;
        const dhc_dhe_start_frame* data_dhe_start_frame;
        const dhc_dhe_end_frame* data_dhe_end_frame;
        const dhc_commode_frame* data_commode_frame;
        const dhc_direct_readout_frame* data_direct_readout_frame;
        const dhc_direct_readout_frame_raw* data_direct_readout_frame_raw;
        const dhc_direct_readout_frame_zsd* data_direct_readout_frame_zsd;
        const dhc_ghost_frame* data_ghost_frame;
        const dhc_onsen_roi_frame* data_onsen_roi_frame;
      };
      unsigned int datasize;
      int type;
      int length;

      dhc_frames(void)
      {
        data = 0;
        datasize = 0;
        type = -1;
        length = 0;
      };
      int getFrameType(void)
      {
        return type;
      };
      void set(void* d, unsigned int t)
      {
        data = d;
        type = t;
        length = 0;
      };
      void set(void* d, unsigned int t, unsigned int l)
      {
        data = d;
        type = t;
        length = l;
      };
      void set(void* d)
      {
        data = d;
        type = ((dhc_frame_header_word0*)data)->getFrameType();
        length = 0;
      };
      inline unsigned int getEventNrLo(void) const   {    return ((ubig16_t*)data)[1];  };
      PXDError::PXDErrorFlags check_padding(void);

      PXDError::PXDErrorFlags check_crc(void);


      unsigned int getFixedSize(void);

    };
  };
};
