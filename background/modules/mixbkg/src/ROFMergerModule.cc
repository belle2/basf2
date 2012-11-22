/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <background/modules/mixbkg/ROFMergerModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ROFMerger)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ROFMergerModule::ROFMergerModule() : Module()
{
  //Set module properties
  setDescription("Merges multiple background ROF ROOT files for a subdetector and a specific background component and generator into one.");

  //addParam("onlyPrimaries", m_onlyPrimaries, "Show only primary particles", true);
}


void ROFMergerModule::initialize()
{

}


void ROFMergerModule::event()
{

}
