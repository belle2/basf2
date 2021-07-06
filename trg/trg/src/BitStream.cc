/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include "trg/trg/Utilities.h"
#include "trg/trg/BitStream.h"

using namespace std;

namespace Belle2 {

  TRGBitStream::TRGBitStream() :
    _name("unknown"),
    _sizeMax(0),
    _size(0),
    _stream(0)
  {
  }

  TRGBitStream::TRGBitStream(int size, const std::string& name) :
    _name(name),
    _sizeMax(size),
    _size(0),
    _stream(0)
  {
  }

  TRGBitStream::TRGBitStream(const TRGBitStream& a) :
    _name("CopyOf" + a._name),
    _sizeMax(a._sizeMax),
    _size(a._size),
    _stream(0)
  {
    for (unsigned i = 0; i < a._stream.size(); i++)
      _stream.push_back(new unsigned(* a._stream[i]));
  }

  TRGBitStream::TRGBitStream(const char* const c, unsigned s) :
    _name("unknown"),
    _sizeMax(0),
    _size(s),
    _stream(0)
  {
    unsigned sz = s / 8;
    if (s % 8) ++sz;
    unsigned sz4 = sz / 4;
    for (unsigned i = 0; i < sz4; i++) {
//  unsigned j = * (unsigned *) c[i * 4];
      unsigned j = * (unsigned*)(& (c[i * 4]));
      _stream.push_back(new unsigned(j));
    }
    if (sz % 4) {
      unsigned j = 0;
      for (unsigned i = 0; i < sz % 4; i++)
        j |= (c[sz4 + i] << i * 8);
      _stream.push_back(new unsigned(j));
    }
  }

  TRGBitStream::~TRGBitStream()
  {
    for (unsigned i = 0; i < _stream.size(); i++)
      delete _stream[i];
  }

  void
  TRGBitStream::dump(const string&,
                     const string& pre) const
  {

    const string tab = "    ";

    cout << pre << _name << ":size=" << dec << _size <<  endl;
    for (unsigned i = 0; i < _stream.size(); i++) {
      cout << pre << tab;
      if (i == _stream.size() - 1) {
        const unsigned last = _size % (sizeof(unsigned) * 8);

        if (last)
          cout << TRGUtilities::streamDisplay(* _stream[i], 0, last - 1);
      } else {
        cout << TRGUtilities::streamDisplay(* _stream[i], 0, 31);
      }
      cout << endl;
    }
  }

  void
  TRGBitStream::append(bool a)
  {
    if (_sizeMax) {
      if (_size >= _sizeMax) {
        cout << "TRGBitStream::append !!! stream is full :current size="
             << _size << ",max size=" << _sizeMax << endl;
        return;
      }
    }

    const unsigned storageSize = sizeof(unsigned) * 8 * _stream.size();
    ++_size;

    if (_size <= storageSize) {
      if (a) {
        // TODO This block does nothing, is it ok?
        //const unsigned position = _size % (sizeof(unsigned) * 8) - 1;
        //unsigned& last = * _stream.back();
        //last |= (1 << position);
      }
    } else {
      if (a)
        _stream.push_back(new unsigned(1));
      else
        _stream.push_back(new unsigned(0));
    }
  }

  vector<TRGSignal>
  TRGBitStream::TRGBitStream2TRGSignal(const TRGClock& clock,
                                       int initialClockPosition,
                                       vector<TRGBitStream*> stream)
  {

    vector<TRGSignal> t;

    //...Check the size of stream...
    if (stream.size() == 0) {
      cout << " !!! TRGBitStream::TRGBitStream2TRGSignal: given stream "
           << "has no data" << endl;
      return t;
    }

    //...Get bit size...
    const unsigned bs = stream[0]->size();
    const unsigned cs = stream.size();

    //...Preparation...
    vector<TRGSignal*> s;
    for (unsigned i = 0; i < bs; i++)
      s.push_back(new TRGSignal());

    //...Bit stream loop...
    for (unsigned i = 0; i < cs; i++) {

      //...Bit position loop...
      for (unsigned j = 0; j < bs; j++) {
        if (stream[i]->bit(j)) {
          TRGTime r(int(initialClockPosition + i), true, clock);
          TRGTime f = r;
          f.shift(1).reverse();
          (* s[j]) |= (r & f);
        }
      }
    }

    //...Return value...
    for (unsigned i = 0; i < s.size(); i++)
      t.push_back(* s[i]);
    return t;
  }


} // namespace Belle2
