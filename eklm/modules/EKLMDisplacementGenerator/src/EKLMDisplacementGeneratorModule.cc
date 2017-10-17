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
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/geometry/AlignmentChecker.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/modules/EKLMDisplacementGenerator/EKLMDisplacementGeneratorModule.h>

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
  addParam("SectorZeroDx", m_SectorZeroDx, "Fix sector dx at 0.", false);
  addParam("SectorZeroDy", m_SectorZeroDy, "Fix sector dy at 0.", false);
  addParam("SectorZeroDalpha", m_SectorZeroDalpha, "Fix sector dalpha at 0.",
           false);
  addParam("SectorDx", m_SectorDx, "Sector dx.", 0.);
  addParam("SectorDy", m_SectorDy, "Sector dy.", 0.);
  addParam("SectorDalpha", m_SectorDalpha, "Sector dalpha.", 0.);
  addParam("InputFile", m_InputFile, "Input file (for mode == 'ROOT' only).",
           std::string(""));
  addParam("OutputFile", m_OutputFile, "Output file.",
           std::string("EKLMDisplacement.root"));
  setPropertyFlags(c_ParallelProcessingCertified);
  m_GeoDat = NULL;
}

EKLMDisplacementGeneratorModule::~EKLMDisplacementGeneratorModule()
{
}

void EKLMDisplacementGeneratorModule::fillZeroDisplacements(EKLMAlignment* alignment)
{
  EKLMAlignmentData alignmentData(0., 0., 0.);
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  int iEndcap, iLayer, iSector, iPlane, iSegment, segment, sector;
  for (iEndcap = 1; iEndcap <= geoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= geoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= geoDat->getNSectors(); iSector++) {
        sector = geoDat->sectorNumber(iEndcap, iLayer, iSector);
        alignment->setSectorAlignment(sector, &alignmentData);
        for (iPlane = 1; iPlane <= geoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= geoDat->getNSegments(); iSegment++) {
            segment = geoDat->segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                            iSegment);
            alignment->setSegmentAlignment(segment, &alignmentData);
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
  fillZeroDisplacements(&alignment);
  saveDisplacement(&alignment);
  Database::Instance().storeData(m_PayloadName, (TObject*)&alignment, iov);
}

void EKLMDisplacementGeneratorModule::generateFixedSectorDisplacement(
  double dx, double dy, double dalpha)
{
  IntervalOfValidity iov(0, 0, -1, -1);
  EKLMAlignment alignment;
  EKLMAlignmentData sectorAlignment(dx, dy, dalpha);
  int iEndcap, iLayer, iSector, sector;
  fillZeroDisplacements(&alignment);
  for (iEndcap = 1; iEndcap <= m_GeoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        sector = m_GeoDat->sectorNumber(iEndcap, iLayer, iSector);
        alignment.setSectorAlignment(sector, &sectorAlignment);
      }
    }
  }
  saveDisplacement(&alignment);
  Database::Instance().storeData(m_PayloadName, (TObject*)&alignment, iov);
}

void EKLMDisplacementGeneratorModule::generateRandomDisplacement(
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
  double d;
  fillZeroDisplacements(&alignment);
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
            if (m_SectorZeroDx)
              d = 0;
            else
              d = gRandom->Uniform(sectorMinDx, sectorMaxDx);
            sectorAlignment.setDx(d);
            if (m_SectorZeroDy)
              d = 0;
            else
              d = gRandom->Uniform(sectorMinDy, sectorMaxDy);
            sectorAlignment.setDy(d);
            if (m_SectorZeroDalpha)
              d = 0;
            else
              d = gRandom->Uniform(sectorMinDalpha, sectorMaxDalpha);
            sectorAlignment.setDalpha(d);
          } while (!alignmentChecker.checkSectorAlignment(
                     iEndcap, iLayer, iSector, &sectorAlignment));
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
                         iEndcap, iLayer, iSector, iPlane, iSegment,
                         &sectorAlignment, &segmentAlignment, false));
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
                    iEndcap, iLayer, iSector, iPlane, iSegment,
                    &sectorAlignment, &segmentAlignment, false))
                goto sector;
            }
          }
        }
      }
    }
  }
  saveDisplacement(&alignment);
  Database::Instance().storeData(m_PayloadName, (TObject*)&alignment, iov);
}

void EKLMDisplacementGeneratorModule::readDisplacementFromROOTFile()
{
  int i, n, iEndcap, iLayer, iSector, iPlane, iSegment, sector, segment, param;
  float value;
  IntervalOfValidity iov(0, 0, -1, -1);
  TFile* f;
  TTree* t_sector, *t_segment;
  EKLMAlignment alignment;
  EKLMAlignmentData* alignmentData;
  fillZeroDisplacements(&alignment);
  f = new TFile(m_InputFile.c_str());
  t_sector = (TTree*)f->Get("eklm_sector");
  t_sector->SetBranchAddress("endcap", &iEndcap);
  t_sector->SetBranchAddress("layer", &iLayer);
  t_sector->SetBranchAddress("sector", &iSector);
  t_sector->SetBranchAddress("param", &param);
  t_sector->SetBranchAddress("value", &value);
  t_segment = (TTree*)f->Get("eklm_segment");
  t_segment->SetBranchAddress("endcap", &iEndcap);
  t_segment->SetBranchAddress("layer", &iLayer);
  t_segment->SetBranchAddress("sector", &iSector);
  t_segment->SetBranchAddress("plane", &iPlane);
  t_segment->SetBranchAddress("segment", &iSegment);
  t_segment->SetBranchAddress("param", &param);
  t_segment->SetBranchAddress("value", &value);
  n = t_sector->GetEntries();
  for (i = 0; i < n; i++) {
    t_sector->GetEntry(i);
    sector = m_GeoDat->sectorNumber(iEndcap, iLayer, iSector);
    alignmentData = alignment.getSectorAlignment(sector);
    switch (param) {
      case 1:
        alignmentData->setDx(value);
        break;
      case 2:
        alignmentData->setDy(value);
        break;
      case 3:
        alignmentData->setDalpha(value);
        break;
    }
  }
  n = t_segment->GetEntries();
  for (i = 0; i < n; i++) {
    t_segment->GetEntry(i);
    segment = m_GeoDat->segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                      iSegment);
    alignmentData = alignment.getSegmentAlignment(segment);
    switch (param) {
      case 1:
        alignmentData->setDy(value);
        break;
      case 2:
        alignmentData->setDalpha(value);
        break;
    }
  }
  saveDisplacement(&alignment);
  Database::Instance().storeData(m_PayloadName, (TObject*)&alignment, iov);
}

void EKLMDisplacementGeneratorModule::studySectorAlignmentLimits(TFile* f)
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
    fillZeroDisplacements(&alignment);
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

void EKLMDisplacementGeneratorModule::studySegmentAlignmentLimits(TFile* f)
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
        fillZeroDisplacements(&alignment);
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

void EKLMDisplacementGeneratorModule::studyAlignmentLimits()
{
  TFile* f;
  f = new TFile(m_OutputFile.c_str(), "recreate");
  studySectorAlignmentLimits(f);
  studySegmentAlignmentLimits(f);
  delete f;
}

void EKLMDisplacementGeneratorModule::saveDisplacement(EKLMAlignment* alignment)
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
    generateFixedSectorDisplacement(m_SectorDx, m_SectorDy, m_SectorDalpha);
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

