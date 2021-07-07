/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingVXD/filterTools/ObserverCheckFilters.h>

using namespace Belle2;

// nullptr pointer
StoreArray<ObserverInfo> ObserverCheckFilters::s_storeArray = StoreArray<ObserverInfo>();

//initialize with an empty object
ObserverInfo ObserverCheckFilters::s_observerInfo = ObserverInfo();
