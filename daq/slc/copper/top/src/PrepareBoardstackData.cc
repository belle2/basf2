#include "daq/slc/copper/top/B2L_common.h"
#include "daq/slc/copper/top/B2L_defs.h"
#include "daq/slc/copper/top/PrepareBoardstackData.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/copper/HSLB.h>
#include <daq/slc/database/DBObject.h>
#include <daq/slc/runcontrol/RCCallback.h>

#include <iostream>
#include <unistd.h>

namespace PrepBoardstackData {

  using namespace std;
  using namespace Belle2;

  map<std::string, int> registerValueMap;

  void LoadDefaultRegisterValues() {}
  void UpdateRegisterFromDatabase(Belle2::HSLB& hslb, const Belle2::DBObject& databaseObject) {}

  void PrepareBoardStack(HSLB& hslb)
  {
    RCCallback dummyCallback;
    PrepareBoardStack(hslb, dummyCallback);
  }

  void PrepareBoardStack(HSLB& hslb, RCCallback& callback)
  {
    int mode_ext = 3;//set standard to FE+ped-sub
    //this callback should get defined in PrepareBoardstackFe.cpp
    callback.get("ScrodfeMode", mode_ext);
    if (mode_ext < 0 || mode_ext > 4) mode_ext = 0;

    Write_Register(hslb, SCROD_AxiCommon_trigMask, 0, 0, 0);
    Write_Register(hslb, SCROD_PS_featureExtMode, mode_ext, 0, 0);

    int carriersDetected = Read_Register(hslb, SCROD_PS_carriersDetected, 0, 0);
    cout << carriersDetected << " carriers detected" << endl;
    for (int i = 0; i < carriersDetected; ++i) {
      PrepareCarrier(hslb, callback, i);
    }
  }

  void PrepareCarrier(HSLB& hslb, RCCallback& callback, const unsigned carrier)
  {
    Write_Register(hslb, CARRIER_PS_calEn, 0xF, carrier, 0);
    Write_Register(hslb, CARRIER_PS_calCh, 7, carrier, 0);
    Write_Register(hslb, CARRIER_AxiCommon_trigMask, 0, carrier, 0);

    for (int i = 0; i < 4; ++i) {
      PrepareAsic(hslb, callback, carrier, i);
    }
  }

  void PrepareAsic(HSLB& hslb, RCCallback& callback, const unsigned carrier,
                   const unsigned asic)
  {
    cout << "carrier " << carrier << " asic " << asic << endl;
    Write_Register(hslb, CARRIER_IRSX_wrAddrMode, wrAddrMode_Cyclic, carrier, asic);
    Write_Register(hslb, CARRIER_IRSX_wrAddrStart, 0x0, carrier, asic);
    Write_Register(hslb, CARRIER_IRSX_wrAddrStop, 0xfd, carrier, asic);
    Write_Register(hslb, CARRIER_IRSX_wrAddrJunk, 0xff, carrier, asic);

    Write_Register(hslb, CARRIER_IRSX_readoutMode, readoutMode_ROI, carrier, asic);

    Write_Register(hslb, CARRIER_IRSX_readoutLookback, 44, carrier, asic);
    Write_Register(hslb, CARRIER_IRSX_writesAfterTrig, 42, carrier, asic);
    Write_Register(hslb, CARRIER_IRSX_readoutWindows, 15, carrier, asic);
    Write_Register(hslb, CARRIER_IRSX_convertResetWait, 0xf, carrier, asic);

    Write_Register(hslb, CARRIER_IRSX_readoutChannels, 0x6000 + 125, carrier, asic);

    Write_Register(hslb, CARRIER_IRSX_enableTestPattern, 0, carrier, asic);
    int phase = 0;
    Write_Register(hslb, CARRIER_IRSX_trigWidth1, 0x00FF0000, carrier, asic);
    int curphase = Read_Register(hslb, CARRIER_IRSX_phaseRead, carrier, asic);
    cout << "current phase " << curphase << endl;

    while (curphase != phase) {
      Write_Asic_Direct_Register(hslb, carrier, asic, 179, 128 + 5 + 64);
      Write_Asic_Direct_Register(hslb, carrier, asic, 179, 5 + 64);
      sleep(1);

      Write_Register(hslb, CARRIER_IRSX_trigWidth1, 0x00FF0000, carrier, asic);
      curphase = Read_Register(hslb, CARRIER_IRSX_phaseRead, carrier, asic);
      cout << "current phase " << curphase << endl;
    }

    cout << "--> Current phase set to " << curphase << endl;
  }
}
