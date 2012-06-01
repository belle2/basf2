//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : LUT.cc
// Section  : TRG CDC
// Owner    : KyungTae KIM (K.U.)
// Email    : ktkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to use LUTs for TRGCDC
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include <fstream>
#include "trg/trg/Debug.h"
#include "trg/cdc/LUT.h"
//#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Link.h"
#include <cstdlib>

using namespace std;

namespace Belle2{

    std::string TRGCDCLUT::version(void) const{
	return string("TRGCDCLUT 0.00");
    }

    TRGCDCLUT::TRGCDCLUT(const string & name, const TRGCDC &TRGCDC)
	: _name(name),
	  _cdc(TRGCDC){
    }
    
    TRGCDCLUT::~TRGCDCLUT(){
    }
	
    void TRGCDCLUT::initialize(const string & filename){
	int lutcomp;
	ifstream openFile;
	openFile.open(filename.c_str());
	if (openFile.fail()) {
	    cout << "TRGCDCLUT !!! can not open file : " << filename << endl;
	    cout << "              LUT is not initialized yet" << endl;
	    return;
	}
	if(std::strstr(filename.c_str(),"LRLUT.coe")){
		for(int i=0; i<2048;i++){
		    openFile >> lutcomp;
		    m_LRLUT[i]=lutcomp;
		}
	}
	else if(strstr(filename.c_str(),"LRLUTIN.coe")){
		for(int i=0; i<32768;i++){
		    openFile >> lutcomp;
		    m_LRLUTIN[i]=lutcomp;
		}
	}
	cout << "TTRGCDCLUT ... LUT(" << _name << ") initilized with "
	     << filename << endl;
    }

	/// using pattenr(ptn) and supler layer id(slid) to check left/right
    int TRGCDCLUT::getLRLUT(int ptn,int slid) const {
	if(slid) return m_LRLUT[ptn];
	else return m_LRLUTIN[ptn];

    }
}

