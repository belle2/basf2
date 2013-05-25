/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOARICHBTESTCREATOR_H
#define GEOARICHBTESTCREATOR_H


#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>


class  G4LogicalVolume;
class G4Material;
namespace Belle2 {
  namespace arich {

    class SensitiveDetector;
    class SensitiveAero;




    /** The creator for the PXD geometry of the Belle II detector.   */
    class GeoARICHBtestCreator : public geometry::CreatorBase {

    public:

      /** Constructor of the GeoPXDCreator class. */
      GeoARICHBtestCreator();

      /** The destructor of the GeoPXDCreator class. */
      virtual ~GeoARICHBtestCreator();

      /**
       * Creates the ROOT Objects for the ARICH Beamtest 2011 geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */

      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    protected:
      void createBtestGeometry(const GearDir& content, G4LogicalVolume& topVolume);

      G4LogicalVolume* buildModule(GearDir Module);
      double getAvgRINDEX(G4Material* material);
      SensitiveDetector* m_sensitive;
      SensitiveAero* m_sensitiveAero;
      int isBeamBkgStudy;
      G4Material*  createAerogel(const char* aeroname, double rind, double trl);
    private:
      int m_runno ;
      std::string m_author;
      int m_neve;
      std::string m_runtype;
      std::string m_hapdID;
      std::string m_aerogelID;
      std::string m_mirrorID;
      double m_rotation;
      double m_rx;
      double m_ry;
      std::string m_mytype;
      std::string m_daqqa;
      std::string m_comment;
      std::string m_datum;
      int m_aerosupport;
      double m_aerogeldx;
      double     m_framedx;
      double     m_rotation1;
      int    m_configuration;
      std::string  m_comment1;
      std::vector<double> m_agelrefind;
      std::vector<double> m_agelthickness;
      std::vector<double> m_ageltrlen;
    };

  }
}

#endif /* GEOARICHBTESTCREATOR_H */
