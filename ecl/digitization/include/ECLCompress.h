/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/utilities/Utils.h>

namespace Belle2 {
  namespace ECL {

    class BitStream;

    /** find the log base 2 of 32-bit v */
    unsigned int ilog2(unsigned int v);

    /** Bit widths for the prefix coding to encode integers which are
     * mainly concentrated around zero and probability density are
     * decreasing for large absolute values.
     * @sa stream_int(BitStream& OUT, int x, const width_t& w) and
     * int fetch_int(BitStream& IN, const width_t& w) function for more details
     */
    struct width_t {
      unsigned char w0, /**< Progressive bit width to encode an integer value */
               w1, /**< Progressive bit width to encode an integer value */
               w2, /**< Progressive bit width to encode an integer value */
               w3; /**< Progressive bit width to encode an integer value */
    };

    /** Abstract class (interface) for ECL waveform
     *  compression/decompression to/from the BitStream storage.
     */
    class ECLCompress {
    public:
      /** virtual destructure */
      virtual ~ECLCompress() {};

      /** Compress the ECL waveform
       * @param out -- where we stream the compressed data
       * @param adc -- the waveform to compress
       */
      virtual void compress(BitStream& out, const int* adc) = 0;

      /** Decompress the ECL waveform
       * @param in -- the stream from which we fetch the compressed data
       * @param adc -- the decompressed waveform
       */
      virtual void uncompress(BitStream& in, int* adc) = 0;
    };

    /** ECL waveform compression/decompression to/from the BitStream
     *  storage with the BASE algorithm
     */
    class ECLBaseCompress: public ECLCompress {
    public:
      void compress(BitStream& out, const int* adc) override;
      void uncompress(BitStream& out, int* adc) override;
    };

    /** ECL waveform compression/decompression to/from the BitStream
     *  storage with the DELTA algorithm
     */
    class ECLDeltaCompress: public ECLCompress {
    public:
      /**
       * function to compress the ECL waveform with the DELTA algorithm
       */
      void compress(BitStream& out, const int* adc) override;
      /**
       * function to decompress the ECL waveform with the DELTA algorithm
       */
      void uncompress(BitStream& out, int* adc) override;
    };

    /** ECL waveform compression/decompression to/from the BitStream
     *  storage based on the Discrete Cosine Transform (DCT)
     */
    class ECLDCTCompress: public ECLCompress {
    public:
      /** Constructor for DCT based compression algorithm
       *  @param scale scale factor for quantization.
       *  @param c0 average waveform amplitude.
       *  @param w DCT coefficient probability density based bit widths for quantized coefficients
       */
      ECLDCTCompress(double scale, double c0, width_t* w);
      /**
       * function to compress
       */
      void compress(BitStream& out, const int* adc) override;
      /**
       * function to decompress
       */
      void uncompress(BitStream& in, int* adc) override;
    protected:
      const double m_scale; /**< Scale factor for quantization. */
      const double m_c0; /**< Average waveform amplitude */
      const width_t* m_widths; /**< Bit widths for the DCT coefficients for prefix encoding. */
    };

    /** The compression algorithm selector function
     *  @param compAlgo compression algorithm -- 1 - BASE, 2 - DELTA, 3 - DCT
     *  @return pointer to the selected algorithm or nullptr in case of the wrong parameter
     */
    ECLCompress* selectAlgo(int compAlgo);
  }
}
