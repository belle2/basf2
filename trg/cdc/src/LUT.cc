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

#include <sstream>
#include <fstream>
#include "trg/trg/Debug.h"
#include "trg/cdc/LUT.h"
//#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Link.h"
#include <cstdlib>
#include <math.h>

using namespace std;

namespace Belle2{

    std::string TRGCDCLUT::version(void) const{
	return string("TRGCDCLUT 1.00");
    }

    TRGCDCLUT::TRGCDCLUT(const string & name, const TRGCDC &TRGCDC)
	: _name(name){
//	  _cdc(TRGCDC){
    }
    
    TRGCDCLUT::TRGCDCLUT(){
    }

    TRGCDCLUT::~TRGCDCLUT(){
    }
	
    void TRGCDCLUT::initialize(const string & filename){
	int lutcomp;
  string coeTrash, coeComp;
	ifstream openFile;
	openFile.open(filename.c_str());
	if (openFile.fail()) {
	    cout << "TRGCDCLUT !!! can not open file : " << filename << endl;
	    cout << "              LUT is not initialized yet" << endl;
	    return;
	}
	if (std::strstr(filename.c_str(),"innerLRLUT_v2.1.coe")){
		for (int i=0;i<=32768;i++){
			openFile >> coeTrash >> coeComp;
			istringstream iComp(coeComp);
			if(i!=0){
				iComp>>m_LRLUTIN[i-1];
			}
		}
	}
	else if(strstr(filename.c_str(),"outerLRLUT_v2.1.coe")){
		for (int i=0;i<=2048;i++){
			openFile >> coeTrash >>  coeComp;
			istringstream iComp(coeComp);
			if(i!=0){
				iComp >> m_LRLUT[i-1];
			}
		}
	}
	else if(std::strstr(filename.c_str(),"LRLUT.coe")){
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
  else if(strstr(filename.c_str(),"TSF.FPGA.SL0.coe")){
    for(int i=0; i<=32768; i++){
      openFile >> coeTrash >> coeComp;
      istringstream iComp(coeComp);
      if(i != 0) {
        iComp >> m_HitLRLUTSL0[i-1];
        //cout<<"HitLRLUTSL0["<<i-1<<"]: "<<m_HitLRLUTSL0[i-1]<<endl;
      }
    }
  }
  else if(strstr(filename.c_str(),"TSF.FPGA.SL1.coe")){
    for(int i=0; i<=2048; i++){
      openFile >> coeTrash >> coeComp;
      //cout<<i<<" "<<coeTrash<<" "<<coeComp<<endl;
      istringstream iComp(coeComp);
      if(i != 0) {
        iComp >> m_HitLRLUTSL1[i-1];
      }
      //if(i == 1) cout<<iComp<<" "<<m_HitLRLUTSL1[i-1]<<endl;
    }
  }
  else if(strstr(filename.c_str(),"TSF.FPGA.SL2.coe")){
    for(int i=0; i<=2048; i++){
      openFile >> coeTrash >> coeComp;
      istringstream iComp(coeComp);
      if(i != 0) {
        iComp >> m_HitLRLUTSL2[i-1];
      }
    }
  }
  else if(strstr(filename.c_str(),"TSF.FPGA.SL3.coe")){
    for(int i=0; i<=2048; i++){
      openFile >> coeTrash >> coeComp;
      istringstream iComp(coeComp);
      if(i != 0) {
        iComp >> m_HitLRLUTSL3[i-1];
      }
    }
  }
  else if(strstr(filename.c_str(),"TSF.FPGA.SL4.coe")){
    for(int i=0; i<=2048; i++){
      openFile >> coeTrash >> coeComp;
      istringstream iComp(coeComp);
      if(i != 0) {
        iComp >> m_HitLRLUTSL4[i-1];
      }
    }
  }
  else if(strstr(filename.c_str(),"TSF.FPGA.SL5.coe")){
    for(int i=0; i<=2048; i++){
      openFile >> coeTrash >> coeComp;
      istringstream iComp(coeComp);
      if(i != 0) {
        iComp >> m_HitLRLUTSL5[i-1];
      }
    }
  }
  else if(strstr(filename.c_str(),"TSF.FPGA.SL6.coe")){
    for(int i=0; i<=2048; i++){
      openFile >> coeTrash >> coeComp;
      istringstream iComp(coeComp);
      if(i != 0) {
        iComp >> m_HitLRLUTSL6[i-1];
      }
    }
  }
  else if(strstr(filename.c_str(),"TSF.FPGA.SL7.coe")){
    for(int i=0; i<=2048; i++){
      openFile >> coeTrash >> coeComp;
      istringstream iComp(coeComp);
      if(i != 0) {
        iComp >> m_HitLRLUTSL7[i-1];
      }
    }
  }
  else if(strstr(filename.c_str(),"TSF.FPGA.SL8.coe")){
    for(int i=0; i<=2048; i++){
      openFile >> coeTrash >> coeComp;
      istringstream iComp(coeComp);
      if(i != 0) {
        iComp >> m_HitLRLUTSL8[i-1];
      }
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

  /// get Hit LR component from pattern id.
  std::string TRGCDCLUT::getHitLRLUT(int ptn,int slid) const {
    if(slid == 0) return m_HitLRLUTSL0[ptn];
    else if(slid == 1) return m_HitLRLUTSL1[ptn];
    else if(slid == 2) return m_HitLRLUTSL2[ptn];
    else if(slid == 3) return m_HitLRLUTSL3[ptn];
    else if(slid == 4) return m_HitLRLUTSL4[ptn];
    else if(slid == 5) return m_HitLRLUTSL5[ptn];
    else if(slid == 6) return m_HitLRLUTSL6[ptn];
    else if(slid == 7) return m_HitLRLUTSL7[ptn];
    else if(slid == 8) return m_HitLRLUTSL8[ptn];
    else return "00000";
  }

  double TRGCDCLUT::getValue(double id) const{
    int range=pow(2, m_bitsize);
    if(id>=range){
      return 0;
    }
    else{
      return double(m_data[id]);
    }
//      return m_data[id];
  }


  void TRGCDCLUT::setDataFile(const string & filename, int nInputBit){
    m_bitsize=nInputBit;
    m_name= filename;

    ifstream openFile;
    string tmpstr;
    int tmpint;
    int i=0;
    int range=pow(2,nInputBit);
    openFile.open(filename.c_str());
    m_data.resize(range);
    while(getline(openFile,tmpstr)){
      if(!(tmpstr.size()==0)){
      if(tmpstr[0]>='0' && tmpstr[0]<='9'){
        tmpint=atoi(tmpstr.c_str());
        m_data[i]=tmpint;
        i++;
      }
      else{
	continue;
      }

    }
    }
    
  }
}

