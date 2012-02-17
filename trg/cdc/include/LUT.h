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
    void doit(void);

    /// get LR componet from pattern id
    int getLRLUT(int);

  private:

    /// Name.
    const std::string _name;

    /// CDCTRG.
    const TRGCDC & _cdc;

    /// left/right lookuptable except for inner-most super layer
    int m_LRLUT[2048];

};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCLUT::name(void) const {
    return _name;
}


} // namespace Belle2

#endif /* TRGCDCLUT_FLAG_ */
