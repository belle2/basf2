/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/VXDTFHelperTools/RawSecMapMergerModule.h>
#include <tracking/dataobjects/VXDTFSecMap.h>
#include <tracking/dataobjects/SecMapVector.h> // needed for rootImport
#include <tracking/dataobjects/VXDTFRawSecMap.h> // needed for rootImport
#include <tracking/dataobjects/FullSecID.h>
#include <framework/gearbox/Const.h>
#include <tracking/dataobjects/FilterID.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Stream.h> // to be able to stream TObjects into xml-coded files

#include <fstream>
#include <iostream>

// #include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <TVector3.h>
#include <TFile.h>
#include <TKey.h>
#include <TCollection.h>
#include <TTree.h>
#include <TBranch.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RawSecMapMerger)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RawSecMapMergerModule::RawSecMapMergerModule() : Module()
{
  //Set module properties
  setDescription("this module takes a root file containing a raw sectorMap created by the SecMapTrainerBaseModule and converts it to a sectormap which can be read by the VXDTF. Please check the parameters to be set...");
//   setPropertyFlags(c_ParallelProcessingCertified); /// WARNING this module should _not_ be used for parallel processing! Its task is to create the sector maps only once...

//   /// setting standard values for steering parameters
//
//   std::vector<int> sampleThreshold = {1, 100};
//   std::vector<double> smallSampleQuantiles = {0., 1.};
//   std::vector<double> sampleQuantiles = {0.001, 0.999};
//   std::vector<double> stretchFactor = {0.02, 0.}; // lower stretchFactor: change by 2%
//   std::string rootFileName = "FilterCalculatorResults";
//
//
//   /// following parameters only needed if importing from root file (-> importROOTorXML = true):
//   addParam("sampleThreshold", m_PARAMsampleThreshold,
//            "only needed if importROOTorXML = true: exactly two entries allowed: first: minimal sample size for sector-combination, second: threshold for 'small samples' where behavior is less strict. If sampleSize is bigger than second, normal behavior is chosen",
//            sampleThreshold);
//
//   addParam("filterRareCombinations", m_PARMfilterRareCombinations,
//            "only needed if importROOTorXML = true: use this member if you want to steer whether rare sector-friend-combinations shall be filtered or not. Set true if you want to filter these combinations or set false if otherwise. ",
//            bool(false));
//
//   addParam("rarenessFilter", m_PARAMrarenessFilter,
//            "only needed if importROOTorXML = true: use this member if you want to steer whether rare sector-friend-combinations shall be filtered or not, here you can set the threshold for filter. 100% = 1. 1% = 0.01%. Example: if you choose 0.01, all friendsectors which occur less often than in 1% of all cases when main sector was used, are deleted in the friendship-relations",
//            double(0.0));
//   addParam("smallSampleQuantiles", m_PARAMsmallSampleQuantiles,
//            "only needed if importROOTorXML = true: behiavior of small sample sizes, exactly two entries allowed: first: lower quantile, second: upper quantile. only values between 0-1 are allowed",
//            smallSampleQuantiles);
//
//   addParam("sampleQuantiles", m_PARAMsampleQuantiles,
//            "only needed if importROOTorXML = true: behiavior of normal sample sizes, exactly two entries allowed: first: lower quantile, second: upper quantile. only values between 0-1 are allowed",
//            sampleQuantiles);
//
//   addParam("stretchFactor", m_PARAMstretchFactor,
//            "only needed if importROOTorXML = true: exactly two entries allowed: first: stretchFactor for small sample size for sector-combination, second: stretchFactor for normal sample size for sector-combination: WARNING if you simply want to produce wider cutoffs in the VXDTF, please use the tuning parameters there! This parameter here is only if you know what you are doing, since it changes the values in the XML-file directly",
//            stretchFactor);
//
//   addParam("rootFileName", m_PARAMrootFileName, "only needed if importROOTorXML = true: sets the root filename", rootFileName);
//
//   addParam("printFinalMaps", m_PARAMprintFinalMaps,
//            "only needed if importROOTorXML = true: if true, a complete list of sectors (B2INFO) and its friends (B2DEBUG-1) will be printed on screen",
//            bool(true));
}






