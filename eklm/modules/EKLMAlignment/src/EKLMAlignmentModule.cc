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
  addParam("RandomDisplacement", m_RandomDisplacement,
           "What should be randomly displaced ('Sector', 'Segment' or 'Both').",
           std::string("Both"));
  addParam("SectorSameDisplacement", m_SectorSameDisplacement,
           "If the displacement should be the same for all sectors.", false);
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

void EKLMAlignmentModule::generateRandomDisplacement(
  bool displaceSector, bool displaceSegment)
{
  IntervalOfValidity iov(0, 0, -1, -1);
  const double sectorMaxDx = 5. * Unit::cm;
  const double sectorMinDx = -5. * Unit::cm;
  const double sectorMaxDy = 5. * Unit::cm;
  const double sectorMinDy = -5. * Unit::cm;
  const double sectorMaxDalpha = 0.02 * Unit::rad;
  const double sectorMinDalpha = -0.02 * Unit::rad;
  const double segmentMaxDx = 1. * Unit::cm;
  const double segmentMinDx = -1. * Unit::cm;
  const double segmentMaxDy = 0.2 * Unit::cm;
  const double segmentMinDy = -0.2 * Unit::cm;
  const double segmentMaxDalpha = 0.003 * Unit::rad;
  const double segmentMinDalpha = -0.003 * Unit::rad;
  EKLMAlignment alignment;
  EKLMAlignmentData sectorAlignment, segmentAlignment, *alignmentData;
  EKLM::AlignmentChecker alignmentChecker(false);
  int iEndcap, iLayer, iSector, iPlane, iSegment, sector, segment;
  EKLM::fillZeroDisplacements(&alignment);
  for (iEndcap = 1; iEndcap <= m_GeoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        if (m_SectorSameDisplacement) {
          if (iEndcap > 1 || iLayer > 1 || iSector > 1) {
            sector = m_GeoDat->sectorNumber(1, 1, 1);
            alignmentData = alignment.getSectorAlignment(sector);
            sector = m_GeoDat->sectorNumber(iEndcap, iLayer, iSector);
            alignment.setSectorAlignment(sector, alignmentData);
            for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
              for (iSegment = 1; iSegment <= m_GeoDat->getNSegments();
                   iSegment++) {
                segment = m_GeoDat->segmentNumber(1, 1, 1, iPlane, iSegment);
                alignmentData = alignment.getSegmentAlignment(segment);
                segment = m_GeoDat->segmentNumber(iEndcap, iLayer, iSector,
                                                  iPlane, iSegment);
                alignment.setSegmentAlignment(segment, alignmentData);
              }
            }
            continue;
          }
        }
sector:
        if (displaceSector) {
          do {
            sectorAlignment.setDx(gRandom->Uniform(sectorMinDx, sectorMaxDx));
            sectorAlignment.setDy(gRandom->Uniform(sectorMinDy, sectorMaxDy));
            sectorAlignment.setDalpha(
              gRandom->Uniform(sectorMinDalpha, sectorMaxDalpha));
          } while (!alignmentChecker.checkSectorAlignment(&sectorAlignment));
          sector = m_GeoDat->sectorNumber(iEndcap, iLayer, iSector);
          alignment.setSectorAlignment(sector, &sectorAlignment);
        }
        if (displaceSegment) {
          for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
            for (iSegment = 1; iSegment <= m_GeoDat->getNSegments();
                 iSegment++) {
              do {
                segmentAlignment.setDx(
                  gRandom->Uniform(segmentMinDx, segmentMaxDx));
                segmentAlignment.setDy(
                  gRandom->Uniform(segmentMinDy, segmentMaxDy));
                segmentAlignment.setDalpha(
                  gRandom->Uniform(segmentMinDalpha, segmentMaxDalpha));
              } while (!alignmentChecker.checkSegmentAlignment(
                         iPlane, iSegment,
                         &sectorAlignment, &segmentAlignment));
              segment = m_GeoDat->segmentNumber(iEndcap, iLayer, iSector,
                                                iPlane, iSegment);
              alignment.setSegmentAlignment(segment, &segmentAlignment);
            }
          }
        } else {
          for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
            for (iSegment = 1; iSegment <= m_GeoDat->getNSegments();
                 iSegment++) {
              if (!alignmentChecker.checkSegmentAlignment(
                    iPlane, iSegment, &sectorAlignment, &segmentAlignment))
                goto sector;
            }
          }
        }
      }
    }
  }
  saveDisplacement(&alignment);
  Database::Instance().storeData("EKLMDisplacement", (TObject*)&alignment, iov);
}

void EKLMAlignmentModule::studySectorAlignmentLimits(TFile* f)
{
  const int nPoints = 1000;
  const float maxDx = 5. * Unit::cm;
  const float minDx = -5. * Unit::cm;
  const float maxDy = 5. * Unit::cm;
  const float minDy = -5. * Unit::cm;
  const float maxDalpha = 0.02 * Unit::rad;
  const float minDalpha = -0.02 * Unit::rad;
  float dx, dy, dalpha;
  int i, alignmentStatus, iEndcap, iLayer, iSector, sector;
  EKLMAlignment alignment;
  EKLMAlignmentData alignmentDataRandom;
  EKLM::AlignmentChecker alignmentChecker(false);
  TTree* t;
  f->cd();
  t = new TTree("sector", "");
  t->Branch("dx", &dx, "dx/F");
  t->Branch("dy", &dy, "dy/F");
  t->Branch("dalpha", &dalpha, "dalpha/F");
  t->Branch("status", &alignmentStatus, "status/I");
  for (i = 0; i < nPoints; i++) {
    EKLM::fillZeroDisplacements(&alignment);
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
          sector = m_GeoDat->sectorNumber(iEndcap, iLayer, iSector);
          alignment.setSectorAlignment(sector, &alignmentDataRandom);
        }
      }
    }
    if (alignmentChecker.checkAlignment(&alignment))
      alignmentStatus = 1;
    else
      alignmentStatus = 0;
    t->Fill();
  }
  t->Write();
}

void EKLMAlignmentModule::studySegmentAlignmentLimits(TFile* f)
{
  const int nPoints = 1000;
  const float maxDx = 9. * Unit::cm;
  const float minDx = -4. * Unit::cm;
  const float maxDy = 0.2 * Unit::cm;
  const float minDy = -0.2 * Unit::cm;
  const float maxDalpha = 0.003 * Unit::rad;
  const float minDalpha = -0.003 * Unit::rad;
  float dx, dy, dalpha;
  int i, alignmentStatus;
  int iEndcap, iLayer, iSector, jPlane, jSegment, segment;
  EKLMAlignment alignment;
  EKLMAlignmentData alignmentDataRandom;
  EKLM::AlignmentChecker alignmentChecker(false);
  TTree* t;
  f->cd();
  t = new TTree("segment", "");
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
        EKLM::fillZeroDisplacements(&alignment);
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
              segment = m_GeoDat->segmentNumber(iEndcap, iLayer, iSector,
                                                jPlane, jSegment);
              alignment.setSegmentAlignment(segment, &alignmentDataRandom);
            }
          }
        }
        if (alignmentChecker.checkAlignment(&alignment))
          alignmentStatus = 1;
        else
          alignmentStatus = 0;
        t->Fill();
      }
    }
  }
  t->Write();
}

void EKLMAlignmentModule::studyAlignmentLimits()
{
  TFile* f;
  f = new TFile(m_OutputFile.c_str(), "recreate");
  studySectorAlignmentLimits(f);
  studySegmentAlignmentLimits(f);
  delete f;
}

void EKLMAlignmentModule::saveDisplacement(EKLMAlignment* alignment)
{
  int iEndcap, iLayer, iSector, iPlane, iSegment, sector, segment, param;
  float value;
  EKLMAlignmentData* alignmentData;
  TFile* f;
  TTree* t_sector, *t_segment;
  f = new TFile(m_OutputFile.c_str(), "recreate");
  t_sector = new TTree("eklm_sector", "");
  t_sector->Branch("endcap", &iEndcap, "endcap/I");
  t_sector->Branch("layer", &iLayer, "layer/I");
  t_sector->Branch("sector", &iSector, "sector/I");
  t_sector->Branch("param", &param, "param/I");
  t_sector->Branch("value", &value, "value/F");
  t_segment = new TTree("eklm_segment", "");
  t_segment->Branch("endcap", &iEndcap, "endcap/I");
  t_segment->Branch("layer", &iLayer, "layer/I");
  t_segment->Branch("sector", &iSector, "sector/I");
  t_segment->Branch("plane", &iPlane, "plane/I");
  t_segment->Branch("segment", &iSegment, "segment/I");
  t_segment->Branch("param", &param, "param/I");
  t_segment->Branch("value", &value, "value/F");
  for (iEndcap = 1; iEndcap <= m_GeoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        sector = m_GeoDat->sectorNumber(iEndcap, iLayer, iSector);
        alignmentData = alignment->getSectorAlignment(sector);
        param = 1;
        value = alignmentData->getDx();
        t_sector->Fill();
        param = 2;
        value = alignmentData->getDy();
        t_sector->Fill();
        param = 3;
        value = alignmentData->getDalpha();
        t_sector->Fill();
        for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= m_GeoDat->getNSegments(); iSegment++) {
            segment = m_GeoDat->segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                              iSegment);
            alignmentData = alignment->getSegmentAlignment(segment);
            param = 1;
            value = alignmentData->getDy();
            t_segment->Fill();
            param = 2;
            value = alignmentData->getDalpha();
            t_segment->Fill();
          }
        }
      }
    }
  }
  f->cd();
  t_sector->Write();
  t_segment->Write();
  delete t_sector;
  delete t_segment;
  delete f;
}

void EKLMAlignmentModule::initialize()
{
  m_GeoDat = &(EKLM::GeometryData::Instance());
  if (m_Mode == "Zero")
    generateZeroDisplacement();
  else if (m_Mode == "Random") {
    if (m_RandomDisplacement == "Sector")
      generateRandomDisplacement(true, false);
    else if (m_RandomDisplacement == "Segment")
      generateRandomDisplacement(false, true);
    else if (m_RandomDisplacement == "Both")
      generateRandomDisplacement(true, true);
    else
      B2FATAL("Unknown random displacement mode.");
  } else if (m_Mode == "Limits")
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

