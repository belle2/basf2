/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHAsicInfo.h>
#include <arich/dbobjects/ARICHFEBoardInfo.h>

using namespace Belle2;
using namespace std;

ARICHAsicInfo* ARICHFEBoardInfo::getAsicInfo(unsigned int i) const
{
  if (i < 4) return m_asicInfo[i];
  else return NULL;
}


void ARICHFEBoardInfo::setAsicInfo(unsigned int i, ARICHAsicInfo* asicInfo)
{
  if (i < 4) m_asicInfo[i] = asicInfo;
}

std::string ARICHFEBoardInfo::getAsicPosition(unsigned int i) const
{
  if (i < 4) return m_asicsFEB[i];
  else return "";
}


void ARICHFEBoardInfo::setAsicPosition(unsigned int i, const std::string& asicsFEB)
{
  if (i < 4) m_asicsFEB[i] = asicsFEB;
}

