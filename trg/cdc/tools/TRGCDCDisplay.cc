//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGCDCDisplay.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A program to display TRGCDC components
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/TrackSegment.h"
#ifdef TRGCDC_DISPLAY
#include "framework/gearbox/Gearbox.h"
#include "trg/cdc/DisplayRphi.h"
#include "trg/cdc/DisplayHough.h"
namespace Belle2_TRGCDC {
    Belle2::TRGCDCDisplayRphi * D = 0;
}
#endif

using namespace std;
using namespace Belle2;
#ifdef TRGCDC_DISPLAY
using namespace Belle2_TRGCDC;
#endif

#define DEBUG_LEVEL     1
#define PROGRAM_NAME    "TRGCDCDisplay"
#define PROGRAM_VERSION "version 0.00"
#define ENV_PATH        "BELLE2_LOCAL_DIR"
#define CONFIG          "TRGCDCWireConfig_0_20101110_0836.dat"

int
main(int , char ** ) {

    cout << PROGRAM_NAME << " ... " << PROGRAM_VERSION << endl;
    const string tab = "    ";

#ifdef TRGCDC_DISPLAY
    //...Gearbox...
    const string path = getenv(ENV_PATH);
    vector<std::string> m_backends;
    string m_filename = path + "/data/geometry/Belle2.xml";
    m_backends.push_back("file:");
    Gearbox &gearbox = Gearbox::getInstance();
    gearbox.setBackends(m_backends);
    gearbox.open(m_filename);

    //...TRGCDC...
    const string cname = path + "/data/trg/" + CONFIG;
//  TRGCDC * cdc = TRGCDC::getTRGCDC(cname, false, 100, 100);
    TRGCDC * cdc = TRGCDC::getTRGCDC(cname);

    //...Display...
    D->clear();
    D->show();

    //...For Iwasaki...
    string inf = "1/8 of TS";
    string stg = "";
    string target = "none";
    for (unsigned i = 0; i < cdc->nTrackSegments(); i++) {
	if (i % 8) continue;
	D->area().append(cdc->trackSegment(i));
    }
    for (unsigned i = 0; i < cdc->nWires(); i++) {
	const TCWire & w = * cdc->wire(i);
	if ((w.layerId() != 2) &&
	    (w.layerId() != 10) &&
	    (w.layerId() != 16) &&
	    (w.layerId() != 22) &&
	    (w.layerId() != 28) &&
	    (w.layerId() != 34) &&
	    (w.layerId() != 40) &&
	    (w.layerId() != 46) &&
	    (w.layerId() != 52)) continue;
	if (i % 8) continue;
	vector<const TCWire *> v;
	v.push_back(cdc->wire(i));
	D->area().append(v, Gdk::Color("red"));
    }
    D->stage(stg);
    D->information(inf);
    D->show();
    D->run();

    //...Draw objects...
    while (1) {

	cout << "Enter a target to display" << endl;
	getline(cin, target);
	if (target == "clear") {
	    D->clear();
	    inf = "";
	    cout << "targets cleared" << endl;
	}
	else if (target.find("TS") != string::npos) {
	    bool found = false;
	    for (unsigned i = 0; i < cdc->nTrackSegments(); i++) {
		if (target == cdc->trackSegment(i).name()) {
		    D->area().append(cdc->trackSegment(i));
		    if (inf.size()) inf += ",";
		    inf += target;
		    D->stage(stg);
		    D->information(inf);
		    D->show();
//		    D->run();
		    found = true;
		    break;
		}		    
	    }
	    if (! found)
		cout << "!!! " << target << " not found" << endl;
	}
	else if (target == "draw") {
	    D->run();
	}
	else if (target == "quit") {
	    break;
	}
	else {
	    cout << "!!! unknown target" << endl;
	}
    }
#endif

    //...Termination...
    cout << PROGRAM_NAME << " ... terminated" << endl;
}
