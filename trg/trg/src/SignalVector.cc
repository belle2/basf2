/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGSignalVector.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a bundle of digitized signals.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES

#include <string>
#include <algorithm>
#include <limits>
#include <iostream>
#include "trg/trg/Utilities.h"
#include "trg/trg/Clock.h"
#include "trg/trg/Signal.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/State.h"

using namespace std;

namespace Belle2 {

  TRGSignalVector::TRGSignalVector(const string& name,
                                   const TRGClock& c,
                                   unsigned size)
    : _name(name),
      _clock(& c)
  {
    for (unsigned i = 0; i < size; i++) {
      const string n = _name + ":bit" + TRGUtilities::itostring(i);
      TRGSignal t(n, c);
      push_back(t);
    }
  }

  TRGSignalVector::TRGSignalVector(const TRGSignalVector& t) :
    std::vector<TRGSignal>(),
    _name("CopyOf" + t._name)
  {
    _clock = t._clock;
    const unsigned n = t.size();
    for (unsigned i = 0; i < n; i++)
      push_back(t[i]);
  }

  TRGSignalVector::TRGSignalVector(const TRGSignal& t) :
    _name("VectorOf" + t.name())
  {
    _clock = & t.clock();
    push_back(t);
  }

  TRGSignalVector::~TRGSignalVector()
  {
  }

  void
  TRGSignalVector::dump(const string& msg,
                        const string& pre) const
  {

    cout << pre << _name << ":" << size() << " signal(s)" << endl;

    const bool det = msg.find("detail") != string::npos;
    const bool clk = msg.find("clock") != string::npos;

    if (det || clk)
      cout << pre << ":clock=" << _clock->name();

    if (det)
      for (unsigned i = 0; i < size(); i++)
        (* this)[i].dump(msg, "    " +  pre + "bit" +
                         TRGUtil::itostring(i) + ":");
    else
      for (unsigned i = 0; i < size(); i++)
        if ((* this)[i].active())
          (* this)[i].dump(msg, "    " + pre + "bit" +
                           TRGUtil::itostring(i) + ":");
  }

// TRGSignalVector
// TRGSignalVector::operator+(const TRGSignalVector & left) const {
//     TRGSignalVector t;
//     t.push_back(this);
//     t.push_back(& left);
//     return t;
// }

  TRGSignalVector&
  TRGSignalVector::operator+=(const TRGSignal& left)
  {
    push_back(left);
    return * this;
  }

  TRGSignalVector&
  TRGSignalVector::operator+=(const TRGSignalVector& left)
  {
    for (unsigned i = 0; i < left.size(); i++)
      push_back(left[i]);
    return * this;
  }

  bool
  TRGSignalVector::active(void) const
  {
    for (unsigned i = 0; i < size(); i++)
      if ((* this)[i].active())
        return true;
    return false;
  }

  bool
  TRGSignalVector::active(int c) const
  {
    TRGState s = state(c);
    if (s.active())
      return true;
    else
      return false;
  }

  std::vector<int>
  TRGSignalVector::stateChanges(void) const
  {
    std::vector<int> list;
    const unsigned n = size();
    for (unsigned i = 0; i < n; i++) {
      vector<int> a = (* this)[i].stateChanges();
      for (unsigned j = 0; j < a.size(); j++)
        list.push_back(a[j]);
    }

    //...Sorting...
    std::sort(list.begin(), list.end());

    //...Remove duplicated clock...
    std::vector<int> list2;
    int last = numeric_limits<int>::min();
    for (unsigned i = 0; i < list.size(); i++) {
      const int j = list[i];
      if (j != last) {
        list2.push_back(j);
        last = j;
      }
    }

    return list2;
  }

  TRGState
  TRGSignalVector::state(int clockPosition) const
  {
    std::vector<bool> list;
    const unsigned n = size();
    for (unsigned i = 0; i < n; i++)
      list.push_back((* this)[i].state(clockPosition));
    return TRGState(list);
  }

  const TRGClock&
  TRGSignalVector::clock(const TRGClock& c)
  {
    _clock = & c;

    for (unsigned i = 0; i < size(); i++) {
      TRGSignal& t = (* this)[i];
      t.clock(c);
    }

    return * _clock;
  }

  const TRGSignalVector&
  TRGSignalVector::set(const TRGState& s, int cp)
  {
    const unsigned n = s.size();
    for (unsigned i = 0; i < n; i++) {
      TRGSignal& signal = (* this)[i];
      signal.set(cp, cp + 1, s[i]);
    }
    return * this;
  }

  bool
  TRGSignalVector::operator==(const TRGSignalVector& a) const
  {
    if (size() != a.size()) {
      // cout << "TRGSignalVector::operator== : different size:"
      //      << size() << "," << a.size() << endl;
      return false;
    }
    for (unsigned i = 0; i < size(); i++) {
      if ((* this)[i] != a[i]) {
        // cout << "TRGSignalVector::operator== : different signal:i="
        //      << i << endl;
        // (* this)[i].dump();
        // a[i].dump();
        return false;
      }
    }

    return true;
  }


} // namespace Belle2
