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

    void calculatePhiUsingDrift(unsigned* nSuperLayers, TRGCDCTrack* aTrack, float* evtTime, double (&phi_w)[9], double (&phi)[9], int (&lutv)[9], double (&drift)[9], int (&mclutv)[9], int &ckt);

    /// initializes Look Up Table (LUT)
    void callLUT(void);

    void initialize(void);

    /// initialize for saving in ROOT.
    void initializeRoot(void);
    /// save initialization values.
    void saveInitializationValues(void);
    /// initialize for saveing in ROOT for each event.
    void initializeEventRoot(void);
    /// save track values.
    void saveTrackValues(double* phi, double* phi_w, double pt, double myphi0, double z0, double cot, int mysign, double zchi2, double* zz, int* lutv, double pt_w, double myphi0_w,  TRGCDCTrack* aTrack, int iFit, double* arcS, TVectorD* wStAxPhi, TVectorD* stAxPhi, double* drift, int* mclutv);

    void terminate(void);
   
    /// returns eventTime
    const TRGCDCEventTime * EvtTime(void) const;

  private:

    /// Name.
    const std::string _name;

    /// CDCTRG.
    const TRGCDC & _cdc;

    /// Geometry of CDC
    double m_rr[9];
    double m_anglest[4];
    double m_ztostraw[4];
    int m_ni[9];

    // Integer conversion parameters
    //For signed bits for myphi0
    int m_intnum2;
    unsigned int m_numbit2;	
    //For signed bits for rho
    int m_intnum3;
    unsigned m_numbit3;	
    //For signed bits for zz
    int m_intnum5;
    unsigned m_numbit5;	
    //For signed bits for rr
    int m_intnum6;
    unsigned m_numbit6;	
    //For signed bits for iezz2
    int m_intnum7;
    unsigned m_numbit7;	
    //For unsigned iz0den
    int m_intnum8;
    unsigned m_numbit8;	
    //For unsigned z0den
    int m_intnum9;
    unsigned m_numbit9;	
    //For unsinged z0num_p1
    int m_intnum10;
    unsigned m_numbit10;	
    //For unsigned z0num_p2
    int m_intnum11;
    unsigned m_numbit11;	

    /// LUTs
    int m_lut00[4096];

    /// LUTs
    int m_lut01[4096];

    /// LUTs
    int m_lut02[4096];

    /// LUTs
    int m_lut03[4096];

    /// LUTs
    int m_zz_0_lut[1024];

    /// LUTs
    int m_zz_1_lut[1024];

    /// LUTs
    int m_zz_2_lut[1024];

    /// LUTs
    int m_zz_3_lut[1024];

    /// LUTs
    int m_iz0den_lut[8192];

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
      // # wires ax1, st1, ...  ax5
      TVectorD* m_nWiresFitter3D;

      // Stores phi hit values
      // axphi1, stphi1, ... axphi5 - with L/R LUT
      TClonesArray* m_tSTrackFitter3D;
      // axphi1, stphi1, ... axphi5 - wire position
      TClonesArray* m_tsPhiTrackFitter3D;
      // ax1, st1, ... ax5 - drift length
      TClonesArray* m_tsDriftTrackFitter3D;
      // ax1, st1, ... ax5 - left right
      TClonesArray* m_tsLRTrackFitter3D;
      // s1, s2, s3, s4, z1, z2, z3, z4. Unit is m.
      TClonesArray* m_szTrackFitter3D;
      // MC TS values. Position on wire. Unit is cm.
      // 3*9 = 27
      // ax1_x, ax1_y, ax1_z, st1_x, st1_y, st1_z, ... ax5_x, ax5_y, ax5_z
      TClonesArray* m_mcTSTrackFitter3D;
      // axphi1, stphi1, ... axphi5 - mc left right
      TClonesArray* m_mcTSLRTrackFitter3D;

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
      // phierror1, zerror1, phierror2, zerror2, ... phierror5
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


      // Stores event and track ID;
      TClonesArray* m_eventTrackIDTrackFitter3D;

    public:
      bool m_boolMc;
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
