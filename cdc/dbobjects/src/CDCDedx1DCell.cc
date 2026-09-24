/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/dbobjects/CDCDedx1DCell.h>

using namespace Belle2;


CDCDedx1DCell::CDCDedx1DCell(short version,
                             const std::vector<std::vector<double>>& groupGains,
                             const std::vector<unsigned int>& layerToGroup)
  : m_version(version),
    m_onedgains(groupGains),
    m_layerToGroup(layerToGroup)
{
  // 1. groups exist
  if (groupGains.empty()) {
    B2ERROR("CDCDedx1DCell: groupGains is empty.");
    return;
  }

  // 2. mapping exists
  if (layerToGroup.empty()) {
    B2ERROR("CDCDedx1DCell: layerToGroup is empty.");
    return;
  }

  // 3. check groups not empty
  for (size_t g = 0; g < groupGains.size(); ++g) {
    if (groupGains[g].empty()) {
      B2ERROR("CDCDedx1DCell: group " << g << " is empty.");
      return;
    }
  }

  // 4. check Layer -> group mapping
  for (size_t layer = 0; layer < layerToGroup.size(); ++layer) {
    if (layerToGroup[layer] >= groupGains.size()) {
      B2ERROR("CDCDedx1DCell: layer " << layer
              << " refers to invalid group "
              << layerToGroup[layer]);
      return;
    }
  }

  m_onedgains = groupGains;
  m_layerToGroup = layerToGroup;
}

bool CDCDedx1DCell::isValidGroupedPayload() const
{
  if (m_onedgains.empty()) {
    B2ERROR("CDCDedx1DCell: no gain groups");
    return false;
  }

  for (unsigned int g = 0; g < m_onedgains.size(); ++g) {
    if (m_onedgains[g].empty()) {
      B2ERROR("CDCDedx1DCell: group " << g << " is empty");
      return false;
    }
  }

  // new-style payload
  if (!m_layerToGroup.empty()) {

    for (unsigned int layer = 0; layer < m_layerToGroup.size(); ++layer) {
      if (m_layerToGroup[layer] >= m_onedgains.size()) {
        B2ERROR("1DCell: invalid group index in layer map");
        return false;
      }
    }

    return true;
  }

  // old-style payload
  if (m_onedgains.size() == 2) {
    return true;
  }

  B2ERROR("CDCDedx1DCell: invalid legacy payload shape");
  return false;
}

double CDCDedx1DCell::getMean(unsigned int layer, unsigned int bin) const
{
  const unsigned int group = getGroup(layer);
  if (group >= m_onedgains.size()) return 1.0;

  if (bin >= m_onedgains[group].size()) return 1.0;

  return m_onedgains[group][bin];
}

void CDCDedx1DCell::setMean(unsigned int layer,
                            unsigned int bin,
                            double value)
{
  const unsigned int group = getGroup(layer);

  if (group >= m_onedgains.size()) return;
  if (bin >= m_onedgains[group].size()) return;

  m_onedgains[group][bin] = value;
}

double CDCDedx1DCell::getMean(unsigned int layer, double enta) const
{
  const unsigned int group = getGroup(layer);

  if (group >= m_onedgains.size()) return 1.0;
  if (m_onedgains[group].empty()) return 1.0;

  const double piby2 = M_PI / 2.0;

  if (enta < -piby2) enta += piby2;
  if (enta >  piby2) enta -= piby2;

  unsigned int nbins = m_onedgains[group].size();

  double binsize = 2.0 * piby2 / nbins;

  int bin = std::floor((enta + piby2) / binsize);

  if (bin < 0 || static_cast<unsigned int>(bin) >= nbins) {
    B2WARNING("Problem with CDC dE/dx 1D binning!");
    return 1.0;
  }

  return m_onedgains[group][bin];
}

CDCDedx1DCell& CDCDedx1DCell::operator*=(CDCDedx1DCell const& rhs)
{
  if (m_version != rhs.getVersion()) {
    B2WARNING("1DCell versions do not match");
    return *this;
  }

  if (!isValidGroupedPayload() || !rhs.isValidGroupedPayload()) {
    B2WARNING("Invalid payload");
    return *this;
  }

  if (m_layerToGroup != rhs.m_layerToGroup) {
    B2WARNING("Layer mapping mismatch");
    return *this;
  }

  if (m_onedgains.size() != rhs.m_onedgains.size()) {
    B2WARNING("Group count mismatch");
    return *this;
  }

  for (unsigned int group = 0; group < m_onedgains.size(); ++group) {

    if (m_onedgains[group].size() != rhs.m_onedgains[group].size()) {
      B2WARNING("Bin count mismatch");
      return *this;
    }

    for (unsigned int bin = 0; bin < m_onedgains[group].size(); ++bin) {
      m_onedgains[group][bin] *= rhs.m_onedgains[group][bin];
    }
  }

  return *this;
}
