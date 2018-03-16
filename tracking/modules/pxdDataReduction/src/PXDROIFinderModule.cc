/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/PXDROIFinderModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <genfit/MaterialEffects.h>
#include <geometry/GeometryManager.h>
#include <TGeoManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <genfit/FieldManager.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <time.h>
#include <list>
#include <genfit/Track.h> //giulia

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDROIFinder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDROIFinderModule::PXDROIFinderModule() : Module()
{
  //Set module properties
  setDescription("This module performs the reduction of the PXD data output");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("recoTrackListName", m_recoTracksListName, " name of the list of the fitted tracks", std::string(""));

  addParam("tolerancePhi", m_tolerancePhi, "Tolerance by finding sensor in phi coordinate (radians).", double(0.15));

  addParam("toleranceZ", m_toleranceZ, "Tolerance by finding sensor in Z coordinate (cm).", double(0.5));

  addParam("sigmaSystU", m_sigmaSystU, " systematic sigma in the u local coordinate ", double(0.025));
  addParam("sigmaSystV", m_sigmaSystV, " systematic sigma in the V local coordinate ", double(0.025));
  addParam("numSigmaTotU", m_numSigmaTotU, " number of sigmas (total) in the U local coordinate ", double(10));
  addParam("numSigmaTotV", m_numSigmaTotV, " number of sigmas (total) in the V local coordinate ", double(10));
  addParam("maxWidthU", m_maxWidthU, " upper limit on width of the ROI in the U local coordinate (cm) ", double(0.5));
  addParam("maxWidthV", m_maxWidthV, " upper limit on width of the ROI in the V local coordinate (cm) ", double(0.5));

  addParam("PXDInterceptListName", m_PXDInterceptListName, "name of the list of interceptions", std::string(""));
  addParam("ROIListName", m_ROIListName, "name of the list of ROIs", std::string(""));

}

PXDROIFinderModule::~PXDROIFinderModule()
{
}


void PXDROIFinderModule::initialize()
{
  StoreArray<RecoTrack> trackList(m_recoTracksListName);

  StoreArray<ROIid> ROIList(m_ROIListName);
  ROIList.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

  StoreArray<PXDIntercept> PXDInterceptList(m_PXDInterceptListName);
  PXDInterceptList.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

  trackList.registerRelationTo(PXDInterceptList);
  PXDInterceptList.registerRelationTo(ROIList);


  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  }

}

void PXDROIFinderModule::beginRun()
{

  B2DEBUG(1, "||| PXDROIFinder Parameters:");
  B2DEBUG(1, "    tolerance: phi = " << m_tolerancePhi);
  B2DEBUG(1, "                z = " << m_toleranceZ);
  B2DEBUG(1, "    n sigma:    u = " << m_numSigmaTotU);
  B2DEBUG(1, "                v = " << m_numSigmaTotV);
  B2DEBUG(1, "    systematic: u = " << m_sigmaSystU);
  B2DEBUG(1, "                v = " << m_sigmaSystV);
  B2DEBUG(1, "    max width:  u = " << m_maxWidthU);
  B2DEBUG(1, "                v = " << m_maxWidthV);

  m_ROIinfo.sigmaSystU = m_sigmaSystU;
  m_ROIinfo.sigmaSystV = m_sigmaSystV;
  m_ROIinfo.numSigmaTotU = m_numSigmaTotU;
  m_ROIinfo.numSigmaTotV = m_numSigmaTotV;
  m_ROIinfo.maxWidthU = m_maxWidthU;
  m_ROIinfo.maxWidthV = m_maxWidthV;
  m_ROIinfo.PXDInterceptListName = m_PXDInterceptListName;
  m_ROIinfo.ROIListName = m_ROIListName;
  m_ROIinfo.recoTracksListName = m_recoTracksListName;

  m_thePXDInterceptor = new PXDInterceptor(&m_ROIinfo, m_toleranceZ, m_tolerancePhi);

  m_thePixelTranslator = new ROIPixelTranslator(&m_ROIinfo);

}


void PXDROIFinderModule::event()
{

  StoreArray<PXDIntercept> PXDInterceptList(m_PXDInterceptListName);

  StoreArray<ROIid> ROIList(m_ROIListName);

  StoreArray<RecoTrack> trackList(m_recoTracksListName);
  B2DEBUG(1, "%%%%%%%% Number of RecoTracks in the events =  " << trackList.getEntries());

  RelationArray recoTrackToPXDIntercepts(trackList, PXDInterceptList);
  recoTrackToPXDIntercepts.create();

  RelationArray PXDInterceptsToROIids(PXDInterceptList, ROIList);
  PXDInterceptsToROIids.create();

  //  timespec time1, time2, time3;

  //  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

  m_thePXDInterceptor->fillInterceptList(&PXDInterceptList, trackList, &recoTrackToPXDIntercepts);

  //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);

  m_thePixelTranslator->fillRoiIDList(&PXDInterceptList, &ROIList);

  // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time3);

}


void PXDROIFinderModule::endRun()
{
  delete m_thePixelTranslator;
  delete m_thePXDInterceptor;
}


void PXDROIFinderModule::terminate()
{
}

