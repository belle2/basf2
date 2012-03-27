/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdSimulation/SVDSignal.h>
#include <boost/foreach.hpp>
#include <algorithm>
#include <utility>

using namespace Belle2;
using namespace SVD;
using namespace boost;
using namespace std;

void SVDSignal::add(double charge, double time, double tau, int particle, int truehit)
{
  m_charge += charge;
  m_functions.push_back(make_tuple(charge, time, tau));
  if (particle >= 0) m_particles[particle] += charge;
  if (truehit >= 0) m_truehits[truehit] += charge;
}

SVDSignal& SVDSignal::operator+=(const SVDSignal& other)
{
  // Add charge
  m_charge += other.charge();
  // Add functions
  copy(other.functions().begin(), other.functions().end(), m_functions.end());
  // Add mc particles
  BOOST_FOREACH(relation_value_type value, other.particles()) {
    m_particles[value.first] += value.second;
  }
  // Add truehits
  BOOST_FOREACH(relation_value_type value, other.truehits()) {
    m_truehits[value.first] += value.second;
  }
  return *this;
}

SVDSignal& SVDSignal::operator*=(double scale)
{
  // Scale charge
  m_charge *= scale;
  // Scale functions
  BOOST_FOREACH(function_list_value_type & params, m_functions) {
    params.get<0>() *= scale;
  }
  // Scale weights of mc particles
  BOOST_FOREACH(relation_value_type & value, m_particles) {
    value.second *= scale;
  }
  // Scale weights of truehits
  BOOST_FOREACH(relation_value_type & value, m_truehits) {
    value.second *= scale;
  }
  return *this;
}

SVDSignal& SVDSignal::operator=(const SVDSignal& other)
{
  // Set charge
  m_charge = other.charge();
  // Set functions
  m_functions.clear();
  copy(other.functions().begin(), other.functions().end(), m_functions.end());
  // Set mc particles
  m_particles.clear();
  for (relations_map::const_iterator rel = other.particles().begin();
       rel != other.particles().end(); ++rel)
    m_particles.insert(make_pair(rel->first, rel->second));
  // Set truehits
  m_truehits.clear();
  for (relations_map::const_iterator rel = other.truehits().begin();
       rel != other.truehits().end(); ++rel)
    m_truehits.insert(make_pair(rel->first, rel->second));
  return *this;
}

double SVDSignal::operator()(double time) const
{
  double result = 0;
  BOOST_FOREACH(const function_list_value_type & params, m_functions) {
    result += waveform(time, params);
  }
  return result;
}

