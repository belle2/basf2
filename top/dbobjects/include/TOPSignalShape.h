/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>
#include <TSpline.h>

namespace Belle2 {

  /**
   * Normalized shape of single photon pulse (waveform)
   * Pulse must be positive
   */
  class TOPSignalShape: public TObject {
  public:

    /**
     * Default constructor
     */
    TOPSignalShape()
    {}

    /**
     * Full constructor
     * @param shape waveform values (unnormalized) of positive pulse
     * @param timeBin time difference between two samples [ns]
     * @param tau time constant of the exponential tail used for extrapolation [ns]
     * @param pole1 noise bandwidth: first pole [GHz]
     * @param pole2 noise bandwidth: second pole [GHz]
     */
    TOPSignalShape(std::vector<double> shape, double timeBin, double tau,
                   double pole1, double pole2);

    /**
     * Copy constructor
     */
    TOPSignalShape(const TOPSignalShape& shape): TObject()
    {
      *this = shape;
      m_interpolator = 0;
    }

    /**
     * Assignment operator
     */
    TOPSignalShape& operator=(const TOPSignalShape& shape)
    {
      if (this != &shape) {
        m_shape = shape.getShape();
        m_tmin = shape.getTMin();
        m_tmax = shape.getTMax();
        m_tau = shape.getTau();
        m_pole1 = shape.getPole1();
        m_pole2 = shape.getPole2();
        if (m_interpolator) delete m_interpolator;
        m_interpolator = 0;
      }
      return *this;
    }

    /**
     * Destructor
     */
    ~TOPSignalShape()
    {
      if (m_interpolator) delete m_interpolator;
    }

    /**
     * Returns value at time t of the normalized waveform using interpolator
     * @param t time [ns] (t = 0 is at 50% CF leading edge crossing)
     * @return normalized value (normalization: peaking value = 1)
     */
    double getValue(double t) const;

    /**
     * Returns time of the first waveform sample
     * @return time [ns]
     */
    double getTMin() const {return m_tmin;}

    /**
     * Returns time of the last waveform sample
     * @return time [ns]
     */
    double getTMax() const {return m_tmax;}

    /**
     * Returns time constant of the exponential used for the extrapolation of tail
     * @return time constant [ns]
     */
    double getTau() const {return m_tau;}

    /**
     * Returns first pole of noise bandwidth
     * @return first pole [GHz]
     */
    double getPole1() const {return m_pole1;}

    /**
     * Returns second pole of noise bandwidth
     * @return second pole [GHz]
     */
    double getPole2() const {return m_pole2;}

    /**
     * Returns waveform values
     * @return vector of normalized values
     */
    const std::vector<double>& getShape() const {return m_shape;}

    /**
     * Returns interpolator
     * @return interpolator
     */
    const TSpline5* getInterpolator() const {return m_interpolator;}

  private:

    std::vector<double> m_shape; /**< waveform values */
    double m_tmin = 0; /**< time of the first waveform sample [ns] */
    double m_tmax = 0; /**< time of the last waveform sample [ns] */
    double m_tau = 0;  /**< time constant of the exponential tail [ns] */
    double m_pole1 = 0; /**< noise bandwidth: first pole [GHz] */
    double m_pole2 = 0; /**< noise bandwidth: second pole [GHz] */

    /** cache for the interpolator */
    mutable TSpline5* m_interpolator = 0; //!

    ClassDef(TOPSignalShape, 1); /**< ClassDef */

  };

} // end namespace Belle2
