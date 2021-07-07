/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingVXD/analyzingTools/AlgoritmType.h>

using namespace Belle2;

/** setting static dictionary translating type -> string */
std::map<Belle2::AlgoritmType::Type, std::string> Belle2::AlgoritmType::s_fromTypeToString = {
  {Belle2::AlgoritmType::UnknownType, std::string("UnknownType")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmBase, std::string("AnalyzingAlgorithmBase")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValuePX, std::string("AnalyzingAlgorithmValuePX")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValuePY, std::string("AnalyzingAlgorithmValuePY")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValuePZ, std::string("AnalyzingAlgorithmValuePZ")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValuePT, std::string("AnalyzingAlgorithmValuePT")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValueP, std::string("AnalyzingAlgorithmValueP")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValuePTheta, std::string("AnalyzingAlgorithmValuePTheta")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValuePPhi, std::string("AnalyzingAlgorithmValuePPhi")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValueDistSeed2IP, std::string("AnalyzingAlgorithmValueDistSeed2IP")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValueDistSeed2IPZ, std::string("AnalyzingAlgorithmValueDistSeed2IPZ")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValueDistSeed2IPXY, std::string("AnalyzingAlgorithmValueDistSeed2IPXY")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmValueQI, std::string("AnalyzingAlgorithmValueQI")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualPX, std::string("AnalyzingAlgorithmResidualPX")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualPY, std::string("AnalyzingAlgorithmResidualPY")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualPZ, std::string("AnalyzingAlgorithmResidualPZ")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualPT, std::string("AnalyzingAlgorithmResidualPT")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualP, std::string("AnalyzingAlgorithmResidualP")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualPTheta, std::string("AnalyzingAlgorithmResidualPTheta")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualPPhi, std::string("AnalyzingAlgorithmResidualPPhi")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualPAngle, std::string("AnalyzingAlgorithmResidualPAngle")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualPTAngle, std::string("AnalyzingAlgorithmResidualPTAngle")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualPosition, std::string("AnalyzingAlgorithmResidualPosition")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmResidualPositionXY, std::string("AnalyzingAlgorithmResidualPositionXY")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmLostUClusters, std::string("AnalyzingAlgorithmLostUClusters")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmLostVClusters, std::string("AnalyzingAlgorithmLostVClusters")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmTotalUClusters, std::string("AnalyzingAlgorithmTotalUClusters")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmTotalVClusters, std::string("AnalyzingAlgorithmTotalVClusters")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmLostUEDep, std::string("AnalyzingAlgorithmLostUEDep")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmLostVEDep, std::string("AnalyzingAlgorithmLostVEDep")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmTotalUEDep, std::string("AnalyzingAlgorithmTotalUEDep")},
  {Belle2::AlgoritmType::AnalyzingAlgorithmTotalVEDep, std::string("AnalyzingAlgorithmTotalVEDep")},
  {Belle2::AlgoritmType::NTypes, std::string("NTypes")}
};


/** setting static dictionary translating string -> type */
std::map<std::string, Belle2::AlgoritmType::Type> Belle2::AlgoritmType::s_fromStringToType = {
  {std::string("UnknownType"), Belle2::AlgoritmType::UnknownType},
  {std::string("AnalyzingAlgorithmBase"), Belle2::AlgoritmType::AnalyzingAlgorithmBase},
  {std::string("AnalyzingAlgorithmValuePX"), Belle2::AlgoritmType::AnalyzingAlgorithmValuePX},
  {std::string("AnalyzingAlgorithmValuePY"), Belle2::AlgoritmType::AnalyzingAlgorithmValuePY},
  {std::string("AnalyzingAlgorithmValuePZ"), Belle2::AlgoritmType::AnalyzingAlgorithmValuePZ},
  {std::string("AnalyzingAlgorithmValuePT"), Belle2::AlgoritmType::AnalyzingAlgorithmValuePT},
  {std::string("AnalyzingAlgorithmValueP"), Belle2::AlgoritmType::AnalyzingAlgorithmValueP},
  {std::string("AnalyzingAlgorithmValuePTheta"), Belle2::AlgoritmType::AnalyzingAlgorithmValuePTheta},
  {std::string("AnalyzingAlgorithmValuePPhi"), Belle2::AlgoritmType::AnalyzingAlgorithmValuePPhi},
  {std::string("AnalyzingAlgorithmValueDistSeed2IP"), Belle2::AlgoritmType::AnalyzingAlgorithmValueDistSeed2IP},
  {std::string("AnalyzingAlgorithmValueDistSeed2IPZ"), Belle2::AlgoritmType::AnalyzingAlgorithmValueDistSeed2IPZ},
  {std::string("AnalyzingAlgorithmValueDistSeed2IPXY"), Belle2::AlgoritmType::AnalyzingAlgorithmValueDistSeed2IPXY},
  {std::string("AnalyzingAlgorithmValueQI"), Belle2::AlgoritmType::AnalyzingAlgorithmValueQI},
  {std::string("AnalyzingAlgorithmResidualPX"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualPX},
  {std::string("AnalyzingAlgorithmResidualPY"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualPY},
  {std::string("AnalyzingAlgorithmResidualPZ"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualPZ},
  {std::string("AnalyzingAlgorithmResidualPT"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualPT},
  {std::string("AnalyzingAlgorithmResidualP"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualP},
  {std::string("AnalyzingAlgorithmResidualPTheta"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualPTheta},
  {std::string("AnalyzingAlgorithmResidualPPhi"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualPPhi},
  {std::string("AnalyzingAlgorithmResidualPAngle"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualPAngle},
  {std::string("AnalyzingAlgorithmResidualPTAngle"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualPTAngle},
  {std::string("AnalyzingAlgorithmResidualPosition"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualPosition},
  {std::string("AnalyzingAlgorithmResidualPositionXY"), Belle2::AlgoritmType::AnalyzingAlgorithmResidualPositionXY},
  {std::string("AnalyzingAlgorithmLostUClusters"), Belle2::AlgoritmType::AnalyzingAlgorithmLostUClusters},
  {std::string("AnalyzingAlgorithmLostVClusters"), Belle2::AlgoritmType::AnalyzingAlgorithmLostVClusters},
  {std::string("AnalyzingAlgorithmTotalUClusters"), Belle2::AlgoritmType::AnalyzingAlgorithmTotalUClusters},
  {std::string("AnalyzingAlgorithmTotalVClusters"), Belle2::AlgoritmType::AnalyzingAlgorithmTotalVClusters},
  {std::string("AnalyzingAlgorithmLostUEDep"), Belle2::AlgoritmType::AnalyzingAlgorithmLostUEDep},
  {std::string("AnalyzingAlgorithmLostVEDep"), Belle2::AlgoritmType::AnalyzingAlgorithmLostVEDep},
  {std::string("AnalyzingAlgorithmTotalUEDep"), Belle2::AlgoritmType::AnalyzingAlgorithmTotalUEDep},
  {std::string("AnalyzingAlgorithmTotalVEDep"), Belle2::AlgoritmType::AnalyzingAlgorithmTotalVEDep},
  {std::string("NTypes"), Belle2::AlgoritmType::NTypes}
};
