/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dataobjects/ARICHDigit.h>

using namespace std;
using namespace Belle2;

ClassImp(ARICHDigit)

DigitBase::EAppendStatus ARICHDigit::addBGDigit(const DigitBase* bg)
{

  const auto* bgDigit = static_cast<const ARICHDigit*>(bg);
  uint8_t bg_bitmap = bgDigit->getBitmap();
  int i = 0;
  while (bg_bitmap >> i) {
    if (m_bitmap == 0xF) break;
    m_bitmap = (m_bitmap << 1) + 1;
    i++;
  }

  return DigitBase::c_DontAppend;

}

