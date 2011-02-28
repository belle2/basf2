//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Debug.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : Debug utility functions
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRG_DEBUG

#include <iostream>
#include "trg/trg/Debug.h"

using namespace std;

namespace Belle2 {

vector<string> TRGDebug::_stages;

void
TRGDebug::enterStage(const string & name) {
    cout << tab() << "--> ";
    _stages.push_back(name);
    cout << name << endl;
}

void
TRGDebug::leaveStage(const string & name) {
    if (name != _stages.back()) {
        cout << "TRGDebug !!! given stage name(" << name << ") doesn't match"
             << " to expected stage name(" << _stages.back() << endl;
        return;
    }
    _stages.pop_back();
    cout << tab() << "<-- ";
    cout << name << endl;
}

string
TRGDebug::tab(void) {
    string t;
    const unsigned n = _stages.size();
    if (n)
        for (unsigned i = 0; i < n; i++)
            t += "    ";
    return t;
}


} // namespace Belle2

#endif
