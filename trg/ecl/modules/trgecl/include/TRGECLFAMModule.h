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
    /** Initilizes TRGECLFAMModule. */
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
    int _debugLevel;
    /** fam Method*/
    int _famMethod;
    /** Time interval */
    int _binTimeInterval;
    /** Waveform */
    int _waveform;
    /** save Beam background tag in TRGECLHit table */
    int _beambkgtag;
    /** save FAM ana table */
    int _famana;
    /** Threshold input*/
    int _threshold;
    /** Set Shaping Function */
    int _FADC;
    /** Use Condition DB*/
    int _ConditionDB;





    /** Config. file name. */
    std::string _configFilename;


  protected:


  private:
    /**  Run number */
    int    m_nRun;
    /**  Event number */
    int    m_nEvent;
    /** Digitized TC E [GeV] */
    std::vector<std::vector<double>> TCDigiE;
    /** Digitized TC T [ns] */
    std::vector<std::vector<double>> TCDigiT;
    /** Fit TC E [GeV] */
    std::vector<std::vector<double>> TCFitE;
    /** Fit TC T [ns] */
    std::vector<std::vector<double>> TCFitT;
    /** Threshold */
    std::vector<int> Threshold;

    StoreArray<TRGECLDigi0> m_TRGECLDigi0; /**< output for TRGECLDigi0 */
    StoreArray<TRGECLWaveform> m_TRGECLWaveform; /**< output for TRGECLWaveform */
    StoreArray<TRGECLHit> m_TRGECLHit; /**< output for TRGECLHit */
    StoreArray<TRGECLFAMAna> m_TRGECLFAMAna; /**< output for TRGECLFAMAna */
    DBArray<TRGECLFAMPara> m_FAMPara; /**< FAM Parameters */

  };

} // namespace Belle2

#endif // TRGECLFAMModule_H
