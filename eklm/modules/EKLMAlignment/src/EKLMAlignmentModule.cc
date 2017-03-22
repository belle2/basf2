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
#include <TTree.h>

/* Belle2 headers. */
#include <eklm/alignment/AlignmentTools.h>
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/geometry/AlignmentChecker.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/modules/EKLMAlignment/EKLMAlignmentModule.h>

#include <framework/core/RandomNumbers.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

REG_MODULE(EKLMAlignment)

EKLMAlignmentModule::EKLMAlignmentModule() : Module()
{
  setDescription("Module for generation of EKLM displacement data.");
  addParam("Mode", m_Mode, "Mode ('Zero', 'Random' or 'Limits').",
           std::string("Zero"));
  addParam("OutputFile", m_OutputFile, "Output file.",
           std::string("EKLMDisplacement.root"));
  setPropertyFlags(c_ParallelProcessingCertified);
  m_GeoDat = NULL;
}

EKLMAlignmentModule::~EKLMAlignmentModule()
{
}

void EKLMAlignmentModule::generateZeroDisplacement()
{
  IntervalOfValidity iov(0, 0, -1, -1);
  EKLMAlignment alignment;
  EKLM::fillZeroDisplacements(&alignment);
  saveDisplacement(&alignment);
  Database::Instance().storeData("EKLMDisplacement", (TObject*)&alignment, iov);
}

void EKLMAlignmentModule::generateRandomDisplacement()
{
  IntervalOfValidity iov(0, 0, -1, -1);
  const double maxDx = 1. * Unit::cm;
  const double minDx = -1. * Unit::cm;
  const double maxDy = 0.2 * Unit::cm;
  const double minDy = -0.2 * Unit::cm;
  const double maxDalpha = 0.003 * Unit::rad;
  const double minDalpha = -0.003 * Unit::rad;
  EKLMAlignment alignment;
  EKLMAlignmentData alignmentData;
  EKLM::AlignmentChecker alignmentChecker;
  int iEndcap, iLayer, iSector, iPlane, iSegment, segment;
  for (iEndcap = 1; iEndcap <= m_GeoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= m_GeoDat->getNSegments(); iSegment++) {
            do {
              alignmentData.setDx(gRandom->Uniform(minDx, maxDx));
              alignmentData.setDy(gRandom->Uniform(minDy, maxDy));
              alignmentData.setDalpha(gRandom->Uniform(minDalpha, maxDalpha));
            } while (!alignmentChecker.checkSegmentAlignment(iPlane, iSegment,
                                                             &alignmentData));
            segment = m_GeoDat->segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                              iSegment);
            alignment.setAlignmentData(segment, &alignmentData);
          }
        }
      }
    }
  }
  saveDisplacement(&alignment);
  Database::Instance().storeData("EKLMDisplacement", (TObject*)&alignment, iov);
}

void EKLMAlignmentModule::studyAlignmentLimits()
{
  TFile* f;
  TTree* t;
  const int nPoints = 1000;
  const float maxDx = 9. * Unit::cm;
  const float minDx = -4. * Unit::cm;
  const float maxDy = 0.2 * Unit::cm;
  const float minDy = -0.2 * Unit::cm;
  const float maxDalpha = 0.003 * Unit::rad;
  const float minDalpha = -0.003 * Unit::rad;
  float dx, dy, dalpha;
  int i, alignmentStatus;
  int iEndcap, iLayer, iSector, iPlane, iSegment, jPlane, jSegment, segment;
  EKLMAlignment alignment;
  EKLMAlignmentData alignmentDataZero(0., 0., 0.);
  EKLMAlignmentData alignmentDataRandom;
  EKLM::AlignmentChecker alignmentChecker;
  f = new TFile(m_OutputFile.c_str(), "recreate");
  t = new TTree("alignment", "");
  t->Branch("plane", &jPlane, "plane/I");
  t->Branch("segment", &jSegment, "segment/I");
  t->Branch("dx", &dx, "dx/F");
  t->Branch("dy", &dy, "dy/F");
  t->Branch("dalpha", &dalpha, "dalpha/F");
  t->Branch("status", &alignmentStatus, "status/I");
  for (jPlane = 1; jPlane <= m_GeoDat->getNPlanes(); jPlane++) {
    printf("Plane %d\n", jPlane);
    for (jSegment = 1; jSegment <= m_GeoDat->getNSegments(); jSegment++) {
      printf("Segment %d\n", jSegment);
      for (i = 0; i < nPoints; i++) {
        dx = gRandom->Uniform(minDx, maxDx);
        dy = gRandom->Uniform(minDy, maxDy);
        dalpha = gRandom->Uniform(minDalpha, maxDalpha);
        alignmentDataRandom.setDx(dx);
        alignmentDataRandom.setDy(dy);
        alignmentDataRandom.setDalpha(dalpha);
        for (iEndcap = 1; iEndcap <= m_GeoDat->getNEndcaps(); iEndcap++) {
          for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iEndcap);
               iLayer++) {
            for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
              for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
                for (iSegment = 1; iSegment <= m_GeoDat->getNSegments();
                     iSegment++) {
                  segment = m_GeoDat->segmentNumber(iEndcap, iLayer, iSector,
                                                    iPlane, iSegment);
                  if (iPlane == jPlane && iSegment == jSegment)
                    alignment.setAlignmentData(segment, &alignmentDataRandom);
                  else
                    alignment.setAlignmentData(segment, &alignmentDataZero);
                }
              }
            }
          }
        }
        if (alignmentChecker.checkAlignment(&alignment))
          alignmentStatus = 1;
        else
          alignmentStatus = 0;
        t->Fill();
        alignment.cleanAlignmentData();
      }
    }
  }
  t->Write();
  delete f;
}

void EKLMAlignmentModule::saveDisplacement(EKLMAlignment* alignment)
{
  int iEndcap, iLayer, iSector, iPlane, iSegment, segment, param;
  float value;
  EKLMAlignmentData* alignmentData;
  TFile* f;
  TTree* t;
  f = new TFile(m_OutputFile.c_str(), "recreate");
  t = new TTree("eklm", "");
  t->Branch("endcap", &iEndcap, "endcap/I");
  t->Branch("layer", &iLayer, "layer/I");
  t->Branch("sector", &iSector, "sector/I");
  t->Branch("plane", &iPlane, "plane/I");
  t->Branch("segment", &iSegment, "segment/I");
  t->Branch("param", &param, "param/I");
  t->Branch("value", &value, "value/F");
  for (iEndcap = 1; iEndcap <= m_GeoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= m_GeoDat->getNSegments(); iSegment++) {
            segment = m_GeoDat->segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                              iSegment);
            alignmentData = alignment->getAlignmentData(segment);
            param = 1;
            value = alignmentData->getDy();
            t->Fill();
            param = 2;
            value = alignmentData->getDalpha();
            t->Fill();
          }
        }
      }
    }
  }
  f->cd();
  t->Write();
  delete t;
  delete f;
}

void EKLMAlignmentModule::initialize()
{
  m_GeoDat = &(EKLM::GeometryData::Instance());
  if (m_Mode == "Zero")
    generateZeroDisplacement();
  else if (m_Mode == "Random")
    generateRandomDisplacement();
  else if (m_Mode == "Limits")
    studyAlignmentLimits();
  else
    B2FATAL("Unknown operation mode.");
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

