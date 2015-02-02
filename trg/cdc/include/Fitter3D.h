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

    int doitComplex(const std::vector<TRGCDCTrack *> & trackListIn,
             std::vector<TRGCDCTrack *> & trackListOut);
    /// Utility functions.
    static double calPhi(TRGCDCSegmentHit const * segmentHit, double eventTime);
    /// Function for mc debugging.
    void getMCValues( TRGCDCTrack* aTrack );

    /// Functions for saving.
    void saveVhdlAndCoe();
    void saveAllSignals();
    void saveIoSignals();


    std::string name(void) const;
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
      /// Map to hold const values for Fitter3D.
      std::map<std::string, double> m_mConstants;
      std::map<std::string, double> m_mConstD;
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
      TFile* m_fileFitter3D;
      TTree* m_treeTrackFitter3D;
      TTree* m_treeConstantsFitter3D;
      std::map<std::string, TVectorD*> m_mTVectorD;
      std::map<std::string, TClonesArray*> m_mTClonesArray;
      std::map<std::string, std::vector<signed long long> > m_mSavedSignals;
      std::map<std::string, std::vector<signed long long> > m_mSavedIoSignals;

  };

} // Namespace Belle2


#endif
