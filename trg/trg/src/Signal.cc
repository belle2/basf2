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

namespace Belle2 {

  TRGSignal::TRGSignal(const TRGClock& c) :
    m_name("?"),
    m_clock(& c),
    m_history()
  {
  }

  TRGSignal::TRGSignal(const TRGTime& t0, const TRGTime& t1) :
    m_name("?"),
    m_clock(& t0.clock()),
    m_history()
  {
#if TRG_DEBUG
    if ((& t0.clock()) != (& t1.clock()))
      std::cout << "TRGSignal !!! signal is made with two different clocks"
                << std::endl
                << "    t0.clock=" << t0.clock().name() << std::endl
                << "    t1.clock=" << t1.clock().name() << std::endl;
#endif

    m_history.push_back(t0);
    m_history.push_back(t1);

#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::TRGSignal(const TRGClock& c, int t0, int t1) :
    m_name("?"),
    m_clock(& c),
    m_history()
  {
    TRGTime time0(t0, true, c);
    TRGTime time1(t1, false, c);
    m_history.push_back(time0);
    m_history.push_back(time1);

#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::TRGSignal(const TRGClock& c, double t0, double t1) :
    m_name("?"),
    m_clock(& c),
    m_history()
  {
    TRGTime time0(t0, true, c);
    TRGTime time1(t1, false, c);
    m_history.push_back(time0);
    m_history.push_back(time1);

#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::TRGSignal(const std::string& name, const TRGClock& c) :
    m_name(name),
    m_clock(& c),
    m_history()
  {
#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::TRGSignal(const TRGSignal& t) :
//  m_history(t.m_history),
    m_name(t.m_name),
    m_clock(t.m_clock)
  {
    const unsigned n = t.m_history.size();
    for (unsigned i = 0; i < n; i++) {
      m_history.push_back(t.m_history[i]);
    }
#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  TRGSignal::TRGSignal(const TRGTime& t) :
    m_name(t.name()),
    m_clock(& t.clock())
  {

    //...Check edge...
    if (t.edge()) {

      //...Store itself...
      m_history.push_back(t);

      //...Set falling edge...
      m_history.push_back(t.clock().maxTRGTime(false));
    } else {

      //...Set rising edge...
      m_history.push_back(t.clock().minTRGTime(true));

      //...Store itself...
      m_history.push_back(t);
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
  TRGSignal::dump(const std::string& msg,
                  const std::string& pre) const
  {

    std::string tmp;
    tmp.resize(m_name.size());
    transform(m_name.cbegin(), m_name.cend(), tmp.begin(), ::toupper);

    const bool ctr = tmp.find("CLOCKCOUNTER") != std::string::npos;

    std::cout << pre << m_name << ":#signal=" << m_history.size();

    if (msg.find("clock") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
      std::cout << ":clock=" << m_clock->name();
    }

    std::cout << std::endl;

    if (m_history.size() && (! ctr)) {
      for (unsigned i = 0; i < m_history.size(); i++)
        m_history[i].dump(msg, pre + "    ");
    }
  }

  TRGSignal
  TRGSignal::operator&(const TRGSignal& left) const
  {

    //...Collect state changes...
    std::vector<int> sc0 = stateChanges();
    std::vector<int> sc1 = left.stateChanges();
    sc0.insert(sc0.end(), sc1.begin(), sc1.end());

    //...Sorting...
    std::sort(sc0.begin(), sc0.end());

    //...Remove multiple same clock...
    std::vector<int> sc2;
    int last = m_clock->min();
    for (unsigned i = 0; i < sc0.size(); i++) {
      const int j = sc0[i];
      if (j != last) {
        sc2.push_back(j);
        last = j;
      }
    }

    //...Make a new signal...
    std::string name = "(" + m_name + ")&(" + left.m_name + ")";
    TRGSignal t(name, * m_clock);
    bool active = false;
    for (unsigned i = 0; i < sc2.size(); i++) {
      const int j = sc2[i];
      if ((! active) and state(j) and left.state(j)) {
        active = true;
        t.m_history.push_back(TRGTime(j, true, * m_clock));
      } else if (active and ((! state(j)) or (! left.state(j)))) {
        active = false;
        t.m_history.push_back(TRGTime(j, false, * m_clock));
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

    m_history.clear();
    m_history = t.m_history;

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * this;
  }

// std::vector<TRGTime>
// TRGSignal::andOperation(const std::vector<TRGTime> & history) {

//     //...And operation...
//     const unsigned n = history.size();
//     unsigned riseC = 0;
//     bool signal = false;
//     std::vector<TRGTime> tmp;
//     for (unsigned i = 0; i < n; i++) {
//         const bool edge = history[i].edge();

//         if (edge)
//             ++riseC;
//         else
//             --riseC;

//         // std::cout << "riseC,i,e,t=" << riseC << "," << i << "," << edge
//         //      << "," << history[i].time() << std::endl;

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
    t.m_history.insert(t.m_history.end(),
                       left.m_history.begin(),
                       left.m_history.end());
    t.m_name = "(" + t.m_name + ")|(" + left.m_name + ")";
    std::sort(t.m_history.begin(), t.m_history.end(), TRGTime::sortByTime);

    //...And operation...
    t.m_history = orOperation(t.m_history);

    return t;
  }

  TRGSignal&
  TRGSignal::operator|=(const TRGSignal& left)
  {
    m_history.insert(m_history.end(),
                     left.m_history.begin(),
                     left.m_history.end());
    this->m_name = "(" + this->m_name + ")&(" + left.m_name + ")";
    std::sort(m_history.begin(), m_history.end(), TRGTime::sortByTime);

    //...And operation...
    m_history = orOperation(m_history);

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * this;
  }

  std::vector<TRGTime>
  TRGSignal::orOperation(const std::vector<TRGTime>& history)
  {

    //...And operation...
    const unsigned n = history.size();
    std::vector<TRGTime> tmp;
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

//         std::cout << "i,time,edge,signal=" << i << "," << history[i].time()
//              << "," << edge << "," << signal << std::endl;

    }

    return tmp;
  }

  TRGSignal&
  TRGSignal::widen(unsigned width)
  {

    //...Check rising edges...
    const unsigned n = m_history.size();
    for (unsigned i = 0; i < n; i++) {
      const bool edge = m_history[i].edge();

      if (! edge) {
        const int t0 = m_history[i - 1].time();
        const int t1 = m_history[i].time();
        if ((t1 - t0) < int(width))
          m_history[i].time(t0 + width);
      }
    }

    // Merge overlapping history
    std::sort(m_history.begin(), m_history.end(), TRGTime::sortByTime);
    m_history = orOperation(m_history);

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * this;
  }

  std::vector<int>
  TRGSignal::stateChanges(void) const
  {
    std::vector<int> list;
    const unsigned n = m_history.size();
    for (unsigned i = 0; i < n; i++)
      list.push_back(m_history[i].time());
    return list;
  }

  const TRGClock&
  TRGSignal::clock(const TRGClock& c)
  {
    m_clock = & c;

    const unsigned n = m_history.size();
    for (unsigned i = 0; i < n; i++)
      m_history[i].clock(c);

    //...Check pulse width...
    for (unsigned i = 0; i < n; i++) {
      const bool edge = m_history[i].edge();

      if (! edge) {
        const int t0 = m_history[i - 1].time();
        const int t1 = m_history[i].time();
        const unsigned w = t1 - t0;
        if (w == 0)
          m_history[i].shift(1);
      }
    }

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * m_clock;
  }

  unsigned
  TRGSignal::width(unsigned a) const
  {
    unsigned j = 0;
    const unsigned n = m_history.size();
    for (unsigned i = 0; i < n; i++) {
      const bool edge = m_history[i].edge();

      if (! edge) {
        const int t0 = m_history[i - 1].time();
        const int t1 = m_history[i].time();
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
      std::cout << "TRGSignal::set ... t0, t1=" << t0 << "," << t1 << std::endl;
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
    std::sort(m_history.begin(), m_history.end(), TRGTime::sortByTime);

#if TRG_DEBUG
    consistencyCheck();
#endif
  }

  bool
  TRGSignal::consistencyCheck(void) const
  {
    const unsigned n = m_history.size();
    if (n % 2) {
      std::cout << "TRGSignal::consistencyCheck !!! "
                << "history has odd number entries : n=" << n << std::endl;
      dump("detail", "!!! ");
    }

    if (n < 2)
      return true;

    unsigned err = 0;
    unsigned errTiming = 0;
    for (unsigned i = 0; i < n; i++) {
      if (i % 2)
        continue;

      const TRGTime& t0 = m_history[i];
      const TRGTime& t1 = m_history[i + 1];

      if (t0.edge() != true)
        err |= (1 << i);
      if (t1.edge() != false)
        err |= (1 << (i + 1));

      if ((t0.time() == t1.time()) || (t0.time() > t1.time()))
        errTiming |= (1 << (i + 1));
    }

    if (err || errTiming) {
      if (err) {
        std::cout << "TRGSignal::consistencyCheck !!! err in edge history"
                  << std::endl;
      }
      if (errTiming) {
        std::cout << "TRGSignal::consistencyCheck !!! err in time history"
                  << std::endl;
      }
      dump("detail", "!!! ");
      return true;
    }

    return false;
  }

  const TRGSignal&
  TRGSignal::invert(void)
  {
    if (m_history.size()) {
      for (unsigned i = 0; i < m_history.size(); i++)
        m_history[i].reverse();
      if (m_history[0].time() > m_clock->min()) {
        TRGTime t0(m_clock->min(), true, * m_clock);
        m_history.insert(m_history.begin(), t0);
      } else {
        m_history.erase(m_history.begin());
      }
      if (m_history.back().time() < m_clock->max()) {
        TRGTime t0(m_clock->max(), false, * m_clock);
        m_history.push_back(t0);
      } else {
        m_history.erase(m_history.end());
      }
    } else {
      TRGTime time0(m_clock->min(), true, * m_clock);
      TRGTime time1(m_clock->max(), false, * m_clock);
      m_history.push_back(time0);
      m_history.push_back(time1);
    }

#if TRG_DEBUG
    consistencyCheck();
#endif

    return * this;
  }

  bool
  TRGSignal::operator==(const TRGSignal& a) const
  {
    if (m_history.size() != a.m_history.size())
      return false;

    for (unsigned i = 0; i < m_history.size(); i++) {
      if (m_history[i] != a.m_history[i])
        return false;
    }

    return true;
  }

} // namespace Belle2
