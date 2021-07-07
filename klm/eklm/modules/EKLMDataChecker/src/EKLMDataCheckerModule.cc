/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/modules/EKLMDataChecker/EKLMDataCheckerModule.h>

/* C++ headers. */
#include <algorithm>

using namespace Belle2;

REG_MODULE(EKLMDataChecker)

EKLMDataCheckerModule::EKLMDataCheckerModule() :
  Module(),
  m_ElementNumbers(&(EKLMElementNumbers::Instance())),
  m_GeoDat(nullptr)
{
  setDescription("EKLM data checker module.");
}

EKLMDataCheckerModule::~EKLMDataCheckerModule()
{
}

void EKLMDataCheckerModule::initialize()
{
  m_Digits.isRequired();
  m_GeoDat = &(EKLM::GeometryData::Instance());
}

void EKLMDataCheckerModule::beginRun()
{
}

void EKLMDataCheckerModule::event()
{
  const uint16_t c_ChargeError = 0x0FFF;
  int i, n, strip;
  std::map<int, StripData>::iterator it;
  StripData data;
  n = m_Digits.getEntries();
  for (i = 0; i < n; i++) {
    KLMDigit* eklmDigit = m_Digits[i];
    if (eklmDigit->getSubdetector() != KLMElementNumbers::c_EKLM)
      continue;
    strip = m_ElementNumbers->stripNumber(
              eklmDigit->getSection(), eklmDigit->getLayer(),
              eklmDigit->getSector(), eklmDigit->getPlane(),
              eklmDigit->getStrip());
    it = m_StripDataMap.find(strip);
    if (it == m_StripDataMap.end()) {
      data.strip = strip;
      data.nDigits = 1;
      data.nBadDigits = (eklmDigit->getCharge() == c_ChargeError) ? 1 : 0;
      m_StripDataMap.insert(std::pair<int, StripData>(strip, data));
    } else {
      it->second.nDigits++;
      if (eklmDigit->getCharge() == c_ChargeError)
        it->second.nBadDigits++;
    }
  }
}

void EKLMDataCheckerModule::endRun()
{
}

static bool compareBadDigitRate(EKLMDataCheckerModule::StripData& dat1,
                                EKLMDataCheckerModule::StripData& dat2)
{
  return (double(dat1.nBadDigits) / dat1.nDigits) >
         (double(dat2.nBadDigits) / dat2.nDigits);
}

static bool compareStripNumber(const EKLMDataCheckerModule::StripData& dat1,
                               const EKLMDataCheckerModule::StripData& dat2)
{
  return dat1.strip < dat2.strip;
}

void EKLMDataCheckerModule::terminate()
{
  int section, layer, sector, plane, strip, stripGlobal;
  std::map<int, StripData>::iterator it;
  std::vector<StripData> stripDataVector;
  std::vector<StripData>::iterator it2, it3, it4;
  for (it = m_StripDataMap.begin(); it != m_StripDataMap.end(); ++it)
    stripDataVector.push_back(it->second);
  sort(stripDataVector.begin(), stripDataVector.end(), compareBadDigitRate);
  printf("EKLM data checker report.\n"
         "Strips with readout errors sorted by error rate:\n");
  it2 = stripDataVector.begin();
  while (it2 != stripDataVector.end()) {
    if (it2->nBadDigits == 0)
      break;
    it3 = it2;
    while (it3 != stripDataVector.end()) {
      if (it3->nBadDigits != it2->nBadDigits || it3->nDigits != it2->nDigits)
        break;
      ++it3;
    }
    sort(it2, it3, compareStripNumber);
    for (it4 = it2; it4 != it3; ++it4) {
      m_ElementNumbers->stripNumberToElementNumbers(
        it4->strip, &section, &layer, &sector, &plane, &strip);
      printf("Section %d, layer %d, sector %d, plane %d, strip %d: %.1f%% "
             "(%d/%d)\n",
             section, layer, sector, plane, strip,
             float(it4->nBadDigits) / it4->nDigits * 100,
             it4->nBadDigits, it4->nDigits);
    }
    it2 = it3;
  }
  printf("Strips with no data collected:\n");
  for (section = 1; section <= m_GeoDat->getNSections(); section++) {
    for (layer = 1; layer <= m_GeoDat->getNDetectorLayers(section); layer++) {
      for (sector = 1; sector <= m_GeoDat->getNSectors(); sector++) {
        for (plane = 1; plane <= m_GeoDat->getNPlanes(); plane++) {
          for (strip = 1; strip <= m_GeoDat->getNStrips(); strip++) {
            stripGlobal = m_ElementNumbers->stripNumber(
                            section, layer, sector, plane, strip);
            it = m_StripDataMap.find(stripGlobal);
            if (it == m_StripDataMap.end()) {
              printf("Section %d, layer %d, sector %d, plane %d, strip %d.\n",
                     section, layer, sector, plane, strip);
            }
          }
        }
      }
    }
  }
}

