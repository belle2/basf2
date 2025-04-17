/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/roiFinding/svd/SVDROIFinderModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/SVDIntercept.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/roiFinding/ROIToUnitTranslator.templateDetails.h>
#include <tracking/roiFinding/VXDInterceptor.templateDetails.h>
#include <genfit/MaterialEffects.h>

#include <vector>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDROIFinder);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDROIFinderModule::SVDROIFinderModule() : Module()
{
  //Set module properties
  setDescription("This module finds the ROI on the SVD planes");
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

  addParam("SVDInterceptListName", m_SVDInterceptListName, "name of the list of interceptions", std::string(""));
  addParam("ROIListName", m_ROIListName, "name of the list of ROIs", std::string(""));
  addParam("ROIFindingForDQM", m_ROIFindingForDQM,
           "Is this ROI finding for DQM? If false, create SVDIntercepts by extrapolating tracks in both directions. If true, only extrapolate backwards.",
           m_ROIFindingForDQM);
}

SVDROIFinderModule::~SVDROIFinderModule()
{
}


void SVDROIFinderModule::initialize()
{
  m_recotracks.isOptional(m_recoTracksListName);

  m_rois.registerInDataStore(m_ROIListName);

  m_intercepts.registerInDataStore(m_SVDInterceptListName);

  m_recotracks.registerRelationTo(m_intercepts);
  m_intercepts.registerRelationTo(m_rois);


  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  }

}

void SVDROIFinderModule::beginRun()
{

  B2DEBUG(21, "||| SVDROIFinder Parameters:");
  B2DEBUG(21, "    tolerance: phi = " << m_tolerancePhi);
  B2DEBUG(21, "                z = " << m_toleranceZ);
  B2DEBUG(21, "    n sigma:    u = " << m_numSigmaTotU);
  B2DEBUG(21, "                v = " << m_numSigmaTotV);
  B2DEBUG(21, "    systematic: u = " << m_sigmaSystU);
  B2DEBUG(21, "                v = " << m_sigmaSystV);
  B2DEBUG(21, "    max width:  u = " << m_maxWidthU);
  B2DEBUG(21, "                v = " << m_maxWidthV);

  m_ROIinfo.sigmaSystU = m_sigmaSystU;
  m_ROIinfo.sigmaSystV = m_sigmaSystV;
  m_ROIinfo.numSigmaTotU = m_numSigmaTotU;
  m_ROIinfo.numSigmaTotV = m_numSigmaTotV;
  m_ROIinfo.maxWidthU = m_maxWidthU;
  m_ROIinfo.maxWidthV = m_maxWidthV;
  m_ROIinfo.PXDInterceptListName = m_SVDInterceptListName;
  m_ROIinfo.ROIListName = m_ROIListName;
  m_ROIinfo.recoTracksListName = m_recoTracksListName;

  m_theSVDInterceptor = new VXDInterceptor<SVDIntercept>(m_toleranceZ, m_tolerancePhi,
                                                         std::vector<float> {3.9, 8.0, 10.4, 13.5},
                                                         VXD::SensorInfoBase::SVD,
                                                         m_ROIFindingForDQM);

  m_theStripTranslator = new ROIToUnitTranslator<SVDIntercept>(&m_ROIinfo);

}


void SVDROIFinderModule::event()
{

  if (!m_recotracks.isValid()) {
    B2DEBUG(21, "RecoTracks array is missing, no SVD ROIs");
    return;
  }


  B2DEBUG(21, "%%%%%%%% Number of RecoTracks in the events =  " << m_recotracks.getEntries());

  RelationArray recoTrackToSVDIntercepts(m_recotracks, m_intercepts);
  recoTrackToSVDIntercepts.create();

  RelationArray SVDInterceptsToROIids(m_intercepts, m_rois);
  SVDInterceptsToROIids.create();

  m_theSVDInterceptor->fillInterceptList(&m_intercepts, m_recotracks, &recoTrackToSVDIntercepts);

  m_theStripTranslator->fillRoiIDList(&m_intercepts, &m_rois);

}


void SVDROIFinderModule::endRun()
{
  delete m_theStripTranslator;
  delete m_theSVDInterceptor;
}


void SVDROIFinderModule::terminate()
{
}

