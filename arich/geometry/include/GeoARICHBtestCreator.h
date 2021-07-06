/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef GEOARICHBTESTCREATOR_H
#define GEOARICHBTESTCREATOR_H


#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>

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

      /** Copy constructor (disabled). */
      GeoARICHBtestCreator(const GeoARICHBtestCreator&) = delete;

      /** The destructor of the GeoPXDCreator class. */
      virtual ~GeoARICHBtestCreator();

      /** Operator = (disabled). */
      GeoARICHBtestCreator& operator=(const GeoARICHBtestCreator&) = delete;

      /**
       * Creates the ROOT Objects for the ARICH Beamtest 2011 geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */

      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    protected:
      //! Creation of the beamtest geometry
      void createBtestGeometry(const GearDir& content, G4LogicalVolume& topVolume);

      //! Build the module.
      G4LogicalVolume* buildModule(GearDir Module);

      //! Get the average refractive index if the material
      double getAvgRINDEX(G4Material* material);
      //! pointer to the sensitive detector
      SensitiveDetector* m_sensitive;
      //! pointer to the sesnitive aerogel
      SensitiveAero* m_sensitiveAero;

      //int isBeamBkgStudy;
      //! create aerogel material
      G4Material*  createAerogel(const char* aeroname, double rind, double trl);
    private:
      //! Beamtest Run number
      int m_runno ;
      //! Beamtest runlog record author
      std::string m_author;
      //! Number of event in the beamtest run
      int m_neve;
      //! Type of the beamtest run
      std::string m_runtype;
      //! ID of the HAPD configuration setup
      std::string m_hapdID;
      //! ID of the aerogel configuration setup
      std::string m_aerogelID;
      //! ID of the mirror configuration setup
      std::string m_mirrorID;
      //! rotation angle of the setup
      double m_rotation;
      //! x shift of the prototype ARICH frame
      double m_rx;
      //! y shift of the prototype ARICH frame
      double m_ry;
      //! type of the run
      std::string m_mytype;
      //! classification of the run
      std::string m_daqqa;
      //! comment in the runlog
      std::string m_comment;
      //! datum of the runlog
      std::string m_datum;

      //! Type of aerogel support - not used at the moment
      int m_aerosupport;
      //! shift of the aerogel center
      double m_aerogeldx;
      //! shift of the frame
      double     m_framedx;
      //! rotation angle of the frame
      double     m_rotation1;
      //! configuration number of the HAPD
      int    m_configuration;
      //! tbc
      std::string  m_comment1;

      //! vector of aerogel refractive indices
      std::vector<double> m_agelrefind;
      //! vector of aerogel thicknesses
      std::vector<double> m_agelthickness;
      //! vector of aerogel transmission lengths
      std::vector<double> m_ageltrlen;
    };

  }
}

#endif /* GEOARICHBTESTCREATOR_H */
