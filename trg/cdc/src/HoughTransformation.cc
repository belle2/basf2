//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughTransformation.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : An abstract class to represent a Hough transformation.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDCHoughTransformation_INLINE_DEFINE_HERE

#include "trg/trg/Point2D.h"
#include "trg/cdc/HoughTransformation.h"

namespace Belle2 {

TRGCDCHoughTransformation::TRGCDCHoughTransformation(const std::string & name)
    : _name(name) {


}

TRGCDCHoughTransformation::~TRGCDCHoughTransformation() {
}

TRGPoint2D
TRGCDCHoughTransformation::convert(const TRGPoint2D & p) const {
    return p;
}

} // namespace Belle

