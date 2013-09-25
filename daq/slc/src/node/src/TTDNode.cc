#include "TTDNode.hh"

#include "FTSW.hh"

#include <util/StringUtil.hh>

#include <sstream>

using namespace B2DAQ;

void TTDNode::clearFTSWs() throw()
{
  for (size_t i = 0; i < _ftsw_v.size(); i++)
    _ftsw_v[i] = NULL;
  _ftsw_i = 0;
}

const std::string TTDNode::getSQLFields() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLFields();
  for (size_t i = 0; i < _ftsw_v.size(); i++) {
    ss << ", ftsw_id_" << i << " smallint";
  }
  return ss.str();
}

const std::string TTDNode::getSQLLabels() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLLabels();
  for (size_t i = 0; i < _ftsw_v.size(); i++) {
    ss << ", ftsw_id_" << i;
  }
  return ss.str();
}

const std::string TTDNode::getSQLValues() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLValues();
  for (size_t i = 0; i < _ftsw_v.size(); i++) {
    int id = (i < (size_t)_ftsw_i && _ftsw_v[i] != NULL) ? _ftsw_v[i]->getID() : -1;
    ss << ", " << id;
  }
  return ss.str();
}
