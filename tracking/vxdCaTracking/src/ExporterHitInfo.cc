/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/ExporterHitInfo.h"
#include <boost/lexical_cast.hpp>
#include <tracking/dataobjects/FullSecID.h>

using namespace std;
using namespace Belle2;
// using namespace Belle2::Tracking;
using boost::lexical_cast;


/** Output of position in a string file: it is a line with Xpos[empty space]Ypos[empty space]Zpos. */
std::string ExporterHitInfo::getPositionFormatted()
{
  string output = lexical_cast<string>(m_hitPos.X()) + " " + lexical_cast<string>(m_hitPos.Y()) + " " + lexical_cast<string>
                  (m_hitPos.Z()) + "\n";
  return output;
}


std::string ExporterHitInfo::getSimpleHitFormatted()
{
  string output = lexical_cast<string>(getParticleID()) + " " + lexical_cast<string>(float(m_hitPos.X())) + " " +
                  lexical_cast<string>(float(m_hitPos.Y())) + " " + lexical_cast<string>(float(m_hitPos.Z())) + " " + lexical_cast<string>(FullSecID(
                        m_sectorID)) + "\n";
  return output;
}


/** Output of covValues in a string file: it is a line with CovUU[empty space]CovUV[empty space]CovVV */
std::string ExporterHitInfo::getCovValuesFormatted()
{
  string output = lexical_cast<string>(m_covVal.X()) + " " + lexical_cast<string>(m_covVal.Y()) + " " + lexical_cast<string>
                  (m_covVal.Z()) + "\n";
  return output;
}


/** Output of additional information in a string file: it is a line with layerID[empty space]hitID[empty space]sensorAngle */
std::string ExporterHitInfo::getAdditionalInfoFormatted()
{
  string output = lexical_cast<string>(m_layerID) + " " + lexical_cast<string>(m_hitID) + " " + lexical_cast<string>
                  (m_sensorAngle) + "\n";
  return output;
}