//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : GDLSignal.cc
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a digitized signal.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <algorithm>
#include <functional>
#include <iostream>
#include "trigger/gdl/GDLClock.h"
#include "trigger/gdl/GDLSignal.h"

using namespace std;

namespace Belle2 {

GDLSignal::GDLSignal() :
    _history(),
    _name("no signal") {
}

GDLSignal::GDLSignal(const GDLSignal & t) :
    _history(t._history),
    _name(t._name) {
}

GDLSignal::GDLSignal(const GDLTime & t) :
    _name(t.name()) {

    //...Check edge...
    if (t.edge()) {

	//...Store itself...
	_history.push_back(t);

	//...Set falling edge...
	_history.push_back(t.clock().maxGDLTime(false));
    }
    else {

	//...Set rising edge...
	_history.push_back(t.clock().minGDLTime(true));

	//...Store itself...
	_history.push_back(t);
    }
}

GDLSignal::~GDLSignal() {
}

void
GDLSignal::dump(const std::string & msg,
		const std::string & pre) const {
    std::cout << pre << _name << ":";

    //...Clock check...
    bool singleClock = true;
    const GDLClock * clk0 = 0;
    if (_history.size()) {
	clk0 = & _history[0].clock();
	for (unsigned i = 1; i < _history.size(); i++) {
	    const GDLClock * clk = & _history[i].clock();
	    if (clk != clk0)
		singleClock = false;
	}
    }
    if (msg.find("clock") != string::npos ||
	msg.find("detail") != string::npos)
	if (singleClock && clk0)
	    cout << "clock=" << clk0->name();
    if (! singleClock)
	cout << "there are multiple clock source";
    cout << endl;

    if (_history.size()) {
	for (unsigned i = 0; i < _history.size(); i++)
	    _history[i].dump(msg, pre + "    ");
    }
}

GDLSignal
GDLSignal::operator&(const GDLSignal & left) const {
    GDLSignal t(* this);
    t._history.insert(t._history.end(),
		      left._history.begin(),
		      left._history.end());
    t._name = "(" + t._name + ")&(" + left._name + ")";
    std::sort(t._history.begin(), t._history.end(), GDLTime::sortByTime);

    //...And operation...
    t._history = andOperation(t._history);

    return t;
}

GDLSignal &
GDLSignal::operator&=(const GDLSignal & left) {
    _history.insert(_history.end(),
		    left._history.begin(),
		    left._history.end());
    this->_name = "(" + this->_name + ")&(" + left._name + ")";
    sort(_history.begin(), _history.end(), GDLTime::sortByTime);

    //...And operation...
    _history = andOperation(_history);

    return * this;
}

std::vector<GDLTime>
GDLSignal::andOperation(const std::vector<GDLTime> & history) {

    //...And operation...
    const unsigned n = history.size();
    unsigned riseC = 0;
    bool signal = false;
    vector<GDLTime> tmp;
    for (unsigned i = 0; i < n; i++) {
	const bool edge = history[i].edge();

	if (edge)
	    ++riseC;
	else
	    --riseC;

// 	cout << "riseC,i,e,t=" << riseC << "," << i << "," << edge << "," << t._history[i].time() << endl;

	if (riseC == 2) {
	    tmp.push_back(history[i]);
	    signal = true;
	}
	else if (signal && (riseC == 1)) {
	    tmp.push_back(history[i]);
	    signal = false;
	}
    }
    return tmp;
}

GDLSignal
GDLSignal::operator|(const GDLSignal & left) const {
    GDLSignal t(* this);
    t._history.insert(t._history.end(),
		      left._history.begin(),
		      left._history.end());
    t._name = "(" + t._name + ")|(" + left._name + ")";
    std::sort(t._history.begin(), t._history.end(), GDLTime::sortByTime);

    //...And operation...
    t._history = orOperation(t._history);

    return t;
}

GDLSignal &
GDLSignal::operator|=(const GDLSignal & left) {
    _history.insert(_history.end(),
		    left._history.begin(),
		    left._history.end());
    this->_name = "(" + this->_name + ")&(" + left._name + ")";
    sort(_history.begin(), _history.end(), GDLTime::sortByTime);

    //...And operation...
    _history = orOperation(_history);

    return * this;
}

std::vector<GDLTime>
GDLSignal::orOperation(const std::vector<GDLTime> & history) {

    //...And operation...
    const unsigned n = history.size();
    vector<GDLTime> tmp;
    unsigned signal = 0;
    for (unsigned i = 0; i < n; i++) {
	const bool edge = history[i].edge();

	if (edge) {
	    if (signal == 0)
		tmp.push_back(history[i]);
	    ++signal;
	}
	else {
	    if (signal == 1)
		tmp.push_back(history[i]);
	    --signal;
	}

// 	cout << "i,time,edge,signal=" << i << "," << history[i].time()
// 	     << "," << edge << "," << signal << endl;

    }
    return tmp;
}

GDLSignal &
GDLSignal::widen(unsigned width) {

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

    return * this;
}

} // namespace Belle2
