//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGCDCTSStreamReader.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A program to read Track Segment (TS) stream data
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES

#include <iostream>
#include <fstream>
#include <string>
#include "trg/trg/BitStream.h"

using namespace std;
using namespace Belle2;

#define DEBUG_LEVEL   1
#define NAME          "TRGCDCTSStreamReader"
#define VERSION       "version 0.01"
#define ENV_PATH      "BELLE2_LOCAL_DIR"

int
main(int argc, char * argv[]) {

    cout << NAME << " ... " << VERSION << endl;
    const string tab = "    ";

    //...Check arguments...
    if (argc < 2) {
        cout << NAME << " !!! one argument necessary" << endl
             << tab << " 1 : Track Segment Stream data file" << endl;
        return -1;
    }

    //...1st argument...
    const string inname = argv[1];
    cout << tab << "Track Segment Stream : " << inname << endl;

    //...Open configuration data...
    ifstream infile(inname.c_str(), ios::in | ios::binary);
    if (infile.fail()) {
        cout << NAME << " !!! can not open file" << endl
             << "    " << inname << endl;
        return -2;
    }

    //...Read stream...
    while (! infile.eof()) {

	//...Read record type...
	unsigned rec = 0;
	infile.read((char *) & rec, 4);
	unsigned siz = 0;
	infile.read((char *) & siz, 4);

	cout << "... " << hex << rec << "," << dec << siz << endl;

	//...Record manipulation...
	switch (rec) {
	case TRGBSRecord_Comment: {
	    unsigned csiz = siz / 8;
	    if (siz % 8) ++csiz;
	    char * buf = new char[csiz + 1];
	    infile.read(buf, csiz);
	    buf[csiz] = 0;

	    cout << "siz,csiz=" << siz << "," << csiz << endl;

	    if (DEBUG_LEVEL)
		cout << "Comment : " << buf << endl;
	    break;
	}

	case TRGBSRecord_BeginRun:
	    if (DEBUG_LEVEL)
		cout << "BeginRun : " << siz << endl;
	    break;

	case TRGBSRecord_EndRun:
	    if (DEBUG_LEVEL)
		cout << "EndRun : " << siz << endl;
	    break;

	case TRGBSRecord_BeginEvent:
	    if (DEBUG_LEVEL)
		cout << "BeginEvent : " << siz << endl;
	    break;

	case TRGBSRecord_EndEvent:
	    if (DEBUG_LEVEL)
		cout << "EndEvent : " << siz << endl;
	    break;

	case TRGBSRecord_Clock: {
	    unsigned clk = 0;
	    infile.read((char *) & clk, 4);
	    if (DEBUG_LEVEL)
		cout << "Clock : " << clk << endl;
	    break;
	}

	case TRGBSRecord_TrackSegmentSL0:
	case TRGBSRecord_TrackSegmentSL1:
	case TRGBSRecord_TrackSegmentSL2:
	case TRGBSRecord_TrackSegmentSL3:
	case TRGBSRecord_TrackSegmentSL4:
	case TRGBSRecord_TrackSegmentSL5:
	case TRGBSRecord_TrackSegmentSL6:
	case TRGBSRecord_TrackSegmentSL7:
	case TRGBSRecord_TrackSegmentSL8: {
	    unsigned csiz = siz / 8;
	    if (siz % 8) ++csiz;
	    char * buf = new char[csiz];
	    infile.read(buf, csiz);

	    TRGBitStream bs = TRGBitStream(buf, siz);
	    bs.dump();
	    break;
	}
	default:
	    break;
	}
    }
    infile.close();

    //...Termination...
    cout << NAME << " ... terminated" << endl;
}
