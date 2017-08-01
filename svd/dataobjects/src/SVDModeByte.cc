/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/dataobjects/SVDModeByte.h>
#include <sstream>

using namespace std;

namespace Belle2 {

  std::ostream& operator<<(std::ostream& os, SVDDAQModeType mode)
  {
    switch (mode) {
      case SVDDAQModeType::zerosupp_6samples :
        os << "0-supp 6 samples";
        break;
      case SVDDAQModeType::zerosupp_3samples :
        os << "0-supp 3 samples";
        break;
      default : os << "unknown mode";
    }
    return os;
  }

  SVDModeByte::operator string() const
  {
    stringstream out;
    if (m_id.parts.triggerBin <= MaxGoodTriggerBin) {
      out << "trg " << static_cast<int>(m_id.parts.triggerBin);
    } else {
      out << "trg *";
    }
    out << " DAQ " << m_id.parts.daqMode;
    return out.str();
  }

  std::ostream& operator<<(std::ostream& out, const SVDModeByte& id)
  {
    out << ((string)id);
    return out;
  }

}
