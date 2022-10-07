/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "tracking/vxdCaTracking/ExporterHitInfo.h"
#include <boost/lexical_cast.hpp>
#include <tracking/dataobjects/FullSecID.h>

using namespace Belle2;
// using namespace Belle2::Tracking;
using boost::lexical_cast;


/** Output of position in a std::string file: it is a line with Xpos[empty space]Ypos[empty space]Zpos. */
std::string ExporterHitInfo::getPositionFormatted()
{
  std::string output = lexical_cast<std::string>(m_hitPos.X()) + " " + lexical_cast<std::string>
                       (m_hitPos.Y()) + " " + lexical_cast<std::string>
                       (m_hitPos.Z()) + "\n";
  return output;
}


std::string ExporterHitInfo::getSimpleHitFormatted()
{
  std::string output = lexical_cast<std::string>(getParticleID()) + " " + lexical_cast<std::string>(float(m_hitPos.X())) + " " +
                       lexical_cast<std::string>(float(m_hitPos.Y())) + " " + lexical_cast<std::string>(float(m_hitPos.Z())) + " " +
                       lexical_cast<std::string>(FullSecID(
                                                   m_sectorID)) + "\n";
  return output;
}


/** Output of covValues in a std::string file: it is a line with CovUU[empty space]CovUV[empty space]CovVV */
std::string ExporterHitInfo::getCovValuesFormatted()
{
  std::string output = lexical_cast<std::string>(m_covVal.X()) + " " + lexical_cast<std::string>
                       (m_covVal.Y()) + " " + lexical_cast<std::string>
                       (m_covVal.Z()) + "\n";
  return output;
}


/** Output of additional information in a std::string file: it is a line with layerID[empty space]hitID[empty space]sensorAngle */
std::string ExporterHitInfo::getAdditionalInfoFormatted()
{
  std::string output = lexical_cast<std::string>(m_layerID) + " " + lexical_cast<std::string>(m_hitID) + " " +
                       lexical_cast<std::string>
                       (m_sensorAngle) + "\n";
  return output;
}
