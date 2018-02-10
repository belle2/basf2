/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/VertexFitting/TreeFitter/ErrCode.h>
using std::ostream;

namespace TreeFitter {
  std::ostream& operator<<(std::ostream& os, const ErrCode& ec)
  {
    unsigned int flag = ec.flag() ;
    os << "flag=" << flag << " " ;
    if (flag) {
      if (flag & ErrCode::pocafailure) os << "pocafailure " ;
      if (flag & ErrCode::baddistance) os << "baddistance " ;
      if (flag & ErrCode::inversionerror) os << "inversionerror " ;
      if (flag & ErrCode::badsetup) os << "badsetup " ;
      if (flag & ErrCode::divergingconstraint) os << "divergingconstraint " ;
      if (flag & ErrCode::slowdivergingfit) os << "slowdivergingfit " ;
      if (flag & ErrCode::fastdivergingfit) os << "fastdivergingfit " ;
      if (flag & ErrCode::filtererror) os << "filtererror " ;
      if (flag & ErrCode::photondimerror) os << "photondimerror" ;
    } else {
      os << "success " ;
    }
    return os ;
  }
}
