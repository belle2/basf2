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

/// A class to fit tracks in 3D
class TRGCDCFitter3D {

  public:

    /// Contructor.
    TRGCDCFitter3D(const std::string & name,
                   const std::string & rootFitter3DFile,
                   const TRGCDC &,
		   const TRGCDCEventTime * eventTime,
                   const bool fLRLUT,
		   const bool fevtTime,
		   const bool fzierror,
		   const bool fmclr,
       bool makeRootFile);

    /// Destructor
    virtual ~TRGCDCFitter3D();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// do track fitting.
    void doit(const std::vector<TRGCDCTrack *> & trackListIn,
             std::vector<TRGCDCTrack *> & trackListOut, int eventNum);
    int doit(const std::vector<TRGCDCTrack *> & trackListIn,
             std::vector<TRGCDCTrack *> & trackListOut);

    /// initializes Look Up Table (LUT)
    void callLUT(void);

    void initialize(void);

    void terminate(void);
   
    /// returns eventTime
    const TRGCDCEventTime * EvtTime(void) const;

  private:

    /// Name.
    const std::string _name;

    /// CDCTRG.
    const TRGCDC & _cdc;

    /// LUTs
    int lut00[4096];

    /// LUTs
    int lut01[4096];

    /// LUTs
    int lut02[4096];

    /// LUTs
    int lut03[4096];

    /// LUTs
    int zz_0_lut[1024];

    /// LUTs
    int zz_1_lut[1024];

    /// LUTs
    int zz_2_lut[1024];

    /// LUTs
    int zz_3_lut[1024];

    // Constants
    double m_Trg_PI;

    // Input Error
    double m_phierror[5];
    double m_zerror[4];
    double m_zerror1[4];
    double m_zerror2[4];

    // EventTime class
    const TRGCDCEventTime * const _eventTime;

    // Event number
    int m_eventNum;

    protected:

      // The filename of root file for Fitter3D
      std::string m_rootFitter3DFilename;

      TFile* m_fileFitter3D;
      TTree* m_treeTrackFitter3D;
      TTree* m_treeConstantsFitter3D;

      // Stores geometry
      // r1, r2, r3, r4, r5, r6, r7, r8, r9, anglest1, anglest2, anglest3, anglest4, ztostraw1, ztostraw2, ztostraw3, ztostraw4
      TVectorD* m_geometryFitter3D;
      // # wires ax1, ax2, ax3, ax4, ax5, st1, st2, st3, st4, st5
      TVectorD* m_nWiresFitter3D;

      // Stores phi hit values
      // axphi1, axphi2, axphi3, axphi4, axphi5, stphi1, stphi2, stphi3, stphi4
      TClonesArray* m_tSTrackFitter3D;
      // MC TS values
      // 3*5 + 3*4 = 27
      // ax1_x, ax1_y, ax1_z, ax2_x, ax2_y, ax2_z, ax3_x, ax3_y, ax3_z, ax4_x, ax4_y, ax4_z, ax5_x, ax5_y, ax5_z, 
      // st1_x, st1_y, st1_z, st2_x, st2_y, st2_z, st3_x, st3_y, st3_z, st4_x, st4_y, st4_z
      TClonesArray* m_mcTSTrackFitter3D;
      // Stores s and z hit values
      // s1, s2, s3, s4, z1, z2, z3, z4
      TClonesArray* m_szTrackFitter3D;

      // Stores MC values
      // MCpT, MCphi0, MCz0, MCtheta, MCcharge
      TClonesArray* m_mcTrackFitter3D;
      // statusbit, pdg, charge
      TClonesArray* m_mcStatusTrackFitter3D;
      TClonesArray* m_mcVertexTrackFitter3D;
      TClonesArray* m_mc4VectorTrackFitter3D;
      TClonesArray* m_mcHelixCenterTrackFitter3D;
      TClonesArray* m_mcImpactPositionTrackFitter3D;

      // Stores error values
      // phierror1, phierror2, phierror3, phierror4, phierror5, zerror1, zerror2, zerror3, zerror4
      TVectorD* m_errorFitter3D;
      // Stores fit values
      // pT, phi0, z0, theta, charge, zhi2
      TClonesArray* m_fitTrackFitter3D;
      // wire (pT, phi0), charge
      TClonesArray* m_wFit2DTrackFitter3D;
      // 2D fit values from 2D fitter (pT, phi0, r0)
      TClonesArray* m_fit2DTrackFitter3D;
      // Stores stereo's axial phi with wire fit
      TClonesArray* m_wStAxPhiTrackFitter3D;
      // Stores stereo's axial phi with drift fit
      TClonesArray* m_stAxPhiTrackFitter3D;


      // Stores Stereo phi candidates upto 25 * 4 layers.
      TClonesArray* m_stTSsTrackFitter3D;
      // Stores event and track ID;
      TClonesArray* m_eventTrackIDTrackFitter3D;

    public:
      bool m_flagRealInt;
      bool m_flagWireLRLUT;
      bool m_flagNonTSStudy;
      bool m_flagEvtTime;
      bool m_flagzierror;
      bool m_flagmclr;
      bool m_makeRootFile;
};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCFitter3D::name(void) const {
    return _name;
}

inline
const TRGCDCEventTime *
TRGCDCFitter3D::EvtTime(void) const {
    return _eventTime;
}

} // namespace Belle2

#endif
