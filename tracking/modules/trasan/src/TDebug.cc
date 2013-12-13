//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TDebug.cc
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : Debug utility functions
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define HEP_SHORT_NAMES

#include <sys/time.h>
#include <sys/resource.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <map>
#include "tracking/modules/trasan/TDebug.h"
#include "tracking/modules/trasan/AList.h"

using namespace std;

namespace Belle {

vector<string> TDebug::_stages;
int TDebug::_level = 0;

void
TDebug::enterStage(const string & name) {
    if (_level)
        cout << tab() << "--> ";
    _stages.push_back(name);
    if (_level)
        cout << name << endl;
}

void
TDebug::leaveStage(const string & name) {
    if (name != _stages.back()) {
        cout << "TDebug !!! given stage name(" << name << ") doesn't match"
             << " to expected stage name(" << _stages.back() << endl;
        return;
    }
    _stages.pop_back();
    if (_level)
        cout << tab() << "<-- " << name << endl;
}

string
TDebug::tab(void) {
    string t;
    const unsigned n = _stages.size();
    for (unsigned i = 0; i < n; i++)
        t += "    ";
    return t;
}

string
TDebug::tab(int extra) {
    string t = tab();
    if (extra > 0)
        for (unsigned i = 0; i < unsigned(extra); i++)
            t += " ";
    return t;
}

int
TDebug::level(void) {
    return _level;
}

int
TDebug::level(int a, bool b) {
    if (! b)
	return _level = a;
    else
	return _level = a;
}

//...Trasan stage strings...
AList<string> Stages;

//...Counter for each stage...
map<string, stageInfo> Counters;



string
Stage(void) {
    string a;
    for (unsigned i = 0; i < (unsigned) Stages.length(); i++) {
        if (i)
            a += "/";
        a += * Stages[i];
    }
    return a;
}

string
Tab(int shift) {
    string a;
    int totalShift = Stages.length() + shift;
    if (totalShift < 0) totalShift = 0;
    if (totalShift)
        for (unsigned i = 0; i < (unsigned) totalShift; i++)
            a += "    ";
    return a;
}

unsigned
EnterStage(const string& a) {
    string* const now = new string(a);
    Stages.append(now);
    map<string, stageInfo>::iterator counter
        = Counters.find(Stage());
    struct rusage ru;
    getrusage(RUSAGE_SELF, & ru);
    if (counter == Counters.end()) {
        struct stageInfo c;
        c.counter = 1;
        c.usedUTime = 0;
        c.lastUTime = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec * 1e-6;
        c.usedSTime = 0;
        c.lastSTime = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec * 1e-6;
        counter = Counters.insert(Counters.end(),
                                  pair<string, stageInfo>
                                  (Stage(), c));
    } else {
        ++counter->second.counter;
        counter->second.lastUTime =
            ru.ru_utime.tv_sec + ru.ru_utime.tv_usec * 1e-6;
        counter->second.lastSTime =
            ru.ru_stime.tv_sec + ru.ru_stime.tv_usec * 1e-6;
    }

    cout << Tab(-1) << "--> " << Stage() << "(" << NestLevel() << ")"
         << ",nCalls=" << counter->second.counter << endl;

    return (unsigned) Stages.length();
}

unsigned
LeaveStage(const string& a) {
    cout << Tab(-1) << "<-- " << Stage() << "(" << NestLevel() << ")"
         << endl;

    const string& now = * Stages.last();
    if (now == a) {
        map<string, stageInfo>::iterator counter
            = Counters.find(Stage());
        if (counter != Counters.end()) {
            struct rusage ru;
            getrusage(RUSAGE_SELF, & ru);
            counter->second.usedUTime += ru.ru_utime.tv_sec
                + ru.ru_utime.tv_usec * 1e-6 - counter->second.lastUTime;
            counter->second.usedSTime += ru.ru_stime.tv_sec
                + ru.ru_stime.tv_usec * 1e-6 - counter->second.lastSTime;
        } else {
            cout << "!!! Stages are something wrong : counter not found"
                 << Stage() << endl;
        }
        delete Stages.last();
        Stages.remove(Stages.last());
    } else {
        cout << "!!! Stages are something wrong" << endl
             << "    Present stage=" << Stage() << endl
             << "    Leaving stage=" << a << endl;
    }
    return (unsigned) Stages.length();
}

unsigned
NestLevel(void) {
    return (unsigned) Stages.length();
}

void
DumpStageInformation(void) {
    cout << "Trasan Stage Information" << endl;
    cout << "    #stages=" << Counters.size() << endl;
// cout << "    #calls :      utime :      stime : stage" << endl;
    cout << "    #calls :   utime(s) : stage" << endl;
    map<string, stageInfo>::iterator p;
    for (p = Counters.begin(); p != Counters.end(); p++) {
        cout.width(10);
        cout << p->second.counter;
        cout << " : ";
        cout.setf(ios::fixed, ios::floatfield);
        cout.precision(3);
        cout.width(10);
        cout << (p->second.usedUTime + p->second.usedSTime);
//  cout << " : ";
//  cout.width(10);
//  cout << p->second.usedSTime;
        cout << " : " << p->first << endl;
    }
    cout.setf(ios::showpoint, ios::floatfield);
// cout << "    " << p->first << " : " << p->second << endl;
}

string
TDebugUtilities::dateString(void) {
    time_t t;
    time(& t);
    struct tm * tl;
    tl = localtime(& t);
    char ts1[80];
    strftime(ts1, sizeof(ts1), "%Y/%m/%d %H:%M %Z", tl);
    return (ts1);
}

string
TDebugUtilities::dateStringF(void) {
    time_t t;
    time(& t);
    struct tm * tl;
    tl = localtime(& t);
    char ts0[80];
    strftime(ts0, sizeof(ts0), "%Y%m%d_%H%M", tl);
    return string(ts0);
}

string
TDebugUtilities::itostring(int i) {
    ostringstream s;
    s << i;
    return s.str();
}

string
TDebugUtilities::dtostring(double d, unsigned int precision) {
    ostringstream s;
    s << setprecision(precision) << d;
    return s.str();
}

string
TDebugUtilities::carstring(const string &s) {
    string ret;
//    const char * p = str;
//    while ( *p && isspace(*p) ) p++;
//    while ( *p && !isspace(*p) ) ret += *(p++);
    int i;
    int len = s.length();
    for (i = 0; i < len; i++) {
        if ( !isspace(s[i]) ) break;
    }
    for (; i < len; i++) {
        if ( !isspace(s[i]) ) {
            ret += s[i];
        } else break;
    }
    return ret;
}

string
TDebugUtilities::cdrstring(const string &s) {
//    const char * p = str;
//    while ( *p && isspace(*p) ) p++;
//    while ( *p && !isspace(*p) ) p++;
//    while ( *p && isspace(*p) ) p++;
    int i;
    int len = s.length();
    for (i = 0; i < len; i++) {
        if ( !isspace(s[i]) ) break;
    }
    for (; i < len; i++) {
        if ( isspace(s[i]) ) break;
    }
    for (; i < len; i++) {
        if ( !isspace(s[i]) ) break;
    }
    return s.substr(i);
}

void
TDebugUtilities::bitDisplay(unsigned val) {
    bitDisplay(val, 31, 0);
}

void
TDebugUtilities::bitDisplay(unsigned val, unsigned f, unsigned l) {
    unsigned i;
    for (i = 0; i < f - l; i++) {
        if ((i % 8) == 0) cout << " ";
        cout << (val >> (f - i)) % 2;
    }
}

string
TDebugUtilities::streamDisplay(unsigned val, unsigned f, unsigned l) {
    string s;
    for (unsigned i = f; i < l + 1; i++) {
        if ((i % 8) == 0) s += " ";
        s += itostring((val >> i) % 2);
    }
    return s;
}

string
TDebugUtilities::streamDisplay(unsigned val) {
    return streamDisplay(val, 0, 63);
}


} // namespace Belle2
