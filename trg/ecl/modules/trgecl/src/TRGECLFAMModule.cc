//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGECLFAMModule.cc
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A trigger module for TRG ECL
//---------------------------------------------------------------
// 1.00 : 2012/05/24 : First version
//---------------------------------------------------------------

#define TRGECLFAM_SHORT_NAMES

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <G4RunManager.hh>

//trg package headers
#include "trg/trg/Debug.h"
#include "trg/ecl/modules/trgecl/TRGECLFAMModule.h"
#include "trg/ecl/TrgEclDigitizer.h"
#include "trg/ecl/TrgEclFAMFit.h"

#include "trg/ecl/dataobjects/TRGECLFAMAna.h"
#include "trg/ecl/dataobjects/TRGECLDigi0.h"
#include "trg/ecl/dataobjects/TRGECLHit.h"
#include "trg/ecl/dataobjects/TRGECLWaveform.h"

#include <stdlib.h>
#include <iostream>

using namespace std;

namespace Belle2 {
//
//
//
  REG_MODULE(TRGECLFAM);
//
//
//
  string
  TRGECLFAMModule::version() const
  {
    return string("TRGECLFAMModule 1.00");
  }
//
//
//
  TRGECLFAMModule::TRGECLFAMModule()
    : Module::Module(),
      _debugLevel(0),
      _famMethod(1),
      _binTimeInterval(125),
      _waveform(0),
      _beambkgtag(0),
      _famana(0),
      _threshold(100.0)

  {

    string desc = "TRGECLFAMModule(" + version() + ")";
    setDescription(desc);

    addParam("DebugLevel", _debugLevel, "TRGECL debug level", _debugLevel);
    addParam("FAMFitMethod", _famMethod, "TRGECLFAM fit method", _famMethod);
    addParam("FAMBinTimeInterval", _binTimeInterval, "TRGECLFAM binTimeInterval",
             _binTimeInterval);
    addParam("TCWaveform", _waveform, "Output the TC waveform ",
             _waveform);
    addParam("FAMAnaTable", _famana, "Save FAM ana table ",
             _famana);
    addParam("BeamBkgTag", _beambkgtag, "Save beambackground tag in TRGECLHit table ",
             _beambkgtag);
    addParam("TCThreshold", _threshold, "Set FAM TC threshold ",
             _threshold);




    if (TRGDebug::level()) {
      std::cout << "TRGECLFAMModule ... created" << std::endl;
    }
  }
//
//
//
  TRGECLFAMModule::~TRGECLFAMModule()
  {

    if (TRGDebug::level()) {
      std::cout << "TRGECLFAMModule ... destructed " << std::endl;
    }
  }
//
//
//
  void
  TRGECLFAMModule::initialize()
  {

    TRGDebug::level(_debugLevel);

    if (TRGDebug::level()) {
      std::cout << "TRGECLFAMModule::initialize ... options" << std::endl;
      std::cout << TRGDebug::tab(4) << "debug level = " << TRGDebug::level()
                << std::endl;
    }
    //
    std::cout << "TRGECLFAMModule::initialize> FAM Fit Method = "
              << _famMethod
              << std::endl;
    std::cout << "TRGECLFAMModule::initialize> FAM Bin of Time Interval = "
              << _binTimeInterval
              << std::endl;
    std::cout << "TRGECLFAMModule::initialize> FAM output TC waveforml = "
              << _waveform
              << std::endl;

    //
    m_nRun   = 0;
    m_nEvent = 1;

    StoreArray<TRGECLDigi0>::registerPersistent();
    StoreArray<TRGECLWaveform>::registerPersistent();
    StoreArray<TRGECLHit>::registerPersistent();
    StoreArray<TRGECLFAMAna>::registerPersistent();

  }
//
//
//
  void
  TRGECLFAMModule::beginRun()
  {

    if (TRGDebug::level()) {
      std::cout << "TRGECLFAMModule ... beginRun called " << std::endl;
    }

  }
//
//
//
  void
  TRGECLFAMModule::event()
  {

    if (TRGDebug::level()) {
      std::cout << "TRGECLFAMMoudle ... event called" << std::endl;
    }
    //
    //
    //
    if (m_nEvent < 1e2) {if (m_nEvent %    10 == 0) {printf("TRGECLFAMModule::event> evtno=%10i\n", m_nEvent);}}
    else if (m_nEvent < 1e3) {if (m_nEvent %   100 == 0) {printf("TRGECLFAMModule::event> evtno=%10i\n", m_nEvent);}}
    else if (m_nEvent < 1e4) {if (m_nEvent %  1000 == 0) {printf("TRGECLFAMModule::event> evtno=%10i\n", m_nEvent);}}
    else if (m_nEvent < 1e5) {if (m_nEvent % 10000 == 0) {printf("TRGECLFAMModule::event> evtno=%10i\n", m_nEvent);}}
    else if (m_nEvent < 1e6) {if (m_nEvent % 100000 == 0) {printf("TRGECLFAMModule::event> evtno=%10i\n", m_nEvent);}}


    //
    //
    //
    // FAM Digitizer
    TrgEclDigitizer* obj_trgeclDigi = new TrgEclDigitizer();
    obj_trgeclDigi-> setWaveform(_waveform);
    obj_trgeclDigi-> setup(m_nEvent);

    if (_famMethod == 2 || _famMethod == 1) {obj_trgeclDigi->  digitization01(TCDigiE, TCDigiT); } // no-fit method = backup method 1
    else if (_famMethod == 3) { obj_trgeclDigi-> digitization02(TCDigiE, TCDigiT); } // orignal method = backup method 2
    obj_trgeclDigi-> save(m_nEvent);

    // FAM Fitter
    TrgEclFAMFit* obj_trgeclfit = new TrgEclFAMFit();
    obj_trgeclfit-> SetBeamBkgTagFlag(_beambkgtag);
    obj_trgeclfit-> SetAnaTagFlag(_famana);
    obj_trgeclfit-> SetThreshold(_threshold);
    obj_trgeclfit-> setup(m_nEvent);

    if (_famMethod == 1) {obj_trgeclfit->  FAMFit01(TCDigiE, TCDigiT); } // fitting method
    else if (_famMethod == 2) {obj_trgeclfit->  FAMFit02(TCDigiE, TCDigiT); } // no-fit method = backup method 1
    else if (_famMethod == 3) { obj_trgeclfit-> FAMFit03(TCDigiE, TCDigiT); } // orignal method = backup method 2
    obj_trgeclfit-> save(m_nEvent);



    //
    //
    //
    m_nEvent++;
    delete obj_trgeclDigi;
    delete obj_trgeclfit;
    //
    //
    //
  }
//
//
//
  void
  TRGECLFAMModule::endRun()
  {
    if (TRGDebug::level()) {
      std::cout << "TRGECLFAMModule ... endRun called " << std::endl;
    }
  }
//
//
//
  void
  TRGECLFAMModule::terminate()
  {
    if (TRGDebug::level()) {
      std::cout << "TRGECLFAMModule ... terminate called " << std::endl;
    }
  }
//
//
//
} // namespace Belle2
