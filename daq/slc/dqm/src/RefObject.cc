#include "daq/slc/dqm/RefObject.h"

#include <sstream>

using namespace Belle2;

const std::string RefObject::ELEMENT_TAG = "ref-object";

std::string RefObject::toXML() const throw()
{
  std::stringstream ss;
  if (m_histo != NULL) {
    ss << "<" << ELEMENT_TAG << " "
       << "data-type='" << m_histo->getDataType() << "' ";
    if (m_histo->getName().size() > 0) ss << "name='" << m_histo->getName() << "' ";
    if (m_histo->getTitle().size() > 0) ss << "title='" << m_histo->getTitle() << "' ";
    if (m_histo->getLine() != NULL) ss << m_histo->getLine()->toString();
    if (m_histo->getFill() != NULL) ss << m_histo->getFill()->toString();
    if (m_histo->getFont() != NULL) ss << m_histo->getFont()->toString();
    ss << "data='" << m_histo->getData().toString() << "' ";
    ss << "/>\n";
  }
  return ss.str();
}

