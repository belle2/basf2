//-----------------------------------------------------------------------------
// $Id: THoughTransformation.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : THoughTransformation.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : An abstract class to represent a Hough transformation.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.3  2005/03/11 03:58:34  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.2  2004/04/23 09:48:24  yiwasaki
// Trasan 3.12 : curlVersion=2 is default
//
// Revision 1.1  2004/02/18 04:07:46  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------

#ifndef THoughTransformation_FLAG_
#define THoughTransformation_FLAG_


#include <string>

namespace Belle {

  class TPoint2D;

/// An abstract class to represent a Hough transformation.
  class THoughTransformation {

  public:
    /// Contructor.
    THoughTransformation(const std::string& name);

    /// Destructor
    virtual ~THoughTransformation();

  public:// Selectors
    /// returns name.
    std::string name(void) const;

    /// returns Y coordinate in a Hough parameter plane.
    virtual float y(float xReal, float yReal, float x) const = 0;

    /// returns true if Y diverges in given region.
    virtual bool diverge(float xReal, float yReal, float x0, float x1)
    const = 0;

    /// returns true if Y diverges in given region.
    virtual bool positiveDiverge(float xReal, float yReal, float x0, float x1)
    const = 0;

    /// returns true if Y diverges in given region.
    virtual bool negativeDiverge(float xReal, float yReal, float x0, float x1)
    const = 0;

//     /// returns a gradient at given regsion.
//     virtual float gradient(float xReal, float yReal, float x) const = 0;

    /// converts Point2D(r, phi) in real plane into Point2D(r, phi) in Hough plane.
    virtual TPoint2D convert(const TPoint2D&) const;

  public:// Modifiers

  private:
    const std::string _name;
  };

  inline
  std::string
  THoughTransformation::name(void) const
  {
    return _name;
  }

} // namespace Belle

#endif
