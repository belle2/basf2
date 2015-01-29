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

      /** Reads drift data from file */
      virtual void getDriftData();

      /** interpolates the drift time for a given distance from the wire */
      double getDriftTime(double R);

      /** Produces the impulse response function */
      virtual void impulseResponse();

      /** Convolves the raw waveform with the impulse response function */
      virtual void convolveWaveform(int detNB);

      /** gets the peak of the waveform from a given detector number */
      virtual void findPeak(int detNB);

      /** print the convolved wafeform to file. Useful for debugging */
      virtual void printConvWaveform(std::string fileName);

      /** reads data from HE3TUBE.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

    private:
      /** filename of drift datafile */
      std::string m_driftDataFile;
      /** sigma of impulse response */
      double m_impulseSigma;
      /** distance from center of tube */
      double radius_drift[248];
      /** drift time for each distance from center of tube */
      double time_drift[248];
      /** X coordinate of tube center */
      double TubeCenterX[8];
      /** Y coordinate of tube center */
      double TubeCenterY[8];
      /** Impulse response function */
      double iResponse[2400] = {0};
      /** size of waveforms */
      const int waveformSize = 14000;
      /** raw waveform for each tube */
      double waveform[8][14000] = {{0}};
      /** waveform convolved with impulse function */
      double convolvedWaveform[8][14000] = {{0}};
      /** peak of waveform */
      double peak[8] = {0};

    };

  }
}

#endif /* HE3DIGITIZERMODULE_H */
