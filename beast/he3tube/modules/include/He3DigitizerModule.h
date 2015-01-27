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

      virtual void getDriftData();

      double getDriftTime(double R);

      virtual void impulseResponse();

      virtual void convolveWaveform(int detNB);

      virtual void findPeak(int detNB);

      virtual void printConvWaveform(std::string fileName);

      virtual void getXMLData();

    private:
      std::string m_driftDataFile;
      double m_impulseSigma;

      double radius_drift[248];
      double time_drift[248];

      double TubeCenterX[8];
      double TubeCenterY[8];

      double iResponse[2400] = {0};
      //double waveform[10000] = {0};


      const int waveformSize = 14000;

      double waveform[8][14000] = {{0}};
      double convolvedWaveform[8][14000] = {{0}};
      double peak[8] = {0};

    };

  }
}

#endif /* HE3DIGITIZERMODULE_H */
