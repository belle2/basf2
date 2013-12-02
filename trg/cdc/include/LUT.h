//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : LUT.h
// Section  : TRG CDC
// Owner    : KyungTae KIM (K.U.)
// Email    : ktkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to use LUTs for TRGCDC
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCLUT_FLAG_
#define TRGCDCLUT_FLAG_

#include <string>
#include <vector>

#ifdef TRGCDC_SHORT_NAMES
#define TCLUT TRGCDCLUT
#endif

namespace Belle2 {

class TRGCDC;
class TRGCDCTrack;

/// A class to use LUTs for TRGCDC
class TRGCDCLUT {

  public:

    /// Contructor.
    TRGCDCLUT(const std::string & name, const TRGCDC &);

    /// Destructor
    virtual ~TRGCDCLUT();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// initialize LookUpTable(LUT)
    void initialize(const std::string & filename);

    /// get LR componet from pattern id. Return value 0:right(decrease phi), 1:left(increase phi), 2:not determined
    int getLRLUT(int,int) const;

    /// get Hit LR component from pattern id.
    /// (00)(00)(0): Left LR, Right LR, Hit/NotHit
    /// 00: Not determined. 10: Right, 11: Left
    std::string getHitLRLUT(int, int) const;

  private:

    /// Name.
    const std::string _name;

    /// CDCTRG.
    const TRGCDC & _cdc;

    /// left/right lookuptable except for inner-most super layer
    int m_LRLUT[2048];

    /// left/right lookuptable for inner-most super layer
    int m_LRLUTIN[32768];

    /// hit/left/right loopuptabels for each super layer
    std::string m_HitLRLUTSL0[32768];
    std::string m_HitLRLUTSL1[2048];
    std::string m_HitLRLUTSL2[2048];
    std::string m_HitLRLUTSL3[2048];
    std::string m_HitLRLUTSL4[2048];
    std::string m_HitLRLUTSL5[2048];
    std::string m_HitLRLUTSL6[2048];
    std::string m_HitLRLUTSL7[2048];
    std::string m_HitLRLUTSL8[2048];

};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCLUT::name(void) const {
    return _name;
}


} // namespace Belle2

#endif /* TRGCDCLUT_FLAG_ */
