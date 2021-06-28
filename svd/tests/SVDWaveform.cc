


#include <framework/dataobjects/RelationElement.h>
#include <svd/simulation/SVDWaveform.h>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace Belle2 {
  namespace SVD {

    /**
     * Check object creation and simple object getters.
     */
    TEST(SVDWaveform, Getters)
    {
      // First create an artificial waveform and load it with data.
      vector<double> charges;
      charges.push_back(10);
      charges.push_back(11);
      charges.push_back(30);
      vector<double> times;
      for (int i = 0; i < 3; ++i) times.push_back(i * 15.0e-9);
      vector<RelationElement::index_type> mcParticles;
      for (int i = 0; i < 3; ++i) mcParticles.push_back(3 * i);
      vector<RelationElement::index_type> trueHits;
      for (int i = 0; i < 3; ++i) trueHits.push_back(7 * i);
      const float tau = 25.0e-9;
      SVDWaveform waveform;
      for (int i = 0; i < 3; ++i) waveform.add(times.at(i), charges.at(i), tau, mcParticles.at(i), trueHits.at(i));
      // Test SVDWaveform::getCharge()
      double totalcharge = 0;
      for (int i = 0; i < 3; ++i) totalcharge += charges.at(i);
      EXPECT_EQ(waveform.getCharge(), totalcharge);
      //Test SVDWaveform::getElementaryWaveforms()
      SVDWaveform::elementary_waveform_list functions = waveform.getElementaryWaveforms();
      for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(functions.at(i).m_initTime, times.at(i));
        EXPECT_EQ(functions.at(i).m_charge, charges.at(i));
        EXPECT_EQ(functions.at(i).m_tau, tau);
        EXPECT_EQ(functions.at(i).m_particle, mcParticles.at(i));
        EXPECT_EQ(functions.at(i).m_truehit, trueHits.at(i));
      }
    }

    /** Check the different constructors. */
    TEST(SVDWaveform, Constructors)
    {
      // Create an artificial waveform and load it with data.
      vector<double> charges;
      charges.push_back(10);
      charges.push_back(11);
      charges.push_back(30);
      vector<double> times;
      for (int i = 0; i < 3; ++i) times.push_back(i * 15.0e-9);
      vector<RelationElement::index_type> mcParticles;
      for (int i = 0; i < 3; ++i) mcParticles.push_back(3 * i);
      vector<RelationElement::index_type> trueHits;
      for (int i = 0; i < 3; ++i) trueHits.push_back(7 * i);
      const float tau = 25.0e-9;
      SVDWaveform waveform;
      for (int i = 0; i < 3; ++i) waveform.add(times.at(i), charges.at(i), tau, mcParticles.at(i), trueHits.at(i));
      // Test the copy constructor
      SVDWaveform waveform2(waveform);
      double totalcharge = 0;
      for (int i = 0; i < 3; ++i) totalcharge += charges.at(i);
      EXPECT_EQ(waveform2.getCharge(), totalcharge);
      //Test SVDWaveform::getElementaryWaveforms()
      SVDWaveform::elementary_waveform_list functions = waveform2.getElementaryWaveforms();
      size_t n = functions.size();
      EXPECT_EQ(n, 3);
      for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(functions.at(i).m_initTime, times.at(i));
        EXPECT_EQ(functions.at(i).m_charge, charges.at(i));
        EXPECT_EQ(functions.at(i).m_tau, tau);
        EXPECT_EQ(functions.at(i).m_particle, mcParticles.at(i));
        EXPECT_EQ(functions.at(i).m_truehit, trueHits.at(i));
      }
    }

    /** Check assignment operators. */
    TEST(SVDWaveform, Assignment)
    {
      // Create an artificial waveform and load it with data.
      vector<double> charges;
      charges.push_back(10);
      charges.push_back(11);
      charges.push_back(30);
      vector<double> times;
      for (int i = 0; i < 3; ++i) times.push_back(i * 15.0e-9);
      vector<RelationElement::index_type> mcParticles;
      for (int i = 0; i < 3; ++i) mcParticles.push_back(3 * i);
      vector<RelationElement::index_type> trueHits;
      for (int i = 0; i < 3; ++i) trueHits.push_back(7 * i);
      const float tau = 25.0e-9;
      SVDWaveform waveform;
      for (int i = 0; i < 3; ++i) waveform.add(times.at(i), charges.at(i), tau, mcParticles.at(i), trueHits.at(i));
      // Test the assignment operator
      SVDWaveform waveform2 = waveform;
      double totalcharge = 0;
      for (int i = 0; i < 3; ++i) totalcharge += charges.at(i);
      EXPECT_EQ(waveform2.getCharge(), totalcharge);
      //Test SVDWaveform::getElementaryWaveforms()
      SVDWaveform::elementary_waveform_list functions = waveform2.getElementaryWaveforms();
      for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(functions.at(i).m_initTime, times.at(i));
        EXPECT_EQ(functions.at(i).m_charge, charges.at(i));
        EXPECT_EQ(functions.at(i).m_tau, tau);
        EXPECT_EQ(functions.at(i).m_particle, mcParticles.at(i));
        EXPECT_EQ(functions.at(i).m_truehit, trueHits.at(i));
      }
    }

    /** Check the waveform values and relations. */
    TEST(SVDWaveform, Waveform)
    {
      // Create an artificial waveform and load it with data.
      vector<double> charges;
      charges.push_back(10);
      charges.push_back(11);
      charges.push_back(30);
      vector<double> times;
      for (int i = 0; i < 3; ++i) times.push_back(i * 15.0e-9);
      vector<RelationElement::index_type> mcParticles;
      for (int i = 0; i < 3; ++i) mcParticles.push_back(3 * i);
      vector<RelationElement::index_type> trueHits;
      for (int i = 0; i < 3; ++i) trueHits.push_back(7 * i);
      const float tau = 25.0e-9; // 25 ns shaping time
      SVDWaveform waveform;
      for (int i = 0; i < 3; ++i) waveform.add(times.at(i), charges.at(i), tau, mcParticles.at(i), trueHits.at(i));
      // Test operator()
      double time = 40.0e-9; // 40 ns
      // expected function value
      double expected_value = 0;
      SVDWaveform::relations_map expected_particles_map;
      SVDWaveform::relations_map expected_truehits_map;
      int i = 0;
      for (SVDWaveform::ElementaryWaveform wave : waveform.getElementaryWaveforms()) {
        double value = waveform.waveform(time, wave);
        expected_value += value;
        expected_particles_map[mcParticles.at(i)] += wave.m_charge;
        expected_truehits_map[trueHits.at(i)] += wave.m_charge;
        i++;
      }
      double function_value = waveform(time);
      EXPECT_EQ(function_value, expected_value);
    }

    /** Check the waveform string representation. */
    TEST(SVDWaveform, toString)
    {
      // Create an artificial waveform and load it with data.
      vector<double> charges;
      charges.push_back(10);
      charges.push_back(11);
      charges.push_back(30);
      vector<double> times;
      for (int i = 0; i < 3; ++i) times.push_back(i * 15.0e-9);
      vector<RelationElement::index_type> mcParticles;
      for (int i = 0; i < 3; ++i) mcParticles.push_back(3 * i);
      vector<RelationElement::index_type> trueHits;
      for (int i = 0; i < 3; ++i) trueHits.push_back(7 * i);
      const float tau = 50.0e-9; // 25 ns shaping time
      SVDWaveform waveform;
      for (int i = 0; i < 3; ++i) waveform.add(times.at(i), charges.at(i), tau, mcParticles.at(i), trueHits.at(i));
      std::ostringstream os;
      for (int i = 0; i < 3; ++i)
        os << i + 1 << '\t' << times.at(i) << '\t' << charges.at(i) << '\t' << tau << std::endl;
      EXPECT_EQ(waveform.toString(), os.str());
    }

  } // namespace SVD
}  // namespace Belle2
