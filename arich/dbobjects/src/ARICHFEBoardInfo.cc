/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

