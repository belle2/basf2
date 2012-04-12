/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSTRIP_H
#define BKLMSTRIP_H

#include <TObject.h>

namespace Belle2 {

  //! Store one BKLM strip hit as a ROOT object
  /*! Per ROOT TObject convention, all data elements are public so that
      they can be seen in a ROOT browser.
  */
  class BKLMStrip : public TObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMStrip();

    //! Constructor with initial values
    BKLMStrip(bool, int, int, int, char, int, double, double);

    //! Copy constructor
    BKLMStrip(const BKLMStrip&);

    //! Destructor
    virtual ~BKLMStrip() {}

    //! returns flag whether hit is in RPC (true) or scintillator (false)
    bool getInRPC() const { return m_InRPC; }

    //! returns end (0=forward or 1=backward) of this strip
    int getFrontBack() const { return m_FrontBack; }

    //! returns sector number of this strip
    int getSector() const { return m_Sector; }

    //! returns layer number of this strip
    int getLayer() const { return m_Layer; }

    //! returns direction ('P' for phi, 'Z' for z) of this strip
    char getDirection() const { return m_Direction; }

    //! returns strip number
    int getStrip() const { return m_Strip; }

    //! returns hit time
    double getTDC() const { return m_TDC; }

    //! returns pulse height
    double getADC() const { return m_ADC; }

    //! determines if two BKLMStrips are equal based on geometry only
    bool match(const BKLMStrip*) const;

    //! sets flag whether hit is in RPC (true) or scintillator (false)
    void setInRPC(bool inRPC) { m_InRPC = inRPC; }

    //! sets end (0=forward or 1=backward) of this strip
    void setFrontBack(int frontBack) { m_FrontBack = frontBack; }

    //! sets sector number of this strip
    void setSector(int sector) { m_Sector = sector; }

    //! sets layer number of this strip
    void setLayer(int layer) { m_Layer = layer; }

    //! sets direction ('P' for phi, 'Z' for z) of this strip
    void setDirection(char direction) { m_Direction = direction; }

    //! sets strip number
    void setStrip(int strip) { m_Strip = strip; }

    //! sets hit time
    void setTDC(double tdc) { m_TDC = tdc; }

    //! sets pulse height
    void setADC(double adc) { m_ADC = adc; }

    //! flag to say whether the hit is in RPC (true) or scintillator (false)
    bool m_InRPC;

    //! barrel end (forward or backward) of the strip
    int m_FrontBack;

    //! sector number of the strip
    int m_Sector;

    //! layer number of the strip
    int m_Layer;

    //! direction of the strip
    char m_Direction;

    //! strip number
    int m_Strip;

    //! global hit time relative to trigger (ns)
    double m_TDC;

    //! pulse height (MeV)
    double m_ADC;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMStrip, 1)

  private:

  };

} // end of namespace Belle2

#endif //BKLMSTRIP_H
