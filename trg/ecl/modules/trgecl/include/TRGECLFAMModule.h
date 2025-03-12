/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLFAMModule_H
#define TRGECLFAMModule_H

#include <string>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>

#include <mdst/dataobjects/EventLevelClusteringInfo.h>

#include "trg/ecl/dataobjects/TRGECLFAMAna.h"
#include "trg/ecl/dataobjects/TRGECLDigi0.h"
#include "trg/ecl/dataobjects/TRGECLHit.h"
#include "trg/ecl/dataobjects/TRGECLWaveform.h"

#include "trg/ecl/dbobjects/TRGECLFAMPara.h"

namespace Belle2 {

  /** A module of FAM */
  class TRGECLFAMModule : public Module {
  public:

    /** Constructor */
    TRGECLFAMModule();

    /** Destructor */
    virtual ~TRGECLFAMModule();
    /** Initializes TRGECLFAMModule. */
    virtual void initialize() override;
    /** Called when new run started.*/
    virtual void beginRun() override;
    /** Called event by event. */
    virtual void event() override;
    /** Called when run ended. */
    virtual void endRun() override;
    /** Called when processing ended.*/
    virtual void terminate() override;

  public:

    /** returns version of TRGECLFAMModule.*/
    std::string version(void) const;

  private: /** Parameters*/

    /** Debug level.*/
    int m_debugLevel;
    /** fam Method*/
    int m_famMethod;
    /** Time interval */
    int m_binTimeInterval;
    /** Waveform */
    int m_SaveTCWaveForm;
    /** save Beam background tag in TRGECLHit table */
    int m_beambkgtag;
    /** save FAM ana table */
    int m_famana;
    /** Threshold input*/
    int m_SetTCEThreshold;
    /** Set Shaping Function */
    int m_FADC;
    /** Use Condition DB*/
    int m_ConditionDB;
    /** Set source of TC data (1:=ECLHit or 2:=ECLSimHit or 3:=ECLHit+TRGECLBGTCHit) */
    /** ("1:=ECLHit" is used for signal w/o bkg, and real time background monitor) */
    int m_SourceOfTC;

    /** Config. file name. */
    std::string m_configFilename;


  protected:


  private:

    /**  Run number */
    int m_nRun;
    /**  Event number */
    int m_nEvent;
    /** Digitized TC E [GeV] */
    std::vector<std::vector<double>> m_TCDigiE;
    /** Digitized TC T [ns] */
    std::vector<std::vector<double>> m_TCDigiT;
    /** Fit TC E [GeV] */
    std::vector<std::vector<double>> m_TCFitE;
    /** Fit TC T [ns] */
    std::vector<std::vector<double>> m_TCFitT;
    /** Threshold */
    std::vector<int> m_TCEThreshold;

    StoreArray<TRGECLDigi0>    m_TRGECLDigi0; /**< output for TRGECLDigi0 */
    StoreArray<TRGECLWaveform> m_TRGECLWaveform; /**< output for TRGECLWaveform */
    StoreArray<TRGECLHit>      m_TRGECLHit; /**< output for TRGECLHit */
    StoreArray<TRGECLFAMAna>   m_TRGECLFAMAna; /**< output for TRGECLFAMAna */
    DBArray<TRGECLFAMPara>     m_FAMPara; /**< FAM Parameters */

    /** EventLevelClusteringInfo. */
    StoreObjPtr<EventLevelClusteringInfo> m_eventLevelClusteringInfo;

  };

} // namespace Belle2

#endif // TRGECLFAMModule_H
