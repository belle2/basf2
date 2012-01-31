//----------------------------------------------------------
// $Id$
//----------------------------------------------------------
// Filename : TRGECLTCMapping.cc
// Section  : TRG ECL
// Owner    : Yuuji Unno
// Email    : yunno@post.kek.jp
//----------------------------------------------------------
// Description : A class to represent ECL
//----------------------------------------------------------
// $Log$
//----------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGECLTCMAPPING_SHORT_NAMES

#include <framework/logging/Logger.h>
#include <trg/ecl/TRGECLTCMapping.h>

using namespace std;
using namespace Belle2;
//
//
//
TRGECLTCMapping::TRGECLTCMapping() : 
  _tcid(-1), _tcsubid(-1),_tcthetaid(-1),_tcphiid(-1)
{
}
//
//
//
int
TRGECLTCMapping::getTCIdFromXtalId(int XtalId)
{
  //
  if (XtalId < 1 || XtalId > 8736) { 
    B2ERROR("TRGECLTCMapping> input XtalId is wrong!!!"); 
  }
  // 'XtalId' is Xtal ID
  // 'XtalId0' is 'XtalId' - 1
  int XtalId0 = XtalId - 1;
  int e_o = 0; // tmporal variable showing even(0) or odd(1).
  if (XtalId0 < 1152) {
    //
    // forward-endcap [Xtal=1-1152, TC=1-80(80)]
    //
    if (XtalId0 <  48) { _tcid = ((XtalId0) / 3) * 5 + 1; } 
    else if (XtalId0 <  96) { _tcid = ((XtalId0 - 48) / 3) * 5 + 1; } 
    else if (XtalId0 < 160) { _tcid = ((XtalId0 - 96) / 4) * 5 + 1; } 
    else if (XtalId0 < 224) { _tcid = ((XtalId0 - 160) / 4) * 5 + 1; } 
    else if (XtalId0 < 288) {
      e_o = (XtalId0 - 224) / 2;
      _tcid = (e_o % 2 == 0) ? e_o * 5 / 2 + 2 : e_o * 5 / 2 + 3;
    } else if (XtalId0 < 384) {
      e_o = (XtalId0 - 288) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 5 / 2 + 2 : e_o * 5 / 2 + 3;
    } else if (XtalId0 < 480) {
      e_o = (XtalId0 - 384) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 5 / 2 + 2 : e_o * 5 / 2 + 3;
    } else if (XtalId0 < 576) {
      e_o = (XtalId0 - 480) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 5 / 2 + 2 : e_o * 5 / 2 + 3;
    } else if (XtalId0 < 672) {
      e_o = (XtalId0 - 576) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 5 / 2 + 2 : e_o * 5 / 2 + 3;
    } else if (XtalId0 < 768) {
      e_o = (XtalId0 - 672) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 5 / 2 + 3 : e_o * 5 / 2 + 2;
    } else if (XtalId0 < 864) {
      e_o = (XtalId0 - 768) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 5 / 2 + 3 : e_o * 5 / 2 + 2;
    } else if (XtalId0 < 1008) {
      e_o = ((XtalId0 - 864) % 9 < 5) ? (XtalId0 - 864) / 9 * 2 : (XtalId0 - 864) / 9 * 2 + 1;
      _tcid = (e_o % 2 == 0) ? e_o * 5 / 2 + 3 : e_o * 5 / 2 + 2;
    } else if (XtalId0 < 1152) {
      e_o = ((XtalId0 - 864) % 9 < 5) ? (XtalId0 - 1008) / 9 * 2 : (XtalId0 - 1008) / 9 * 2 + 1;
      _tcid = (e_o % 2 == 0) ? e_o * 5 / 2 + 3 : e_o * 5 / 2 + 2;
    }
  } else if (XtalId0 < 7776) {
    //
    // Barrel [Xtal=1153-7776, TC=81-512(432)]
    //
    int offset = ((XtalId0 - 1152) / 4) / 144;
    _tcid = (((XtalId0 - 1152) / 4) % 36) * 12 + 81 + offset;
  } else {
    //
    // Backward-endcap [Xtal=7777-8736, TC=513-576(64)]
    //
    if (XtalId0 < 7920) {
      e_o = ((XtalId0 - 7776) % 9 < 5) ? (XtalId0 - 7776) / 9 * 2 : (XtalId0 - 7776) / 9 * 2 + 1;
      _tcid = (e_o % 2 == 0) ? e_o * 2 + 513 + 2 : e_o * 2 + 513 - 1;
    } else if (XtalId0 < 8064) {
      e_o = ((XtalId0 - 7920) % 9 < 5) ? (XtalId0 - 7920) / 9 * 2 : (XtalId0 - 7920) / 9 * 2 + 1;
      _tcid = (e_o % 2 == 0) ? e_o * 2 + 513 + 2 : e_o * 2 + 513 - 1;
    } else if (XtalId0 < 8160) {
      e_o = (XtalId0 - 8064) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 2 + 513 + 2 : e_o * 2 + 513 - 1;
    } else if (XtalId0 < 8256) {
      e_o = (XtalId0 - 8160) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 2 + 513 + 2 : e_o * 2 + 513 - 1;
    } else if (XtalId0 < 8352) {
      e_o = (XtalId0 - 8256) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 2 + 513 + 3 : e_o * 2 + 513 - 2;
    } else if (XtalId0 < 8448) {
      e_o = (XtalId0 - 8352) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 2 + 513 + 3 : e_o * 2 + 513 - 2;
    } else if (XtalId0 < 8544) {
      e_o = (XtalId0 - 8448) / 3;
      _tcid = (e_o % 2 == 0) ? e_o * 2 + 513 + 3 : e_o * 2 + 513 - 2;
    } else if (XtalId0 < 8608) {
      e_o = (XtalId0 - 8544) / 2;
      _tcid = (e_o % 2 == 0) ? e_o * 2 + 513 + 3 : e_o * 2 + 513 - 2;
    } else if (XtalId0 < 8672) {
      e_o = (XtalId0 - 8608) / 2;
      _tcid = (e_o % 2 == 0) ? e_o * 2 + 513 + 3 : e_o * 2 + 513 - 2;
    } else if (XtalId0 < 8736) {
      e_o = (XtalId0 - 8672) / 2;
      _tcid = (e_o % 2 == 0) ? e_o * 2 + 513 + 3 : e_o * 2 + 513 - 2;
    }
  }
  if (_tcid < 1 || _tcid > 576) { 
    B2ERROR("TRGECLTCMapping> output TCId is wrong!!!"); 
  }
  return _tcid;
}
//
//
//
int
TRGECLTCMapping::getTCSubIdFromXtalId(int XtalId)
{
  if (XtalId < 1 || XtalId > 8736) { 
    B2ERROR("TRGECLTCMapping> input XtalId is wrong!!!"); 
  }
  if(XtalId <= 1152){
    //
    // FW XtalId = 1 - 1152
    //
    if (XtalId <= 96){ _tcsubid = 1;}
    else if (XtalId <= 160){
      int aaa = (XtalId-97)%4;
      _tcsubid = (aaa==0 || aaa==3) ? 0 : 1;
    }
    else if (XtalId <= 224){_tcsubid = 0;}
    else if (XtalId <= 384){_tcsubid = 1;}
    else if (XtalId <= 480){
      int aaa = (XtalId-385)%3;
      _tcsubid = (aaa==1) ? 1 : 0;
    }
    else if (XtalId <= 672){_tcsubid = 0;}
    else if (XtalId <= 770){_tcsubid = 1;}
    else if (XtalId <= 863){
      int aaa = ((XtalId-771)%6)/3;
      _tcsubid = (aaa==0) ? 0 : 1;
    }
    else if (XtalId <= 1007){
      int aaa = (XtalId-864)%9;
      _tcsubid = (aaa==0 || aaa==1 || aaa==4 || aaa==7) ? 1 : 0;
    }
    else if (XtalId <= 1008){_tcsubid = 1;}
    else if (XtalId <= 1152){
      int aaa = (XtalId-1009)%9;
      _tcsubid = (aaa==0) ? 1 : 0;
    }
  }else if(XtalId <= 7776){
    //
    // BR XtalId = 1153 - 7776
    //
    if (XtalId <= 7488){
      int aaa = (XtalId-1153)%4;
      _tcsubid = (aaa<2 ) ? 1 : 0;
    } else { _tcsubid = 1;}
  }else{
    //
    // BW XtalId 7777 - 8736
    //
    if (XtalId <= 7922){_tcsubid = 0;}
    else if (XtalId <= 8061){
      int aaa = (XtalId-7923)%9;
      _tcsubid = (aaa==0 || aaa==1 || aaa==5 || aaa==6 ) ? 1 : 0;
    }
    else if (XtalId <= 8062){_tcsubid = 0;}
    else if (XtalId <= 8256){_tcsubid = 1;}
    else if (XtalId <= 8449){_tcsubid = 0;}
    else if (XtalId <= 8542){
      int aaa = (XtalId-8450)%3;
      _tcsubid = (aaa<2 ) ? 1 : 0;
    }
    else{_tcsubid = 1;}
  }
  if (_tcsubid < 0 || _tcsubid > 1) { 
    B2ERROR("TRGECLTCMapping> output TCSubId is wrong!!!"); 
  }
  return _tcsubid;
}
//
//
//
int
TRGECLTCMapping::getTCThetaIdFromTCId(int TCId){

  if (TCId < 1 || TCId > 576) { 
    B2ERROR("TRGECLTCMapping> input TCId is wrong!!!"); 
  }
  int TCId0 = TCId - 1;
  // forward-endcap  [Xtal=   1-1152, TC=  1- 80(80)]
  // Barrel          [Xtal=1153-7776, TC= 81-512(432)]
  // Backward-endcap [Xtal=7777-8736, TC=513-576(64)]
  if      (TCId0 < 80){_tcthetaid = (TCId0%5<3) ? TCId0%5 : 5-TCId0%5;}
  else if (TCId0 <512){_tcthetaid = (TCId0-80)%12+3;}
  else{
    if (TCId0 == 512){_tcthetaid = 16;}
    else             {_tcthetaid = ((TCId0-513)/2)%2 ? 16:15;}
  }
  if (_tcthetaid < 0 || _tcthetaid > 16) { 
    B2ERROR("TRGECLTCMapping> output TCThetaId is wrong!!!"); 
  }
  return _tcthetaid;
}
//
//
//
int
TRGECLTCMapping::getTCPhiIdFromTCId(int TCId){

  if (TCId < 1 || TCId > 576) { 
    B2ERROR("TRGECLTCMapping> input TCId is wrong!!!"); 
  }
  int TCId0 = TCId-1;
  // forward-endcap  [Xtal=   1-1152, TC=  1- 80(80)]
  // Barrel          [Xtal=1153-7776, TC= 81-512(432)]
  // Backward-endcap [Xtal=7777-8736, TC=513-576(64)]
  if      (TCId0 <  80){_tcphiid = (TCId0%5<3) ? _tcphiid=(TCId0/5)*2 : _tcphiid=1+(TCId0/5)*2;}
  else if (TCId0 < 512){_tcphiid = (TCId0-80)/12;}
  else                 {_tcphiid = ((TCId0-512)/2)%2 ? (TCId0-512)/2-1 : (TCId0-512)/2+1;}
  if (_tcphiid < 0 || _tcphiid > 35) { 
    B2ERROR("TRGECLTCMapping> output TCPhiId is wrong!!!"); 
  }
  return _tcphiid;
}
//
//===<END>
//
