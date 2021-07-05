/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGCDCModule.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------
// 0.00 : 2010/10/08 : First version
//-----------------------------------------------------------------------------

#ifndef TRGCDCModule_H
#define TRGCDCModule_H

#include <string>
#include "framework/core/Module.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/SteppingAction.h"

namespace Belle2 {

/// A module to simulate the CDC trigger. This module requires CDC
/// simulation data or trigger data taken by real hardwares. The
/// output is the reconstructed track parameters.

  class TRGCDCModule : public Module {

  public:

    /// Constructor
    TRGCDCModule();
//  TRGCDCModule(const std::string & type);

    /// Destructor
    virtual ~TRGCDCModule();

    /// Initilizes TRGCDCModule.
    virtual void initialize() override;

    /// Called when new run started.
    virtual void beginRun() override;

    /// Called event by event.
    virtual void event() override;

    /// Called when run ended.
    virtual void endRun() override;

    /// Called when processing ended.
    virtual void terminate() override;

  public:

    /// returns version of TRGCDCModule.
    std::string version(void) const;

  private: // Parameters

    /// Debug level.
    int _debugLevel;

    /// Config. file name.
    std::string _configFilename;

    /// returnValue module's name. Can be ["", "TSF", "ETF", "find2D", "fit2D", "find3D", "fit3D"]. No name means all.
    std::vector<std::string> _returnValueModuleNames;

//    /// The filename of LUT for the inner-most track segments.
//    std::string _innerTSLUTDataFilename;
//
//    /// The filename of LUT for outer track segments.
//    std::string _outerTSLUTDataFilename;

    /// The filename of LUT for the inner-most track segments.(tmp)
    std::string _innerTSLUTFilename;

    /// The filename of LUT for outer track segments.(tmp)
    std::string _outerTSLUTFilename;

//    /// The filename of LUT for track segment finder SL0.
//    std::string _tsfLUTSL0DataFilename;
//
//    /// The filename of LUT for track segment finder SL1.
//    std::string _tsfLUTSL1DataFilename;
//
//    /// The filename of LUT for track segment finder SL2.
//    std::string _tsfLUTSL2DataFilename;
//
//    /// The filename of LUT for track segment finder SL3.
//    std::string _tsfLUTSL3DataFilename;
//
//    /// The filename of LUT for track segment finder SL4.
//    std::string _tsfLUTSL4DataFilename;
//
//    /// The filename of LUT for track segment finder SL5.
//    std::string _tsfLUTSL5DataFilename;
//
//    /// The filename of LUT for track segment finder SL6.
//    std::string _tsfLUTSL6DataFilename;
//
//    /// The filename of LUT for track segment finder SL7.
//    std::string _tsfLUTSL7DataFilename;
//
//    /// The filename of LUT for track segment finder SL8.
//    std::string _tsfLUTSL8DataFilename;

    /// The filename of root file for TRGCDC
    std::string _rootTRGCDCFilename;

    /// The filename of root file for Fitter3D
    std::string _rootFitter3DFilename;

    /// Curl back stop parameter. 0:do nothing, 1:stop curling. Default is 0.
    int _curlBackStop;

    /// Mode for TRGCDC simulation. 0th bit : fast simulation switch, 1st bit : firmware simulation switch.
    int _simulationMode;

    /// Switch for the fast simulation. 0:do everything, 1:stop after the track segment simulation. Default is 0.
    int _fastSimulationMode;

    /// Switch for the firmware simulation. 0:do nothing, 1:do everything
    int _firmwareSimulationMode;

    /// Start clock for the firmware simultion. (clock=cdcfe)
    int _firmwareSimulationStart;

    /// Stop clock for the firmware simultion. (clock=cdcfe)
    int _firmwareSimulationStop;

    /// Switch for TRGCDC.root file.
    bool _fileTRGCDC;

    /// Switch to activate perfect 2D finder.
    bool _perfect2DFinder;

    /// Switch to activate perfect 3D finder.
    bool _perfect3DFinder;

    /// Min. peak height for the Hough finder.
    int _hFinderPeakMin;

    /// Hough mapping file for charge plus.
    std::string _hFinderMappingFilePlus;

    /// Hough mapping file for charge minus.
    std::string _hFinderMappingFileMinus;

    /// Hough finder doit version.
    int _hFinderDoit;

    /// Switch for the Logic/LUT TSF.
    bool _fLogicLUTTSF;

    /// Switch for the LR LUT in Fitter3D.
    bool _fLRLUT;

    /// Switch for Fitter3D's MC L/R information.
    bool _fFitter3Dsmclr;

    /// Switch to us 2D fit or Hough finder results.
    bool _fFitter3Ds2DFit;

    /// Switch to us wire 2D fit or drift 2D fit.
    bool _fFitter3Ds2DFitDrift;

    /// Parameter for Hit inefficiency.
    double _wireHitInefficiency;

    /// Switch for TSF.root file.
    bool _fileTSF;

    /// Switch for ETF.root file.
    bool _fileETF;

    /// Switch for selecting ETF version.
    int _fverETF;

    /// Switch for printing Firmware inputs for ETF.
    bool _fprintFirmETF;

    /// Switch for Hough3D.root file.
    bool _fileHough3D;

    /// Sets mode of 3DFinder.
    int _finder3DMode;

    /// Switch for Fitter3D.root file.
    bool _fileFitter3D;

    /// Switch for using simple x-t curve or non-linear x-t curve. 0: non-linear 1: simple.
    bool _fXtSimpleFitter3D;

    /// Switch for TRG CDC input mode
    int _trgCDCDataInputMode;

    /** Name of the CDCHit StoreArray used as input. */
    std::string m_inputCollectionName;
    /** Name of the StoreArray holding the tracks made by the 2D finder. */
    std::string m_2DfinderCollectionName;
    /** Name of the StoreArray holding the tracks made by the 2D fitter. */
    std::string m_2DfitterCollectionName;
    /** Name of the StoreArray holding the tracks made by the 3D fitter. */
    std::string m_3DfitterCollectionName;

  private:

    /// A pointer to a TRGCDC;
    TRGCDC* _cdc;

    /// A pointer to a TRGCDCSteppingAction which stops curl backs.
    TRGCDCSteppingAction* _sa;
  };

} // namespace Belle2

#endif // TRGCDCModule_H
