/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <b2bii/dataobjects/BelleTrkExtra.h>

using namespace Belle2;

void BelleTrkExtra::setTrackFirstX(double first_x)
{
  m_firstx = first_x;
}

void BelleTrkExtra::setTrackFirstY(double first_y)
{
  m_firsty = first_y;
}

void BelleTrkExtra::setTrackFirstZ(double first_z)
{
  m_firstz = first_z;
}

void BelleTrkExtra::setTrackLastX(double last_x)
{
  m_lastx = last_x;
}

void BelleTrkExtra::setTrackLastY(double last_y)
{
  m_lasty = last_y;
}

void BelleTrkExtra::setTrackLastZ(double last_z)
{
  m_lastz = last_z;
}

