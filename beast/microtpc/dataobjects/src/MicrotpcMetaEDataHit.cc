/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/microtpc/dataobjects/MicrotpcMetaEDataHit.h>

using namespace std;
using namespace Belle2;

std::vector<int> MicrotpcMetaEDataHit::getcolumn() const
{
  std::vector<int> result;
  result.reserve(m_column.size());
  for (int i = 0; i < (int)m_column.size(); i++) {
    result.emplace_back(m_column[i]);
  }
  return result;
}
std::vector<int> MicrotpcMetaEDataHit::getrow() const
{
  std::vector<int> result;
  result.reserve(m_row.size());
  for (int i = 0; i < (int)m_row.size(); i++) {
    result.emplace_back(m_row[i]);
  }
  return result;
}
std::vector<int> MicrotpcMetaEDataHit::getTOT() const
{
  std::vector<int> result;
  result.reserve(m_TOT.size());
  for (int i = 0; i < (int)m_TOT.size(); i++) {
    result.emplace_back(m_TOT[i]);
  }
  return result;
}
std::vector<int> MicrotpcMetaEDataHit::getBCID() const
{
  std::vector<int> result;
  result.reserve(m_BCID.size());
  for (int i = 0; i < (int)m_BCID.size(); i++) {
    result.emplace_back(m_BCID[i]);
  }
  return result;
}

