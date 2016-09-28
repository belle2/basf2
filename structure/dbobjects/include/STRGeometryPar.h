/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexandre BEAULIEU                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>

#include <TObject.h>
#include <string>

namespace Belle2 {

  class GearDir;

  /**
  * The Class for STR geometry
  */

  class STRGeometryPar: public TObject {

  public:

    //! Default constructor
    STRGeometryPar() {}

    //! Constructor using Gearbox
    explicit STRGeometryPar(const GearDir&);

    //! Destructor
    ~STRGeometryPar();

    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

    //
    // Setters and Getters
    //
    ///////////////////////

    //! Get STR geometry version
    int getVersion() const { return m_Version; }

    //! Set STR geometry version
    void setVersion(int version) { m_Version = version; }

    //
    // Shield patameters
    //


    //! Get the number of layers in the shield
    int getNLayers(int iShield) const { return m_NLayers[iShield] ; }

    //! Set the number of layers in the shield
    void setNLayers(int iShield, int nLayers) { m_NLayers[iShield] = nLayers ;}

    //! Get the material of a layer (as a std::string)
    std::string getLayerMaterial(int iShield, int layer) const { return m_LayerMaterial[iShield].at(layer);}

    //! Set the material of a layer (as a std::string)
    void setLayerMaterial(int iShield, int layer, std::string material) { m_LayerMaterial[iShield].at(layer) = material; }

    //! Get Number of polycone planes in each layer
    int getLayerNPlanes(int iShield, int layer) const
    {
      return m_LayerNPlanes[iShield].at(layer);
    }

    //! Set Number of polycone planes in each layer
    void setLayerNPlanes(int iShield, int layer, int nplanes)
    {
      m_LayerNPlanes[iShield].at(layer) = nplanes;
    }

    //! Get Z-position of one plane (w.r.t. IP)
    double getLayerPlaneZ(int iShield, int layer, int plane) const
    {
      return m_LayerPlaneZ[iShield].at(layer).at(plane);
    }

    //! Get Z-positions of all planes (w.r.t. IP)
    const double* getLayerPlaneZ(int iShield, int layer) const
    {
      return &m_LayerPlaneZ[iShield].at(layer).at(0);
    }


    //! Set Z-position of each plane (w.r.t. IP)
    void setLayerPlaneZ(int iShield, int layer, int plane, double z)
    {
      m_LayerPlaneZ[iShield].at(layer).at(plane) = z;
    }


    //! Get Inner radius of one plane
    double getLayerPlaneInnerRadius(int iShield, int layer, int plane) const
    {
      return m_LayerPlaneInnerRadius[iShield].at(layer).at(plane);
    }

    //! Get Inner radius of all planes
    const  double* getLayerPlaneInnerRadius(int iShield, int layer) const
    {
      return &m_LayerPlaneInnerRadius[iShield].at(layer).at(0);
    }

    //! Set Inner radii of the planes
    void setLayerPlaneInnerRadius(int iShield, int layer, int plane, double r)
    {
      m_LayerPlaneInnerRadius[iShield].at(layer).at(plane) = r;
    }

    //! Get Outer radii of one planes
    double getLayerPlaneOuterRadius(int iShield, int layer, int plane) const
    {
      return m_LayerPlaneOuterRadius[iShield].at(layer).at(plane);
    }

    //! Get Outer radii of all planes
    const double* getLayerPlaneOuterRadius(int iShield, int layer) const
    {
      return &m_LayerPlaneOuterRadius[iShield].at(layer).at(0);
    }

    //! Set Outer radii of the planes
    void setLayerPlaneOuterRadius(int iShield, int layer, int plane, double r)
    {
      m_LayerPlaneOuterRadius[iShield].at(layer).at(plane) = r;
    }


    //
    // Constants that will _always_ be true
    // These ate not design parameters but fundamental
    // characteristics of the system. e.g. we will always
    // have only one forward and one backward shield.
    //
    static const int NECLSHIELDS = 2;
    static const int FWD_ECLSHIELD = 0;
    static const int BWD_ECLSHIELD = 1;



  private:


    //! Read ECL shield parameters
    /*!
      \param side is either "FWD_Shield" or "BWD_Shield". Checking is done first in this method
    */
    void ReadShield(const GearDir& content, std::string side);

    //! static pointer to the singleton instance of this class
    static STRGeometryPar* m_Instance;


    //
    //         Data members:
    // these are the shield parameters
    //
    ////////////////////////////////////

    //! Geometry version
    int m_Version;

    //
    // Forward shield
    //

    //! Number of layers in the shield
    int m_NLayers[NECLSHIELDS];

    //! Material of each of the layers
    std::vector<std::string> m_LayerMaterial[NECLSHIELDS];

    //! Number of polycone planes in each layer
    std::vector<int> m_LayerNPlanes[NECLSHIELDS];

    //! Z-position of each plane (w.r.t. IP)
    std::vector< std::vector<double> > m_LayerPlaneZ[NECLSHIELDS];

    //! Inner radii of the planes
    std::vector< std::vector<double> > m_LayerPlaneInnerRadius[NECLSHIELDS];

    //! Outer radii of the planes
    std::vector< std::vector<double> > m_LayerPlaneOuterRadius[NECLSHIELDS];



    //
    // For ROOT objects
    //

    ClassDef(STRGeometryPar, 0);  /**< ClassDef, must be the last term before the closing {}*/


  };
} // end of namespace Belle2


