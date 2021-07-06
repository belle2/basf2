/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dataobjects/ARICHDigit.h>

using namespace std;
using namespace Belle2;

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

