//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGSignalBundle.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a bundle of SignalVectors.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES

#include <string>
#include <cstring>
#include <algorithm>
#include <limits>
#include <iostream>
#include "trg/trg/Constants.h"
#include "trg/trg/Utilities.h"
#include "trg/trg/Debug.h"
#include "trg/trg/Clock.h"
#include "trg/trg/Signal.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/SignalBundle.h"
#include "trg/trg/State.h"

using namespace std;

namespace Belle2 {

TRGSignalBundle::TRGSignalBundle(const TRGClock & c)
    : _name("unknown"),
      _clock(& c) {
}

TRGSignalBundle::TRGSignalBundle(const string & name, const TRGClock & c)
    : _name(name),
      _clock(& c) {
}

TRGSignalBundle::TRGSignalBundle(const string & name,
				 const TRGClock & c,
				 const TRGSignalBundle & input,
                                 const unsigned outputBitSize,
				 TRGState (* packer)(const TRGState &))
    : _name(name),
      _clock(& c) {
 
    //...Get state information...
    const vector<int> states = input.stateChanges();
    const unsigned nStates = states.size();

    //...Loop over all states...
    vector<TRGState *> outputStates;
    for (unsigned i = 0; i < nStates; i++) {
	TRGState s = input.state(states[i]);

        if (TRGDebug::level()) {
            cout << TRGDebug::tab() << "Clock=" << states[i] << endl;
        }

	outputStates.push_back(new TRGState((* packer)(s)));
    }

    //...Bit size of output...
    // unsigned outputSize = 0;
    // if (outputStates.size())
    //     outputSize = outputStates.back()->size();
   
    //...Creat a SignalVector...
    TRGSignalVector * sb = new TRGSignalVector(_name, c, outputBitSize);
 
    //...Make a SignalVector...
    // const TRGState &os0 = * outputStates[0];
    //       sb->set(os0, 0); delete &os0;
    // const unsigned n = outputStates.size();   // same as nStates
    
    for (unsigned i = 0; i < nStates; i++) {
      	const TRGState * s = outputStates[i];
	sb->set((*s), states[i]);
	delete s;
    }

    push_back(sb);
}

TRGSignalBundle::TRGSignalBundle(const string & name,
				 const TRGClock & c,
				 const TRGSignalBundle & input,
                                 const unsigned outputBitSize,
				 TRGState (* packer)(const TRGState &,
                                                     TRGState &,
                                                     bool &))
    : _name(name),
      _clock(& c) {
 
    //...Get state information...
    const vector<int> states = input.stateChanges();
    const unsigned nStates = states.size();

    //...Loop over all states...
    vector<TRGState *> outputStates;
    vector<int> newStates;
    int lastClock = states[0] - 1;
    for (unsigned i = 0; i < nStates; i++) {

        //...Check clock position...
        if (states[i] <= lastClock)
            continue;

        TRGState r(7);
        bool active = true;
        int nLoops = 0;
        while (active) {
            int clk = states[i] + nLoops;
            lastClock = clk;

            if (TRGDebug::level()) {
                cout << TRGDebug::tab() << "0:Clock=" << clk << ",active="
                     << active << endl;
                r.dump("", TRGDebug::tab() + "reg=");
            }

            TRGState s = input.state(clk);
            outputStates.push_back(new TRGState((* packer)(s, r, active)));
            newStates.push_back(clk);
            ++nLoops;

            if (TRGDebug::level()) {
                cout << TRGDebug::tab() << "1:Clock=" << clk << ",active="
                     << active << endl;
                r.dump("", TRGDebug::tab() + "reg=");
            }
        }
    }

    //...Size of output...
    // unsigned outputSize = 0;
    // if (outputStates.size())
    //     outputSize = outputStates.back()->size();
   
    //...Creat a SignalVector...
    TRGSignalVector * sb = new TRGSignalVector(_name, c, outputBitSize);
 
    //...Make a SignalVector...
    // const TRGState &os0 = * outputStates[0];
    //       sb->set(os0, 0); delete &os0;
    // const unsigned n = outputStates.size();   // same as nStates
    
    const unsigned n = outputStates.size();
    for (unsigned i = 0; i < n; i++) {
      	const TRGState * s = outputStates[i];
	sb->set((*s), newStates[i]);
	delete s;
    }

    push_back(sb);
}

TRGSignalBundle::~TRGSignalBundle() {
}

void
TRGSignalBundle::dump(const string & msg,
		      const string & pre) const {
    cout << pre << _name << ":" << size() << " signal vector(s)" << endl;
    for (unsigned i = 0; i < size(); i++)
	(* this)[i]->dump(msg, "    " + pre);
}

bool
TRGSignalBundle::active(void) const {
    for (unsigned i = 0; i < size(); i++)
	if ((* this)[i]->active())
	    return true;
    return false;
}

std::vector<int>
TRGSignalBundle::stateChanges(void) const {

    //...List for clock positions...
    std::vector<int> list;

    //...Append the first clock point...
    list.push_back(_clock->min());

    //...Pick up all clock points with state changes...
    const unsigned n = size();
    for (unsigned i = 0; i < n; i++) {
	vector<int> a =(* this)[i]->stateChanges();
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
TRGSignalBundle::state(int clockPosition) const {
    TRGState s;
    const unsigned n = size();
    for (unsigned i = 0; i < n; i++) {
	TRGState t = (* this)[i]->state(clockPosition);
	s += t;
    }
    return s;
}

const TRGClock &
TRGSignalBundle::clock(const TRGClock & c) {
    _clock = & c;

    for (unsigned i = 0; i < size(); i++) {
	TRGSignalVector & t = * (* this)[i];
	t.clock(c);
    }

    return * _clock;
}

TRGSignal
TRGSignalBundle::ored(void) const {

    //...Get state information...
    const vector<int> states = stateChanges();
    const unsigned nStates = states.size();

    //...Output...
    TRGSignal ored;

    //...Loop over all states...
    vector<TRGState *> outputStates;
    for (unsigned i = 0; i < nStates; i++) {
        if (TRGDebug::level())
            cout << TRGDebug::tab() << "Clock=" << states[i] << endl;

        // if (active(states[i])) {
        //     TRGSignal p(* _clock, 

        // }


        cout << "TRGSginalBundle::ored !!! not completed yet" << endl;
    }


    

    return TRGSignal();
}

} // namespace Belle2
