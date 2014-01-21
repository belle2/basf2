//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGDL.cc
// Section  : TRG GDL
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent GDL.
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGGDL_SHORT_NAMES

#include <fstream>
#include "trg/trg/Debug.h"
#include "trg/trg/Time.h"
#include "trg/trg/State.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/gdl/TRGGDL.h"

using namespace std;

namespace Belle2 {

string
TRGGDL::name(void) const {
    return "TRGGDL";
}

string
TRGGDL::version(void) const {
    return string("TRGGDL 0.00");
}

TRGGDL *
TRGGDL::_gdl = 0;

TRGGDL*
TRGGDL::getTRGGDL(const string & configFile,
                  unsigned simulationMode,
                  unsigned fastSimulationMode,
                  unsigned firmwareSimulationMode) {
    if (_gdl) {
        //delete _gdl;
        _gdl = 0;
    }

    if (configFile != "good-bye") {
        _gdl = new TRGGDL(configFile,
                          simulationMode,
                          fastSimulationMode,
                          firmwareSimulationMode);
    }
    else {
        cout << "TRGGDL::getTRGGDL ... good-bye" << endl;
        //        delete _gdl;
        _gdl = 0;
    }

    return _gdl;
}

TRGGDL*
TRGGDL::getTRGGDL(void) {
    if (! _gdl)
        cout << "TRGGDL::getTRGGDL !!! TRGGDL is not created yet" << endl;
    return _gdl;
}

TRGGDL::TRGGDL(const string & configFile,
               unsigned simulationMode,
               unsigned fastSimulationMode,
               unsigned firmwareSimulationMode)
    : _debugLevel(0),
      _configFilename(configFile),
      _simulationMode(simulationMode),
      _fastSimulationMode(fastSimulationMode),
      _firmwareSimulationMode(firmwareSimulationMode),
      _clock(Belle2_GDL::GDLSystemClock),
      _offset(15.3) {

    if (TRGDebug::level()) {
        cout << "TRGGDL ... TRGGDL initializing with " << _configFilename
             << endl
             << "           mode=0x" << hex << _simulationMode << dec << endl;
    }

    initialize();

    if (TRGDebug::level()) {
        cout << "TRGGDL ... TRGGDL created with " << _configFilename << endl;
        _clock.dump();
    }
}

void
TRGGDL::initialize(void) {
}

void
TRGGDL::terminate(void) {
}

void
TRGGDL::dump(const string & msg) const {
    if (msg.find("name")    != string::npos ||
        msg.find("version") != string::npos ||
        msg.find("detail")  != string::npos ||
        msg == "") {
    }
    if (msg.find("detail") != string::npos ||
        msg.find("state") != string::npos) {
        cout << "Debug Level=" << _debugLevel;
        cout << endl;
    }
}

void
TRGGDL::clear(void) {
}

void
TRGGDL::fastClear(void) {
}

void
TRGGDL::update(bool) {
}

TRGGDL::~TRGGDL() {
    clear();
}

void
TRGGDL::simulate(void) {
    const bool fast = (_simulationMode & 1);
    const bool firm = (_simulationMode & 2);
    if (fast)
        fastSimulation();
    if (firm)
        firmwareSimulation();
}

void
TRGGDL::fastSimulation(void) {
}

void
TRGGDL::firmwareSimulation(void) {
}

void
TRGGDL::configure(void) {

    //...Open configuration file...
    ifstream infile(_configFilename.c_str(), ios::in);
    if (infile.fail()) {
        cout << "TRGGDL !!! can not open file" << endl
             << "    " << _configFilename << endl;
        return;
    }

    //...Read configuration data...
    char b[800];
    unsigned lines = 0;
    string cdcVersion = "";
    string configVersion = "";
    while (! infile.eof()) {
        infile.getline(b, 800);
        const string l(b);
        string cdr = l;
        ++lines;
    }
    infile.close();
}

} // namespace Belle2
