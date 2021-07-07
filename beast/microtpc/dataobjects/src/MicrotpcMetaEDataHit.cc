/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

