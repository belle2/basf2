#include <daq/slc/copper/top/b2l_top_board.h>
#include <daq/slc/copper/top/b2l_top_asic.h>
#include <daq/slc/copper/top/b2l_top.h>
#include <daq/slc/copper/top/topreg.h>
#include <daq/slc/copper/top/TOPFEE.h>

#include <daq/slc/copper/HSLB.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

namespace Belle2 {

  namespace TOPDAQ {

    void config1boardstack(HSLB& hslb, RCCallback& callback)
    {
      b2l_write(hslb, SCROD_AxiCommon_fanoutResetL, 1);
      for (int carrier = 0; carrier < 4; carrier++) {
        config1carrier(hslb, callback, carrier);
      }

    }
    void config1carrier(HSLB& hslb, RCCallback& callback, int carrier)
    {
      // Turn on carrier clock distribution (SSTin clock fanout)
      b2l_write(hslb, CARRIER_AxiCommon_fanoutResetL, 1, carrier);

      // set the SCROD ID
      unsigned int SCROD_ID = b2l_read(hslb, SCROD_AxiVersion_UserID);
      SCROD_ID = SCROD_ID / 16; // move ID to bits 19:4
      int val = (SCROD_ID) + carrier;
      b2l_write(hslb, CARRIER_AxiCommon_irsxDataSelect, val, carrier);

      // "power-cycle" the ASICs
      b2l_write(hslb, CARRIER_PS_asicRegEn, 0x00, carrier);// almost certainly unnecessary;
      b2l_write(hslb, CARRIER_PS_asicRegEn, 0x0F, carrier);

      // enable the preamps
      b2l_write(hslb, CARRIER_PS_ampRegEn, 0x3, carrier);// just turn these on
      b2l_write(hslb, CARRIER_PS_ampEn, 0xF, carrier);

      // set the vPed values
      b2l_write(hslb, CARRIER_PS_vPed0, vPed, carrier);
      b2l_write(hslb, CARRIER_PS_vPed1, vPed, carrier);
      b2l_write(hslb, CARRIER_PS_vPed2, vPed, carrier);
      b2l_write(hslb, CARRIER_PS_vPed3, vPed, carrier);

      for (int asic = 0; asic < 4; asic++) {
        config1asic(hslb, callback, carrier, asic);
      }
    }

  }

}

