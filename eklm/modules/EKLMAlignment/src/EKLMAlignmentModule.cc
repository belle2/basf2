/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TFile.h>

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/geometry/EKLMObjectNumbers.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/modules/EKLMAlignment/EKLMAlignmentModule.h>
#include <framework/core/ModuleManager.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

using namespace Belle2;

REG_MODULE(EKLMAlignment)

EKLMAlignmentModule::EKLMAlignmentModule() : Module()
{
  setDescription("Module for generation of EKLM displacement data.");
  addParam("OutputFile", m_OutputFile, "Output file.",
           std::string("EKLMDisplacement.root"));
  setPropertyFlags(c_ParallelProcessingCertified);
}

EKLMAlignmentModule::~EKLMAlignmentModule()
{
}

void EKLMAlignmentModule::initialize()
{
  TFile* f = new TFile(m_OutputFile.c_str(), "recreate");
  EKLMAlignment alignment;
  EKLMAlignmentData alignmentData(0., 0., 0.);
  int iEndcap, iLayer, iSector, iPlane, iSegment, segment;
  for (iEndcap = 1; iEndcap <= 2; iEndcap++) {
    for (iLayer = 1; iLayer <= EKLM::GeometryData::Instance().
         getNDetectorLayers(iEndcap); iLayer++) {
      for (iSector = 1; iSector <= 4; iSector++) {
        for (iPlane = 1; iPlane <= 2; iPlane++) {
          for (iSegment = 1; iSegment <= 5; iSegment++) {
            segment = EKLM::segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                          iSegment);
            alignment.setAlignmentData(segment, &alignmentData);
          }
        }
      }
    }
  }
  alignment.Write("EKLMDisplacement");
  delete f;
}

void EKLMAlignmentModule::beginRun()
{
}

void EKLMAlignmentModule::event()
{
}

void EKLMAlignmentModule::endRun()
{
}

void EKLMAlignmentModule::terminate()
{
}

