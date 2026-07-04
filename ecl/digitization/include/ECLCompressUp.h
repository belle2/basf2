/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace ECL {

    class BitStream;

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
    class ECLCompressUp {
    public:
      /** virtual destructure */
      virtual ~ECLCompressUp() {};

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
    class ECLBaseCompressUp: public ECLCompressUp {
    public:
      void compress(BitStream& out, const int* adc) override;
      void uncompress(BitStream& out, int* adc) override;
    };

    /** ECL waveform compression/decompression to/from the BitStream
     *  storage with the DELTA algorithm
     */
    class ECLDeltaCompressUp: public ECLCompressUp {
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

    /** The compression algorithm selector function
     *  @param compAlgo compression algorithm -- 1 - BASE, 2 - DELTA
     *  @return pointer to the selected algorithm or nullptr in case of the wrong parameter
     */
    ECLCompressUp* selectAlgoUp(int compAlgo);
  }
}
