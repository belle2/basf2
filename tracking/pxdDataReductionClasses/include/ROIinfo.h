#pragma once

#include <string>

/** ROIinfo contains the parameters that can be changed by the user in the python script.
 */

struct ROIinfo {
  double sigmaSystU; /**< fixed width to add in quadrature to the extrapolation error and obtain the ROI U width */
  double sigmaSystV; /**< fixed width to add in quadrature to the extrapolation error and obtain the ROI V width */
  double numSigmaTotU; /**< number of sigma (stat+syst) determining the U width of the ROI*/
  double numSigmaTotV; /**< number of sigma (stat+syst) determining the U width of the ROI*/
  double maxWidthU; /**< maximum U width of the ROI */
  double maxWidthV; /**< maximum U width of the ROI */
  std::string gfTrackCandsColName; /**< TrackCand list name*/
  std::string PXDInterceptListName; /**< Intercept list name*/
  std::string ROIListName; /**< ROI list name*/
  std::string recoTracksListName; /**< track list name*/
};
