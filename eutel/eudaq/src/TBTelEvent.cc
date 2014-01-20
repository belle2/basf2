#include <eutel/eudaq/TBTelEvent.h>

#include <cassert>

size_t TBTelEvent::getIdFromPlane(unsigned short planeNo)
{
  // check whether this plane numbers already exists in this event
  std::map<unsigned short, size_t>::const_iterator it = m_planeToId.find(planeNo);
  if (it == m_planeToId.end()) {
    // no data for this plane number present so far -> create the
    // appropriate data structure and add the data for this pixel
    m_Digits.push_back(std::shared_ptr<std::vector<Belle2::TelDigit> >(new std::vector<Belle2::TelDigit>()));
    m_Planes->push_back(planeNo);
    m_planeToId[ planeNo ] = m_Digits.size() - 1;
    return m_Digits.size() - 1;
  }
  // already data present -> add to the existing data structure
  else {
    return it->second;
  }
}

void TBTelEvent::addPlane(unsigned short planeNo,
                          const std::vector<unsigned short>& cols,
                          const std::vector<unsigned short>& rows,
                          const std::vector<unsigned short>& signals)
{
  assert(cols.size() == rows.size() && cols.size() == signals.size());
  size_t id = getIdFromPlane(planeNo);
  for (size_t iVec = 0; iVec < cols.size(); ++iVec) {
    m_Digits.at(id)->push_back(Belle2::TelDigit(planeNo,
                                                cols.at(iVec),
                                                rows.at(iVec),
                                                signals.at(iVec)));
  }
}

void TBTelEvent::addPixel(unsigned short planeNo,
                          unsigned short col,
                          unsigned short row,
                          unsigned short signal)
{
  m_Digits.at(getIdFromPlane(planeNo))->push_back(Belle2::TelDigit(planeNo, col, row, signal));
}

std::shared_ptr<const std::vector<Belle2::TelDigit> > TBTelEvent::getPlaneDigits(unsigned short planeNo) const
{
  // check whether this plane numbers exists in this event
  // return a NULL shared_ptr if it does not
  std::map<unsigned short, size_t>::const_iterator it = m_planeToId.find(planeNo);
  if (it == m_planeToId.end()) {
    return std::shared_ptr<const std::vector<Belle2::TelDigit> >(NULL);
  } else {
    return m_Digits.at(it->second);
  }
}
