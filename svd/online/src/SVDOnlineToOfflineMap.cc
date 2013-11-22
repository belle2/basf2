/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "svd/online/SVDOnlineToOfflineMap.h"
#include "framework/logging/Logger.h"
#include <boost/foreach.hpp>
#include <iostream>

using namespace Belle2;
using namespace std;
using boost::property_tree::ptree;

SVDOnlineToOfflineMap::SVDOnlineToOfflineMap(const string& xml_filename)
{

  // Create an empty property tree object

  ptree propertyTree;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  read_xml(xml_filename, propertyTree);

  // traverse pt: let us navigate through the daughters of <SVD>
  BOOST_FOREACH(ptree::value_type const & v, propertyTree.get_child("SVD")) {

    // if the daughter is a <layer> then read it!
    if (v.first == "layer")
      ReadLayer(v.second.get<int>("<xmlattr>.n"), v.second);
  }
}

void
SVDOnlineToOfflineMap::ReadLayer(int nlayer, ptree const& xml_layer)
{

  // traverse xml_layer: let us navigate through the daughters of <layer>

  BOOST_FOREACH(ptree::value_type const & v, xml_layer) {
    // if the daughter is a <ladder> then read it!
    if (v.first == "ladder")
      ReadLadder(nlayer, v.second.get<int>("<xmlattr>.n") , v.second);
  }
}

void
SVDOnlineToOfflineMap::ReadLadder(int nlayer, int nladder, ptree const& xml_ladder)
{

  // traverse xml_ladder: let us navigate through the daughters of <ladder>

  BOOST_FOREACH(ptree::value_type const & v, xml_ladder) {
    // if the daughter is a <sensor> then read it!
    if (v.first == "sensor")
      ReadSensor(nlayer, nladder, v.second.get<int>("<xmlattr>.n") , v.second);
  }
}

void
SVDOnlineToOfflineMap::ReadSensor(int nlayer, int nladder, int nsensor, ptree const& xml_sensor)
{

  // traverse xml_sensor: let us navigate through the daughters of <sensor>

  BOOST_FOREACH(ptree::value_type const & v, xml_sensor) {
    // if the daughter is one side <> then read it!
    if (v.first == "side") {
      std::string tagSide = v.second.get<std::string>("<xmlattr>.side");

      bool isOnSideU = (tagSide == "U" || tagSide == "u");
      bool isOnSideV = (tagSide == "V" || tagSide == "v");

      if ((! isOnSideU) && (! isOnSideV)) {
        std::stringstream errMsg;
        errMsg << "Side '" << tagSide << "' on" << endl
               << "layer:  " << nlayer << endl
               << "ladder: " << nladder << endl
               << "sensor: " << nsensor << endl
               << "is neither 'U' nor 'V'" << endl;
        throw errMsg.str().c_str();
      }

      ReadSensorSide(nlayer, nladder, nsensor, isOnSideU, v.second);
    }
  }

}

void
SVDOnlineToOfflineMap::ReadSensorSide(int nlayer, int nladder, int nsensor, bool isU,
                                      ptree const& xml_side)
{

  // traverse xml_sensor: let us navigate through the daughters of <side>

  BOOST_FOREACH(ptree::value_type const & v, xml_side) {
    // if the daughter is a <chip>
    if (v.first == "chip") {
      auto tags = v.second;
      unsigned char  chipN = tags.get<unsigned char>("<xmlattr>.n");
      unsigned char  FADCn = tags.get<unsigned char>("<xmlattr>.FADCn");
      unsigned short stripNumberCh0  = tags.get<unsigned short>("<xmlattr>.strip_number_of_ch0");
      unsigned short stripNumberCh127 = tags.get<unsigned short>("<xmlattr>.strip_number_of_ch127");

      addChip(chipN, FADCn, nlayer, nladder, nsensor, isU, stripNumberCh0,
              stripNumberCh127 > stripNumberCh0);
    }

  }

}

void
SVDOnlineToOfflineMap::addFADC(unsigned char FADCn)
{
  if (FADCn < m_chipIsInTheDAQ.size()) {
    B2WARNING("Adding FADC #" <<  FADCn << " to the SVD On-line to Off-line map "
              << "whose present lenght is: " << m_chipIsInTheDAQ.size());
    return;
  }

  while (FADCn >= m_chipIsInTheDAQ.size()) {
    m_chipIsInTheDAQ.push_back(0);
    vector<VxdID> an_empty_vector_ofVxdID;
    m_VxdID.push_back(an_empty_vector_ofVxdID);
    m_isOnUside.push_back(0);
    vector<unsigned short> an_empty_vector_of_ushort;
    m_channel0Strip.push_back(an_empty_vector_of_ushort);
    m_parallel.push_back(0);

  }
}

void
SVDOnlineToOfflineMap::addChip(unsigned char  chipN,
                               unsigned char  FADCn,
                               int nlayer, int nladder, int nsensor, bool isU,
                               unsigned short stripNumberCh0,
                               bool           isParallel)
{

  if (FADCn >= m_chipIsInTheDAQ.size())
    addFADC(FADCn);

  while (chipN >= m_VxdID[FADCn].size()) {
    m_VxdID[FADCn].push_back(VxdID(0));
    m_channel0Strip[FADCn].push_back(0u);
  }

  if (m_chipIsInTheDAQ[FADCn] & (1 << chipN)) {
    B2WARNING("APV25 #" <<  chipN << " FADC #" << FADCn
              << "already present in the On-line to Off-line map "
              << "no action taken"
             );
    return;
  }

  m_VxdID[FADCn][chipN]         = VxdID(nlayer, nladder, nsensor);
  m_isOnUside[FADCn]           |= (isU ? 1 : 0) << chipN;
  m_channel0Strip[FADCn][chipN] = stripNumberCh0;
  m_parallel[FADCn]            |= (isParallel ? 1 : 0) << chipN;
  m_chipIsInTheDAQ[FADCn]      |= 1 << chipN;
}
