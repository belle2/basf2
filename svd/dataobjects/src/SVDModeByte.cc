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

  std::ostream& operator<<(std::ostream& os, SVDRunType mode)
  {
    switch (mode) {
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
    return os;
  }

  std::ostream& operator<<(std::ostream& os, SVDEventType evt)
  {
    switch (evt) {
      case SVDEventType::global_run:
        os << "global";
        break;
      case SVDEventType::local_run:
        os << "local";
        break;
    }
    return os;
  }

  std::ostream& operator<<(std::ostream& os, SVDDAQModeType mode)
  {
    switch (mode) {
      case SVDDAQModeType::daq_1sample:
        os << "1 sample";
        break;
      case SVDDAQModeType::daq_3samples:
        os << "3 samples";
        break;
      case SVDDAQModeType::daq_6samples:
        os << "6 samples";
        break;
      case SVDDAQModeType::daq_unknown:
        os << "unknown";
        break;
    }
    return os;
  }

  SVDModeByte::operator string() const
  {
    stringstream out;
    out << m_id.parts.runType;
    out << "/" << m_id.parts.eventType;
    out << "/" << m_id.parts.daqMode;
    if (m_id.parts.triggerBin <= MaxGoodTriggerBin) {
      out << "/" << static_cast<int>(m_id.parts.triggerBin);
    } else {
      out << "/???";
    }
    return out.str();
  }

  std::ostream& operator<<(std::ostream& out, const SVDModeByte& id)
  {
    out << ((string)id);
    return out;
  }

}
