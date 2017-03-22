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
    TRGCDCLUT();

    /// Destructor
    virtual ~TRGCDCLUT();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// set LUT data.
    void setDataFile(const std::string& filename, int);

    /// get LUT Values
    double getValue(double) const;

    int getValue(unsigned) const;

  private:

    /// LUT data
    std::vector<int> m_data;

    /// Input bit size
    int m_bitsize;

    /// LUT name.
    std::string m_name;


  };

//-----------------------------------------------------------------------------

  inline
  std::string
  TRGCDCLUT::name(void)const
  {
    return m_name;
  }

} // namespace Belle2

#endif /* TRGCDCLUT_FLAG_ */
