/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef TRCECLDQMMODULE_h
#define TRCECLDQMMODULE_h

#include <framework/core/HistoModule.h>
#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerEvtStore.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerSumStore.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>

class TH1;
class TH2;

namespace Belle2 {

  //! module of ecl trigger DQM
  class TRGECLDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGECLDQMModule();
    //! Destrunctor
    virtual ~TRGECLDQMModule();

  public:
    //! initialize
    virtual void initialize() override;
    //! begin Run
    virtual void beginRun() override;
    //! Event
    virtual void event() override;
    //! End Run
    virtual void endRun() override;
    //! terminate
    virtual void terminate() override;
    //! Define Histogram
    virtual void defineHisto() override;

  private:
    //! The number of clocks to group the number of TCs of ECLTRG
    int m_grpclknum;

    //! TCId histogram
    TH1* h_TCId = nullptr;
    //! TCthetaId histogram
    TH1* h_TCthetaId = nullptr;
    //! TCphiId histogram
    TH1* h_TCphiId_BWD = nullptr;
    //! TCphiId histogram
    TH1* h_TCphiId_BR = nullptr;
    //! TCphiId histogram
    TH1* h_TCphiId_FWD = nullptr;
    //! TC Energy
    TH1* h_TCEnergy = nullptr;
    //! Total  Energy
    TH1* h_TotalEnergy = nullptr;
    //! TC Energy histogram on narrow range
    TH1* h_Narrow_TCEnergy = nullptr;
    //! Total  Energy  on narrow range
    TH1* h_Narrow_TotalEnergy = nullptr;
    //! N of TC Hit / event
    TH1* h_n_TChit_event = nullptr;
    //! N of TC Hit / event vs. time since injection
    TH1* h_n_TChit_clean = nullptr;
    //! N of TC Hit / events in the injection BG clean region vs. time since injection
    TH1* h_n_TChit_injHER = nullptr;
    //! N of TC Hit / events in the HER injection BG region vs. time since injection
    TH1* h_n_TChit_injLER = nullptr;
    //! N of TC Hit / events in the LER injection BG region vs. time since injection
    TH2* h_nTChit_injtime = nullptr;
    //! N of Cluster / event
    TH1* h_Cluster = nullptr;
    //! TC Timing / event
    TH1* h_TCTiming = nullptr;
    //! Event Timing / event
    TH1* h_TRGTiming = nullptr;
    //! TC Timing / event
    TH1* h_Cal_TCTiming = nullptr;
    //! Event Timing / event
    TH1* h_Cal_TRGTiming = nullptr;
    //! ECL Trigger Bit
    TH1* h_ECL_TriggerBit = nullptr;
    //! Energy sum of 2 Top energetic clusters when 3D bhabnha bit on
    TH1* h_Cluster_Energy_Sum = nullptr;

    //! N of TC Hit / event per two ETM clocks
    TH1* h_n_TChit_event_clkgrp = nullptr;
    //! N of TC Hit / event vs. time since injection per two ETM clocks
    TH1* h_n_TChit_clean_clkgrp = nullptr;
    //! N of TC Hit / events in the injection BG clean region vs. time since injection per two ETM clocks
    TH1* h_n_TChit_injHER_clkgrp = nullptr;
    //! N of TC Hit / events in the HER injection BG region vs. time since injection per two ETM clocks
    TH1* h_n_TChit_injLER_clkgrp = nullptr;
    //! N of TC Hit / events in the LER injection BG region vs. time since injection per two ETM clocks
    TH2* h_nTChit_injtime_clkgrp = nullptr;

    //! N of TC Hit / event per two ETM clocks
    TH1* h_n_TChit_part_event_clkgrp[3] = {nullptr};
    //! N of TC Hit / event vs. time since injection per two ETM clocks for each part (FWD, BRL, and BWD)
    TH1* h_n_TChit_part_clean_clkgrp[3] = {nullptr};
    //! N of TC Hit / events in the injection BG clean region vs. time since injection per two ETM clocks for each part (FWD, BRL, and BWD)
    TH1* h_n_TChit_part_injHER_clkgrp[3] = {nullptr};
    //! N of TC Hit / events in the HER injection BG region vs. time since injection per two ETM clocks for each part (FWD, BRL, and BWD)
    TH1* h_n_TChit_part_injLER_clkgrp[3] = {nullptr};
    //! N of TC Hit / events in the LER injection BG region vs. time since injection per two ETM clocks for each part (FWD, BRL, and BWD)
    TH2* h_nTChit_part_injtime_clkgrp[3] = {nullptr};

    //! Hit TCId
    std::vector<int> m_TCId;
    //! Hit TCHitWin
    std::vector<int> m_TCHitWin;
    //! Hit TC Energy
    std::vector<double> m_TCEnergy;
    //! Hit TC Timing
    std::vector<double> m_TCTiming;
    //! FAM Revolution Clk
    std::vector<double> m_RevoFAM;
    //! Event Timing
    std::vector<double> m_FineTiming;
    //! GDL Revolution Clk
    std::vector<double> m_RevoTrg;


    //! Trg ECL Unpakcer TC output
    StoreArray<TRGECLUnpackerStore> m_trgeclHitArray;
    //! Trg ECL Unpakcer Event output
    StoreArray<TRGECLUnpackerEvtStore> m_trgeclEvtArray;
    //! Trg Ecl Unpacker Summary output
    StoreArray<TRGECLUnpackerSumStore> m_trgeclSumArray;
    //! Trg ECL Cluster output
    StoreArray<TRGECLCluster> m_trgeclCluster;
    //! Array to access the FTSW information
    StoreObjPtr<EventLevelTriggerTimeInfo> m_trgTime;

    //! DB pointerto access the hardware clock information
    DBObjPtr<HardwareClockSettings> m_hwclkdb;
  };

}

#endif
