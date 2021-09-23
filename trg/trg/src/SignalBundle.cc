/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#define TRG_SHORT_NAMES

#include <string>
#include <algorithm>
#include <limits>
#include <iostream>
#include <fstream>
#include "trg/trg/Utilities.h"
#include "trg/trg/Debug.h"
#include "trg/trg/Clock.h"
#include "trg/trg/Signal.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/SignalBundle.h"
#include "trg/trg/State.h"

using namespace std;

namespace Belle2 {

  TRGSignalBundle::TRGSignalBundle(const TRGClock& c)
    : _name("unknown"),
      _clock(& c)
  {
  }

  TRGSignalBundle::TRGSignalBundle(const string& name, const TRGClock& c)
    : _name(name),
      _clock(& c)
  {
  }

  TRGSignalBundle::TRGSignalBundle(const string& name,
                                   const TRGClock& c,
                                   const TRGSignalBundle& input,
                                   const unsigned outputBitSize,
                                   TRGState(* packer)(const TRGState&))
    : _name(name),
      _clock(& c)
  {

    //...Get state information...
    const vector<int> states = input.stateChanges();
    const unsigned nStates = states.size();

    //...Loop over all states...
    vector<TRGState*> outputStates;
    for (unsigned i = 0; i < nStates; i++) {
      TRGState s = input.state(states[i]);

      // if (TRGDebug::level()) {
      //     cout << TRGDebug::tab() << "Clock=" << states[i] << endl;
      // }

      outputStates.push_back(new TRGState((* packer)(s)));
    }

    //...Bit size of output...
    // unsigned outputSize = 0;
    // if (outputStates.size())
    //     outputSize = outputStates.back()->size();

    //...Creat a SignalVector...
    TRGSignalVector* sb = new TRGSignalVector(_name, c, outputBitSize);

    //...Make a SignalVector...
    // const TRGState &os0 = * outputStates[0];
    //       sb->set(os0, 0); delete &os0;
    // const unsigned n = outputStates.size();   // same as nStates

    for (unsigned i = 0; i < nStates; i++) {
      const TRGState* s = outputStates[i];
      sb->set((*s), states[i]);
      delete s;
    }

    push_back(sb);
  }

  TRGSignalBundle::TRGSignalBundle(const string& name,
                                   const TRGClock& c,
                                   const TRGSignalBundle& input,
                                   const unsigned outputBitSize,
                                   const unsigned registerBitSize,
                                   TRGState(* packer)(const TRGState&,
                                                      TRGState&,
                                                      bool&))
    : _name(name),
      _clock(& c)
  {

    const string sn = "TRGSignalBundle constructor(4)";
    TRGDebug::enterStage(sn);

    //...Get state information...
    const vector<int> states = input.stateChanges();
    const unsigned nStates = states.size();

    if (TRGDebug::level()) {
      cout << TRGDebug::tab() << "#states=" << nStates << endl;
      for (unsigned i = 0; i < nStates; i++)
        cout << TRGDebug::tab() << i << " : " << states[i] << endl;
    }

    //...Input is stably inactive...
    if (nStates == 0) {
      TRGSignalVector* sb = new TRGSignalVector(_name, c, outputBitSize);
      push_back(sb);
      TRGDebug::leaveStage(sn);
      return;
    }

    //...Loop over all states...
    vector<TRGState*> outputStates;
    vector<int> newStates;
    int lastClock = states[0] - 1;
    TRGState* r = new TRGState(registerBitSize);
    for (unsigned i = 0; i < nStates; i++) {

      if (TRGDebug::level())
        cout << TRGDebug::tab() << "Last clock=" << lastClock << endl;

      //...Check clock position...
      if (states[i] <= lastClock)
        continue;

      if (TRGDebug::level()) {
        cout << TRGDebug::tab() << "state=" << i << endl;
        r->dump("", TRGDebug::tab() + "reg=");
      }

      bool active = true;
      int nLoops = 0;
      // cppcheck-suppress knownConditionTrueFalse
      while (active) {
        int clk = states[i] + nLoops;
        lastClock = clk;

        TRGState s = input.state(clk);
        outputStates.push_back(new TRGState((* packer)(s, * r, active)));
        newStates.push_back(clk);
        ++nLoops;

        if (TRGDebug::level()) {
          cout << TRGDebug::tab() << "state=" << i << ",clock=" << clk <<
               ",LogicActive=" << active << endl;
          r->dump("", TRGDebug::tab() + "reg=");
        }
      }
    }
    delete r;

    //...Size of output...
    // unsigned outputSize = 0;
    // if (outputStates.size())
    //     outputSize = outputStates.back()->size();

    //...Creat a SignalVector...
    TRGSignalVector* sb = new TRGSignalVector(_name, c, outputBitSize);

    //...Make a SignalVector...
    // const TRGState &os0 = * outputStates[0];
    //       sb->set(os0, 0); delete &os0;
    // const unsigned n = outputStates.size();   // same as nStates

    const unsigned n = outputStates.size();
    for (unsigned i = 0; i < n; i++) {
      const TRGState* s = outputStates[i];
      sb->set((* s), newStates[i]);
      delete s;
    }

    push_back(sb);

    TRGDebug::leaveStage(sn);
  }

  TRGSignalBundle::~TRGSignalBundle()
  {
  }

  void
  TRGSignalBundle::dump(const string& msg,
                        const string& pre) const
  {
    cout << pre << _name << ":" << size() << " signal vector(s)" << endl;
    for (unsigned i = 0; i < size(); i++)
      (* this)[i]->dump(msg, "    " + pre);
  }

  bool
  TRGSignalBundle::active(void) const
  {
    for (unsigned i = 0; i < size(); i++)
      if ((* this)[i]->active())
        return true;
    return false;
  }

  std::vector<int>
  TRGSignalBundle::stateChanges(void) const
  {

    //...List for clock positions...
    std::vector<int> list;

    //...Pick up all clock points with state changes...
    const unsigned n = size();
    for (unsigned i = 0; i < n; i++) {
      vector<int> a = (* this)[i]->stateChanges();
      for (unsigned j = 0; j < a.size(); j++)
        list.push_back(a[j]);
    }

    //...Append the first clock point if there are state changes...
    if (list.size())
      list.push_back(_clock->min());

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
  TRGSignalBundle::state(int clockPosition) const
  {
    TRGState s;
    const unsigned n = size();
    for (unsigned i = 0; i < n; i++) {
      TRGState t = (* this)[i]->state(clockPosition);
      s += t;
    }
    return s;
  }

  const TRGClock&
  TRGSignalBundle::clock(const TRGClock& c)
  {
    _clock = & c;

    for (unsigned i = 0; i < size(); i++) {
      TRGSignalVector& t = * (* this)[i];
      t.clock(c);
    }

    return * _clock;
  }

  TRGSignal
  TRGSignalBundle::ored(void) const
  {

    //...Get state information...
    const vector<int> states = stateChanges();
    const unsigned nStates = states.size();

    //...Output...
    TRGSignal ored;

    //...Loop over all states...
    // vector<TRGState *> outputStates;
    for (unsigned i = 0; i < nStates; i++) {
      // if (TRGDebug::level())
      //     cout << TRGDebug::tab() << "Clock=" << states[i] << endl;

      // if (active(states[i])) {
      //     TRGSignal p(* _clock,

      // }


      cout << "TRGSginalBundle::ored !!! not completed yet" << endl;
    }

    return TRGSignal();
  }

  void
  TRGSignalBundle::dumpCOE(const string& fnIn, int start , int stop) const
  {

    string fn = fnIn;

    if (fnIn.size() == 0)
      fn = name();

    //...Open a file to output...
    ofstream file(fn + ".coe", ios::out);
    if (! file.is_open()) {
      cout << "!!! " << name() << " can not open file : " << fn << endl;
      return;
    }

    // //...Determine start clock to output...
    // int clk0 = _clock->max();
    // vector<int> sc = stateChanges();
    // for (unsigned i = 0; i < sc.size(); i++) {
    //     if (state(sc[i]).active()) {
    //         clk0 = sc[i];
    //         break;
    //     }
    // }

    // //...Determine end clock to output...
    // int clk1 = _clock->min();
    // for (unsigned i = 0; i < sc.size(); i++) {
    //     if (sc[i] < clk0)
    //         continue;

    //     if (state(sc[i]).active()) {
    //         clk1 = sc[i];
    //     }
    // }

    // //...No active case...
    // if (clk0 > clk1) {
    //     clk0 = 0;
    //     clk1 = 1;
    // }

    // if (TRGDebug::level()) {
    //     cout << TRGDebug::tab() << "SignalBundle::dumpCOE : " << name()
    //          << endl;
    //     cout << TRGDebug::tab() << "    clock region to output" << endl;
    //     cout << TRGDebug::tab() << "    start=" << clk0 << endl;
    //     cout << TRGDebug::tab() << "    end  =" << clk1 << endl;
    // }

    // //...32 clocks as default : 2016/04...
    // clk0 = 0;
    // clk1 = 31;

    int clk0 = start;
    int clk1 = stop;

    TRGState tmp = state(0);
    const TRGSignalVector& cc = _clock->clockCounter();

    file << "; " << name() << endl;
    file << "; generated at " << TRGUtil::dateString() << endl;
    file << "; start clock = " << clk0 << endl;
    file << "; end clock = " << clk1 << endl;
    file << "; bit size = " << tmp.size() << " + clock counter(" << cc.size()
         << ")" << endl;
    file << ";" << endl;
    file << "memory_initialization_radix=2;" << endl;
    file << "memory_initialization_vector=" << endl;

    for (int i = clk0; i <= clk1; i++) {
      TRGState sd = state(i);
      TRGState sc = cc.state(i);
      file << sc;
      file << sd;
      if (i == clk1)
        file << ";" << endl;
      else
        file << "," << endl;
    }

    file.close();

  }

} // namespace Belle2
