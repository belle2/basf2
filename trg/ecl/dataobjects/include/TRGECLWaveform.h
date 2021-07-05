/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLWaveform.h
// Section  : TRG ECL

#ifndef TRGECLWAVEFORM_H
#define TRGECLWAVEFORM_H

#include <TObject.h>


namespace Belle2 {
  //!  Digitize result
  class TRGECLWaveform: public TObject {
  private:

    //! Event Id
    int m_eventid;
    //! TC id
    int m_tcid;
    //! phi id of TC
    int m_phiid;
    //! Theta id of TC
    int m_thetaid;
    //! Waveform
    double m_TCDigiE[64];

  public:
    //! Empty constructor
    //! Recommended for ROOT IO
    TRGECLWaveform()
    {
      m_eventid = 0;
      m_tcid = 0;
      m_phiid = 0;
      m_thetaid = 0;
      for (int i = 0; i < 64; i++) m_TCDigiE[i] = 0;
    }
    //! Useful Constructor
    TRGECLWaveform(int tcid, double* digiarray)
    {
      m_tcid = tcid;
      m_eventid = 0;
      m_phiid = 0;
      m_thetaid = 0;
      for (int i = 0 ; i < 64; i++) {
        m_TCDigiE[i] = digiarray[i];
      }

    }
    ~TRGECLWaveform() {};
    //! Get TC Id
    int getTCID() const
    {
      return m_tcid;
    }
    //! Get WaveForm
    const double* getWaveform() const
    {
      return m_TCDigiE;
    }
    //! Fill Waveform
    void fillWaveform(double* wf) const
    {
      for (int i = 0; i < 64; i++) wf[i] = m_TCDigiE[i];
    }
    //! Set Theta and Phi Id of TC
    void setThetaPhiIDs(int thid, int phid)
    {
      m_phiid = phid;
      m_thetaid = thid;
    }
    //! Get Theta Id
    int getThetaID() const
    {
      return m_thetaid;
    }
    //! Get Phi ID
    int getPhiID() const
    {
      return m_phiid;
    }

    //! the class title
    ClassDef(TRGECLWaveform, 1); /*< the class title */
  };
} //! End namespace Belle2

#endif
