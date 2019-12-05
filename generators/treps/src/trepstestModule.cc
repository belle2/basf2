/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/treps/trepstestModule.h>

/* --------------- WARNING ---------------------------------------------- *
   If you have more complex parameter types in your class then simple int,
   double or std::vector of those you might need to uncomment the following
   include directive to avoid an undefined reference on compilation.
   * ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>



using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

trepstestModule::trepstestModule() : Module()
{

  // Parameter definitions

}

void trepstestModule::initialize()
{
}

void trepstestModule::event()
{
}

void trepstestModule::terminate()
{
}



