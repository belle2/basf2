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

  const SVDModeByte::baseType SVDModeByte::c_DefaultID = 151;

  SVDModeByte::operator string() const
  {
    stringstream os;
    switch (m_id.parts.runType) {
      case SVDRunType::raw:
        os << "raw";
        break;
      case SVDRunType::transparent:
        os << "transpt";
        break;
      case SVDRunType::zero_suppressed:
        os << "0-suppr";
        break;
      case SVDRunType::zero_suppressed_timefit:
        os << "0-suppr+tfit";
        break;
    }
    os << "/";
    switch (m_id.parts.eventType) {
      case SVDEventType::global_run:
        os << "global";
        break;
      case SVDEventType::local_run:
        os << "local";
        break;
    }
    os << "/";
    switch (m_id.parts.daqMode) {
      case SVDDAQModeType::daq_1sample:
        os << "1 sample";
        break;
      case SVDDAQModeType::daq_3samples:
        os << "3 samples";
        break;
      case SVDDAQModeType::daq_6samples:
        os << "6 samples";
        break;
      default:
        os << "unknown";
    }
    os << "/";
    if (m_id.parts.triggerBin <= MaxGoodTriggerBin) {
      os << static_cast<int>(m_id.parts.triggerBin);
    } else {
      os << "???";
    }
    return os.str();
  }

  std::ostream& operator<<(std::ostream& out, const SVDModeByte& id)
  {
    out << ((string)id);
    return out;
  }

}
