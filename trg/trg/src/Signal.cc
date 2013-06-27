//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGSignal.cc
// Section  : TRG
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
#include "trg/trg/Clock.h"
#include "trg/trg/Signal.h"

using namespace std;

namespace Belle2 {

TRGSignal::TRGSignal(const TRGClock & c) :
    _name("no signal"),
    _clock(& c),
    _history() {
}

TRGSignal::TRGSignal(const string & name, const TRGClock & c) :
    _name(name),
    _clock(& c),
    _history() {
}

TRGSignal::TRGSignal(const TRGSignal & t) :
//  _history(t._history),
    _name(t._name),
    _clock(t._clock) {
    const unsigned n = t._history.size();
    for (unsigned i = 0; i < n; i++) {
	_history.push_back(t._history[i]);
    }
}

TRGSignal::TRGSignal(const TRGTime & t) :
    _name(t.name()),
    _clock(& t.clock()) {

    //...Check edge...
    if (t.edge()) {

        //...Store itself...
        _history.push_back(t);

        //...Set falling edge...
        _history.push_back(t.clock().maxTRGTime(false));
    }
    else {

        //...Set rising edge...
        _history.push_back(t.clock().minTRGTime(true));

        //...Store itself...
        _history.push_back(t);
    }
}

TRGSignal::~TRGSignal() {
}

void
TRGSignal::dump(const string & msg,
                const string & pre) const {

    bool first = true;

    if (msg.find("detail") != string::npos) {
        first = false;
        cout << pre << _name;
    }

    cout << ":#signal=" << _history.size();

    if (msg.find("clock") != string::npos ||
        msg.find("detail") != string::npos) {
	cout << ":clock=" << _clock->name();
        cout << endl;
    }

    if (_history.size()) {
        string tab = pre;
        if (! first)
            tab += "    ";
        for (unsigned i = 0; i < _history.size(); i++)
            _history[i].dump(msg, tab);
    }
}

TRGSignal
TRGSignal::operator&(const TRGSignal & left) const {
    TRGSignal t(* this);
    t._history.insert(t._history.end(),
                      left._history.begin(),
                      left._history.end());
    t._name = "(" + t._name + ")&(" + left._name + ")";
    sort(t._history.begin(), t._history.end(), TRGTime::sortByTime);

    //...And operation...
    t._history = andOperation(t._history);

    return t;
}

TRGSignal &
TRGSignal::operator&=(const TRGSignal & left) {
    _history.insert(_history.end(),
                    left._history.begin(),
                    left._history.end());
    this->_name = "(" + this->_name + ")&(" + left._name + ")";
    sort(_history.begin(), _history.end(), TRGTime::sortByTime);

    //...And operation...
    _history = andOperation(_history);

    return * this;
}

vector<TRGTime>
TRGSignal::andOperation(const vector<TRGTime> & history) {

    //...And operation...
    const unsigned n = history.size();
    unsigned riseC = 0;
    bool signal = false;
    vector<TRGTime> tmp;
    for (unsigned i = 0; i < n; i++) {
        const bool edge = history[i].edge();

        if (edge)
            ++riseC;
        else
            --riseC;

//         cout << "riseC,i,e,t=" << riseC << "," << i << "," << edge << "," << t._history[i].time() << endl;

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

TRGSignal
TRGSignal::operator|(const TRGSignal & left) const {
    TRGSignal t(* this);
    t._history.insert(t._history.end(),
                      left._history.begin(),
                      left._history.end());
    t._name = "(" + t._name + ")|(" + left._name + ")";
    sort(t._history.begin(), t._history.end(), TRGTime::sortByTime);

    //...And operation...
    t._history = orOperation(t._history);

    return t;
}

TRGSignal &
TRGSignal::operator|=(const TRGSignal & left) {
    _history.insert(_history.end(),
                    left._history.begin(),
                    left._history.end());
    this->_name = "(" + this->_name + ")&(" + left._name + ")";
    sort(_history.begin(), _history.end(), TRGTime::sortByTime);

    //...And operation...
    _history = orOperation(_history);

    return * this;
}

vector<TRGTime>
TRGSignal::orOperation(const vector<TRGTime> & history) {

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
        }
        else {
            if (signal == 1)
                tmp.push_back(history[i]);
            --signal;
        }

//         cout << "i,time,edge,signal=" << i << "," << history[i].time()
//              << "," << edge << "," << signal << endl;

    }
    return tmp;
}

TRGSignal &
TRGSignal::widen(unsigned width) {

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

std::vector<int>
TRGSignal::stateChanges(void) const {
    std::vector<int> list;
    const unsigned n = _history.size();
    for (unsigned i = 0; i < n; i++)
	list.push_back(_history[i].time());
    return list;
}

} // namespace Belle2
