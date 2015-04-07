/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam de Jong                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HE3DIGITIZERMODULE_H
#define HE3DIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <vector>

namespace Belle2 {
  namespace he3tube {
    /**
     * He3 tube digitizer
     *
     * Creates He3Hits from He3SimHits
     *
     */
    class He3DigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      He3DigitizerModule();

      /**  */
      virtual ~He3DigitizerModule();

      /**  */
      virtual void initialize();

      /**  */
      virtual void beginRun();

      /**  */
      virtual void event();

      /**  */
      virtual void endRun();
      /**  */
      virtual void terminate();


    private:

      /** Reads drift data from file */
      virtual void getDriftData();

      /** interpolates the drift time for a given distance from the wire */
      double getDriftTime(double R);

      /** Produces the impulse response function */
      virtual void impulseResponse();

      /** Convolves the raw waveform with the impulse response function and returns the peak of the convolved waveform */
      double convolveWaveform(double* waveform);

      /** print the convolved wafeform to file. Useful for debugging */
      virtual void printConvWaveform(int eventNum, int detNB, double* convolvedWaveform);

      /** reads data from HE3TUBE.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** filename of drift datafile */
      std::string m_driftDataFile;
      /** sigma of impulse response */
      double m_impulseSigma;
      /** distance from center of tube */
      std::vector<double> radius_drift;
      /** drift time for each distance from center of tube */
      std::vector<double> time_drift;
      /** size of waveforms */
      static const int waveformSize = 15000;
      /** number of detectors. Read from HE3TUBE.xml*/
      int numOfTubes = 0;
      /** X coordinate of tube center */
      std::vector<double> TubeCenterX;
      /** Y coordinate of tube center */
      std::vector<double> TubeCenterY;
      /** Impulse response function */
      double iResponse[2400] = {0};
      /** Event counter */
      int Event = 0;
      /** Conversion to ADC counts*/
      double m_ConversionFactor = 6828 / 0.3538;

    };

  }
}

#endif /* HE3DIGITIZERMODULE_H */
