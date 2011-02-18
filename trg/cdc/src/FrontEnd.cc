//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : FrontEnd.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC front-end board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Wire.h"

using namespace std;

namespace Belle2 {

TRGCDCFrontEnd::TRGCDCFrontEnd(const std::string & name,
                               const TRGClock & clock)
    : TRGBoard(name, clock) {
}

TRGCDCFrontEnd::~TRGCDCFrontEnd() {
}

string
TRGCDCFrontEnd::version(void) {
    return ("TRGCDCFrontEnd version 0.01");
}

TRGCDCFrontEnd::boardType
TRGCDCFrontEnd::type(void) const {
    if ((* this)[0]->localLayerId() < 3)
        return inner;
    return outer;
}

int
TRGCDCFrontEnd::implementation(const TRGCDCFrontEnd::boardType & type,
                               ofstream & outfile) {

    string cname = "UNKNOWN";
    if (type == TRGCDCFrontEnd::inner) {
        outfile << "-- inner type" << endl;
        cname = "TRGCDCFrontEndInner";
    }
    else {
        outfile << "-- outer type" << endl;
        cname = "TRGCDCFrontEndOuter";
    }

    outfile << "--" << endl;
    outfile << endl;
    outfile << "    component " << cname << endl;
    int err = implementationPort(type, outfile);
    outfile << "    end component;" << endl;

    outfile << "--------------------------------------------------------------"
            << endl;

    outfile << "entity " << cname << "is" << endl;
    err = implementationPort(type, outfile);
    outfile << "end " << cname << ";" << endl;
    outfile << endl;
    outfile << "architecture Behavioral of " << cname << " is" << endl;
    outfile << "    -- local" << endl;
    outfile << endl;
    outfile << "begin" << endl;
    outfile << "end Behavioral;" << endl;

    return err;
}

int
TRGCDCFrontEnd::implementationPort(const TRGCDCFrontEnd::boardType & type,
                                   ofstream & outfile) {
    outfile << "    port (" << endl;

    //...125MHz clock (TRG system clock)...
    outfile << "       -- 125MHz clock (TRG system clock)" << endl;
    outfile << "          clk : in std_logic;" << endl;

    //...Coarse timing...
    outfile << "       -- Coarse timing (counter with 125MHz clock)" << endl;
    outfile << "          tmc : in std_logic_vector(0 to 12);" << endl;

    //...Hit pattern...
    outfile << "       -- Hit pattern(48 bits for 48 wires)" << endl;
    outfile << "          ptn : in std_logic_vector(0 to 47);" << endl;

    //...Fine timing for each wire...
    outfile << "       -- Fine timing within 125MHz clock" << endl;
    for (unsigned i = 0; i < 48; i++) {
        outfile << "          w" << TRGUtil::itostring(i)
                << " : in std_logic_vector(0 to 3);" << endl;
    }

    //...Hit pattern output...
    outfile << "       -- Hit pattern output" << endl;
    outfile << "          hit : out std_logic_vector(0 to 47);" << endl;
    outfile << "       -- 4 bit fine timing for 16 priority wires" << endl;
    outfile << "          pri : out std_logic_vector(0 to 63);" << endl;
    outfile << "       -- 2 bit fine timing for the fastest hit in 16 TS"
            << endl;
    outfile << "          fst : out std_logic_vector(0 to 31)" << endl;
    outfile << "    );" << endl;

    return 0;
}

} // namespace Belle2
