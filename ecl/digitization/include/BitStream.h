/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <vector>
#include <framework/utilities/Utils.h>

namespace Belle2::ECL {

  /** Bit stream struct.
   * Class contains vector of unsigned ints as a storage and current bit position in the storage.
   * One can put and fetch up to 32 bits at once.
   */
  class BitStream {
  public:
    /** Default constructor for ROOT.
     *  Current position is at begining of the storage.
     */
    BitStream(): m_pos(0), m_store(1, 0) {}

    /** Constructor with the reserved and cleared storage prepared for
     *  incoming bits. Be sure the size is enough for incoming data
     *  since the class does not check bounds.  Current position is at
     *  begining of the storage.
     */
    explicit BitStream(int n): m_pos(0), m_store(n, 0) {}

    /** Push n least significant bits of "value" to the stream. Update current position accordingly.
     * @param value -- value to put in the stream
     * @param n -- how many least signfificant bits of "value" put in the stream (n<=32)
     */
    void putNBits(unsigned int value, unsigned int n)
    {
      unsigned int bpos = m_pos % 32, wpos = m_pos / 32;
      value &= 0xffffffffu >> (32 - n);
// check if we have enough space and double in size if necessary.
      if (branch_unlikely(m_pos + n > m_store.size() * 32)) m_store.resize(2 * m_store.size(), 0);
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
// make sure we don't access memory we don't own
      if (branch_unlikely(m_pos + n > m_store.size() * 32)) throw std::range_error("Not enough bits in stream");
      unsigned int res = m_store[wpos] >> bpos;
      if (bpos + n > 32) res |= m_store[wpos + 1] << (32 - bpos);
      m_pos += n;
      return res & (0xffffffffu >> (32 - n));
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
}
