/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>
#include <vector>


namespace Belle2 {



  /**
  * The Class for VXD placement payload
  */
  class VXDGeoPlacementPar: public TObject {
  public:

    /** Constructor */
    VXDGeoPlacementPar(const std::string& name = "", double u = 0, double v = 0,
                       std::string w = "bottom", double woffset = 0);
    //! Destructor
    ~VXDGeoPlacementPar() {}
    /** get name of the component */
    const std::string& getName() const { return m_name; }
    /** set name of the component */
    void setName(const std::string& name) { m_name = name; }
    /** get local u coordinate where to place the component */
    double getU() const { return m_u; }
    /** set local u coordinate where to place the component */
    void setU(double u)  {  m_u = u; }
    /** get local v coordinate where to place the component */
    double getV() const { return m_v; }
    /** set local v coordinate where to place the component */
    void setV(double v) { m_v = v; }
    /** get local w position where to place the component */
    const std::string& getW() const { return m_w; }
    /** set local w position where to place the component */
    void setW(std::string&  w) {m_w = w;}
    /** get offset to local w position where to place the component */
    double getWOffset() const { return m_woffset; }
    /** set offset to local w position where to place the component */
    void setWOffset(double woffset) { m_woffset = woffset; }
  private:
    /** Name of the component */
    std::string m_name;
    /** u coordinate where to place the component */
    double m_u;
    /** v coordinate where to place the component */
    double m_v;
    /** w coordinate where to place the component */
    std::string m_w;
    /** Offset to the w placement of the component */
    double m_woffset;

    ClassDef(VXDGeoPlacementPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

