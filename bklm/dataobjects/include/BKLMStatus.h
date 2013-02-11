/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSTATUS_H
#define BKLMSTATUS_H

namespace Belle2 {

  const unsigned int STATUS_MC      = 0x40000000;
  const unsigned int STATUS_DECAYED = 0x20000000;
  const unsigned int STATUS_INRPC   = 0x00000001;

} // end of namespace Belle2

#endif //BKLMSTATUS_H
