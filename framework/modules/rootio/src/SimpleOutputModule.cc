/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/rootio/SimpleOutputModule.h>
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SimpleOutput)

void SimpleOutputModule::initialize()
{
  B2WARNING("The SimpleOutput module is deprecated. Please change SimpleOutput to RootOutput in your steering files.");
  RootOutputModule::initialize();
}
