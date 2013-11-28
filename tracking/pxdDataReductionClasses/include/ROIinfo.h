#pragma once

#include <string>

struct ROIinfo {
  double sigmaSystU;
  double sigmaSystV;
  double numSigmaTotU;
  double numSigmaTotV;
  double maxWidthU;
  double maxWidthV;
  std::string gfTrackCandsColName;
  std::string PXDInterceptListName;
  std::string ROIListName;
  std::string badTracksListName;
  std::string gfTracksListName;
};
