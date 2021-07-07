/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <geometry/Materials.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/utilities/TestHelpers.h>

#include <G4Element.hh>
#include <G4Material.hh>
#include <G4OpticalSurface.hh>
#include <G4MaterialPropertiesTable.hh>

using namespace std;

namespace Belle2 {
  namespace geometry {

    /** Check that we can find hydrogen and that the basic Parameters are correct.
     */
    TEST(Materials, Element)
    {
      Materials& m = Materials::getInstance();
      G4Element* e1 = m.getElement("H");
      ASSERT_TRUE(e1);
      EXPECT_EQ("H", e1->GetName());
      EXPECT_EQ("H", e1->GetSymbol());
      EXPECT_EQ(1., e1->GetZ());
    }

    /** Check if we find the Air Material which is named G4_AIR in Geant4
     *  So check if Air and G4_AIR refer to the same material
     */
    TEST(Materials, Material)
    {
      G4Material* m1 = Materials::get("Air");
      G4Material* m2 = Materials::get("G4_AIR");
      EXPECT_EQ(m1, m2);
      ASSERT_TRUE(m1);
      EXPECT_EQ("G4_AIR", m1->GetName());
      EXPECT_TRUE(Materials::get("Si"));
    }

    /** Check creation of a simple mixture with fractions of other materials
     *  The density of the new material should be equal to the weighted sum of the original densities
     */
    TEST(Materials, Create)
    {
      Gearbox& gb = Gearbox::getInstance();
      vector<string> backends;
      backends.emplace_back("string:<Material name='Test'><state>Gas</state><Components>"
                            "<Material fraction='1.5'>Si</Material>"
                            "<Material fraction='2.5612'>Au</Material>"
                            "</Components></Material>");
      gb.setBackends(backends);
      gb.open();
      Materials& m = Materials::getInstance();
      ASSERT_EQ(0, gb.getNumberNodes("/Material/density"));
      ASSERT_EQ(1, gb.getNumberNodes("/Material/@name"));

      G4Material* au = Materials::get("Au");
      G4Material* si = Materials::get("Si");
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_TRUE(mat);
      EXPECT_EQ("Test", mat->GetName());
      EXPECT_DOUBLE_EQ((2.5612 * au->GetDensity() + 1.5 * si->GetDensity()) / 4.0612, mat->GetDensity());
      gb.close();
    }

    /** When adding elements one has to specify a density since elements do not have a density
     */
    TEST(Materials, CreateDensityError)
    {
      Gearbox& gb = Gearbox::getInstance();
      vector<string> backends;
      backends.emplace_back("string:<Material name='Test1'><state>Solid</state><Components>"
                            "<Material fraction='0.5'>Si</Material>"
                            "<Element fraction='0.5'>Au</Element>"
                            "</Components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials& m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_FALSE(mat);
      gb.close();
    }

    /** Same as above, but with density so it should work
     */
    TEST(Materials, CreateDensity)
    {
      Gearbox& gb = Gearbox::getInstance();
      vector<string> backends;
      backends.emplace_back("string:<Material name='Test2'><state>Liquid</state><density>1</density><Components>"
                            "<Material>Si</Material>"
                            "<Element>Au</Element>"
                            "</Components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials& m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_TRUE(mat);
      gb.close();
    }

    /** When adding unknown materials we should get NULL
     */
    TEST(Materials, CreateMaterialError)
    {
      Gearbox& gb = Gearbox::getInstance();
      vector<string> backends;
      backends.emplace_back("string:<Material name='Test3'><Components>"
                            "<Material>Foo</Material>"
                            "</Components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials& m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_FALSE(mat);
      gb.close();
    }

    /** When adding unknown elements we should get NULL
     */
    TEST(Materials, CreateElementError)
    {
      Gearbox& gb = Gearbox::getInstance();
      vector<string> backends;
      backends.emplace_back("string:<Material name='Test4'><density>1</density><Components>"
                            "<Element>Foo</Element>"
                            "</Components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials& m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_FALSE(mat);
      gb.close();
    }

    /** Check the OpticalSurface setting
     */
    TEST(Materials, OpticalSurface)
    {
      Gearbox& gb = Gearbox::getInstance();
      vector<string> backends;
      backends.emplace_back("string:<test><Surface/>"
                            "<Surface name='test'><Model>unified</Model><Finish>Ground</Finish>"
                            "<Type>x_ray</Type><Value>2.0</Value></Surface>"
                            "<Surface><Model>not existing</Model></Surface>"
                            "<Surface><Finish>not existing</Finish></Surface>"
                            "<Surface><Type>not existing</Type></Surface></test>"
                           );
      gb.setBackends(backends);
      gb.open();

      Materials& m = Materials::getInstance();

      G4OpticalSurface* surf1 = m.createOpticalSurface(GearDir("/test/Surface[1]"));
      ASSERT_TRUE(surf1);
      EXPECT_EQ("OpticalSurface", surf1->GetName());
      EXPECT_EQ(glisur, surf1->GetModel());
      EXPECT_EQ(polished, surf1->GetFinish());
      EXPECT_EQ(dielectric_dielectric, surf1->GetType());
      EXPECT_EQ(1.0, surf1->GetPolish());

      G4OpticalSurface* surf2 = m.createOpticalSurface(GearDir("/test/Surface[2]"));
      ASSERT_TRUE(surf2);
      EXPECT_EQ("test", surf2->GetName());
      EXPECT_EQ(unified, surf2->GetModel());
      EXPECT_EQ(ground, surf2->GetFinish());
      EXPECT_EQ(x_ray, surf2->GetType());
      EXPECT_EQ(2.0, surf2->GetSigmaAlpha());

      EXPECT_B2FATAL(m.createOpticalSurface(GearDir("/test/Surface[3]")));
      EXPECT_B2FATAL(m.createOpticalSurface(GearDir("/test/Surface[4]")));
      EXPECT_B2FATAL(m.createOpticalSurface(GearDir("/test/Surface[5]")));
    }

    /** Check the material properties (need to be checked)
     */
    TEST(Materials, Properties)
    {
      Gearbox& gb = Gearbox::getInstance();
      vector<string> backends;
      backends.emplace_back("string:<Material name='TestProperties'>"
                            "<Components><Material>Si</Material></Components>"
                            "<Property name='RINDEX' unit='eV'>"
                            "<value energy='1.0'>1.40</value>"
                            "<value energy='1.5'>1.41</value>"
                            "<value energy='2.0'>1.42</value>"
                            "<value energy='3.5'>1.43</value>"
                            "</Property>"
                            "</Material>");
      gb.setBackends(backends);
      gb.open();
      Materials& m = Materials::getInstance();

      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_TRUE(mat);
      G4MaterialPropertiesTable* properties = mat->GetMaterialPropertiesTable();
      ASSERT_TRUE(properties);
      G4MaterialPropertyVector* property = properties->GetProperty("RINDEX");
      ASSERT_TRUE(property);
      EXPECT_EQ(4u, property->GetVectorLength());
      EXPECT_DOUBLE_EQ(1.40, property->GetMinValue());
      EXPECT_DOUBLE_EQ(1.43, property->GetMaxValue());
      EXPECT_DOUBLE_EQ(1 * CLHEP::eV, property->GetMinLowEdgeEnergy());
      EXPECT_DOUBLE_EQ(3.5 * CLHEP::eV, property->GetMaxLowEdgeEnergy());
      EXPECT_DOUBLE_EQ(1.0 * CLHEP::eV, property->Energy(0));
      EXPECT_DOUBLE_EQ(1.5 * CLHEP::eV, property->Energy(1));
      EXPECT_DOUBLE_EQ(2.0 * CLHEP::eV, property->Energy(2));
      EXPECT_DOUBLE_EQ(3.5 * CLHEP::eV, property->Energy(3));
      EXPECT_DOUBLE_EQ(1.40, (*property)[0]);
      EXPECT_DOUBLE_EQ(1.41, (*property)[1]);
      EXPECT_DOUBLE_EQ(1.42, (*property)[2]);
      EXPECT_DOUBLE_EQ(1.43, (*property)[3]);
      gb.close();
    }
  } // namespace geometry
}  // namespace Belle2
