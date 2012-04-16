/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/simulation/SVDSignal.h>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

using namespace Belle2;
using namespace SVD;

boost::tuple<double, const SVDSignal::relations_map&, const SVDSignal::relations_map&>
SVDSignal::operator()(double time)
{
  double wave_sum = 0;
  m_particles.clear();
  m_truehits.clear();
  BOOST_FOREACH(SVDSignal::Wave wave, m_functions) {
    double wave_value = waveform(time, wave);
    wave_sum += wave_value;
    m_particles[wave.m_particle] += static_cast<float>(wave_value);
    m_truehits[wave.m_truehit] += static_cast<float>(wave_value);
  }
  return boost::make_tuple(wave_sum, boost::cref(m_particles), boost::cref(m_truehits));
}

