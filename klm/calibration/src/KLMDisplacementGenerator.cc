/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMDisplacementGenerator.h>

/* KLM headers. */
#include <klm/eklm/geometry/AlignmentChecker.h>
#include <klm/eklm/geometry/GeometryData.h>

/* Belle 2 headers. */
#include <framework/gearbox/Unit.h>

/* ROOT headers. */
#include <TFile.h>
#include <TRandom.h>
#include <TTree.h>

using namespace Belle2;

KLMDisplacementGenerator::KLMDisplacementGenerator() :
  m_GeoDat(&(EKLM::GeometryData::Instance())),
  m_ElementNumbers(&(KLMElementNumbers::Instance())),
  m_eklmElementNumbers(&(EKLMElementNumbers::Instance()))
{
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
            segment = m_eklmElementNumbers->segmentNumber(
                        iSection, iLayer, iSector, iPlane, iSegment);
            segmentAlignment->setSegmentAlignment(segment, &alignmentData);
          }
        }
      }
    }
  }
}

void KLMDisplacementGenerator::generateFixedModuleDisplacement(
  double deltaU, double deltaV, double deltaGamma)
{
  EKLMAlignment alignment;
  EKLMSegmentAlignment segmentAlignment;
  KLMAlignmentData moduleAlignment(deltaU, deltaV, 0, 0, 0, deltaGamma);
  int iSection, iLayer, iSector;
  fillZeroDisplacements(&alignment, &segmentAlignment);
  for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        int module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
        alignment.setModuleAlignment(module, &moduleAlignment);
      }
    }
  }
}

void KLMDisplacementGenerator::generateRandomDisplacement(
  EKLMAlignment* alignment, EKLMSegmentAlignment* segmentAlignment,
  bool displaceModule, bool displaceSegment, bool moduleSameDisplacement,
  bool moduleZeroDeltaU, bool moduleZeroDeltaV, bool moduleZeroDeltaGamma)
{
  const double moduleMaxDeltaU = 5. * Unit::cm;
  const double moduleMinDeltaU = -5. * Unit::cm;
  const double moduleMaxDeltaV = 5. * Unit::cm;
  const double moduleMinDeltaV = -5. * Unit::cm;
  const double moduleMaxDeltaGamma = 0.02 * Unit::rad;
  const double moduleMinDeltaGamma = -0.02 * Unit::rad;
  const double segmentMaxDeltaU = 1. * Unit::cm;
  const double segmentMinDeltaU = -1. * Unit::cm;
  const double segmentMaxDeltaV = 0.2 * Unit::cm;
  const double segmentMinDeltaV = -0.2 * Unit::cm;
  const double segmentMaxDeltaGamma = 0.003 * Unit::rad;
  const double segmentMinDeltaGamma = -0.003 * Unit::rad;
  KLMAlignmentData moduleAlignmentData, segmentAlignmentData, *alignmentData;
  EKLM::AlignmentChecker alignmentChecker(false);
  int iSection, iLayer, iSector, iPlane, iSegment, segment;
  double d;
  fillZeroDisplacements(alignment, segmentAlignment);
  for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        if (moduleSameDisplacement) {
          if (iSection > 1 || iLayer > 1 || iSector > 1) {
            int module = m_ElementNumbers->moduleNumberEKLM(1, 1, 1);
            alignmentData = const_cast<KLMAlignmentData*>(
                              alignment->getModuleAlignment(module));
            module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
            alignment->setModuleAlignment(module, alignmentData);
            for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
              for (iSegment = 1; iSegment <= m_GeoDat->getNSegments();
                   iSegment++) {
                segment = m_eklmElementNumbers->segmentNumber(1, 1, 1, iPlane, iSegment);
                alignmentData = const_cast<KLMAlignmentData*>(
                                  segmentAlignment->getSegmentAlignment(segment));
                segment = m_eklmElementNumbers->segmentNumber(
                            iSection, iLayer, iSector, iPlane, iSegment);
                segmentAlignment->setSegmentAlignment(segment, alignmentData);
              }
            }
            continue;
          }
        }
module:
        if (displaceModule) {
          do {
            if (moduleZeroDeltaU)
              d = 0;
            else
              d = gRandom->Uniform(moduleMinDeltaU, moduleMaxDeltaU);
            moduleAlignmentData.setDeltaU(d);
            if (moduleZeroDeltaV)
              d = 0;
            else
              d = gRandom->Uniform(moduleMinDeltaV, moduleMaxDeltaV);
            moduleAlignmentData.setDeltaV(d);
            if (moduleZeroDeltaGamma)
              d = 0;
            else
              d = gRandom->Uniform(moduleMinDeltaGamma, moduleMaxDeltaGamma);
            moduleAlignmentData.setDeltaGamma(d);
          } while (!alignmentChecker.checkSectorAlignment(
                     iSection, iLayer, iSector, &moduleAlignmentData));
          int module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
          alignment->setModuleAlignment(module, &moduleAlignmentData);
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
                         &moduleAlignmentData, &segmentAlignmentData, false));
              segment = m_eklmElementNumbers->segmentNumber(
                          iSection, iLayer, iSector, iPlane, iSegment);
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
                    &moduleAlignmentData, &segmentAlignmentData, false))
                goto module;
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
  TFile* f;
  TTree* tEKLMModule, *tEKLMSegment;
  KLMAlignmentData* alignmentData;
  fillZeroDisplacements(alignment, segmentAlignment);
  f = new TFile(inputFile);
  tEKLMModule = (TTree*)f->Get("eklm_module");
  tEKLMModule->SetBranchAddress("section", &iSection);
  tEKLMModule->SetBranchAddress("layer", &iLayer);
  tEKLMModule->SetBranchAddress("sector", &iSector);
  tEKLMModule->SetBranchAddress("param", &param);
  tEKLMModule->SetBranchAddress("value", &value);
  tEKLMSegment = (TTree*)f->Get("eklm_segment");
  tEKLMSegment->SetBranchAddress("section", &iSection);
  tEKLMSegment->SetBranchAddress("layer", &iLayer);
  tEKLMSegment->SetBranchAddress("sector", &iSector);
  tEKLMSegment->SetBranchAddress("plane", &iPlane);
  tEKLMSegment->SetBranchAddress("segment", &iSegment);
  tEKLMSegment->SetBranchAddress("param", &param);
  tEKLMSegment->SetBranchAddress("value", &value);
  n = tEKLMModule->GetEntries();
  for (i = 0; i < n; i++) {
    tEKLMModule->GetEntry(i);
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
  n = tEKLMSegment->GetEntries();
  for (i = 0; i < n; i++) {
    tEKLMSegment->GetEntry(i);
    segment = m_eklmElementNumbers->segmentNumber(
                iSection, iLayer, iSector, iPlane, iSegment);
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

void KLMDisplacementGenerator::studyModuleAlignmentLimits(TFile* f)
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
  t = new TTree("module", "");
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
              segment = m_eklmElementNumbers->segmentNumber(
                          iSection, iLayer, iSector, jPlane, jSegment);
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
  studyModuleAlignmentLimits(f);
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
  TTree* tEKLMModule, *tEKLMSegment;
  f = new TFile(outputFile, "recreate");
  tEKLMModule = new TTree("eklm_module", "");
  tEKLMModule->Branch("section", &iSection, "section/I");
  tEKLMModule->Branch("layer", &iLayer, "layer/I");
  tEKLMModule->Branch("sector", &iSector, "sector/I");
  tEKLMModule->Branch("param", &param, "param/I");
  tEKLMModule->Branch("value", &value, "value/F");
  tEKLMSegment = new TTree("eklm_segment", "");
  tEKLMSegment->Branch("section", &iSection, "section/I");
  tEKLMSegment->Branch("layer", &iLayer, "layer/I");
  tEKLMSegment->Branch("sector", &iSector, "sector/I");
  tEKLMSegment->Branch("plane", &iPlane, "plane/I");
  tEKLMSegment->Branch("segment", &iSegment, "segment/I");
  tEKLMSegment->Branch("param", &param, "param/I");
  tEKLMSegment->Branch("value", &value, "value/F");
  for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        int module = m_ElementNumbers->moduleNumberEKLM(iSection, iSector, iLayer);
        alignmentData = const_cast<KLMAlignmentData*>(
                          alignment->getModuleAlignment(module));
        param = 1;
        value = alignmentData->getDeltaU();
        tEKLMModule->Fill();
        /* cppcheck-suppress redundantAssignment */
        param = 2;
        /* cppcheck-suppress redundantAssignment */
        value = alignmentData->getDeltaV();
        tEKLMModule->Fill();
        /* cppcheck-suppress redundantAssignment */
        param = 6;
        /* cppcheck-suppress redundantAssignment */
        value = alignmentData->getDeltaGamma();
        tEKLMModule->Fill();
        for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= m_GeoDat->getNSegments(); iSegment++) {
            segment = m_eklmElementNumbers->segmentNumber(
                        iSection, iLayer, iSector, iPlane, iSegment);
            alignmentData = const_cast<KLMAlignmentData*>(
                              segmentAlignment->getSegmentAlignment(segment));
            param = 2;
            value = alignmentData->getDeltaV();
            tEKLMSegment->Fill();
            /* cppcheck-suppress redundantAssignment */
            param = 6;
            /* cppcheck-suppress redundantAssignment */
            value = alignmentData->getDeltaGamma();
            tEKLMSegment->Fill();
          }
        }
      }
    }
  }
  f->cd();
  tEKLMModule->Write();
  tEKLMSegment->Write();
  delete tEKLMModule;
  delete tEKLMSegment;
  delete f;
}
