/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

