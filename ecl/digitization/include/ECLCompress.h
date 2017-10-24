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
  // find the log base 2 of 32-bit v
  unsigned int ilog2(unsigned int v);

  /*!
    bit stream struct
  */
  class BitStream {
  public:
    // default constructor for ROOT
    BitStream(): _pos(0) {}

    // constructor with reserved space -- for packing
    BitStream(int n): _pos(0)
    {
      _store.resize(n);
      memset(_store.data(), 0, _store.size()*sizeof(unsigned int));
    }

    // push n least significant bits of "value" to the stream. Update current position accordingly
    void putNBits(unsigned int value, unsigned int n)
    {
      unsigned int bpos = _pos % 32, wpos = _pos / 32;
      value &= (1 << n) - 1;
      _store[wpos] |= value << bpos;
      if (bpos + n > 32) _store[wpos + 1] = value >> (32 - bpos);
      _pos += n;
    }

    // fetch n bits. Update current position accordingly
    unsigned int getNBits(unsigned int n)
    {
      unsigned int bpos = _pos % 32, wpos = _pos / 32;
      unsigned int res = _store[wpos] >> bpos;
      if (bpos + n > 32) res |= _store[wpos + 1] << (32 - bpos);
      _pos += n;
      return res & ((1 << n) - 1);
    }

    // get current position in the stream
    size_t getPos() const {return _pos;}

    // set position in the stream
    void setPos(size_t pos) {_pos = pos;}

    // get data vector
    std::vector<unsigned int>& getStore() {return _store;}

    // resize data vector to the current length
    void resize()
    {
      _store.resize((_pos + 31) / 32);
    }
  protected:
    // position in the stream
    size_t _pos;
    // storage
    std::vector<unsigned int> _store;
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
}
#endif
