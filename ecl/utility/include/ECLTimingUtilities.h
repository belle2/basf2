/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace ECL {
    /**
     * Class to storing timing calibration functions
     */
    class ECLTimingUtilities {

    public:

      /**
       * Constructor
       */
      ECLTimingUtilities();


      /**
       * Time walk function for the pre-amps
       * i.e. the energy dependence correction to the time
       * t-t0 = p1 + pow( (p3/(amplitude+p2)), p4 ) + p5*exp(-amplitude/p6)      ("Energy dependence equation")
       * or "energy dependence equation using Alex function"
       * In units of clock ticks: 1/(4fRF) = 0.4913 ns/clock tick
       */
      double energyDependentTimeOffsetElectronic(const double amplitude) const;


      /**
       * Sets the time walk function parameters to the values given by the user
       */
      void setTimeWalkFuncParams(const double p1, const double p2, const double p3, const double p4, const double p5, const double p6)
      {
        energyDependenceTimeOffsetFitParam_p1 = p1 ;
        energyDependenceTimeOffsetFitParam_p2 = p2 ;
        energyDependenceTimeOffsetFitParam_p3 = p3 ;
        energyDependenceTimeOffsetFitParam_p4 = p4 ;
        energyDependenceTimeOffsetFitParam_p5 = p5 ;
        energyDependenceTimeOffsetFitParam_p6 = p6 ;
      }

    private:

      // Parameters of the energy dependent time walk function
      //   Default values measured by Alex Kuzmin
      double energyDependenceTimeOffsetFitParam_p1 = 0  ;               /**< p1 in "energy dependence equation" */
      double energyDependenceTimeOffsetFitParam_p2 = 88449. ;           /**< p2 in "energy dependence equation" */
      double energyDependenceTimeOffsetFitParam_p3 = 0.20867E+06 ;      /**< p3 in "energy dependence equation" */
      double energyDependenceTimeOffsetFitParam_p4 = 3.1482 ;           /**< p4 in "energy dependence equation" */
      double energyDependenceTimeOffsetFitParam_p5 = 7.4747 ;           /**< p5 in "energy dependence equation" */
      double energyDependenceTimeOffsetFitParam_p6 = 1279.3 ;           /**< p6 in "energy dependence equation" */

    }; // ECLTimingUtilities class
  } // namespace ECL
} // namespace Belle2
