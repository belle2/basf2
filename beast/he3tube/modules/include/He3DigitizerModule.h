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

#include <TFile.h>
#include <TTree.h>

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

      /** creates waveform, returns peak */
      double WaveformMaker(std::vector<double> edepVec, std::vector<double> timeVec, std::vector<double> RVec, double lowTime);
      /** loads the simulated signals from a root file */
      void loadGarfieldData();
      /** Produces the impulse response function */
      virtual void impulseResponse();
      /** reads data from HE3TUBE.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** filename of Garfield datafile */
      std::string m_GarfDataFile;
      /** size of waveforms */
      static const int waveformSize = 5500;
      /** size of impulse response */
      static const int iResponseLength = 5500;
      /** number of different radius values in the simulated signal file */
      static const int numRadii = 1000;
      /** size of the garfield simulated signals */
      static const int garfSignalSize = 5000;
      /** garfied simulated signal variable */
      double garfSignal[garfSignalSize];
      /** number of detectors. Read from the gearbox*/
      int numOfTubes = 0;
      /** X coordinate of tube center */
      std::vector<double> TubeCenterX;
      /** Y coordinate of tube center */
      std::vector<double> TubeCenterY;
      /** inner radius of the tube, read from the gearbox*/
      double rTube;
      /** radius of the sense wire */
      double rWire = 0.030;
      /** Impulse response function */
      double iResponse[iResponseLength];
      /** Event counter */
      int Event = 0;
      /** Conversion to ADC counts, set in steering file*/
      double m_ConversionFactor;
      /** ionization energy of He3 */
      double IonizationE = 1;
      /** TFile containing garfield signals */
      TFile* f_sim;
      /** TTree containing garfield signals */
      TTree* t_sim;

    };

  }
}

#endif /* HE3DIGITIZERMODULE_H */
