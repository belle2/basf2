/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <algorithm>
#include <iostream>
#include <cctype>
#include "trg/trg/Clock.h"
#include "trg/trg/Signal.h"

using namespace std;

namespace Belle2 {

  TRGSignal::TRGSignal(const TRGClock& c) :
    _name("?"),
    _clock(& c),
    _history()
  {
  }

  TRGSignal::TRGSignal(const TRGTime& t0, const TRGTime& t1) :
    _name("?"),
    _clock(& t0.clock()),
    _history()
  {
#if TRG_DEBUG
    if ((& t0.clock()) != (& t1.clock()))
      cout << "TRGSignal !!! signal is made with two different clocks"
           << endl
           << "    t0.clock=" << t0.clock().name() << endl
           << "    t1.clock=" << t1.clock().name() << endl;
#endif

    _history.push_back(t0);
    _history.push_back(t1);

#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::TRGSignal(const TRGClock& c, int t0, int t1) :
    _name("?"),
    _clock(& c),
    _history()
  {
    TRGTime time0(t0, true, c);
    TRGTime time1(t1, false, c);
    _history.push_back(time0);
    _history.push_back(time1);

#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::TRGSignal(const TRGClock& c, double t0, double t1) :
    _name("?"),
    _clock(& c),
    _history()
  {
    TRGTime time0(t0, true, c);
    TRGTime time1(t1, false, c);
    _history.push_back(time0);
    _history.push_back(time1);

#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::TRGSignal(const string& name, const TRGClock& c) :
    _name(name),
    _clock(& c),
    _history()
  {
#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::TRGSignal(const TRGSignal& t) :
//  _history(t._history),
    _name(t._name),
    _clock(t._clock)
  {
    const unsigned n = t._history.size();
    for (unsigned i = 0; i < n; i++) {
      _history.push_back(t._history[i]);
    }
#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::TRGSignal(const TRGTime& t) :
    _name(t.name()),
    _clock(& t.clock())
  {

    //...Check edge...
    if (t.edge()) {

      //...Store itself...
      _history.push_back(t);

      //...Set falling edge...
      _history.push_back(t.clock().maxTRGTime(false));
    } else {

      //...Set rising edge...
      _history.push_back(t.clock().minTRGTime(true));

      //...Store itself...
      _history.push_back(t);
    }
#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::~TRGSignal()
  {
#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  void
  TRGSignal::dump(const string& msg,
                  const string& pre) const
  {

    string tmp;
    tmp.resize(_name.size());
    transform(_name.cbegin(), _name.cend(), tmp.begin(), ::toupper);

    const bool ctr = tmp.find("CLOCKCOUNTER") != string::npos;

    cout << pre << _name << ":#signal=" << _history.size();

    if (msg.find("clock") != string::npos ||
        msg.find("detail") != string::npos) {
      cout << ":clock=" << _clock->name();
    }

    cout << endl;

    if (_history.size() && (! ctr)) {
      for (unsigned i = 0; i < _history.size(); i++)
        _history[i].dump(msg, pre + "    ");
    }
  }

  TRGSignal
  TRGSignal::operator&(const TRGSignal& left) const
  {

    //...Collect state changes...
    vector<int> sc0 = stateChanges();
    vector<int> sc1 = left.stateChanges();
    sc0.insert(sc0.end(), sc1.begin(), sc1.end());

    //...Sorting...
    std::sort(sc0.begin(), sc0.end());

    //...Remove multiple same clock...
    vector<int> sc2;
    int last = _clock->min();
    for (unsigned i = 0; i < sc0.size(); i++) {
      const int j = sc0[i];
      if (j != last) {
        sc2.push_back(j);
        last = j;
      }
    }

    //...Make a new signal...
    string name = "(" + _name + ")&(" + left._name + ")";
    TRGSignal t(name, * _clock);
    bool active = false;
    for (unsigned i = 0; i < sc2.size(); i++) {
      const int j = sc2[i];
      if ((! active) & state(j) & left.state(j)) {
        active = true;
        t._history.push_back(TRGTime(j, true, * _clock));
      } else if (active & ((! state(j)) | (! left.state(j)))) {
        active = false;
        t._history.push_back(TRGTime(j, false, * _clock));
      }
    }

#if TRG_DEBUG
    consistencyCheck();
#endif

    return t;
  }

  TRGSignal&
  TRGSignal::operator&=(const TRGSignal& left)
  {
    TRGSignal t = (* this) & left;

    _history.clear();
    _history = t._history;

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * this;
  }

// vector<TRGTime>
// TRGSignal::andOperation(const vector<TRGTime> & history) {

//     //...And operation...
//     const unsigned n = history.size();
//     unsigned riseC = 0;
//     bool signal = false;
//     vector<TRGTime> tmp;
//     for (unsigned i = 0; i < n; i++) {
//         const bool edge = history[i].edge();

//         if (edge)
//             ++riseC;
//         else
//             --riseC;

//         // cout << "riseC,i,e,t=" << riseC << "," << i << "," << edge
//         //      << "," << history[i].time() << endl;

//         if (riseC == 2) {
//             tmp.push_back(history[i]);
//             signal = true;
//         }
//         else if (signal && (riseC == 1)) {
//             tmp.push_back(history[i]);
//             signal = false;
//         }
//     }

//     return tmp;
// }

  TRGSignal
  TRGSignal::operator|(const TRGSignal& left) const
  {
    TRGSignal t(* this);
    t._history.insert(t._history.end(),
                      left._history.begin(),
                      left._history.end());
    t._name = "(" + t._name + ")|(" + left._name + ")";
    std::sort(t._history.begin(), t._history.end(), TRGTime::sortByTime);

    //...And operation...
    t._history = orOperation(t._history);

    return t;
  }

  TRGSignal&
  TRGSignal::operator|=(const TRGSignal& left)
  {
    _history.insert(_history.end(),
                    left._history.begin(),
                    left._history.end());
    this->_name = "(" + this->_name + ")&(" + left._name + ")";
    std::sort(_history.begin(), _history.end(), TRGTime::sortByTime);

    //...And operation...
    _history = orOperation(_history);

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * this;
  }

  vector<TRGTime>
  TRGSignal::orOperation(const vector<TRGTime>& history)
  {

    //...And operation...
    const unsigned n = history.size();
    vector<TRGTime> tmp;
    unsigned signal = 0;
    for (unsigned i = 0; i < n; i++) {
      const bool edge = history[i].edge();

      if (edge) {
        if (signal == 0)
          tmp.push_back(history[i]);
        ++signal;
      } else {
        if (signal == 1)
          tmp.push_back(history[i]);
        --signal;
      }

//         cout << "i,time,edge,signal=" << i << "," << history[i].time()
//              << "," << edge << "," << signal << endl;

    }

    return tmp;
  }

  TRGSignal&
  TRGSignal::widen(unsigned width)
  {

    //...Check rising edges...
    const unsigned n = _history.size();
    for (unsigned i = 0; i < n; i++) {
      const bool edge = _history[i].edge();

      if (! edge) {
        const int t0 = _history[i - 1].time();
        const int t1 = _history[i].time();
        if ((t1 - t0) < int(width))
          _history[i].time(t0 + width);
      }
    }

    // Merge overlapping history
    std::sort(_history.begin(), _history.end(), TRGTime::sortByTime);
    _history = orOperation(_history);

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * this;
  }

  std::vector<int>
  TRGSignal::stateChanges(void) const
  {
    std::vector<int> list;
    const unsigned n = _history.size();
    for (unsigned i = 0; i < n; i++)
      list.push_back(_history[i].time());
    return list;
  }

  const TRGClock&
  TRGSignal::clock(const TRGClock& c)
  {
    _clock = & c;

    const unsigned n = _history.size();
    for (unsigned i = 0; i < n; i++)
      _history[i].clock(c);

    //...Check pulse width...
    for (unsigned i = 0; i < n; i++) {
      const bool edge = _history[i].edge();

      if (! edge) {
        const int t0 = _history[i - 1].time();
        const int t1 = _history[i].time();
        const unsigned w = t1 - t0;
        if (w == 0)
          _history[i].shift(1);
      }
    }

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * _clock;
  }

  unsigned
  TRGSignal::width(unsigned a) const
  {
    unsigned j = 0;
    const unsigned n = _history.size();
    for (unsigned i = 0; i < n; i++) {
      const bool edge = _history[i].edge();

      if (! edge) {
        const int t0 = _history[i - 1].time();
        const int t1 = _history[i].time();
        const unsigned w = t1 - t0;
        if (j == a) {
          return w;
        }
        ++j;
      }
    }
    return 0;
  }

  const TRGSignal&
  TRGSignal::set(int t0, int t1, bool state)
  {
    if (! state)
      return unset(t0, t1);

    TRGSignal s(clock(), t0, t1);
    (* this) |= s;

#if TRG_DEBUG
    if (consistencyCheck())
      cout << "TRGSignal::set ... t0, t1=" << t0 << "," << t1 << endl;
#endif

    return * this;
  }

  const TRGSignal&
  TRGSignal::unset(int t0, int t1)
  {
    if (active(t0, t1)) {
      TRGSignal x(clock(), t0, t1);
      x.invert();
      (* this) &= x;
    }

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * this;
  }

  void
  TRGSignal::sort(void)
  {
    std::sort(_history.begin(), _history.end(), TRGTime::sortByTime);

#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  bool
  TRGSignal::consistencyCheck(void) const
  {
    const unsigned n = _history.size();
    if (n % 2) {
      cout << "TRGSignal::consistencyCheck !!! "
           << "history has odd number entires : n=" << n << endl;
      dump("detail", "!!! ");
    }

    if (n < 2)
      return true;

    unsigned err = 0;
    unsigned errTiming = 0;
    for (unsigned i = 0; i < n; i++) {
      if (i % 2)
        continue;

      const TRGTime& t0 = _history[i];
      const TRGTime& t1 = _history[i + 1];

      if (t0.edge() != true)
        err |= (1 << i);
      if (t1.edge() != false)
        err |= (1 << (i + 1));

      if ((t0.time() == t1.time()) || (t0.time() > t1.time()))
        errTiming |= (1 << (i + 1));
    }

    if (err || errTiming) {
      if (err) {
        cout << "TRGSignal::consistencyCheck !!! err in edge history"
             << endl;
      }
      if (errTiming) {
        cout << "TRGSignal::consistencyCheck !!! err in time history"
             << endl;
      }
      dump("detail", "!!! ");
      return true;
    }

    return false;
  }

  const TRGSignal&
  TRGSignal::invert(void)
  {
    if (_history.size()) {
      for (unsigned i = 0; i < _history.size(); i++)
        _history[i].reverse();
      if (_history[0].time() > _clock->min()) {
        TRGTime t0(_clock->min(), true, * _clock);
        _history.insert(_history.begin(), t0);
      } else {
        _history.erase(_history.begin());
      }
      if (_history.end()->time() < _clock->max()) {
        TRGTime t0(_clock->max(), false, * _clock);
        _history.push_back(t0);
      } else {
        _history.erase(_history.end());
      }
    } else {
      TRGTime time0(_clock->min(), true, * _clock);
      TRGTime time1(_clock->max(), false, * _clock);
      _history.push_back(time0);
      _history.push_back(time1);
    }

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * this;
  }

  bool
  TRGSignal::operator==(const TRGSignal& a) const
  {
    if (_history.size() != a._history.size())
      return false;

    for (unsigned i = 0; i < _history.size(); i++) {
      if (_history[i] != a._history[i])
        return false;
    }

    return true;
  }

} // namespace Belle2
