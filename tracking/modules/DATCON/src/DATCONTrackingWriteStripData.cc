/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONTrackingModule.h>

using namespace std;
using namespace Belle2;

/*
 * Save digits.
 */
void
DATCONTrackingModule::saveStrips()
{
  VxdID sensorID;
  bool uSide;
  short strip;
  float samples[6];
  DATCONSVDDigit::APVRawSamples rawSamples;
  short adcID;
  ofstream fadc01, fadc40, fadc41, fadc42, fadc81;

  fadc01.open("bmm_01.dat", ios::trunc | ios::out);
  fadc40.open("bmm_40.dat", ios::trunc | ios::out);
  fadc41.open("bmm_41.dat", ios::trunc | ios::out);
  fadc42.open("bmm_42.dat", ios::trunc | ios::out);
  fadc81.open("bmm_81.dat", ios::trunc | ios::out);

  if (!fadc01) {
    B2ERROR("Error opening file fadc01");
  }
  if (!fadc40) {
    B2ERROR("Error opening file fadc40");
  }
  if (!fadc41) {
    B2ERROR("Error opening file fadc41");
  }
  if (!fadc42) {
    B2ERROR("Error opening file fadc42");
  }
  if (!fadc81) {
    B2ERROR("Error opening file fadc81");
  }

  fadc01 << "Event " << eventnumber << endl;
  fadc40 << "Event " << eventnumber << endl;
  fadc41 << "Event " << eventnumber << endl;
  fadc42 << "Event " << eventnumber << endl;
  fadc81 << "Event " << eventnumber << endl;

  fadc01 << "FADC 01" << endl;
  fadc40 << "FADC 64" << endl;
  fadc41 << "FADC 65" << endl;
  fadc42 << "FADC 66" << endl;
  fadc81 << "FADC 129" << endl;

  /* Loop over all Digits and aggregate first all samples from a
   * single strip.
   * We assume they are sorted!
   */
  for (auto& digit : storeDATCONSVDDigits) {
    sensorID = digit.getSensorID();
    uSide = digit.isUStrip();
    strip = digit.getCellID();
    rawSamples = digit.getRawSamples();

    for (int i = 0; i < 6; i++) {
      samples[i] = rawSamples[i];
    }

    short layer = sensorID.getLayerNumber();
    short sensor = sensorID.getSensorNumber();

    if (layer == 3) {
      if (sensor == 1) {
        if (uSide) {
          adcID = 0;
          writeStrip(strip, adcID, samples, fadc40);
        } else {
          adcID = 1;
          writeStrip(strip, adcID, samples, fadc40);
        }
      } else if (sensor == 2) {
        if (uSide) {
          adcID = 0;
          writeStrip(strip, adcID, samples, fadc01);
        } else {
          adcID = 0;
          writeStrip(strip, adcID, samples, fadc81);
        }
      }
    } else if (layer == 4) {
      if (sensor == 1) {
        if (uSide) {
          adcID = 2;
          writeStrip(strip, adcID, samples, fadc40);
        } else {
          adcID = 3;
          writeStrip(strip, adcID, samples, fadc40);
        }
      } else if (sensor == 2) {
        if (uSide) {
          adcID = 0;
          writeStrip(strip, adcID, samples, fadc42);
        } else {
          adcID = 1;
          writeStrip(strip, adcID, samples, fadc42);
        }
      } else if (sensor == 3) {
        if (uSide) {
          adcID = 1;
          writeStrip(strip, adcID, samples, fadc01);
        } else {
          adcID = 1;
          writeStrip(strip, adcID, samples, fadc81);
        }
      }
    } else if (layer == 5) {
      if (sensor == 1) {
        if (uSide) {
          adcID = 4;
          writeStrip(strip, adcID, samples, fadc40);
        } else {
          adcID = 5;
          writeStrip(strip, adcID, samples, fadc40);
        }
      } else if (sensor == 2) {
        if (uSide) {
          adcID = 2;
          writeStrip(strip, adcID, samples, fadc42);
        } else {
          adcID = 3;
          writeStrip(strip, adcID, samples, fadc42);
        }
      } else if (sensor == 3) {
        if (uSide) {
          adcID = 2;
          writeStrip(strip, adcID, samples, fadc01);
        } else {
          adcID = 2;
          writeStrip(strip, adcID, samples, fadc81);
        }
      } else if (sensor == 4) {
        if (uSide) {
          adcID = 4;
          writeStrip(strip, adcID, samples, fadc42);
        } else {
          adcID = 5;
          writeStrip(strip, adcID, samples, fadc42);
        }
      }
    } else if (layer == 6) {
      if (sensor == 1) {
        if (uSide) {
          adcID = 6;
          writeStrip(strip, adcID, samples, fadc40);
        } else {
          adcID = 7;
          writeStrip(strip, adcID, samples, fadc40);
        }
      } else if (sensor == 2) {
        if (uSide) {
          adcID = 2;
          writeStrip(strip, adcID, samples, fadc41);
        } else {
          adcID = 3;
          writeStrip(strip, adcID, samples, fadc41);
        }
      } else if (sensor == 3) {
        if (uSide) {
          adcID = 6;
          writeStrip(strip, adcID, samples, fadc42);
        } else {
          adcID = 7;
          writeStrip(strip, adcID, samples, fadc42);
        }
      } else if (sensor == 4) {
        if (uSide) {
          adcID = 3;
          writeStrip(strip, adcID, samples, fadc01);
        } else {
          adcID = 3;
          writeStrip(strip, adcID, samples, fadc81);
        }
      } else if (sensor == 5) {
        if (uSide) {
          adcID = 4;
          writeStrip(strip, adcID, samples, fadc01);
        } else {
          adcID = 4;
          writeStrip(strip, adcID, samples, fadc81);
        }
      }
    }

  }

  fadc01.close();
  fadc40.close();
  fadc41.close();
  fadc42.close();
  fadc81.close();
}

/*
 * Write strips to file.
 */
void
DATCONTrackingModule::writeStrip(short strip, short adcID, float* samples, ofstream& of)
{
  short strips_per_apv = 128;
  short apv, rest_strip;
  short apv_offset = 0;

  apv_offset = adcID * 6;
  apv = (strip / strips_per_apv);
  rest_strip = strip - (apv * strips_per_apv);
  apv += apv_offset;

  /* Write to stream, format:
    * apv strip sample1 sample2 sample3 sample4 sample5 sample6
    */
  of << apv << " " << rest_strip << " " << samples[0] << " " << samples[1] << " " << samples[2] << " "
     << samples[3] << " " << samples[4] << " " << samples[5] << endl;

}
