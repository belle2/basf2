/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
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
#include <eklm/calibration/EKLMAlignmentAlongStripsAlgorithm.h>
#include <eklm/geometry/GeometryData.h>

using namespace Belle2;

static bool compareSegmentSignificance(
  std::pair<int, double>& first, std::pair<int, double>& second)
{
  return first.second > second.second;
}

EKLMAlignmentAlongStripsAlgorithm::EKLMAlignmentAlongStripsAlgorithm() :
  CalibrationAlgorithm("EKLMAlignmentAlongStripsCollector")
{
  m_OutputFile = "";
}

EKLMAlignmentAlongStripsAlgorithm::~EKLMAlignmentAlongStripsAlgorithm()
{
}

CalibrationAlgorithm::EResult EKLMAlignmentAlongStripsAlgorithm::calibrate()
{
  int i, n, nEndcaps, nSectors, nLayers, nDetectorLayers, nPlanes, nSegments;
  int segment, segmentGlobal;
  std::vector<std::pair<int, double> > segmentSignificance;
  std::vector<std::pair<int, double> >::iterator it;
  int iEndcap, iLayer, iSector, iPlane, iSegment;
  int**** *nHits, *** *averageHits;
  double nHitsSegment, nHitsAverage, nSigma;
  bool found;
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  struct Event* event = NULL;
  TFile* f_out;
  TTree* t_out;
  std::shared_ptr<TTree> t_in = getObjectPtr<TTree>("calibration_data");
  t_in->SetBranchAddress("event", &event);
  nEndcaps = geoDat->getNEndcaps();
  nSectors = geoDat->getNSectors();
  nLayers = geoDat->getNLayers();
  nPlanes = geoDat->getNPlanes();
  nSegments = geoDat->getNSegments();
  nHits = new int**** [nEndcaps];
  averageHits = new int** *[nEndcaps];
  for (iEndcap = 0; iEndcap < nEndcaps; iEndcap++) {
    nHits[iEndcap] = new int** *[nLayers];
    averageHits[iEndcap] = new int** [nLayers];
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      nHits[iEndcap][iLayer] = new int** [nSectors];
      averageHits[iEndcap][iLayer] = new int* [nPlanes];
      for (iPlane = 0; iPlane < nPlanes; iPlane++) {
        averageHits[iEndcap][iLayer][iPlane] = new int[nSegments];
        for (iSegment = 0; iSegment < nSegments; iSegment++) {
          averageHits[iEndcap][iLayer][iPlane][iSegment] = 0;
        }
      }
      for (iSector = 0; iSector < nSectors; iSector++) {
        nHits[iEndcap][iLayer][iSector] = new int* [nPlanes];
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          nHits[iEndcap][iLayer][iSector][iPlane] = new int[nSegments];
          for (iSegment = 0; iSegment < nSegments; iSegment++) {
            nHits[iEndcap][iLayer][iSector][iPlane][iSegment] = 0;
          }
        }
      }
    }
  }
  n = t_in->GetEntries();
  for (i = 0; i < n; i++) {
    t_in->GetEntry(i);
    segment = (event->strip - 1) / geoDat->getNStripsSegment();
    nHits[event->endcap - 1][event->layer - 1][event->sector - 1]
    [event->plane - 1][segment]++;
    averageHits[event->endcap - 1][event->layer - 1]
    [getAveragedPlane(event->sector, event->plane)][segment]++;
  }
  for (iEndcap = 0; iEndcap < nEndcaps; iEndcap++) {
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      for (iPlane = 0; iPlane < nPlanes; iPlane++) {
        for (iSegment = 0; iSegment < nSegments; iSegment++) {
          averageHits[iEndcap][iLayer][iPlane][iSegment] /= nSectors;
        }
      }
    }
  }
  for (iEndcap = 0; iEndcap < nEndcaps; iEndcap++) {
    nDetectorLayers = geoDat->getNDetectorLayers(iEndcap + 1);
    for (iLayer = 0; iLayer < nDetectorLayers; iLayer++) {
      for (iSector = 0; iSector < nSectors; iSector++) {
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          for (iSegment = 0; iSegment < nSegments; iSegment++) {
            segmentGlobal = geoDat->segmentNumber(iEndcap + 1, iLayer + 1,
                                                  iSector + 1, iPlane + 1,
                                                  iSegment + 1);
            nHitsSegment = nHits[iEndcap][iLayer][iSector][iPlane][iSegment];
            nHitsAverage = averageHits[iEndcap][iLayer]
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
    geoDat->segmentNumberToElementNumbers(it->first, &iEndcap, &iLayer,
                                          &iSector, &iPlane, &iSegment);
    printf("Segment %d (endcap %d, layer %d, sector %d, plane %d, segment %d):"
           " %.1f sigma\n", it->first, iEndcap, iLayer, iSector, iPlane,
           iSegment, it->second);
    found = true;
  }
  if (!found)
    printf("none found.\n");
  for (iEndcap = 0; iEndcap < nEndcaps; iEndcap++) {
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      for (iPlane = 0; iPlane < nPlanes; iPlane++) {
        delete[] averageHits[iEndcap][iLayer][iPlane];
      }
      for (iSector = 0; iSector < nSectors; iSector++) {
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          delete[] nHits[iEndcap][iLayer][iSector][iPlane];
        }
        delete[] nHits[iEndcap][iLayer][iSector];
      }
      delete[] nHits[iEndcap][iLayer];
      delete[] averageHits[iEndcap][iLayer];
    }
    delete[] nHits[iEndcap];
    delete[] averageHits[iEndcap];
  }
  delete[] nHits;
  delete[] averageHits;
  if (m_OutputFile != "") {
    f_out = new TFile(m_OutputFile.c_str(), "recreate");
    t_out = new TTree("tree", "");
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

