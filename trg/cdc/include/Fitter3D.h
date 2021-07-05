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
// Filename : Fitter3D.h
// Section  : TRG CDC
// Owner    : KyungTae KIM (K.U.)
// Email    : ktkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to fit tracks in 3D
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCFitter3D_FLAG_
#define TRGCDCFitter3D_FLAG_

#include <string>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TVectorD.h>

#ifdef TRGCDC_SHORT_NAMES
#define TCFitter3D TRGCDCFitter3D
#endif

namespace Belle2 {

  class TRGCDC;
  class TRGCDCTrack;
  class TRGCDCSegmentHit;
  class TRGCDCJSignal;
  class TRGCDCJLUT;
  class TRGCDCJSignalData;

  /// A class to fit tracks in 3D
  class TRGCDCFitter3D {

  public:

    /// Constructor.
    TRGCDCFitter3D(const std::string& name,
                   const std::string& rootFitter3DFile,
                   const TRGCDC&,
                   const std::map<std::string, bool>& flags);
    /// Destructor.
    virtual ~TRGCDCFitter3D();

  public:

    /// Initialization.
    void initialize();
    /// Termination.
    void terminate();

    /// Does track fitting.
    int doit(std::vector<TRGCDCTrack*>& trackList);

    /// Does track fitting using JSignals.
    int doitComplex(std::vector<TRGCDCTrack*>& trackList);

    /// Utility functions.
    static double calPhi(TRGCDCSegmentHit const* segmentHit, double eventTime);
    /// Function for mc debugging.
    static void getMCValues(const TRGCDC& m_cdc_in, TRGCDCTrack* aTrack, const std::map<std::string, double>& m_mConstD_in,
                            std::map<std::string, double>& m_mDouble_in, std::map<std::string, std::vector<double> >& m_mVector_in);

    /// Checks if axial track has 5 TSs. One per each superlayer.
    bool isAxialTrackFull(const TRGCDCTrack& aTrack);

    /// Checks if stereo track has 4 TSs. One per each superlayer.
    bool isStereoTrackFull(const TRGCDCTrack& aTrack);

    /// Finds which axial superlayers has TSs. useAxSL array indicating hit superlayers.
    static void findHitAxialSuperlayers(const TRGCDCTrack& aTrack, std::vector<double>& useAxSL , bool printError);

    /// Finds which stereo superlayers has TSs. useStSL array indicating hit superlayers.
    static void findHitStereoSuperlayers(const TRGCDCTrack& aTrack, std::vector<double>& useStSL, bool printError);

    /// Removes TSs that are not possible with track Pt.
    void removeImpossibleStereoSuperlayers(std::vector<double>& useStSL);

    /// Selects priority TSs when there are multiple candidate TSs for a superlayer.
    static void selectAxialTSs(const TRGCDCTrack& aTrack, std::vector<int>& bestTSIndex);

    /// Does 2D fit. Returns 0 if fit is done successfully. m_mBool should have fIsPrintError, fmcLR, fLRLUT, fmcLR, f2DFit, f2DFitDrift. m_mConstD should have Trg_PI. m_mConstV should have nWires, driftPhi2DError, wirePhi2DError, rr, rr2D.
    static int do2DFit(TRGCDCTrack& aTrack, const std::map<std::string, bool>& m_mBool_in,
                       const std::map<std::string, double>& m_mConstD_in,
                       std::map<std::string, std::vector<double> >& m_mConstV_in, std::map<std::string, double>& m_mDouble_in,
                       std::map<std::string, std::vector<double> >& m_mVector_in);

    /// Print's information for debugging 3D.
    void print3DInformation(int iTrack);

    /// Functions for saving.
    /// Save VHDL and code files.
    void saveVhdlAndCoe();
    /// Saves all signals for debugging.
    void saveAllSignals();
    /// Saves all I/O signals for debugging.
    void saveIoSignals();

    /// Gets name of class.
    std::string name(void) const;
    /// Gets version of class.
    std::string version(void) const;

    /// Get stereo geometry
    static void getStereoGeometry(std::map<std::string, std::vector<double> >& stGeometry);

    /// Get stereo Xt
    static void getStereoXt(std::vector<double> const& stPriorityLayer, std::vector<std::vector<double> >& stXts, bool isSimple = 0);

    /// Get constants for firmwareFit
    static void getConstants(std::map<std::string, double>& mConstD, std::map<std::string, std::vector<double> >& mConstV,
                             bool isXtSimple = 0);

  private:
    /// Name.
    const std::string m_name;

    /// CDCTRG.
    const TRGCDC& m_cdc;

    /// Map to hold double values for Fitter3D.
    std::map<std::string, double> m_mDouble;
    /// Map to hold vector values for Fitter3D.
    std::map<std::string, std::vector<double> > m_mVector;
    /// Map to hold constant values for Fitter3D.
    std::map<std::string, double> m_mConstD;
    /// Map to hold constant vectcors for Fitter3D.
    std::map<std::string, std::vector<double> > m_mConstV;
    /// Map to hold input options.
    std::map<std::string, bool> m_mBool;

    /// Map to hold JSignals.
    std::map<std::string, TRGCDCJSignal> m_mSignalStorage;
    /// Map to hold JLuts.
    std::map<std::string, TRGCDCJLUT*> m_mLutStorage;
    /// For VHDL code.
    TRGCDCJSignalData* m_commonData;

    /// Members for saving.
    std::string m_rootFitter3DFileName;

    /// Tfile for Fitter3D root file.
    TFile* m_fileFitter3D;

    /// TTree for tracks of fitter3D.
    TTree* m_treeTrackFitter3D;

    /// TTree for constants of fitter3D.
    TTree* m_treeConstantsFitter3D;

    /// TVectorD map for saving values to root file.
    std::map<std::string, TVectorD*> m_mTVectorD;
    /// TClonesArray map for saving values to root file.
    std::map<std::string, TClonesArray*> m_mTClonesArray;
    /// Array of saved signals.
    std::map<std::string, std::vector<signed long long> > m_mSavedSignals;
    /// Array of I/O signals.
    std::map<std::string, std::vector<signed long long> > m_mSavedIoSignals;

  };

} /// Namespace Belle2


#endif
