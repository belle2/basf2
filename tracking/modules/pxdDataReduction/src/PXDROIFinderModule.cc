/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/PXDROIFinderModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <genfit/MaterialEffects.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDROIFinder);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDROIFinderModule::PXDROIFinderModule() : Module(), m_ROIinfo()
{
  //Set module properties
  setDescription("This module performs the reduction of the PXD data output");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("recoTrackListName", m_recoTracksListName, " name of the list of the fitted tracks", std::string(""));

  addParam("PXDInterceptListName", m_PXDInterceptListName, "name of the list of interceptions", std::string(""));
  addParam("ROIListName", m_ROIListName, "name of the list of ROIs", std::string(""));


  addParam("tolerancePhi", m_tolerancePhi,
           "Tolerance by finding sensor in phi coordinate (radians). Will only be used if overrideDBROICalculation is set to True.",
           double(0.15));
  addParam("toleranceZ", m_toleranceZ,
           "Tolerance by finding sensor in Z coordinate (cm). Will only be used if overrideDBROICalculation is set to True.",
           double(0.5));
  addParam("sigmaSystU", m_sigmaSystU,
           "Systematic sigma in the u local coordinate. Will only be used if overrideDBROICalculation is set to True.",
           double(0.025));
  addParam("sigmaSystV", m_sigmaSystV,
           "Systematic sigma in the V local coordinate. Will only be used if overrideDBROICalculation is set to True.",
           double(0.025));
  addParam("numSigmaTotU", m_numSigmaTotU,
           "Number of sigmas (total) in the U local coordinate. Will only be used if overrideDBROICalculation is set to True.",
           double(10));
  addParam("numSigmaTotV", m_numSigmaTotV,
           "Number of sigmas (total) in the V local coordinate. Will only be used if overrideDBROICalculation is set to True.",
           double(10));
  addParam("maxWidthU", m_maxWidthU,
           "Upper limit on width of the ROI in the U local coordinate (cm). Will only be used if overrideDBROICalculation is set to True.",
           double(0.5));
  addParam("maxWidthV", m_maxWidthV,
           "Upper limit on width of the ROI in the V local coordinate (cm). Will only be used if overrideDBROICalculation is set to True.",
           double(0.5));
  addParam("overrideDBROICalculation", m_overrideDBROICalculation,
           "Override the parameters from DB for testing or debugging. Required to be True to use the other ROI calculation parameters.",
           false);
}


void PXDROIFinderModule::initialize()
{
  m_RecoTracks.isOptional(m_recoTracksListName);
  m_ROIs.registerInDataStore(m_ROIListName, DataStore::c_ErrorIfAlreadyRegistered);
  m_PXDIntercepts.registerInDataStore(m_PXDInterceptListName, DataStore::c_ErrorIfAlreadyRegistered);

  m_RecoTracks.registerRelationTo(m_PXDIntercepts);
  m_PXDIntercepts.registerRelationTo(m_ROIs);

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  }

}

void PXDROIFinderModule::beginRun()
{
  if (not m_ROICalculationParameters.isValid()) {
    // No valid ROI calculation parameters from DB, crash in any case, even though module parameters are provided.
    B2FATAL("No ROI configuration for the current run found (missing ROICalculationParameters payload).");
  } else if (m_ROICalculationParameters.isValid() and not m_overrideDBROICalculation) {
    // Use ROI calculation parameters from DB.
    m_tolerancePhi = m_ROICalculationParameters->getToleranceZ();
    m_toleranceZ = m_ROICalculationParameters->getTolerancePhi();
    m_numSigmaTotU = m_ROICalculationParameters->getNumSigmaTotU();
    m_numSigmaTotV = m_ROICalculationParameters->getNumSigmaTotV();
    m_sigmaSystU = m_ROICalculationParameters->getSigmaSystU();
    m_sigmaSystV = m_ROICalculationParameters->getSigmaSystV();
    m_maxWidthU = m_ROICalculationParameters->getMaxWidthU();
    m_maxWidthV = m_ROICalculationParameters->getMaxWidthV();
  } else if (m_ROICalculationParameters.isValid() and m_overrideDBROICalculation) {
    // Use ROI calculation parameters from module parameters.
    B2DEBUG(29, "ROI calculation parameters from DB are valid, but they are overridden by the module parameters.\n"
            "  --> Using module parameters instead of parameters from DB.");
  }

  B2DEBUG(29, "||| PXDROIFinder Parameters:");
  B2DEBUG(29, "    tolerance: phi = " << m_tolerancePhi);
  B2DEBUG(29, "                z = " << m_toleranceZ);
  B2DEBUG(29, "    n sigma:    u = " << m_numSigmaTotU);
  B2DEBUG(29, "                v = " << m_numSigmaTotV);
  B2DEBUG(29, "    systematic: u = " << m_sigmaSystU);
  B2DEBUG(29, "                v = " << m_sigmaSystV);
  B2DEBUG(29, "    max width:  u = " << m_maxWidthU);
  B2DEBUG(29, "                v = " << m_maxWidthV);

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
  B2DEBUG(29, "%%%%%%%% Number of RecoTracks in the events =  " << m_RecoTracks.getEntries());

  RelationArray recoTrackToPXDIntercepts(m_RecoTracks, m_PXDIntercepts);
  recoTrackToPXDIntercepts.create();

  RelationArray PXDInterceptsToROIids(m_PXDIntercepts, m_ROIs);
  PXDInterceptsToROIids.create();

  if (m_thePXDInterceptor) m_thePXDInterceptor->fillInterceptList(&m_PXDIntercepts, m_RecoTracks, &recoTrackToPXDIntercepts);

  if (m_thePixelTranslator) m_thePixelTranslator->fillRoiIDList(&m_PXDIntercepts, &m_ROIs);

}


void PXDROIFinderModule::endRun()
{
  if (m_thePixelTranslator) delete m_thePixelTranslator;
  if (m_thePXDInterceptor) delete m_thePXDInterceptor;
}

