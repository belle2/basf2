/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "vxdtf/trackletFilters.h"
#include "vxdtf/sector.h"
#include "vxdtf/secMapVector.h"
#include "vxdtf/hittfinfo.h"
#include "vxdtf/filterID.h"
// #include "vxdtf/filters.cc"
#include "vxdtf/collectortfinfo.h"
#include "vxdtf/twohitfilters.h"
#include "vxdtf/trackcandidatetfinfo.h"
#include "vxdtf/threehitfilters.h"
#include "vxdtf/sectortools.h"
#include "vxdtf/sectortfinfo.h"
#include "vxdtf/fullSecID.h"
#include "vxdtf/fourhitfilters.h"
#include "vxdtf/basetfinfo.h"
#include "vxdtf/clustertfinfo.h"
#include "vxdtf/celltfinfo.h"
#include "vxdtf/vXDTFRawSecMap.h"

/// for the redesign:
// #include "vxdtfRedesign/observers.cc"
// #include "vxdtfRedesign/oldVSNewSegFinderFilters.cc"
// #include "vxdtfRedesign/twoHitFilters.cc"


/// for testing:
#include "vxdtf/sandBox4Testing.h"
