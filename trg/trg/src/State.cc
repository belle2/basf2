//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGState.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a state of multi bits
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <algorithm>
#include <iostream>
#include <ctype.h>
#include <cstring>
#include "trg/trg/Utilities.h"
#include "trg/trg/State.h"

using namespace std;

namespace Belle2 {

  const unsigned
  TRGState::_su = sizeof(unsigned);

  const unsigned
  TRGState::_bsu = 8 * sizeof(unsigned);

  TRGState::TRGState(unsigned bitSize)
    : _size(bitSize),
      _n(0),
      _state(0)
  {
    _n = _size / _bsu;
    if (_size % _bsu) ++_n;
    if (_n)
      _state = (unsigned*) calloc(_n, _su);
  }

  TRGState::TRGState(unsigned bitSize, unsigned value)
    : _size(bitSize),
      _n(0),
      _state(0)
  {
    _n = _size / _bsu;
    if (_size % _bsu) ++_n;
    if (_n)
      _state = (unsigned*) calloc(_n, _su);

    for (unsigned i = 0; i < bitSize; i++) {
      const unsigned wp = i / _bsu;
      const unsigned bp = i % _bsu;
      if (value & (1 << i))
        _state[wp] |= (1 << bp);
    }
  }

  TRGState::TRGState(unsigned bitSize, const bool* const s)
    : _size(bitSize),
      _n(0),
      _state(0)
  {

    _n = _size / _bsu;
    if (_size % _bsu) ++_n;
    _state = (unsigned*) calloc(_n, _su);

    for (unsigned i = 0; i < _size; i++) {
      const unsigned wp = i / _bsu;
      const unsigned bp = i % _bsu;
      if (s[i])
        _state[wp] |= (1 << bp);
      else
        _state[wp] &= ~(1 << bp);
    }
  }

  TRGState::TRGState(vector<bool> states)
    : _size(0),
      _n(0),
      _state(0)
  {

    _size = states.size();
    _n = _size / _bsu;
    if (_size % _bsu) ++_n;
    _state = (unsigned*) calloc(_n, _su);

    for (unsigned i = 0; i < _size; i++) {
      const unsigned wp = i / _bsu;
      const unsigned bp = i % _bsu;
      if (states[i])
        _state[wp] |= (1 << bp);
      else
        _state[wp] &= ~(1 << bp);

//  cout << "size,given,states=" << _size << "," << states[i] << ","
//       << _state[wp] << endl;

    }
  }

  TRGState::TRGState(vector<unsigned>& states, unsigned order)
    : _size(0),
      _n(0),
      _state(0)
  {

    _size = states.size() * _bsu;
    _n = _size / _bsu;
    _state = (unsigned*) calloc(_n, _su);

    for (unsigned i = 0; i < _n; i++) {

      if (order == 0) _state[i] = states[i];
      else _state[_n - 1 - i] = states[i];
//  cout << "size,given,states=" << _size << "," << states[i] << ","
//       << _state[wp] << endl;

    }
  }

  TRGState::TRGState(const char* inChar, unsigned inType)
    : _size(0),
      _n(0),
      _state(0)
  {

    if (inType == 0) {
      _size = strlen(inChar);
      // Check if all values are binary
      for (unsigned iBit = 0; iBit < _size; iBit++) {
        if (!(inChar[iBit] == '0' || inChar[iBit] == '1')) {
#ifdef TRG_DEBUG
          cout << "TRGState::TRGState !!! invalid char found : aborted"
               << endl;
          cout << "                       invalid char = [" << inChar[iBit]
               << "]" << endl;
#endif
          return;
        }
      }
    } else if (inType == 1) {
      _size = strlen(inChar) * 4;
      // Check if all values are hex
      for (unsigned iChar = 0; iChar < _size / 4; iChar++) {
        if (!isxdigit(inChar[iChar])) {
#ifdef TRG_DEBUG
          cout << "TRGState::TRGState !!! invalid char found : aborted"
               << endl;
          cout << "                       invalid char = ["
               << inChar[iChar] << "]" << endl;
#endif
          return;
        }
      }
    } else {
#ifdef TRG_DEBUG
      cout << "TRGState::TRGState !!! invalid type : aborted"
           << endl;
      cout << "                       invalid type = " << inType
           << endl;
#endif
      return;
    }

    _n = _size / _bsu;
    if (_size % _bsu) ++_n;
    _state = (unsigned*) calloc(_n, _su);
    if (inType == 0) {
      for (unsigned iBit = 0; iBit < _size; iBit++) {
        const unsigned wp = iBit / _bsu;
        const unsigned bp = iBit % _bsu;
        if (inChar[_size - 1 - iBit] == '1')
          _state[wp] |= (1 << bp);
        else
          _state[wp] &= ~(1 << bp);
      }
    } else if (inType == 1) {
      for (unsigned iChar = 0; iChar < _size / 4; iChar++) {
        if (iChar % 8 == 0) _state[iChar / 8] = 0;
        short unsigned t_int;
        unsigned charP = _size / 4 - 1 - iChar;
        if (inChar[charP] > 47 && inChar[charP] < 58) t_int = inChar[charP] - 48;
        else t_int = inChar[charP] - 97 + 10;
        _state[iChar / 8] += t_int << ((iChar % 8) * 4);
      }
    }
  }

  TRGState::~TRGState()
  {
    if (_state)
      free(_state);
  }

  void
  TRGState::dump(const string& msg,
                 const string& pre) const
  {

//  bool bin = true;
    bool large = (_size > 60) || msg.find("large") != string::npos;

    if (! large) {
      cout << pre << "size=" << _size << ",";
      for (unsigned i = 0; i < _size; i++) {
        const unsigned j = _size - i - 1;
        if ((j % 8) == 7)
          cout << "_";
        if ((* this)[j])
          cout << "1";
        else
          cout << "0";
      }
      cout << dec << endl;
    } else {
      cout << pre << "size=" << _size << endl;

      const unsigned nPerLines = 64;
      const unsigned lines = _size / nPerLines + 1;
      const unsigned rem = _size % nPerLines;

      cout << "TRGState" <<
           "    +56      +48      +40      +32      +24      +16       +8"
           << "       +0" << endl;

      bool skipLast = false;
      for (unsigned i = 0; i < lines; i++) {
        const unsigned n0 = (i == 0) ? rem - 1 : nPerLines - 1;
        const int n1 = 0;
        const int os = (lines - i - 1) * nPerLines;

        const string b = TRGUtilities::itostring(os);

        TRGState s = subset(n1 + os, n0 - n1 + 1);

//            s.dump("", "*** ");

        bool skip = false;
        if (! s.active())
          skip = true;

        if (skip) {
          if (!skipLast) {
            cout << "... (all zero)" << endl;
          }

          skipLast = true;
          continue;
        }

        skipLast = false;

        cout.width(5);
        cout << b;
        cout << " ";

        if (i == 0) {
          unsigned n = nPerLines - rem + (nPerLines - rem) / 8;
          if (rem % 8) ++n;
          for (unsigned j = 0; j < n; j++)
            cout << " ";
        }

        for (int j = n0; j >= n1; --j) {
          const unsigned v = (* this)[j + os];

          if ((j % 8) == 7)
            cout << "_";
          if (v)
            cout << "1";
          else
            cout << "0";
        }
        cout << endl;
      }
    }

    // else {
    //     cout << pre << "size=" << _size << ",0x";
    //     bool first = true;
    //     for (unsigned i = 0; i < _n; i++) {
    //         const unsigned j = _n - i - 1;
    //         if (((j % 4) == 3) && (! first))
    //      cout << "_";
    //         cout << hex << _state[j];
    //         first = false;
    //     }
    //     cout << dec << endl;

    //     cout << pre << "size=" << _size << ",0x";
    //     bool first = true;
    //     for (unsigned i = 0; i < _n; i++) {
    //         const unsigned j = _n - i - 1;
    //         if (! first)
    //      cout << "_";
    //         cout << hex << _state[j];
    //         cout << "(";
    //         for (unsigned k = 0; k < 4; k++) {
    //      unsigned c = ((_state[j] >> (4 - k + 1) * 8) & 0xff);
    //      if (c < 0x10)
    //          cout << ".";
    //      cout << c;
    //         }
    //         cout << ")";
    //         first = false;
    //     }
    //     cout << dec << endl;
    // }
  }

  TRGState&
  TRGState::operator+=(const TRGState& a)
  {

    //...Check final size...
    const unsigned sizeNew = _size + a.size();
    const unsigned sizeMax = _n * _bsu;
    const unsigned oldSize = _size;

    //...Necessary to extend...
    if (sizeNew > sizeMax) {
      unsigned nNew = sizeNew / _bsu;
      if (sizeNew % _bsu) ++nNew;
      unsigned* tmp = (unsigned*) calloc(nNew, _su);
      for (unsigned i = 0; i < _n; i++) {
        tmp[i] = _state[i];
      }
      if (_state)
        free(_state);
      _state = tmp;
      _n = nNew;
    }
    _size = sizeNew;

    for (unsigned i = 0; i < a.size(); i++) {
      unsigned j = i + oldSize;
      const unsigned wp = j / _bsu;
      const unsigned bp = j % _bsu;
      if (a[i])
        _state[wp] |= (1 << bp);
      else
        _state[wp] &= ~(1 << bp);

//  cout << "size,given,states=" << _size << "," << states[i] << ","
//       << _state[wp] << endl;

    }

    return (* this);
  }

  TRGState
  TRGState::subset(unsigned s, unsigned n) const
  {
    vector<bool> stack;
    for (unsigned i = s; i < s + n; i++) {
      if ((* this)[i])
        stack.push_back(true);
      else
        stack.push_back(false);
    }
    return TRGState(stack);
  }

  // cppcheck-suppress operatorEqToSelf
  TRGState& TRGState::operator=(const TRGState& a)
  {
    if (_state)
      free(_state);

    _size = a._size;
    _n = a._n;
    _state = (unsigned*) calloc(_n, _su);
    for (unsigned i = 0; i < _n; i++) {
      _state[i] = a._state[i];
    }

    return * this;
  }

  bool
  TRGState::operator<(const TRGState& a) const
  {
    const unsigned long long n0(* this);
    const unsigned long long n1(a);
    if (n0 < n1)
      return true;
    return false;
  }

} // namespace Belle2

namespace std {

  ostream&
  operator<<(ostream& out, const Belle2::TRGState& s)
  {
    for (unsigned i = 0; i < s.size(); i++) {
      const unsigned j = s.size() - i - 1;
      if (s.active(j))
        out << "1";
      else
        out << "0";
    }
    return out;
  }

}
