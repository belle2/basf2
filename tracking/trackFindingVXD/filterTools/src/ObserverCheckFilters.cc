/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Lueck                                             *
*                                                                        *
* this is a modified copy of                                             *
* tracking/trackFindingVXD/filterTools/include/ObserverCheckMCPurity.h   *
*                                                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/trackFindingVXD/filterTools/ObserverCheckFilters.h>

using namespace Belle2;

// nullptr pointer
StoreArray<ObserverInfo> ObserverCheckFilters::s_storeArray = StoreArray<ObserverInfo>();

//initialize with an empty object
ObserverInfo ObserverCheckFilters::s_observerInfo = ObserverInfo();
