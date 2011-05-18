/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef IRSIMHIT_H
#define IRSIMHIT_H

#include <TObject.h>
#include <TVector3.h>
#include <string>

#define DEFAULT_IRSIMHITS           "IRSimHits"
#define DEFAULT_IRSIMHITSREL "MCParticlesToIRSimHits"


namespace Belle2 {

  /** Class for saving raw hit data of the IR.  */
  class IRSimHit : public TObject {
  public:

    /** Empty constructor for ROOT IO. */
    IRSimHit():
        m_posIn(0, 0, 0),
        m_momIn(0, 0, 0),
        m_posOut(0, 0, 0),
        m_momOut(0, 0, 0),
        m_PDGcode(0),
        m_depEnergy(0),
        m_Volname("noname") {}

    /** Full Constructor. */
    IRSimHit(
      TVector3 posIn,
      TVector3 momIn,
      TVector3 posOut,
      TVector3 momOut,
      int PDGcode,
      float depEnergy,
      std::string Volname
    ) {
      m_posIn = posIn;
      m_momIn = momIn;
      m_posOut = posOut;
      m_momOut = momOut;
      m_PDGcode = PDGcode;
      m_depEnergy = depEnergy;
      m_Volname = Volname;
    }

    ~IRSimHit();

    /** The method to set.*/
    void setposIn(TVector3 posIn);
    void setmomIn(TVector3 momIn);
    void setposOut(TVector3 posOut);
    void setmomOut(TVector3 momOut);
    void setPDGcode(int PDGcode);
    void setdepEnergy(float depEnergy);
    void setVolname(std::string Volname);

    /** The method to get.*/
    TVector3 getposIn();
    TVector3 getmomIn();
    TVector3 getposOut();
    TVector3 getmomOut();
    int getPDGcode();
    float getdepEnergy();
    std::string getVolname();

  protected:

  private:

    TVector3 m_posIn;
    TVector3 m_momIn;
    TVector3 m_posOut;
    TVector3 m_momOut;
    int m_PDGcode;
    float m_depEnergy;
    std::string m_Volname;

    /** ROOT Macro.*/
    ClassDef(IRSimHit, 1);

  };

} // end namespace Belle2

#endif
