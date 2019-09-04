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
#include <klm/eklm/geometry/AlignmentChecker.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/calibration/KLMDisplacementGenerator.h>

#include <framework/core/RandomNumbers.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

KLMDisplacementGenerator::KLMDisplacementGenerator()
{
  m_GeoDat = &(EKLM::GeometryData::Instance());
  m_ElementNumbers = &(KLMElementNumbers::Instance());
}

KLMDisplacementGenerator::~KLMDisplacementGenerator()
{
}

void KLMDisplacementGenerator::fillZeroDisplacements(
  EKLMAlignment* alignment, EKLMSegmentAlignment* segmentAlignment)
{
  KLMAlignmentData alignmentData(0, 0, 0, 0, 0, 0);
  int iSection, iLayer, iSector, iPlane, iSegment, segment;
  for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        int module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
        alignment->setModuleAlignment(module, &alignmentData);
        for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= m_GeoDat->getNSegments(); iSegment++) {
            segment = m_GeoDat->segmentNumber(iSection, iLayer, iSector, iPlane,
                                              iSegment);
            segmentAlignment->setSegmentAlignment(segment, &alignmentData);
          }
        }
      }
    }
  }
}

void KLMDisplacementGenerator::generateZeroDisplacement()
{
  IntervalOfValidity iov(0, 0, -1, -1);
  EKLMAlignment alignment;
  EKLMSegmentAlignment segmentAlignment;
  fillZeroDisplacements(&alignment, &segmentAlignment);
}

void KLMDisplacementGenerator::generateFixedSectorDisplacement(
  double deltaU, double deltaV, double deltaGamma)
{
  IntervalOfValidity iov(0, 0, -1, -1);
  EKLMAlignment alignment;
  EKLMSegmentAlignment segmentAlignment;
  KLMAlignmentData sectorAlignment(deltaU, deltaV, 0, 0, 0, deltaGamma);
  int iSection, iLayer, iSector;
  fillZeroDisplacements(&alignment, &segmentAlignment);
  for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        int module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
        alignment.setModuleAlignment(module, &sectorAlignment);
      }
    }
  }
}

void KLMDisplacementGenerator::generateRandomDisplacement(
  EKLMAlignment* alignment, EKLMSegmentAlignment* segmentAlignment,
  bool displaceSector, bool displaceSegment, bool sectorSameDisplacement,
  bool sectorZeroDeltaU, bool sectorZeroDeltaV, bool sectorZeroDeltaGamma)
{
  IntervalOfValidity iov(0, 0, -1, -1);
  const double sectorMaxDeltaU = 5. * Unit::cm;
  const double sectorMinDeltaU = -5. * Unit::cm;
  const double sectorMaxDeltaV = 5. * Unit::cm;
  const double sectorMinDeltaV = -5. * Unit::cm;
  const double sectorMaxDeltaGamma = 0.02 * Unit::rad;
  const double sectorMinDeltaGamma = -0.02 * Unit::rad;
  const double segmentMaxDeltaU = 1. * Unit::cm;
  const double segmentMinDeltaU = -1. * Unit::cm;
  const double segmentMaxDeltaV = 0.2 * Unit::cm;
  const double segmentMinDeltaV = -0.2 * Unit::cm;
  const double segmentMaxDeltaGamma = 0.003 * Unit::rad;
  const double segmentMinDeltaGamma = -0.003 * Unit::rad;
  KLMAlignmentData sectorAlignmentData, segmentAlignmentData, *alignmentData;
  EKLM::AlignmentChecker alignmentChecker(false);
  int iSection, iLayer, iSector, iPlane, iSegment, segment;
  double d;
  fillZeroDisplacements(alignment, segmentAlignment);
  for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        if (sectorSameDisplacement) {
          if (iSection > 1 || iLayer > 1 || iSector > 1) {
            int module = m_ElementNumbers->moduleNumberEKLM(1, 1, 1);
            alignmentData = const_cast<KLMAlignmentData*>(
                              alignment->getModuleAlignment(module));
            module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
            alignment->setModuleAlignment(module, alignmentData);
            for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
              for (iSegment = 1; iSegment <= m_GeoDat->getNSegments();
                   iSegment++) {
                segment = m_GeoDat->segmentNumber(1, 1, 1, iPlane, iSegment);
                alignmentData = const_cast<KLMAlignmentData*>(
                                  segmentAlignment->getSegmentAlignment(segment));
                segment = m_GeoDat->segmentNumber(iSection, iLayer, iSector,
                                                  iPlane, iSegment);
                segmentAlignment->setSegmentAlignment(segment, alignmentData);
              }
            }
            continue;
          }
        }
sector:
        if (displaceSector) {
          do {
            if (sectorZeroDeltaU)
              d = 0;
            else
              d = gRandom->Uniform(sectorMinDeltaU, sectorMaxDeltaU);
            sectorAlignmentData.setDeltaU(d);
            if (sectorZeroDeltaV)
              d = 0;
            else
              d = gRandom->Uniform(sectorMinDeltaV, sectorMaxDeltaV);
            sectorAlignmentData.setDeltaV(d);
            if (sectorZeroDeltaGamma)
              d = 0;
            else
              d = gRandom->Uniform(sectorMinDeltaGamma, sectorMaxDeltaGamma);
            sectorAlignmentData.setDeltaGamma(d);
          } while (!alignmentChecker.checkSectorAlignment(
                     iSection, iLayer, iSector, &sectorAlignmentData));
          int module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
          alignment->setModuleAlignment(module, &sectorAlignmentData);
        }
        if (displaceSegment) {
          for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
            for (iSegment = 1; iSegment <= m_GeoDat->getNSegments();
                 iSegment++) {
              do {
                segmentAlignmentData.setDeltaU(
                  gRandom->Uniform(segmentMinDeltaU, segmentMaxDeltaU));
                segmentAlignmentData.setDeltaV(
                  gRandom->Uniform(segmentMinDeltaV, segmentMaxDeltaV));
                segmentAlignmentData.setDeltaGamma(
                  gRandom->Uniform(segmentMinDeltaGamma, segmentMaxDeltaGamma));
              } while (!alignmentChecker.checkSegmentAlignment(
                         iSection, iLayer, iSector, iPlane, iSegment,
                         &sectorAlignmentData, &segmentAlignmentData, false));
              segment = m_GeoDat->segmentNumber(iSection, iLayer, iSector,
                                                iPlane, iSegment);
              segmentAlignment->setSegmentAlignment(
                segment, &segmentAlignmentData);
            }
          }
        } else {
          for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
            for (iSegment = 1; iSegment <= m_GeoDat->getNSegments();
                 iSegment++) {
              if (!alignmentChecker.checkSegmentAlignment(
                    iSection, iLayer, iSector, iPlane, iSegment,
                    &sectorAlignmentData, &segmentAlignmentData, false))
                goto sector;
            }
          }
        }
      }
    }
  }
}

void KLMDisplacementGenerator::readDisplacementFromROOTFile(
  EKLMAlignment* alignment, EKLMSegmentAlignment* segmentAlignment,
  const char* inputFile)
{
  /* cppcheck-suppress variableScope */
  int i, n, iSection, iLayer, iSector, iPlane, iSegment, segment, param;
  float value;
  IntervalOfValidity iov(0, 0, -1, -1);
  TFile* f;
  TTree* t_sector, *t_segment;
  KLMAlignmentData* alignmentData;
  fillZeroDisplacements(alignment, segmentAlignment);
  f = new TFile(inputFile);
  t_sector = (TTree*)f->Get("eklm_sector");
  t_sector->SetBranchAddress("section", &iSection);
  t_sector->SetBranchAddress("layer", &iLayer);
  t_sector->SetBranchAddress("sector", &iSector);
  t_sector->SetBranchAddress("param", &param);
  t_sector->SetBranchAddress("value", &value);
  t_segment = (TTree*)f->Get("eklm_segment");
  t_segment->SetBranchAddress("section", &iSection);
  t_segment->SetBranchAddress("layer", &iLayer);
  t_segment->SetBranchAddress("sector", &iSector);
  t_segment->SetBranchAddress("plane", &iPlane);
  t_segment->SetBranchAddress("segment", &iSegment);
  t_segment->SetBranchAddress("param", &param);
  t_segment->SetBranchAddress("value", &value);
  n = t_sector->GetEntries();
  for (i = 0; i < n; i++) {
    t_sector->GetEntry(i);
    int module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
    alignmentData = const_cast<KLMAlignmentData*>(
                      alignment->getModuleAlignment(module));
    switch (param) {
      case 1:
        alignmentData->setDeltaU(value);
        break;
      case 2:
        alignmentData->setDeltaV(value);
        break;
      case 3:
        alignmentData->setDeltaGamma(value);
        break;
    }
  }
  n = t_segment->GetEntries();
  for (i = 0; i < n; i++) {
    t_segment->GetEntry(i);
    segment = m_GeoDat->segmentNumber(iSection, iLayer, iSector, iPlane,
                                      iSegment);
    alignmentData = const_cast<KLMAlignmentData*>(
                      segmentAlignment->getSegmentAlignment(segment));
    switch (param) {
      case 1:
        alignmentData->setDeltaV(value);
        break;
      case 2:
        alignmentData->setDeltaGamma(value);
        break;
    }
  }
}

void KLMDisplacementGenerator::studySectorAlignmentLimits(TFile* f)
{
  const int nPoints = 1000;
  const float maxDeltaU = 5. * Unit::cm;
  const float minDeltaU = -5. * Unit::cm;
  const float maxDeltaV = 5. * Unit::cm;
  const float minDeltaV = -5. * Unit::cm;
  const float maxDeltaGamma = 0.02 * Unit::rad;
  const float minDeltaGamma = -0.02 * Unit::rad;
  float deltaU, deltaV, deltaGamma;
  int i, alignmentStatus, iSection, iLayer, iSector;
  EKLMAlignment alignment;
  EKLMSegmentAlignment segmentAlignment;
  KLMAlignmentData alignmentDataRandom;
  EKLM::AlignmentChecker alignmentChecker(false);
  TTree* t;
  f->cd();
  t = new TTree("sector", "");
  t->Branch("deltaU", &deltaU, "deltaU/F");
  t->Branch("deltaV", &deltaV, "deltaV/F");
  t->Branch("deltaGamma", &deltaGamma, "deltaGamma/F");
  t->Branch("status", &alignmentStatus, "status/I");
  for (i = 0; i < nPoints; i++) {
    fillZeroDisplacements(&alignment, &segmentAlignment);
    deltaU = gRandom->Uniform(minDeltaU, maxDeltaU);
    deltaV = gRandom->Uniform(minDeltaV, maxDeltaV);
    deltaGamma = gRandom->Uniform(minDeltaGamma, maxDeltaGamma);
    alignmentDataRandom.setDeltaU(deltaU);
    alignmentDataRandom.setDeltaV(deltaV);
    alignmentDataRandom.setDeltaGamma(deltaGamma);
    for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
      for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
           iLayer++) {
        for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
          int module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
          alignment.setModuleAlignment(module, &alignmentDataRandom);
        }
      }
    }
    if (alignmentChecker.checkAlignment(&alignment, &segmentAlignment))
      alignmentStatus = 1;
    else
      alignmentStatus = 0;
    t->Fill();
  }
  t->Write();
}

void KLMDisplacementGenerator::studySegmentAlignmentLimits(TFile* f)
{
  const int nPoints = 1000;
  const float maxDeltaU = 9. * Unit::cm;
  const float minDeltaU = -4. * Unit::cm;
  const float maxDeltaV = 0.2 * Unit::cm;
  const float minDeltaV = -0.2 * Unit::cm;
  const float maxDeltaGamma = 0.003 * Unit::rad;
  const float minDeltaGamma = -0.003 * Unit::rad;
  float deltaU, deltaV, deltaGamma;
  int i, alignmentStatus;
  int iSection, iLayer, iSector, jPlane, jSegment, segment;
  EKLMAlignment alignment;
  EKLMSegmentAlignment segmentAlignment;
  KLMAlignmentData alignmentDataRandom;
  EKLM::AlignmentChecker alignmentChecker(false);
  TTree* t;
  f->cd();
  t = new TTree("segment", "");
  t->Branch("plane", &jPlane, "plane/I");
  t->Branch("segment", &jSegment, "segment/I");
  t->Branch("deltaU", &deltaU, "deltaU/F");
  t->Branch("deltaV", &deltaV, "deltaV/F");
  t->Branch("deltaGamma", &deltaGamma, "deltaGamma/F");
  t->Branch("status", &alignmentStatus, "status/I");
  for (jPlane = 1; jPlane <= m_GeoDat->getNPlanes(); jPlane++) {
    printf("Plane %d\n", jPlane);
    for (jSegment = 1; jSegment <= m_GeoDat->getNSegments(); jSegment++) {
      printf("Segment %d\n", jSegment);
      for (i = 0; i < nPoints; i++) {
        fillZeroDisplacements(&alignment, &segmentAlignment);
        deltaU = gRandom->Uniform(minDeltaU, maxDeltaU);
        deltaV = gRandom->Uniform(minDeltaV, maxDeltaV);
        deltaGamma = gRandom->Uniform(minDeltaGamma, maxDeltaGamma);
        alignmentDataRandom.setDeltaU(deltaU);
        alignmentDataRandom.setDeltaV(deltaV);
        alignmentDataRandom.setDeltaGamma(deltaGamma);
        for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
          for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
               iLayer++) {
            for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
              segment = m_GeoDat->segmentNumber(iSection, iLayer, iSector,
                                                jPlane, jSegment);
              segmentAlignment.setSegmentAlignment(
                segment, &alignmentDataRandom);
            }
          }
        }
        if (alignmentChecker.checkAlignment(&alignment, &segmentAlignment))
          alignmentStatus = 1;
        else
          alignmentStatus = 0;
        t->Fill();
      }
    }
  }
  t->Write();
}

void KLMDisplacementGenerator::studyAlignmentLimits(const char* outputFile)
{
  TFile* f;
  f = new TFile(outputFile, "recreate");
  studySectorAlignmentLimits(f);
  studySegmentAlignmentLimits(f);
  delete f;
}

void KLMDisplacementGenerator::saveDisplacement(
  EKLMAlignment* alignment, EKLMSegmentAlignment* segmentAlignment,
  const char* outputFile)
{
  int iSection, iLayer, iSector, iPlane, iSegment, segment, param;
  float value;
  KLMAlignmentData* alignmentData;
  TFile* f;
  TTree* t_sector, *t_segment;
  f = new TFile(outputFile, "recreate");
  t_sector = new TTree("eklm_sector", "");
  t_sector->Branch("section", &iSection, "section/I");
  t_sector->Branch("layer", &iLayer, "layer/I");
  t_sector->Branch("sector", &iSector, "sector/I");
  t_sector->Branch("param", &param, "param/I");
  t_sector->Branch("value", &value, "value/F");
  t_segment = new TTree("eklm_segment", "");
  t_segment->Branch("section", &iSection, "section/I");
  t_segment->Branch("layer", &iLayer, "layer/I");
  t_segment->Branch("sector", &iSector, "sector/I");
  t_segment->Branch("plane", &iPlane, "plane/I");
  t_segment->Branch("segment", &iSegment, "segment/I");
  t_segment->Branch("param", &param, "param/I");
  t_segment->Branch("value", &value, "value/F");
  for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        int module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
        alignmentData = const_cast<KLMAlignmentData*>(
                          alignment->getModuleAlignment(module));
        param = 1;
        value = alignmentData->getDeltaU();
        t_sector->Fill();
        /* cppcheck-suppress redundantAssignment */
        param = 2;
        /* cppcheck-suppress redundantAssignment */
        value = alignmentData->getDeltaV();
        t_sector->Fill();
        /* cppcheck-suppress redundantAssignment */
        param = 6;
        /* cppcheck-suppress redundantAssignment */
        value = alignmentData->getDeltaGamma();
        t_sector->Fill();
        for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= m_GeoDat->getNSegments(); iSegment++) {
            segment = m_GeoDat->segmentNumber(iSection, iLayer, iSector, iPlane,
                                              iSegment);
            alignmentData = const_cast<KLMAlignmentData*>(
                              segmentAlignment->getSegmentAlignment(segment));
            param = 2;
            value = alignmentData->getDeltaV();
            t_segment->Fill();
            /* cppcheck-suppress redundantAssignment */
            param = 6;
            /* cppcheck-suppress redundantAssignment */
            value = alignmentData->getDeltaGamma();
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
