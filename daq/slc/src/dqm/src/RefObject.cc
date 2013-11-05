#include "dqm/RefObject.h"

#include <sstream>

using namespace Belle2;

const std::string RefObject::ELEMENT_TAG = "ref-object";

std::string RefObject::toXML() const throw()
{
  std::stringstream ss;
  if (_histo != NULL) {
    ss << "<" << ELEMENT_TAG << " "
       << "data-type='" << _histo->getDataType() << "' ";
    if (_histo->getName().size() > 0) ss << "name='" << _histo->getName() << "' ";
    if (_histo->getTitle().size() > 0) ss << "title='" << _histo->getTitle() << "' ";
    if (_histo->getLine() != NULL) ss << _histo->getLine()->toString();
    if (_histo->getFill() != NULL) ss << _histo->getFill()->toString();
    if (_histo->getFont() != NULL) ss << _histo->getFont()->toString();
    ss << "data='" << _histo->getData().toString() << "' ";
    ss << "/>\n";
  }
  return ss.str();
}

