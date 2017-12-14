#include <eutel/eudaq/TBTelEvent.h>

#include <cassert>

size_t TBTelEvent::getIdFromPlane(unsigned short planeNo, DigitType type)
{
  // check whether this plane numbers already exists in this event
  std::map<unsigned short, size_t>::const_iterator it;
  if (type == DigitType::Telescope) {
    it = m_telPlaneToId.find(planeNo);
    // already data present -> add to the existing data structure
    if (it != m_telPlaneToId.end()) {
      return it->second;
    }
  } else if (type == DigitType::PXD) {
    it = m_pxdPlaneToId.find(planeNo);
    // already data present -> add to the existing data structure
    if (it != m_pxdPlaneToId.end()) {
      return it->second;
    }
  }

  // no data for this telescope/pxd plane number present so far -> create the
  // appropriate data structure and add the data for this pixel
  size_t id = 0;
  if (type == DigitType::Telescope) {
    m_TelDigits.push_back(std::shared_ptr<std::vector<Belle2::TelDigit> >(new std::vector<Belle2::TelDigit>()));
    m_TelPlanes->push_back(planeNo);
    m_telPlaneToId[ planeNo ] = m_TelDigits.size() - 1;
    id = m_TelDigits.size() - 1;
  } else if (type == DigitType::PXD) {
    m_PXDDigits.push_back(std::shared_ptr<std::vector<Belle2::PXDDigit> >(new std::vector<Belle2::PXDDigit>()));
    m_PXDPlanes->push_back(planeNo);
    m_pxdPlaneToId[ planeNo ] = m_PXDDigits.size() - 1;
    id = m_PXDDigits.size() - 1;
  }
  return id;
}

void TBTelEvent::addTelPlane(unsigned short planeNo,
                             const std::vector<unsigned short>& cols,
                             const std::vector<unsigned short>& rows,
                             const std::vector<unsigned short>& signals)
{
  assert(cols.size() == rows.size() && cols.size() == signals.size());
  size_t id = getIdFromPlane(planeNo, DigitType::Telescope);
  for (size_t iVec = 0; iVec < cols.size(); ++iVec) {
    m_TelDigits.at(id)->push_back(Belle2::TelDigit(planeNo,
                                                   // uCellID -> colum id
                                                   cols.at(iVec),
                                                   // vCellID -> row id
                                                   rows.at(iVec),
                                                   // charge value
                                                   signals.at(iVec)));
  }
}

void TBTelEvent::addTelPixel(unsigned short planeNo,
                             unsigned short col,
                             unsigned short row,
                             unsigned short signal)
{
  m_TelDigits.at(getIdFromPlane(planeNo, DigitType::Telescope))->push_back(Belle2::TelDigit(planeNo,
      // uCellID -> colum id
      col,
      // vCellID -> row id
      row,
      // charge value
      signal));
}

std::shared_ptr<const std::vector<Belle2::TelDigit> > TBTelEvent::getTelPlaneDigits(unsigned short planeNo) const
{
  // check whether this plane numbers exists in this event
  // return a NULL shared_ptr if it does not
  std::map<unsigned short, size_t>::const_iterator it = m_telPlaneToId.find(planeNo);
  if (it == m_telPlaneToId.end()) {
    return std::shared_ptr<const std::vector<Belle2::TelDigit> >(NULL);
  } else {
    return m_TelDigits.at(it->second);
  }
}

void TBTelEvent::addPXDPlane(unsigned short planeNo,
                             const std::vector<unsigned short>& cols,
                             const std::vector<unsigned short>& rows,
                             const std::vector<unsigned short>& signals)
{
  assert(cols.size() == rows.size() && cols.size() == signals.size());
  size_t id = getIdFromPlane(planeNo, DigitType::PXD);
  for (size_t iVec = 0; iVec < cols.size(); ++iVec) {
    m_PXDDigits.at(id)->push_back(Belle2::PXDDigit(planeNo,
                                                   // uCellID -> colum id
                                                   cols.at(iVec),
                                                   // vCellID -> row id
                                                   rows.at(iVec),
                                                   // charge value
                                                   signals.at(iVec)));
  }
}

void TBTelEvent::addPXDPixel(unsigned short planeNo,
                             unsigned short col,
                             unsigned short row,
                             unsigned short signal)
{
  m_PXDDigits.at(getIdFromPlane(planeNo, DigitType::PXD))->push_back(Belle2::PXDDigit(planeNo,
      // uCellID -> colum id
      col,
      // vCellID -> row id
      row,
      // charge value
      signal));
}

std::shared_ptr<const std::vector<Belle2::PXDDigit> > TBTelEvent::getPXDPlaneDigits(unsigned short planeNo) const
{
  // check whether this plane numbers exists in this event
  // return a NULL shared_ptr if it does not
  std::map<unsigned short, size_t>::const_iterator it = m_pxdPlaneToId.find(planeNo);
  if (it == m_pxdPlaneToId.end()) {
    return std::shared_ptr<const std::vector<Belle2::PXDDigit> >(NULL);
  } else {
    return m_PXDDigits.at(it->second);
  }
}
