/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHHapdChipInfo.h>
#include <arich/dbobjects/ARICHHapdInfo.h>

using namespace Belle2;
using namespace std;

ARICHHapdChipInfo* ARICHHapdInfo::getHapdChipInfo(unsigned int i) const
{
  if (i < c_NumberOfChips) return m_HAPDChipInfo[i];
  else return NULL;
}


void ARICHHapdInfo::setHapdChipInfo(unsigned int i, ARICHHapdChipInfo* chipInfo)
{
  if (i < c_NumberOfChips) m_HAPDChipInfo[i] = chipInfo;
}

