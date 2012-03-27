/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDSIGNAL_H
#define SVDSIGNAL_H

#include <framework/dataobjects/RelationElement.h>
#include <boost/tuple/tuple.hpp>
#include <deque>
#include <math.h>

namespace Belle2 {
  namespace SVD {

    /**
     * The SVD signal class.
     *
     * The class implements the waveform appearing on an SVD strip as a result of
     * ionization by particles passing through the detector.
     * The class accumulates elementary waveforms formed by chargelets contributing to the
     * charge accumulated on the strip.
     * The elementary waveforms are currently of the form
     * w(t|delta,tau) = (t-delta)/tau * exp((t-delta)/tau) for t >= delta,
     *                  otherwise 0.
     * with delta >= 0 being the initial time, and tau the characteristic time.
     * The class is a functor returning values of summary waveform at a given time.
     * FIXME: Poisson and gaussian noises will be added externally. To add Poisson,
     * the charges must be in electrons.
     */

    class SVDSignal {

    public:

      /** Type to store contributions to strip signal by different particles */
      typedef std::map<RelationElement::index_type, RelationElement::weight_type> relations_map;
      typedef std::map<RelationElement::index_type, RelationElement::weight_type>::value_type relation_value_type;
      /** Type to store elementary waveforms.
       *  Waveforms are parameterized (in this order) by scale factor (charge), time constant, and width.
       */
      typedef std::deque< boost::tuple<double, double, double> > function_list;
      typedef std::deque< boost::tuple<double, double, double> >::value_type function_list_value_type;

      /** Default constructor */
      SVDSignal(): m_charge(0) {}

      /** Add a chargelet to the strip signal.
       * @param charge Charge in electrons to be added
       * @param time Time of arrival of the chargelet to the sensitive surface of the sensor.
       * @param tau Characteristic time of waveform decay. FIXME: Is it possible to pass this
       *            in a more elegant manner?
       * @param particle Index of the particle contributing the charge, -1 for no particle/noise
       * @param truehit Index of the truehit corresponding to the particle that contributed
       * the charge.
       * Noise can be added externally:
       * - Poisson noise is added to charges of individual charges of elementary
       * waveforms
       * - Electronic noise is added via a special generator function.
       */
      void add(double charge, double time, double tau, int particle = -1, int truehit = -1);

      /** Make the SVDSignals addable. */
      SVDSignal& operator+=(const SVDSignal& other);

      /** Make the SVDSignals scalable. */
      SVDSignal& operator*=(double scale);

      /** Make the SVDSignal assignable. */
      SVDSignal& operator=(const SVDSignal& other);

      /** Waveform shape.*/
      double waveform(double time, double charge, double initTime, double tau) const {
        if (time < initTime) return 0;
        if (tau <= 0) return 0;
        double z = (time - initTime) / tau;
        return charge * z * exp(-z);
      }

      /** Waveform taking parameters in a boost::tuple. */
      double waveform(double time, const function_list_value_type& params) const
      { return waveform(time, params.get<0>(), params.get<1>(), params.get<2>()); }

      /** Make the SVDSignal a functor. */
      double operator()(double time) const;

      /** Return the charge collected in the strip */
      double charge() const { return m_charge; }

      /** Return the list of waveform parameters */
      const function_list& functions() const { return m_functions; }

      /** Return the map containing all particle contributions to the strip charge */
      const relations_map& particles() const { return m_particles; }

      /** Return the map containing all truehit contributions to the strip charge */
      const relations_map& truehits() const { return m_truehits; }

    protected:

      /** charge of the pixel */
      double m_charge;

      /** list of elementary waveform parameters.*/
      function_list m_functions;

      /** particle contributions to the charge */
      relations_map m_particles;

      /** truehit contributions to the charge */
      relations_map m_truehits;

    }; // class SVDDigit

  } // end namespace svd
} // end namespace Belle2

#endif
