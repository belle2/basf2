/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/calibration/EKLMAlignmentAlongStripsAlgorithm.h>

/* KLM headers. */
#include <klm/eklm/geometry/GeometryData.h>

/* ROOT headers. */
#include <TFile.h>
#include <TTree.h>

using namespace Belle2;

static bool compareSegmentSignificance(
  const std::pair<int, double>& first, const std::pair<int, double>& second)
{
  return first.second > second.second;
}

EKLMAlignmentAlongStripsAlgorithm::EKLMAlignmentAlongStripsAlgorithm() :
  CalibrationAlgorithm("EKLMAlignmentAlongStripsCollector"),
  m_OutputFile("")
{
}

EKLMAlignmentAlongStripsAlgorithm::~EKLMAlignmentAlongStripsAlgorithm()
{
}

CalibrationAlgorithm::EResult EKLMAlignmentAlongStripsAlgorithm::calibrate()
{
  /* cppcheck-suppress variableScope */
  int i, n, nSections, nSectors, nLayers, nDetectorLayers, nPlanes, nSegments;
  /* cppcheck-suppress variableScope */
  int segment, segmentGlobal;
  std::vector<std::pair<int, double> > segmentSignificance;
  std::vector<std::pair<int, double> >::iterator it;
  int iSection, iLayer, iSector, iPlane, iSegment;
  int**** *nHits, *** *averageHits;
  double nHitsSegment, nHitsAverage, nSigma;
  bool found;
  const EKLMElementNumbers* elementNumbers = &(EKLMElementNumbers::Instance());
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  struct Event* event = nullptr;
  std::shared_ptr<TTree> t_in = getObjectPtr<TTree>("calibration_data");
  t_in->SetBranchAddress("event", &event);
  nSections = geoDat->getNSections();
  nSectors = geoDat->getNSectors();
  nLayers = geoDat->getNLayers();
  nPlanes = geoDat->getNPlanes();
  nSegments = geoDat->getNSegments();
  nHits = new int**** [nSections];
  averageHits = new int** *[nSections];
  for (iSection = 0; iSection < nSections; iSection++) {
    nHits[iSection] = new int** *[nLayers];
    averageHits[iSection] = new int** [nLayers];
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      nHits[iSection][iLayer] = new int** [nSectors];
      averageHits[iSection][iLayer] = new int* [nPlanes];
      for (iPlane = 0; iPlane < nPlanes; iPlane++) {
        averageHits[iSection][iLayer][iPlane] = new int[nSegments];
        for (iSegment = 0; iSegment < nSegments; iSegment++) {
          averageHits[iSection][iLayer][iPlane][iSegment] = 0;
        }
      }
      for (iSector = 0; iSector < nSectors; iSector++) {
        nHits[iSection][iLayer][iSector] = new int* [nPlanes];
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          nHits[iSection][iLayer][iSector][iPlane] = new int[nSegments];
          for (iSegment = 0; iSegment < nSegments; iSegment++) {
            nHits[iSection][iLayer][iSector][iPlane][iSegment] = 0;
          }
        }
      }
    }
  }
  n = t_in->GetEntries();
  for (i = 0; i < n; i++) {
    t_in->GetEntry(i);
    segment = (event->strip - 1) / elementNumbers->getNStripsSegment();
    nHits[event->section - 1][event->layer - 1][event->sector - 1]
    [event->plane - 1][segment]++;
    averageHits[event->section - 1][event->layer - 1]
    [getAveragedPlane(event->sector, event->plane)][segment]++;
  }
  for (iSection = 0; iSection < nSections; iSection++) {
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      for (iPlane = 0; iPlane < nPlanes; iPlane++) {
        for (iSegment = 0; iSegment < nSegments; iSegment++) {
          averageHits[iSection][iLayer][iPlane][iSegment] /= nSectors;
        }
      }
    }
  }
  for (iSection = 0; iSection < nSections; iSection++) {
    nDetectorLayers = geoDat->getNDetectorLayers(iSection + 1);
    for (iLayer = 0; iLayer < nDetectorLayers; iLayer++) {
      for (iSector = 0; iSector < nSectors; iSector++) {
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          for (iSegment = 0; iSegment < nSegments; iSegment++) {
            segmentGlobal = elementNumbers->segmentNumber(
                              iSection + 1, iLayer + 1, iSector + 1, iPlane + 1,
                              iSegment + 1);
            nHitsSegment = nHits[iSection][iLayer][iSector][iPlane][iSegment];
            nHitsAverage = averageHits[iSection][iLayer]
                           [getAveragedPlane(iSector + 1, iPlane + 1)]
                           [iSegment];
            nSigma = (nHitsSegment - nHitsAverage) /
                     sqrt(nHitsSegment + nHitsAverage -
                          2.0 / nSectors * sqrt(nHitsSegment * nHitsAverage));
            segmentSignificance.push_back(
              std::pair<int, double>(segmentGlobal, nSigma));
          }
        }
      }
    }
  }
  sort(segmentSignificance.begin(), segmentSignificance.end(),
       compareSegmentSignificance);
  printf("Checking for significantly shifted segments:\n");
  found = false;
  for (it = segmentSignificance.begin(); it != segmentSignificance.end();
       ++it) {
    if (it->second < 3.0)
      break;
    elementNumbers->segmentNumberToElementNumbers(
      it->first, &iSection, &iLayer, &iSector, &iPlane, &iSegment);
    printf("Segment %d (section %d, layer %d, sector %d, plane %d, segment %d):"
           " %.1f sigma\n", it->first, iSection, iLayer, iSector, iPlane,
           iSegment, it->second);
    found = true;
  }
  if (!found)
    printf("none found.\n");
  for (iSection = 0; iSection < nSections; iSection++) {
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      for (iPlane = 0; iPlane < nPlanes; iPlane++) {
        delete[] averageHits[iSection][iLayer][iPlane];
      }
      for (iSector = 0; iSector < nSectors; iSector++) {
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          delete[] nHits[iSection][iLayer][iSector][iPlane];
        }
        delete[] nHits[iSection][iLayer][iSector];
      }
      delete[] nHits[iSection][iLayer];
      delete[] averageHits[iSection][iLayer];
    }
    delete[] nHits[iSection];
    delete[] averageHits[iSection];
  }
  delete[] nHits;
  delete[] averageHits;
  if (m_OutputFile != "") {
    TFile* f_out = new TFile(m_OutputFile.c_str(), "recreate");
    TTree* t_out = new TTree("tree", "");
    t_out->Branch("event", event);
    n = t_in->GetEntries();
    for (i = 0; i < n; i++) {
      t_in->GetEntry(i);
      t_out->Fill();
    }
    f_out->cd();
    t_out->Write();
    delete t_out;
    delete f_out;
  }
  return CalibrationAlgorithm::c_OK;
}

void EKLMAlignmentAlongStripsAlgorithm::setOutputFile(const char* outputFile)
{
  m_OutputFile = outputFile;
}

int EKLMAlignmentAlongStripsAlgorithm::
getAveragedPlane(int sector, int plane) const
{
  if (sector == 1 || sector == 4)
    return plane - 1;
  return 2 - plane;
}

