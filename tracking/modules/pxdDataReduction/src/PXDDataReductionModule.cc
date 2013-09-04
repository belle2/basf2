/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/PXDDataReductionModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <GFMaterialEffects.h>
#include <GFTGeoMaterialInterface.h>
#include <GFTrackCand.h>
#include <geometry/GeometryManager.h>
#include <TGeoManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <GFFieldManager.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <time.h>
#include <list>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDataReduction)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDataReductionModule::PXDDataReductionModule() : Module()
{
  //Set module properties
  setDescription("This module performs the reduction of the PXD data output");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);


  addParam("trackCandCollName", m_gfTrackCandsColName, " name of the input collection of track candidates", std::string(""));
  addParam("numIterKalmanFilter", m_numIterKalmanFilter, " number of iterations of the kalman filter ", int(2));

  addParam("sigmaSystU", m_sigmaSystU, " systematic sigma in the u local coordinate ", double(0.025));
  addParam("sigmaSystV", m_sigmaSystV, " systematic sigma in the V local coordinate ", double(0.025));
  addParam("numSigmaTotU", m_numSigmaTotU, " number of sigmas (total) in the U local coordinate ", double(10));
  addParam("numSigmaTotV", m_numSigmaTotV, " number of sigmas (total) in the V local coordinate ", double(10));
  addParam("maxWidthU", m_maxWidthU, " upper limit on width of the ROI in the U local coordinate (cm) ", double(0.5));
  addParam("maxWidthV", m_maxWidthV, " upper limit on width of the ROI in the V local coordinate (cm) ", double(0.5));

  addParam("PXDInterceptListName", m_PXDInterceptListName, "name of the list of interceptions", std::string(""));
  addParam("ROIListName", m_ROIListName, "name of the list of ROIs", std::string(""));

}

PXDDataReductionModule::~PXDDataReductionModule()
{
}


void PXDDataReductionModule::initialize()
{

  StoreArray<GFTrackCand>::required(m_gfTrackCandsColName);
  StoreArray<ROIid>::registerPersistent(m_ROIListName);
  StoreArray<PXDIntercept>::registerPersistent(m_PXDInterceptListName);

  RelationArray::registerPersistent<GFTrackCand, PXDIntercept>(m_gfTrackCandsColName, m_PXDInterceptListName);
  RelationArray::registerPersistent<PXDIntercept, ROIid>(m_PXDInterceptListName, m_ROIListName);


  if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
    //pass the magnetic field to genfit
    GFFieldManager::getInstance()->init(new GFGeant4Field());
    GFMaterialEffects::getInstance()->init(new GFTGeoMaterialInterface());
  }

  GFMaterialEffects::getInstance()->setMscModel("Highland");


}

void PXDDataReductionModule::beginRun()
{
  thePXDInterceptor = new PXDInterceptor();
  thePXDInterceptor->setNumIterKalmanFilter(m_numIterKalmanFilter);

  thePixelTranslator = new ROIPixelTranslator(m_sigmaSystU, m_sigmaSystV, m_numSigmaTotU, m_numSigmaTotV, m_maxWidthU, m_maxWidthV);

}


void PXDDataReductionModule::event()
{

  StoreArray<PXDIntercept> PXDInterceptList(m_PXDInterceptListName);
  PXDInterceptList.create();
  StoreArray<ROIid> ROIList(m_ROIListName);
  ROIList.create();

  StoreArray<GFTrackCand> trackCandList(m_gfTrackCandsColName);
  B2DEBUG(1, "%%%%%%%% EVENT # of tracks =  " << trackCandList.getEntries());


  RelationArray gfTrackCandToPXDIntercepts(trackCandList, PXDInterceptList);
  gfTrackCandToPXDIntercepts.create();

  RelationArray PXDInterceptsToROIids(PXDInterceptList, ROIList);
  PXDInterceptsToROIids.create();


  //  timespec time1, time2, time3;

  //  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

  thePXDInterceptor->fillInterceptList(&PXDInterceptList, trackCandList, &gfTrackCandToPXDIntercepts);

  //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);

  thePixelTranslator->fillRoiIDList(&PXDInterceptList, &ROIList);

  // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time3);

}


void PXDDataReductionModule::endRun()
{
  delete thePXDInterceptor;
}


void PXDDataReductionModule::terminate()
{



}

