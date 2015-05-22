/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZ.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZTemp.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DNormed.h"
#include "tracking/trackFindingVXD/TwoHitFilters/SlopeRZ.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZSquared.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance2DXYSquared.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DSquared.h"


#include "tracking/trackFindingVXD/FilterTools/Shortcuts.h"
#include "tracking/trackFindingVXD/FilterTools/Observer.h"

#include "tracking/trackFindingVXD/environment/VXDTFEnvironment.h"
#include "tracking/trackFindingVXD/environment/VXDTFSegmentFilter.h"
#include "tracking/modules/VXDTFRedesign/SegmentFilterConverterModule.h"
#include "tracking/vxdCaTracking/PassData.h"
#include "tracking/dataobjects/VXDTFSecMap.h"
#include "tracking/dataobjects/FilterID.h"

#include "framework/gearbox/Const.h"
#include "framework/gearbox/GearDir.h" // needed for reading xml-files
#include "framework/datastore/StoreObjPtr.h"

#include <TFile.h>
#include <TTree.h>

#include <math.h>
#include <algorithm>

using namespace Belle2;
using namespace std;

REG_MODULE(SegmentFilterConverter);

SegmentFilterConverterModule::SegmentFilterConverterModule() : Module()
{
  setDescription("To convert the XML description of the passes into the new "
                 "format");


  vector<string> sectorSetupDefault = {"std"};

  m_variables.resize(FilterID::numFilters);

  vector<bool>   activateTRUE  = { true };
  vector<bool>   activateFALSE = { false };
  vector<double> tuneZERO = { 0 };


  m_variables[ FilterID::distance3D       ] = make_tuple(string("Distance3D"),
                                                         tuneZERO, activateTRUE);

  m_variables[ FilterID::distanceXY       ] = make_tuple(string("DistanceXY"),
                                                         tuneZERO, activateTRUE);

  m_variables[ FilterID::distanceZ        ] = make_tuple(string("DistanceZ"),
                                                         tuneZERO, activateFALSE);

  m_variables[ FilterID::slopeRZ          ] = make_tuple(string("SlopeRZ"),
                                                         tuneZERO, activateTRUE);

  m_variables[ FilterID::normedDistance3D ] = make_tuple(string("NormedDistance3D"),
                                                         tuneZERO, activateFALSE);

  for (int i = 0; i < FilterID::numFilters ; i++) {
    if (VariableName(i) == string())
      continue;
    addParam(string("activate") + VariableName(i),
             VariableEnables(i),
             "set True/False for each setup individually."
             "The first value apply to the first setup, the second "
             "value to the second setup, etc. The last entry apply "
             "to all the remaining setup",
             VariableEnables(i));
    addParam(string("tune") + VariableName(i),
             VariableTunings(i),
             "tune the cutoff values for each setup in percentage. "
             "The first value apply to the first setup, the second "
             "value to the second setup, etc. The last entry apply "
             "to all the remaining setup",
             tuneZERO);
  }

  addParam("sectorSetup",
           m_PARAMsectorSetup,
           "lets you chose the sectorSetup (compatibility of sensors, "
           "individual cutoffs,...). "
           "Accepts 'std', 'low', 'high' and 'personal'. "
           "The chosen setup corresponds to a xml-file in "
           "../tracking/data/friendList_XXX.xml.",
           sectorSetupDefault);

  addParam("tuneCutoffs",
           m_PARAMtuneCutoffs,
           "for rapid changes of all cutoffs (no personal xml files needed), "
           "reduces/enlarges the range of the cutoffs in percent "
           "(lower and upper values are changed by this value). "
           "Only valid in range -99% < x < +1000%",
           double(0.22));

}

void
SegmentFilterConverterModule::initialize()
{
  StoreObjPtr< VXDTFEnvironment > environment("", DataStore::c_Persistent);
  environment.registerInDataStore(DataStore::c_DontWriteOut);
}

void
SegmentFilterConverterModule::beginRun()
{

  retrieveFromXML();

}

void
SegmentFilterConverterModule::retrieveFromXML(void)
{
  for (unsigned int i = 0; i < m_PARAMsectorSetup.size() ; ++i) {

    PassData* newPass = new PassData;

    newPass->sectorSetup = m_PARAMsectorSetup.at(i);
    newPass->generalTune = m_PARAMtuneCutoffs; // for all passes the same

    VXDTFSecMap::Class(); // needed by root.Perhaps will be removed with root 6
    string chosenSetup = newPass->sectorSetup;
    string directory = "/Detector/Tracking/CATFParameters/" + chosenSetup;

    try {
      auto TObjectPointer = Gearbox::getInstance().getTObject(directory.c_str());
      m_SectorMaps.push_back(dynamic_cast<const VXDTFSecMap*>(TObjectPointer));
    } catch (exception& e) {
      B2WARNING("could not load sectorMap."
                "Reason: exception thrown: " << e.what() <<
                ", this means you have to check whether the sectorMaps "
                "stored in ../tracking/data/VXDTFindex.xml and/or "
                "../testbeam/vxd/data/VXDTFindexTF.xml are uncommented "
                "and locally unpacked and available!")
    }
  }
}

void
SegmentFilterConverterModule::event()
{
  StoreObjPtr< VXDTFEnvironment > environment("", DataStore::c_Persistent);
  if (! environment) {
    environment.create();
    initVXDTFEnvironment();
  }
}

void
SegmentFilterConverterModule::initVXDTFEnvironment(void)
{
  for (unsigned int i = 0; i < m_PARAMsectorSetup.size() ; ++i)
    initSectorMapFilter(i);
}

void
SegmentFilterConverterModule::initSectorMapFilter(int setupIndex)
{

  const VXDTFSecMap* sectorMap = m_SectorMaps[ setupIndex ];
  StoreObjPtr< VXDTFEnvironment > environment("", DataStore::c_Persistent);
  VXDTFSegmentFilter* segmentFilter = new VXDTFSegmentFilter();

  for (auto friendsBlob : sectorMap->getSectorMap()) {
    FullSecID innerSectorID = FullSecID(friendsBlob.first);

    for (auto outerSectorBlob : friendsBlob.second) {
      FullSecID outerSectorID = FullSecID(outerSectorBlob.first);
      float min[ FilterID::numFilters ] = { -INFINITY };
      float max[ FilterID::numFilters ] = { +INFINITY };

      for (auto filterBlob :  outerSectorBlob.second) {
        unsigned int filterID = filterBlob.first;
        if (filterID >= FilterID::numFilters) {
          B2WARNING(" the filterID: " << filterID <<
                    " does not belong to the range "
                    "(0,FilterID::numFilters ) that is (0," <<
                    FilterID::numFilters << ") for friends sectors " <<
                    innerSectorID << " " << outerSectorID)
          continue;
        }
        min[filterID] = filterBlob.second.first *
                        (1. - m_PARAMtuneCutoffs - VariableTuning(filterID, setupIndex));
        max[filterID] = filterBlob.second.second *
                        (1. + m_PARAMtuneCutoffs + VariableTuning(filterID, setupIndex));
      }


      cout << VariableEnable(FilterID::distance3D , setupIndex);

      auto friendSectorsSegmentFilter =
        (
          (
            min[ FilterID::distance3D ] <
            Distance3DSquared<VXDTFHit>() <
            max[ FilterID::distance3D ]
          ).observe(Observer()).enable(VariableEnable(FilterID::distance3D, setupIndex)) &&

          (
            min[ FilterID::distanceXY ] <
            Distance2DXYSquared<VXDTFHit>() <
            max[ FilterID::distanceXY ]
          ).observe(Observer()).enable(VariableEnable(FilterID::distanceXY, setupIndex)) &&

          (
            min[ FilterID::distanceZ ] <
            Distance1DZ<VXDTFHit>() <
            max[ FilterID::distanceZ ]
          )/*.observe(Observer())*/.enable(VariableEnable(FilterID::distanceZ, setupIndex)) &&


          (
            min[ FilterID::slopeRZ ] <
            SlopeRZ<VXDTFHit>() <
            max[ FilterID::slopeRZ ]
          ).observe(Observer()).enable(VariableEnable(FilterID::slopeRZ, setupIndex)) &&

          (
            Distance3DNormed<VXDTFHit>() <
            max[ FilterID::normedDistance3D ]
          ).enable(VariableEnable(FilterID::normedDistance3D, setupIndex))

        );

      segmentFilter->addFriendsSectorFilter(innerSectorID, outerSectorID,
                                            friendSectorsSegmentFilter);
    }
  }
  environment->assignSegmentFilter(sectorMap->getMapName(), segmentFilter);
}



void
SegmentFilterConverterModule::endRun()
{

  bool skipWritingARootFile = true;

  if (skipWritingARootFile)
    return;

  TFile* file = new TFile("test.root", "RECREATE");

  int i = 0;
  for (auto sectorMap : m_SectorMaps) {


    TTree* tree = new TTree(m_PARAMsectorSetup.at(i).c_str(),
                            m_PARAMsectorSetup.at(i).c_str());
    i++;
    float dummyLim = 1.;
    auto filterPersistent(dummyLim < Distance3DSquared<VXDTFHit>() < -dummyLim  &&
                          dummyLim < Distance2DXYSquared<VXDTFHit>() < -dummyLim &&
                          dummyLim < Distance1DZ<VXDTFHit>() < -dummyLim &&
                          dummyLim < SlopeRZ<VXDTFHit>() < -dummyLim &&
                          Distance3DNormed<VXDTFHit>() < -dummyLim
                         );


    filterPersistent.persist(tree, "t");

    for (auto friendsBlob : sectorMap->getSectorMap()) {
      unsigned int innerSectorID = friendsBlob.first;

      for (auto outerSectorBlob : friendsBlob.second) {
        unsigned int outerSectorID = outerSectorBlob.first;
        float min[ FilterID::numFilters ] = { -INFINITY };
        float max[ FilterID::numFilters ] = { +INFINITY };

        for (auto filterBlob :  outerSectorBlob.second) {
          unsigned int filterID = filterBlob.first;
          if (filterID >= FilterID::numFilters) {
            B2WARNING(" the filterID: " << filterID <<
                      " does not belong to the range "
                      "(0,FilterID::numFilters ) that is (0," <<
                      FilterID::numFilters << ") for friends sectors " <<
                      innerSectorID << " " << outerSectorID)
            continue;
          }

          min[filterID] = filterBlob.second.first;
          max[filterID] = filterBlob.second.second;
        }

        filterPersistent = (
                             min[ FilterID::distance3D ] <
                             Distance3DSquared<VXDTFHit>() <
                             max[ FilterID::distance3D ] &&

                             min[ FilterID::distanceXY ] <
                             Distance2DXYSquared<VXDTFHit>() <
                             max[ FilterID::distanceXY ] &&

                             min[ FilterID::distanceZ ] <
                             Distance1DZ<VXDTFHit>() <
                             max[ FilterID::distanceZ ] &&

                             min[ FilterID::slopeRZ ] <
                             SlopeRZ<VXDTFHit>() <
                             max[ FilterID::slopeRZ ] &&

                             Distance3DNormed<VXDTFHit>() <
                             max[ FilterID::normedDistance3D ]

                           );

        tree->Fill();
      }
    }
  }
  file->Write();
  file->Close();

  delete file;

}

SegmentFilterConverterModule::JakobVariable_t&
SegmentFilterConverterModule::Variable(unsigned int n)
{
  static JakobVariable_t inCaseOfErrors;
  if (n >= m_variables.size()) {
    B2WARNING("Trying to access an undefined variable");
    return inCaseOfErrors;
  }
  return m_variables[n];

}

string
SegmentFilterConverterModule::VariableName(unsigned int variableIndex)
{
  return get<0>(Variable(variableIndex)) ;
}

std::vector<double>&
SegmentFilterConverterModule::VariableTunings(unsigned int variableIndex)
{
  return get<1>(Variable(variableIndex)) ;
}

double
SegmentFilterConverterModule::VariableTuning(unsigned int variableIndex,
                                             unsigned int setupIndex)
{
  auto tunings = VariableTunings(variableIndex);
  if (tunings.size() == 0)
    return 0.;
  setupIndex = std::min(setupIndex, (unsigned int)(tunings.size() - 1));
  return tunings[ setupIndex ];
}


std::vector<bool>&
SegmentFilterConverterModule::VariableEnables(unsigned int variableIndex)
{
  return get<2>(Variable(variableIndex)) ;
}

const bool&
SegmentFilterConverterModule::VariableEnable(unsigned int variableIndex,
                                             unsigned int setupIndex)
{
  std::vector< bool >& enables = VariableEnables(variableIndex);
  setupIndex = std::min(setupIndex, (unsigned int) enables.size() - 1);
  return enables[ setupIndex ];

}
