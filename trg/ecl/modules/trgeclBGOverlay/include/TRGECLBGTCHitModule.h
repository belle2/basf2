//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : TRGECLBGTCHITModule.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------
// Description : A trigger module for TRG ECL
//---------------------------------------------------------
// 0.00 : 2021/06/xx : First version
//---------------------------------------------------------

#ifndef TRGECLBGTCHitModule_H
#define TRGECLBGTCHitModule_H

#include <string>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBArray.h>

#include "trg/ecl/TrgEclMapping.h"
#include "ecl/dataobjects/ECLHit.h"
#include "trg/ecl/dataobjects/TRGECLBGTCHit.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"

namespace Belle2 {

  class TRGECLBGTCHitModule : public Module {
  public:

    /** Constructor */
    TRGECLBGTCHitModule();

    /** Destructor */
    virtual ~TRGECLBGTCHitModule();
    /** Initilizes TRGECLBGTCHitModule. */
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

    /** returns version of TRGECLBGTCHitModule.*/
    std::string version(void) const;

  private: /** Parameters*/

    // generate simulation TCHit data objection
    void genSimulationObj();
    // generate random trigger data TCHit data objection
    void genRandomTrgObj();
    // TC energy cut (GeV) for simulation data
    double m_simTCEnergyCut;
    // Debug level
    int m_debugLevel;

  protected:

  private:
    // hardware configuration
    TrgEclMapping* m_TCMap;
    // ECL data object
    StoreArray<ECLHit> m_eclHits;
    // ecl trigger ETM dataobject
    StoreArray<TRGECLUnpackerStore> m_trgeclUnpackerStores;
    // ecl trigger BG TC dataobject
    StoreArray<TRGECLBGTCHit> m_trgeclBGTCHits;
  };

} // namespace Belle2

#endif // TRGECLBGTCHitModule_H
