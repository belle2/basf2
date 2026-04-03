/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/dbobjects/CDCDedxCosineCor.h>

#include <cmath>

using namespace Belle2;

CDCDedxCosineCor::CDCDedxCosineCor(
  const std::vector<std::vector<double>>& groupCosgains,
  const std::vector<unsigned int>& layerToGroup)
{

  // 1. check groups exist
  if (groupCosgains.empty()) {
    B2ERROR("CDCDedxCosineCor: groupCosgains is empty.");
    return;
  }

  // 2. check layer mapping exists
  if (layerToGroup.empty()) {
    B2ERROR("CDCDedxCosineCor: layerToGroup is empty.");
    return;
  }

  for (size_t g = 0; g < groupCosgains.size(); ++g) {
    if (groupCosgains[g].empty()) {
      B2ERROR("CDCDedx1DCell: group " << g << " is empty.");
      return;
    }
  }

  // 4. check Layer -> group mapping
  for (size_t layer = 0; layer < layerToGroup.size(); layer++) {

    if (layerToGroup[layer] >= groupCosgains.size()) {
      B2ERROR("CDCDedxCosineCor: layer " << layer
              << " refers to invalid group "
              << layerToGroup[layer]);
      return;
    }
  }

  // 5. store data only after checks pass
  m_groupCosgains = groupCosgains;
  m_layerToGroup = layerToGroup;

// summary
  B2INFO("Cosine groups: " << m_groupCosgains.size());
  B2INFO("Cosine bins per group: " << m_groupCosgains[0].size());

  for (size_t layer = 0; layer < m_layerToGroup.size(); layer++) {
    B2INFO("layer " << layer << " -> group " << m_layerToGroup[layer]);
  }

}

void CDCDedxCosineCor::setCosCor(unsigned int bin, double value)
{
  if (bin < m_cosgains.size()) m_cosgains[bin] = value;
  else B2ERROR("CDCDedxCosineCor: invalid bin number, value not set");
}

void CDCDedxCosineCor::setCosCor(unsigned int group, unsigned int bin, double value)
{
  if (group >= m_groupCosgains.size()) {
    B2ERROR("CDCDedxCosineCor: invalid group number, value not set");
    return;
  }

  if (bin >= m_groupCosgains[group].size()) {
    B2ERROR("CDCDedxCosineCor: invalid bin number, value not set");
    return;
  }

  m_groupCosgains[group][bin] = value;
}

bool CDCDedxCosineCor::isValidGroupedPayload() const
{
  // Check that grouped cosine constants exist
  if (m_groupCosgains.empty()) {
    B2ERROR("CDCDedxCosineCor : no gain groups");
    return false;
  }

  // Check that the layer-to-group mapping exists
  if (m_layerToGroup.empty()) return false;

  // Verify that every layer refers to a valid group index
  for (unsigned int layer = 0; layer < m_layerToGroup.size(); ++layer) {
    if (m_layerToGroup[layer] >= m_groupCosgains.size()) {
      B2ERROR("CDCDedxCosineCor: layer-to-group map points to invalid group");
      return false;
    }
  }

  for (unsigned int g = 0; g < m_groupCosgains.size(); ++g) {
    if (m_groupCosgains[g].empty()) {
      B2ERROR("CDCDedxCosineCor: group " << g << " is empty");
      return false;
    }
  }

  // All checks passed
  return true;
}

double CDCDedxCosineCor::getMean(unsigned int bin) const
{
  if (bin < m_cosgains.size()) return m_cosgains[bin];
  return 1.0;
}

double CDCDedxCosineCor::getMean(double costh) const
{
  if (m_cosgains.empty()) return 1.0;
  return getMeanFromVector(m_cosgains, costh);
}


double CDCDedxCosineCor::getMean(unsigned int layer, unsigned int bin) const
{
  if (isGrouped()) {
    if (!isValidGroupedPayload()) return 1.0;

    if (layer >= m_layerToGroup.size()) {
      B2ERROR("CDCDedxCosineCor: invalid layer index");
      return 1.0;
    }

    const unsigned int group = m_layerToGroup[layer];
    if (bin >= m_groupCosgains[group].size()) {
      B2ERROR("CDCDedxCosineCor: invalid bin number");
      return 1.0;
    }

    return m_groupCosgains[group][bin];
  }

  return getMean(bin);
}

double CDCDedxCosineCor::getMean(unsigned int layer, double costh) const
{
  if (isGrouped()) {
    if (!isValidGroupedPayload()) return 1.0;

    if (layer >= m_layerToGroup.size()) {
      B2ERROR("CDCDedxCosineCor: invalid layer index");
      return 1.0;
    }

    const unsigned int group = m_layerToGroup[layer];
    return getMeanFromVector(m_groupCosgains[group], costh);
  }

  return getMean(costh);
}


double CDCDedxCosineCor::getMeanFromVector(const std::vector<double>& gains, double costh) const
{
  if (std::abs(costh) > 1.0) return 0.0;
  if (gains.empty()) return 1.0;

  // gains are stored at the center of the bins
  // find the bin center immediately preceding this value of costh
  const double binsize = 2.0 / gains.size();
  const int bin = std::floor((costh - 0.5 * binsize + 1.0) / binsize);

  int thisbin = bin;
  int nextbin = bin + 1;

  // extrapolation
  // extrapolate backward for lowest half-bin and center positive half-bin
  // extrapolate forward for highest half-bin and center negative half-bin
  if ((costh + 1.0) < (binsize / 2.0) || (costh > 0.0 && std::fabs(costh) < (binsize / 2.0))) {
    thisbin = bin + 1;
    nextbin = bin + 2;
  } else if ((costh - 1.0) > -1.0 * (binsize / 2.0) || (costh < 0.0 && std::fabs(costh) < (binsize / 2.0))) {
    thisbin = bin - 1;
    nextbin = bin;
  }

  const double frac = ((costh - 0.5 * binsize + 1.0) / binsize) - thisbin;

  if (thisbin < 0 || static_cast<unsigned int>(nextbin) >= gains.size()) {
    B2WARNING("Problem with extrapolation of CDC dE/dx cosine correction");
    return 1.0;
  }

  return (gains[nextbin] - gains[thisbin]) * frac + gains[thisbin];
}

bool CDCDedxCosineCor::multiplyGains(std::vector<double>& lhs, const std::vector<double>& rhs) const
{
  if (rhs.empty() || lhs.size() % rhs.size() != 0) {
    return false;
  }

  const int scale = std::floor(static_cast<double>(lhs.size()) / rhs.size() + 0.001);

  for (unsigned int bin = 0; bin < lhs.size(); ++bin) {
    lhs[bin] *= rhs[std::floor(bin / static_cast<double>(scale) + 0.001)];
  }

  return true;
}

CDCDedxCosineCor& CDCDedxCosineCor::operator*=(const CDCDedxCosineCor& rhs)
{
  // old x old
  if (!isGrouped() && !rhs.isGrouped()) {
    if (!multiplyGains(m_cosgains, rhs.getCosCor())) {
      B2WARNING("Cosine gain parameters do not match, cannot merge!");
    }
    return *this;
  }

  // grouped x grouped
  if (isGrouped() && rhs.isGrouped()) {
    if (!isValidGroupedPayload() || !rhs.isValidGroupedPayload()) {
      B2WARNING("Invalid grouped cosine payload, cannot merge!");
      return *this;
    }

    if (m_layerToGroup != rhs.getLayerMap()) {
      B2WARNING("layer grouping does not match, cannot merge grouped cosine payloads!");
      return *this;
    }

    const std::vector<std::vector<double>>& rhsGroups = rhs.getGroupCosCor();

    if (m_groupCosgains.size() != rhsGroups.size()) {
      B2WARNING("Number of cosine groups does not match, cannot merge!");
      return *this;
    }

    for (unsigned int group = 0; group < m_groupCosgains.size(); ++group) {
      if (!multiplyGains(m_groupCosgains[group], rhsGroups[group])) {
        B2WARNING("Grouped cosine gain parameters do not match, cannot merge!");
        return *this;
      }
    }
    return *this;
  }

  B2WARNING("Cannot merge old-style and grouped cosine payloads directly!");
  return *this;
}