/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/FullSecID.h"
#include <framework/logging/Logger.h>
#include <limits> // needed for numeric_limits<t>::max()
#include <assert.h> // testing purposes
#include <iostream> // cerr, testing purposes
#include <boost/format.hpp> // needed for xml-data-compatibility
#include <sstream> // stringstream, needed for xml-data-compatibility
#include <vector> // needed for xml-data-compatibility
#include <boost/algorithm/string.hpp> // needed for xml-data-compatibility


using namespace std;
using namespace Belle2;

const int FullSecID::LayerBits = 4;
const int FullSecID::SubLayerBits = 1;
const int FullSecID::VxdIDBits = 16;
const int FullSecID::SectorBits = 11;
const int FullSecID::Bits = LayerBits + SubLayerBits + VxdIDBits + SectorBits;
const int FullSecID::MaxLayer = (1 << LayerBits) - 1;
const int FullSecID::MaxSubLayer = (1 << SubLayerBits) - 1;
const int FullSecID::MaxVxdID = (1 << VxdIDBits) - 1;
const int FullSecID::MaxSector = (1 << SectorBits) - 1;
const int FullSecID::MaxID = std::numeric_limits<unsigned int>::max();
const int FullSecID::LayerBitShift   = SubLayerBits + VxdIDBits + SectorBits;
const int FullSecID::SubLayerBitShift   = VxdIDBits + SectorBits;
const int FullSecID::VxdIDBitShift  = SectorBits;
const int FullSecID::SubLayerMask = MaxSubLayer << SubLayerBitShift;
const int FullSecID::VxdIDMask = MaxVxdID << VxdIDBitShift;
const int FullSecID::SectorMask = MaxSector;



FullSecID::FullSecID(std::string sid)
{
  vector<string> stringSegments;
  boost::split(stringSegments, sid, boost::is_any_of("_"));

  unsigned int LayerID = stringSegments[0][0] - '0'; // since chars are (compared to strings) very problematic to convert to ints, this solution is very dirty but at least it's short and easy to read.
  unsigned int SubLayerID = stringSegments[0][1] - '0';
  unsigned int UniID =  atoi(stringSegments[1].c_str());   // C++ 11: std::stoi( stringSegments[1] )
  unsigned int sectorNumber = atoi(stringSegments[2].c_str());
  B2DEBUG(1000, "FullSecID-constructor: Value before converting: " << sid << ", after converting: layerID " << LayerID << ", subLayerID " << SubLayerID << ", UniID " << UniID << ", secID " << sectorNumber);
  assert(LayerID < MaxLayer + 1);
  assert(SubLayerID < MaxSubLayer + 1);
  assert(UniID < MaxVxdID + 1);
  assert(sectorNumber < MaxSector + 1);

  LayerID <<= LayerBitShift;
  SubLayerID <<= SubLayerBitShift;
  UniID <<= VxdIDBitShift;
  m_fullSecID = LayerID | SubLayerID | UniID | sectorNumber;
}


FullSecID::FullSecID(VxdID vxdID, bool subLayerID, unsigned int sectorNumber):
  m_fullSecID(0)   // setting to 0 to be shure that value is 0 when using it within the brackets
{
  unsigned int LayerID = vxdID.getLayerNumber();
  unsigned int SubLayerID = subLayerID; // converting to int
  unsigned int UniID = vxdID;

  B2DEBUG(1000, "FullSecID-constructor: LayerID " << LayerID << ", MaxLayer " << MaxLayer << ", SubLayerID " << SubLayerID << ", MaxSubLayer " << MaxSubLayer << ", UniID " << UniID << ", MaxVxdID " << MaxVxdID << ", sectorNumber " << sectorNumber << ", MaxSector " << MaxSector);
  assert(LayerID < MaxLayer + 1);
  assert(SubLayerID < MaxSubLayer + 1);
  assert(UniID < MaxVxdID + 1);
  assert(sectorNumber < MaxSector + 1);

  LayerID <<= LayerBitShift;
  SubLayerID <<= SubLayerBitShift;
  UniID <<= VxdIDBitShift;
  m_fullSecID = LayerID | SubLayerID | UniID | sectorNumber; // should be the same as below
//  m_fullSecID = LayerID + SubLayerID + UniID + sectorNumber; // should be the same as above
}


std::string FullSecID::getFullSecString()
{
//  stringstream aSecIDString;
//  aSecIDString << getLayerID() << getSubLayerID() << "_" << getUniID() << "_" << getSecID();
//  return aSecIDString;
//  cerr << "getFullSecString: " << (boost::format("%1%0_%2%_%3%") % aLayerID % aUniID % aSecID).str() << endl;
  return (boost::format("%1%%2%_%3%_%4%") % getLayerID() % getSubLayerID() % getUniID() % getSecID()).str();
}
