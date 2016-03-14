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
  class TRGCDCEventTime;
  class TRGCDCSegmentHit;
  class TRGCDCJSignal;
  class TRGCDCJLUT;
  class TRGCDCJSignalData;

  /// A class to fit tracks in 3D
  class TRGCDCFitter3D {

    public:

      /// Constructor.
      TRGCDCFitter3D(const std::string & name,
                   const std::string & rootFitter3DFile,
                   const TRGCDC &,
                   const std::map<std::string, bool> & flags);
      /// Destructor.
      virtual ~TRGCDCFitter3D();

    public:

    /// Initialization.
    void initialize();
    /// Termination.
    void terminate();

    /// Does track fitting.
    int doit(const std::vector<TRGCDCTrack *> & trackListIn,
             std::vector<TRGCDCTrack *> & trackListOut);

    /// Does track fitting using JSignals.
    int doitComplex(const std::vector<TRGCDCTrack *> & trackListIn,
             std::vector<TRGCDCTrack *> & trackListOut);
    /// Utility functions.
    static double calPhi(TRGCDCSegmentHit const * segmentHit, double eventTime);
    /// Function for mc debugging.
    void getMCValues( TRGCDCTrack* aTrack );

    /// Functions for saving.
    // Save VHDL and code files.
    void saveVhdlAndCoe();
    // Saves all signals for debugging.
    void saveAllSignals();
    // Saves all I/O signals for debugging.
    void saveIoSignals();

    // Gets name of class.
    std::string name(void) const;
    // Gets version of class.
    std::string version(void) const;
    
    private:
      /// Name.
      const std::string m_name;

      /// CDCTRG.
      const TRGCDC & m_cdc;

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

      // Map to hold JSignals.
      std::map<std::string, TRGCDCJSignal> m_mSignalStorage;
      // Map to hold JLuts.
      std::map<std::string, TRGCDCJLUT*> m_mLutStorage;
      // For VHDL code.
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

} // Namespace Belle2


#endif
