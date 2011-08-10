#include <geometry/Materials.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

#include <G4Element.hh>
#include <G4Material.hh>
#include <G4OpticalSurface.hh>
#include <G4MaterialPropertiesTable.hh>

using namespace std;

/** stream operator for G4MPVEntry, needed for testing of PropertyTable */
ostream& operator<<(ostream &out, G4MPVEntry entry)
{
  out << "G4MPVEntry(" << entry.GetPhotonEnergy() << "," << entry.GetProperty() << ")";
  return out;
}

namespace Belle2 {
  namespace geometry {

    TEST(Materials, Element)
    {
      //Check that we can find hydrogen and that the basic Parameters
      //are correct.
      Materials &m = Materials::getInstance();
      G4Element* e1 = m.getElement("H");
      ASSERT_TRUE(e1);
      EXPECT_EQ("H", e1->GetName());
      EXPECT_EQ("H", e1->GetSymbol());
      EXPECT_EQ(1., e1->GetZ());
    }

    TEST(Materials, Material)
    {
      //Check if we find the Air Material which is named G4_AIR in Geant4
      //So check if Air and G4_AIR refer to the same material
      G4Material* m1 = Materials::get("Air");
      G4Material* m2 = Materials::get("G4_AIR");
      EXPECT_EQ(m1, m2);
      ASSERT_TRUE(m1);
      EXPECT_EQ("G4_AIR", m1->GetName());
      EXPECT_TRUE(Materials::get("Si"));
    }

    TEST(Materials, Create)
    {
      //Check creation of a simple mixture with fractions of other materials
      //The density of the new material should be equal to the weighted sum
      //of the original densities
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='Test'><state>Gas</state><Components>"
                         "<Material fraction='1.5'>Si</Material>"
                         "<Material fraction='2.5612'>Au</Material>"
                         "</Components></Material>");
      gb.setBackends(backends);
      gb.open();
      Materials &m = Materials::getInstance();
      ASSERT_EQ(0, gb.getNumberNodes("/Material/density"));
      ASSERT_EQ(1, gb.getNumberNodes("/Material/@name"));

      G4Material* au = Materials::get("Au");
      G4Material* si = Materials::get("Si");
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_TRUE(mat);
      EXPECT_EQ("Test", mat->GetName());
      EXPECT_DOUBLE_EQ((2.5612*au->GetDensity() + 1.5*si->GetDensity()) / 4.0612, mat->GetDensity());
      gb.close();
    }

    TEST(Materials, CreateDensityError)
    {
      //When adding elements one has to specify a density since elements do not
      //have a density
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='Test1'><state>Solid</state><Components>"
                         "<Material fraction='0.5'>Si</Material>"
                         "<Element fraction='0.5'>Au</Element>"
                         "</Components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials &m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_FALSE(mat);
      gb.close();
    }

    TEST(Materials, CreateDensity)
    {
      //Same as above, but with density so it should work
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='Test2'><state>Liquid</state><density>1</density><Components>"
                         "<Material>Si</Material>"
                         "<Element>Au</Element>"
                         "</Components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials &m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_TRUE(mat);
      gb.close();
    }

    TEST(Materials, CreateMaterialError)
    {
      //When adding unknown materials we should get NULL
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='Test3'><Components>"
                         "<Material>Foo</Material>"
                         "</Components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials &m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_FALSE(mat);
      gb.close();
    }

    TEST(Materials, CreateElementError)
    {
      //When adding unknown elements we should get NULL
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='Test4'><density>1</density><Components>"
                         "<Element>Foo</Element>"
                         "</Components></Material>");
      gb.setBackends(backends);
      gb.open();

      Materials &m = Materials::getInstance();
      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_FALSE(mat);
      gb.close();
    }

    TEST(Materials, OpticalSurface)
    {
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<test><Surface/>"
                         "<Surface name='test'><Model>unified</Model><Finish>Ground</Finish>"
                         "<Type>x_ray</Type><Value>2.0</Value></Surface>"
                         "<Surface><Model>not existing</Model></Surface>"
                         "<Surface><Finish>not existing</Finish></Surface>"
                         "<Surface><Type>not existing</Type></Surface></test>"
                        );
      gb.setBackends(backends);
      gb.open();

      Materials &m = Materials::getInstance();

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

      G4OpticalSurface* surf3 = m.createOpticalSurface(GearDir("/test/Surface[3]"));
      EXPECT_FALSE(surf3);
      G4OpticalSurface* surf4 = m.createOpticalSurface(GearDir("/test/Surface[4]"));
      EXPECT_FALSE(surf4);
      G4OpticalSurface* surf5 = m.createOpticalSurface(GearDir("/test/Surface[5]"));
      EXPECT_FALSE(surf5);
    }

    TEST(Materials, Properties)
    {
      Gearbox &gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("string:<Material name='TestProperties'>"
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
      Materials &m = Materials::getInstance();

      G4Material* mat = m.createMaterial(GearDir("/Material"));
      ASSERT_TRUE(mat);
      G4MaterialPropertiesTable *properties = mat->GetMaterialPropertiesTable();
      ASSERT_TRUE(properties);
      G4MaterialPropertyVector *property = properties->GetProperty("RINDEX");
      ASSERT_TRUE(property);
      EXPECT_EQ(property->Entries(), 4);
      EXPECT_DOUBLE_EQ(property->GetMinProperty(), 1.40);
      EXPECT_DOUBLE_EQ(property->GetMaxProperty(), 1.43);
      EXPECT_DOUBLE_EQ(property->GetMinPhotonEnergy(), 1*eV);
      EXPECT_DOUBLE_EQ(property->GetMaxPhotonEnergy(), 3.5*eV);
      EXPECT_EQ(property->GetEntry(0), G4MPVEntry(1.0*eV, 1.40));
      EXPECT_EQ(property->GetEntry(1), G4MPVEntry(1.5*eV, 1.41));
      EXPECT_EQ(property->GetEntry(2), G4MPVEntry(2.0*eV, 1.42));
      EXPECT_EQ(property->GetEntry(3), G4MPVEntry(3.5*eV, 1.43));
      gb.close();
    }
  } // namespace geometry
}  // namespace Belle2
