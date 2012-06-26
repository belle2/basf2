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

#ifdef TRGCDC_SHORT_NAMES
#define TCFitter3D TRGCDCFitter3D
#endif

namespace Belle2 {

class TRGCDC;
class TRGCDCTrack;

/// A class to fit tracks in 3D
class TRGCDCFitter3D {

  public:

    /// Contructor.
    TRGCDCFitter3D(const std::string & name,
                   const TRGCDC &);

    /// Destructor
    virtual ~TRGCDCFitter3D();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// do track fitting.
    int doit(const std::vector<TRGCDCTrack *> & trackListIn,
             std::vector<TRGCDCTrack *> & trackListOut);

    /// initializes Look Up Table (LUT)
    void callLUT(void);

    void initialize(void);

    void terminate(void);

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

    protected:
      TFile* m_fileFitter3D;
      TTree* m_treeFitter3D;
      TClonesArray* m_tSTrackFitter3D;
      TClonesArray* m_fitTrackFitter3D;

    public:
      bool m_flagRealInt;
      bool m_flagWireLRLUT;
      bool m_flagNonTSStudy;
};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCFitter3D::name(void) const {
    return _name;
}

} // namespace Belle2

#endif
