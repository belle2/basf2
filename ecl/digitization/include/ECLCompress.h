/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexei Sibidanov                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCOMPRESS_H
#define ECLCOMPRESS_H

#include <algorithm>
#include <vector>
#include <string.h>

namespace Belle2 {
  /** find the log base 2 of 32-bit v */
  unsigned int ilog2(unsigned int v);

  /** Bit stream struct.
   * Class contains vector of unsigned ints as a storage and current bit position in the storage.
   * One can put and fetch up to 32 bits at once.
   */
  class BitStream {
  public:
    /** Default constructor for ROOT.
     *  Current position is at begining of the storage.
     */
    BitStream(): m_pos(0) {}

    /** Constructor with the reserved and cleared storage prepared for
     *  incoming bits. Be sure the size is enough for incoming data
     *  since the class does not check bounds.  Current position is at
     *  begining of the storage.
     */
    BitStream(int n): m_pos(0)
    {
      m_store.resize(n);
      memset(m_store.data(), 0, m_store.size()*sizeof(unsigned int));
    }

    /** Push n least significant bits of "value" to the stream. Update current position accordingly.
     * @param value -- value to put in the stream
     * @param n -- how many least signfificant bits of "value" put in the stream (n<=32)
     */
    void putNBits(unsigned int value, unsigned int n)
    {
      unsigned int bpos = m_pos % 32, wpos = m_pos / 32;
      value &= (1 << n) - 1;
      m_store[wpos] |= value << bpos;
      if (bpos + n > 32) m_store[wpos + 1] = value >> (32 - bpos);
      m_pos += n;
    }

    /** Fetch n bits. Update current position accordingly
     * @param n -- how many bits fetch from the current position in the stream
     * @return n fetched bits
     */
    unsigned int getNBits(unsigned int n)
    {
      unsigned int bpos = m_pos % 32, wpos = m_pos / 32;
      unsigned int res = m_store[wpos] >> bpos;
      if (bpos + n > 32) res |= m_store[wpos + 1] << (32 - bpos);
      m_pos += n;
      return res & ((1 << n) - 1);
    }

    /** Get current position in the stream. */
    size_t getPos() const {return m_pos;}

    /** Set position in the stream. */
    void setPos(size_t pos) {m_pos = pos;}

    /** Get data vector. */
    std::vector<unsigned int>& getStore() {return m_store;}

    /** Resize data vector to the current position. */
    void resize()
    {
      m_store.resize((m_pos + 31) / 32);
    }
  protected:
    size_t m_pos; /**< Current position in the storage. */
    std::vector<unsigned int> m_store; /**< The bit storage. */
  };

  class ECLCompress {
  public:
    virtual ~ECLCompress() {};
    virtual void compress(BitStream& out, const int* adc) = 0;
    virtual void uncompress(BitStream& in, int* adc) = 0;
  };

  class ECLBaseCompress: public ECLCompress {
  public:
    void compress(BitStream& out, const int* adc);
    void uncompress(BitStream& out, int* adc);
  };

  class ECLDeltaCompress: public ECLCompress {
  public:
    void compress(BitStream& out, const int* adc);
    void uncompress(BitStream& out, int* adc);
  };

  class ECLDCTCompress: public ECLCompress {
  public:
    void compress(BitStream& out, const int* adc);
    void uncompress(BitStream& out, int* adc);
  };

  ECLCompress* selectAlgo(int compAlgo);
}
#endif
