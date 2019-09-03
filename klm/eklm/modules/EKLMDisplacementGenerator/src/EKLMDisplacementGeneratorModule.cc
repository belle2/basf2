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
#include <klm/eklm/modules/EKLMDisplacementGenerator/EKLMDisplacementGeneratorModule.h>

#include <framework/core/RandomNumbers.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

REG_MODULE(EKLMDisplacementGenerator)

EKLMDisplacementGeneratorModule::EKLMDisplacementGeneratorModule() : Module()
{
  setDescription("Module for generation of EKLM displacement or "
                 "alignment data.");
  addParam("PayloadName", m_PayloadName,
           "Payload name ('EKLMDisplacement' or 'EKLMAlignment')",
           std::string("EKLMDisplacement"));
  addParam("Mode", m_Mode,
           "Mode ('Zero', 'FixedSector', 'Random', 'ROOT', 'Limits').",
           std::string("Zero"));
  addParam("RandomDisplacement", m_RandomDisplacement,
           "What should be randomly displaced ('Sector', 'Segment' or 'Both').",
           std::string("Both"));
  addParam("SectorSameDisplacement", m_SectorSameDisplacement,
           "If the displacement should be the same for all sectors.", false);
  addParam("SectorZeroDeltaU", m_SectorZeroDeltaU, "Fix sector deltaU at 0.", false);
  addParam("SectorZeroDeltaV", m_SectorZeroDeltaV, "Fix sector deltaV at 0.", false);
  addParam("SectorZeroDeltaGamma", m_SectorZeroDeltaGamma, "Fix sector deltaGamma at 0.",
           false);
  addParam("SectorDeltaU", m_SectorDeltaU, "Sector deltaU.", 0.);
  addParam("SectorDeltaV", m_SectorDeltaV, "Sector deltaV.", 0.);
  addParam("SectorDeltaGamma", m_SectorDeltaGamma, "Sector deltaGamma.", 0.);
  addParam("InputFile", m_InputFile, "Input file (for mode == 'ROOT' only).",
           std::string(""));
  addParam("OutputFile", m_OutputFile, "Output file.",
           std::string("EKLMDisplacement.root"));
  setPropertyFlags(c_ParallelProcessingCertified);
  m_GeoDat = nullptr;
}

EKLMDisplacementGeneratorModule::~EKLMDisplacementGeneratorModule()
{
}

void EKLMDisplacementGeneratorModule::fillZeroDisplacements(
  EKLMAlignment* alignment, EKLMSegmentAlignment* segmentAlignment)
{
  KLMAlignmentData alignmentData(0, 0, 0, 0, 0, 0);
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  int iSection, iLayer, iSector, iPlane, iSegment, segment, sector;
  for (iSection = 1; iSection <= geoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= geoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= geoDat->getNSectors(); iSector++) {
        sector = geoDat->sectorNumber(iSection, iLayer, iSector);
        alignment->setModuleAlignment(sector, &alignmentData);
        for (iPlane = 1; iPlane <= geoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= geoDat->getNSegments(); iSegment++) {
            segment = geoDat->segmentNumber(iSection, iLayer, iSector, iPlane,
                                            iSegment);
            segmentAlignment->setSegmentAlignment(segment, &alignmentData);
          }
        }
      }
    }
  }
}

void EKLMDisplacementGeneratorModule::generateZeroDisplacement()
{
  IntervalOfValidity iov(0, 0, -1, -1);
  EKLMAlignment alignment;
  EKLMSegmentAlignment segmentAlignment;
  fillZeroDisplacements(&alignment, &segmentAlignment);
  saveDisplacement(&alignment, &segmentAlignment);
  Database::Instance().storeData(m_PayloadName, (TObject*)&alignment, iov);
}

void EKLMDisplacementGeneratorModule::generateFixedSectorDisplacement(
  double deltaU, double deltaV, double deltaGamma)
{
  IntervalOfValidity iov(0, 0, -1, -1);
  EKLMAlignment alignment;
  EKLMSegmentAlignment segmentAlignment;
  KLMAlignmentData sectorAlignment(deltaU, deltaV, 0, 0, 0, deltaGamma);
  int iSection, iLayer, iSector, sector;
  fillZeroDisplacements(&alignment, &segmentAlignment);
  for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        sector = m_GeoDat->sectorNumber(iSection, iLayer, iSector);
        alignment.setModuleAlignment(sector, &sectorAlignment);
      }
    }
  }
  saveDisplacement(&alignment, &segmentAlignment);
  Database::Instance().storeData(m_PayloadName, (TObject*)&alignment, iov);
}

void EKLMDisplacementGeneratorModule::generateRandomDisplacement(
  bool displaceSector, bool displaceSegment)
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
  EKLMAlignment alignment;
  EKLMSegmentAlignment segmentAlignment;
  KLMAlignmentData sectorAlignmentData, segmentAlignmentData, *alignmentData;
  EKLM::AlignmentChecker alignmentChecker(false);
  int iSection, iLayer, iSector, iPlane, iSegment, sector, segment;
  double d;
  fillZeroDisplacements(&alignment, &segmentAlignment);
  for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        if (m_SectorSameDisplacement) {
          if (iSection > 1 || iLayer > 1 || iSector > 1) {
            sector = m_GeoDat->sectorNumber(1, 1, 1);
            alignmentData = const_cast<KLMAlignmentData*>(
                              alignment.getModuleAlignment(sector));
            sector = m_GeoDat->sectorNumber(iSection, iLayer, iSector);
            alignment.setModuleAlignment(sector, alignmentData);
            for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
              for (iSegment = 1; iSegment <= m_GeoDat->getNSegments();
                   iSegment++) {
                segment = m_GeoDat->segmentNumber(1, 1, 1, iPlane, iSegment);
                alignmentData = const_cast<KLMAlignmentData*>(
                                  segmentAlignment.getSegmentAlignment(segment));
                segment = m_GeoDat->segmentNumber(iSection, iLayer, iSector,
                                                  iPlane, iSegment);
                segmentAlignment.setSegmentAlignment(segment, alignmentData);
              }
            }
            continue;
          }
        }
sector:
        if (displaceSector) {
          do {
            if (m_SectorZeroDeltaU)
              d = 0;
            else
              d = gRandom->Uniform(sectorMinDeltaU, sectorMaxDeltaU);
            sectorAlignmentData.setDeltaU(d);
            if (m_SectorZeroDeltaV)
              d = 0;
            else
              d = gRandom->Uniform(sectorMinDeltaV, sectorMaxDeltaV);
            sectorAlignmentData.setDeltaV(d);
            if (m_SectorZeroDeltaGamma)
              d = 0;
            else
              d = gRandom->Uniform(sectorMinDeltaGamma, sectorMaxDeltaGamma);
            sectorAlignmentData.setDeltaGamma(d);
          } while (!alignmentChecker.checkSectorAlignment(
                     iSection, iLayer, iSector, &sectorAlignmentData));
          sector = m_GeoDat->sectorNumber(iSection, iLayer, iSector);
          alignment.setModuleAlignment(sector, &sectorAlignmentData);
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
              segmentAlignment.setSegmentAlignment(
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
  saveDisplacement(&alignment, &segmentAlignment);
  Database::Instance().storeData(m_PayloadName, (TObject*)&alignment, iov);
}

void EKLMDisplacementGeneratorModule::readDisplacementFromROOTFile()
{
  /* cppcheck-suppress variableScope */
  int i, n, iSection, iLayer, iSector, iPlane, iSegment, sector, segment, param;
  float value;
  IntervalOfValidity iov(0, 0, -1, -1);
  TFile* f;
  TTree* t_sector, *t_segment;
  EKLMAlignment alignment;
  EKLMSegmentAlignment segmentAlignment;
  KLMAlignmentData* alignmentData;
  fillZeroDisplacements(&alignment, &segmentAlignment);
  f = new TFile(m_InputFile.c_str());
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
    sector = m_GeoDat->sectorNumber(iSection, iLayer, iSector);
    alignmentData = const_cast<KLMAlignmentData*>(
                      alignment.getModuleAlignment(sector));
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
                      segmentAlignment.getSegmentAlignment(segment));
    switch (param) {
      case 1:
        alignmentData->setDeltaV(value);
        break;
      case 2:
        alignmentData->setDeltaGamma(value);
        break;
    }
  }
  saveDisplacement(&alignment, &segmentAlignment);
  Database::Instance().storeData(m_PayloadName, (TObject*)&alignment, iov);
}

void EKLMDisplacementGeneratorModule::studySectorAlignmentLimits(TFile* f)
{
  const int nPoints = 1000;
  const float maxDeltaU = 5. * Unit::cm;
  const float minDeltaU = -5. * Unit::cm;
  const float maxDeltaV = 5. * Unit::cm;
  const float minDeltaV = -5. * Unit::cm;
  const float maxDeltaGamma = 0.02 * Unit::rad;
  const float minDeltaGamma = -0.02 * Unit::rad;
  float deltaU, deltaV, deltaGamma;
  int i, alignmentStatus, iSection, iLayer, iSector, sector;
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
          sector = m_GeoDat->sectorNumber(iSection, iLayer, iSector);
          alignment.setModuleAlignment(sector, &alignmentDataRandom);
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

void EKLMDisplacementGeneratorModule::studySegmentAlignmentLimits(TFile* f)
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

void EKLMDisplacementGeneratorModule::studyAlignmentLimits()
{
  TFile* f;
  f = new TFile(m_OutputFile.c_str(), "recreate");
  studySectorAlignmentLimits(f);
  studySegmentAlignmentLimits(f);
  delete f;
}

void EKLMDisplacementGeneratorModule::saveDisplacement(
  EKLMAlignment* alignment, EKLMSegmentAlignment* segmentAlignment)
{
  int iSection, iLayer, iSector, iPlane, iSegment, sector, segment, param;
  float value;
  KLMAlignmentData* alignmentData;
  TFile* f;
  TTree* t_sector, *t_segment;
  f = new TFile(m_OutputFile.c_str(), "recreate");
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
        sector = m_GeoDat->sectorNumber(iSection, iLayer, iSector);
        alignmentData = const_cast<KLMAlignmentData*>(
                          alignment->getModuleAlignment(sector));
        param = 1;
        value = alignmentData->getDeltaU();
        t_sector->Fill();
        /* cppcheck-suppress redundantAssignment */
        param = 2;
        /* cppcheck-suppress redundantAssignment */
        value = alignmentData->getDeltaV();
        t_sector->Fill();
        /* cppcheck-suppress redundantAssignment */
        param = 3;
        /* cppcheck-suppress redundantAssignment */
        value = alignmentData->getDeltaGamma();
        t_sector->Fill();
        for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= m_GeoDat->getNSegments(); iSegment++) {
            segment = m_GeoDat->segmentNumber(iSection, iLayer, iSector, iPlane,
                                              iSegment);
            alignmentData = const_cast<KLMAlignmentData*>(
                              segmentAlignment->getSegmentAlignment(segment));
            param = 1;
            value = alignmentData->getDeltaV();
            t_segment->Fill();
            /* cppcheck-suppress redundantAssignment */
            param = 2;
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

void EKLMDisplacementGeneratorModule::initialize()
{
  m_GeoDat = &(EKLM::GeometryData::Instance());
  if (!((m_PayloadName == "EKLMDisplacement") ||
        (m_PayloadName == "EKLMAlignment")))
    B2FATAL("Incorrect payload name. Only 'EKLMDisplacement' and "
            "'EKLMAlignment' are allowed.");
  if (m_Mode == "Zero") {
    generateZeroDisplacement();
  } else if (m_Mode == "FixedSector") {
    generateFixedSectorDisplacement(m_SectorDeltaU, m_SectorDeltaV, m_SectorDeltaGamma);
  } else if (m_Mode == "Random") {
    if (m_RandomDisplacement == "Sector")
      generateRandomDisplacement(true, false);
    else if (m_RandomDisplacement == "Segment")
      generateRandomDisplacement(false, true);
    else if (m_RandomDisplacement == "Both")
      generateRandomDisplacement(true, true);
    else
      B2FATAL("Unknown random displacement mode.");
  } else if (m_Mode == "ROOT") {
    readDisplacementFromROOTFile();
  } else if (m_Mode == "Limits")
    studyAlignmentLimits();
  else
    B2FATAL("Unknown operation mode.");
}

void EKLMDisplacementGeneratorModule::beginRun()
{
}

void EKLMDisplacementGeneratorModule::event()
{
}

void EKLMDisplacementGeneratorModule::endRun()
{
}

void EKLMDisplacementGeneratorModule::terminate()
{
}

