/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/GeometryData2.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

static bool compareLength(double a, double b)
{
  return a < b;
}

EKLM::GeometryData::GeometryData(bool global) : m_TransformData(global)
{
  m_nStrip = -1;
  m_nStripDifferent = -1;
  m_StripLenToAll = NULL;
  m_StripAllToLen = NULL;
}

EKLM::GeometryData::~GeometryData()
{
  if (m_StripLenToAll != NULL)
    free(m_StripLenToAll);
  if (m_StripAllToLen != NULL)
    free(m_StripAllToLen);
}

void EKLM::GeometryData::read()
{
  const char err[] = "Strip sorting algorithm error.";
  int i;
  char str[32];
  double l;
  double* stripLen;
  std::vector<double> strips;
  std::vector<double>::iterator it;
  std::map<double, int> mapLengthStrip;
  std::map<double, int> mapLengthStrip2;
  std::map<double, int>::iterator itm;
  /* Fill strip data. */
  GearDir gd2("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/Endcap/"
              "Layer/Sector/Plane/Strips");
  m_nStrip = gd2.getNumberNodes("Strip");
  stripLen = (double*)malloc(m_nStrip * sizeof(double));
  if (stripLen == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < m_nStrip; i++) {
    GearDir gds(gd2);
    snprintf(str, 32, "/Strip[%d]", i + 1);
    gds.append(str);
    stripLen[i] = gds.getLength("Length");
    strips.push_back(stripLen[i]);
    mapLengthStrip.insert(std::pair<double, int>(stripLen[i], i));
  }
  sort(strips.begin(), strips.end(), compareLength);
  l = strips[0];
  m_nStripDifferent = 1;
  for (it = strips.begin(); it != strips.end(); ++it) {
    if ((*it) != l) {
      l = (*it);
      m_nStripDifferent++;
    }
  }
  m_StripLenToAll = (int*)malloc(m_nStripDifferent * sizeof(int));
  if (m_StripLenToAll == NULL)
    B2FATAL(MemErr);
  i = 0;
  l = strips[0];
  itm = mapLengthStrip.find(l);
  if (itm == mapLengthStrip.end())
    B2FATAL(err);
  m_StripLenToAll[i] = itm->second;
  mapLengthStrip2.insert(std::pair<double, int>(l, i));
  for (it = strips.begin(); it != strips.end(); ++it) {
    if ((*it) != l) {
      l = (*it);
      i++;
      itm = mapLengthStrip.find(l);
      if (itm == mapLengthStrip.end())
        B2FATAL(err);
      m_StripLenToAll[i] = itm->second;
      mapLengthStrip2.insert(std::pair<double, int>(l, i));
    }
  }
  m_StripAllToLen = (int*)malloc(m_nStrip * sizeof(int));
  if (m_StripAllToLen == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < m_nStrip; i++) {
    itm = mapLengthStrip2.find(stripLen[i]);
    if (itm == mapLengthStrip2.end())
      B2FATAL(err);
    m_StripAllToLen[i] = itm->second;
  }
  free(stripLen);
}

int EKLM::GeometryData::getStripLengthIndex(int positionIndex)
{
  return m_StripAllToLen[positionIndex];
}

int EKLM::GeometryData::getStripPositionIndex(int lengthIndex)
{
  return m_StripLenToAll[lengthIndex];
}

int EKLM::GeometryData::getNStripsDifferentLength()
{
  return m_nStripDifferent;
}

