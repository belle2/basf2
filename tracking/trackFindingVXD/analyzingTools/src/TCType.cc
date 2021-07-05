/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingVXD/analyzingTools/TCType.h>

using namespace Belle2;

/** setting static dictionary translating type -> string */
std::map<Belle2::TCType::Type, std::string> Belle2::TCType::s_fromTypeToString = {
  {Belle2::TCType::UnknownType, std::string("UnknownType")},
  {Belle2::TCType::Unclassified, std::string("Unclassified")},
  {Belle2::TCType::Lost, std::string("Lost")},
  {Belle2::TCType::RefOutlier, std::string("RefOutlier")},
  {Belle2::TCType::Ghost, std::string("Ghost")},
  {Belle2::TCType::SmallStump, std::string("SmallStump")},
  {Belle2::TCType::Clone, std::string("Clone")},
  {Belle2::TCType::Contaminated, std::string("Contaminated")},
  {Belle2::TCType::Clean, std::string("Clean")},
  {Belle2::TCType::Perfect, std::string("Perfect")},
  {Belle2::TCType::AllTCTypes, std::string("AllTCTypes")},
  {Belle2::TCType::Reference, std::string("Reference")},
  {Belle2::TCType::NTypes, std::string("NTypes")}
};


/** setting static dictionary translating string -> type */
std::map<std::string, Belle2::TCType::Type> Belle2::TCType::s_fromStringToType = {
  {std::string("UnknownType"), Belle2::TCType::UnknownType},
  {std::string("Unclassified"), Belle2::TCType::Unclassified},
  {std::string("Lost"), Belle2::TCType::Lost},
  {std::string("RefOutlier"), Belle2::TCType::RefOutlier},
  {std::string("Ghost"), Belle2::TCType::Ghost},
  {std::string("SmallStump"), Belle2::TCType::SmallStump},
  {std::string("Clone"), Belle2::TCType::Clone},
  {std::string("Contaminated"), Belle2::TCType::Contaminated},
  {std::string("Clean"), Belle2::TCType::Clean},
  {std::string("Perfect"), Belle2::TCType::Perfect},
  {std::string("AllTCTypes"), Belle2::TCType::AllTCTypes},
  {std::string("Reference"), Belle2::TCType::Reference},
  {std::string("NTypes"), Belle2::TCType::NTypes}
};