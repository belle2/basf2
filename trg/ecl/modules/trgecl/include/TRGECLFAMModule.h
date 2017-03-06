//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : TRGECLFAMModule.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------
// Description : A trigger module for TRG ECL
//---------------------------------------------------------
// 0.00 : 2011/11/12 : First version
//---------------------------------------------------------

#ifndef TRGECLFAMModule_H
#define TRGECLFAMModule_H

#include <string>
#include <framework/core/Module.h>

namespace Belle2 {

  /*! A module of FAM */
  class TRGECLFAMModule : public Module {


  public:

    //    virtual ModulePtr newModule(){
    //      ModulePtr nm(new TRGECLModule(false)); return nm;
    //    };

    /** Constructor */
    TRGECLFAMModule();

    /** Destructor */
    virtual ~TRGECLFAMModule();
    /** Initilizes TRGECLFAMModule. */
    virtual void initialize();
    /** Called when new run started.*/
    virtual void beginRun();
    /** Called event by event. */
    virtual void event();
    /** Called when run ended. */
    virtual void endRun();
    /** Called when processing ended.*/
    virtual void terminate();

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
    /** Set Threshold */
    int _threshold;





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




  };

} // namespace Belle2

#endif // TRGECLFAMModule_H
