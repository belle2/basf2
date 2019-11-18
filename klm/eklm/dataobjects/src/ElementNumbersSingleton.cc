/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>

using namespace Belle2;

const EKLM::ElementNumbersSingleton& EKLM::ElementNumbersSingleton::Instance()
{
  static EKLM::ElementNumbersSingleton ens;
  return ens;
}

EKLM::ElementNumbersSingleton::ElementNumbersSingleton()
{
}

EKLM::ElementNumbersSingleton::~ElementNumbersSingleton()
{
}

