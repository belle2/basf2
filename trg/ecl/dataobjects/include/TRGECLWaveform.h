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
    //! TC Energy and Timing
    //! Raw TC energy

    double m_TCDigiE[64];

  public:
    //! Empty constructor
    //! Recommended for ROOT IO
    TRGECLWaveform()
    {
      m_eventid = 0;
      m_tcid = 0;

      for (int i = 0; i < 64; i++) m_TCDigiE[i] = 0;
    }
    //! Useful Constructor
    TRGECLWaveform(int tcid, double* digiarray)
    {
      m_tcid = tcid;

      for (int i = 0 ; i < 64; i++) {
        m_TCDigiE[i] = digiarray[i];
      }

    }
    ~TRGECLWaveform() {};

    int getTCID() const
    {
      return m_tcid;
    }
    const double* getWaveform() const
    {
      return m_TCDigiE;
    }
    void fillWaveform(double* wf) const
    {
      for (int i = 0; i < 64; i++) wf[i] = m_TCDigiE[i];
    }
    void setThetaPhiIDs(int thid, int phid)
    {
      m_phiid = phid;
      m_thetaid = thid;
    }
    int getThetaID() const
    {
      return m_thetaid;
    }
    int getPhiID() const
    {
      return m_phiid;
    }

    //! the class title
    ClassDef(TRGECLWaveform, 1); /*< the class title */
  };
} //! End namespace Belle2

#endif
