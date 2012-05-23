


#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <framework/dataobjects/RelationElement.h>
#include <svd/simulation/SVDSignal.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <math.h>

using namespace std;

namespace Belle2 {
  namespace SVD {

    /**
     * Check creation and simple object getters.
     */
    TEST(SVDSignal, Getters)
    {
      // First create an artificial signal and load it with data.
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
      SVDSignal signal;
      for (int i = 0; i < 3; ++i) signal.add(times.at(i), charges.at(i), tau, mcParticles.at(i), trueHits.at(i));
      // Test SVDSignal::getCharge()
      double totalcharge = 0;
      for (int i = 0; i < 3; ++i) totalcharge += charges.at(i);
      EXPECT_EQ(signal.getCharge(), totalcharge);
      //Test SVDSignal::getFunctions()
      SVDSignal::function_list functions = signal.getFunctions();
      for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(functions.at(i).m_initTime, times.at(i));
        EXPECT_EQ(functions.at(i).m_charge, charges.at(i));
        EXPECT_EQ(functions.at(i).m_tau, tau);
        EXPECT_EQ(functions.at(i).m_particle, mcParticles.at(i));
        EXPECT_EQ(functions.at(i).m_truehit, trueHits.at(i));
      }
    }

    TEST(SVDSignal, Constructors)
    {
      // Create an artificial signal and load it with data.
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
      SVDSignal signal;
      for (int i = 0; i < 3; ++i) signal.add(times.at(i), charges.at(i), tau, mcParticles.at(i), trueHits.at(i));
      // Test the copy constructor
      SVDSignal signal2(signal);
      double totalcharge = 0;
      for (int i = 0; i < 3; ++i) totalcharge += charges.at(i);
      EXPECT_EQ(signal2.getCharge(), totalcharge);
      //Test SVDSignal::getFunctions()
      SVDSignal::function_list functions = signal2.getFunctions();
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

    TEST(SVDSignal, Assignment)
    {
      // Create an artificial signal and load it with data.
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
      SVDSignal signal;
      for (int i = 0; i < 3; ++i) signal.add(times.at(i), charges.at(i), tau, mcParticles.at(i), trueHits.at(i));
      // Test the assignment operator
      SVDSignal signal2 = signal;
      double totalcharge = 0;
      for (int i = 0; i < 3; ++i) totalcharge += charges.at(i);
      EXPECT_EQ(signal2.getCharge(), totalcharge);
      //Test SVDSignal::getFunctions()
      SVDSignal::function_list functions = signal2.getFunctions();
      for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(functions.at(i).m_initTime, times.at(i));
        EXPECT_EQ(functions.at(i).m_charge, charges.at(i));
        EXPECT_EQ(functions.at(i).m_tau, tau);
        EXPECT_EQ(functions.at(i).m_particle, mcParticles.at(i));
        EXPECT_EQ(functions.at(i).m_truehit, trueHits.at(i));
      }
    }

    TEST(SVDSignal, Operators)
    {
      // Create an artificial signal and load it with data.
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
      SVDSignal signal;
      for (int i = 0; i < 3; ++i) signal.add(times.at(i), charges.at(i), tau, mcParticles.at(i), trueHits.at(i));
      // Test the scaling operator
      double scale = 3.0;
      SVDSignal signal2 = signal;
      signal2 *= scale;
      double totalcharge = 0;
      for (int i = 0; i < 3; ++i) totalcharge += charges.at(i);
      EXPECT_EQ(signal2.getCharge(), scale * totalcharge);
      //Test SVDSignal::getFunctions()
      SVDSignal::function_list functions = signal2.getFunctions();
      for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(functions.at(i).m_initTime, times.at(i));
        EXPECT_EQ(functions.at(i).m_charge, scale * charges.at(i));
        EXPECT_EQ(functions.at(i).m_tau, tau);
        EXPECT_EQ(functions.at(i).m_particle, mcParticles.at(i));
        EXPECT_EQ(functions.at(i).m_truehit, trueHits.at(i));
      }
      /* Test the += operator. */
      signal += signal2;
      EXPECT_EQ(signal.getCharge(), (1.0 + scale)*totalcharge);
      //Test SVDSignal::getFunctions()
      functions = signal.getFunctions();
      EXPECT_EQ(functions.size(), 6);
      for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(functions.at(i).m_initTime, times.at(i));
        EXPECT_EQ(functions.at(i).m_charge, charges.at(i));
        EXPECT_EQ(functions.at(i).m_tau, tau);
        EXPECT_EQ(functions.at(i).m_particle, mcParticles.at(i));
        EXPECT_EQ(functions.at(i).m_truehit, trueHits.at(i));
        // the signal2 contribution
        EXPECT_EQ(functions.at(3 + i).m_initTime, times.at(i));
        EXPECT_EQ(functions.at(3 + i).m_charge, scale * charges.at(i));
        EXPECT_EQ(functions.at(3 + i).m_tau, tau);
        EXPECT_EQ(functions.at(3 + i).m_particle, mcParticles.at(i));
        EXPECT_EQ(functions.at(3 + i).m_truehit, trueHits.at(i));
      }
    }

    TEST(SVDSignal, Waveform)
    {
      // Create an artificial signal and load it with data.
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
      SVDSignal signal;
      for (int i = 0; i < 3; ++i) signal.add(times.at(i), charges.at(i), tau, mcParticles.at(i), trueHits.at(i));
      // Test operator()
      double time = 40.0e-9; // 40 ns
      // expected function value
      double expected_value = 0;
      SVDSignal::relations_map expected_particles_map;
      SVDSignal::relations_map expected_truehits_map;
      int i = 0;
      BOOST_FOREACH(SVDSignal::Wave wave, signal.getFunctions()) {
        double value = signal.waveform(time, wave);
        expected_value += value;
        expected_particles_map[mcParticles.at(i)] += value;
        expected_truehits_map[trueHits.at(i)] += value;
        i++;
      }
      double function_value;
      SVDSignal::relations_map particles;
      SVDSignal::relations_map truehits;
      boost::tie(function_value, particles, truehits) = signal(time);
      EXPECT_EQ(function_value, expected_value);
      //Particles
      size_t np_expected = expected_particles_map.size();
      size_t np_actual = particles.size();
      EXPECT_EQ(np_actual, np_expected);
      size_t nt_expected = expected_truehits_map.size();
      size_t nt_actual = truehits.size();
      EXPECT_EQ(nt_actual, nt_expected);
      for (int j = 0; j < 3; ++j) {
        EXPECT_EQ(particles[mcParticles.at(j)], expected_particles_map[mcParticles.at(j)]);
        EXPECT_EQ(truehits[trueHits.at(j)], expected_truehits_map[trueHits.at(j)]);
      }
    }




  } // namespace SVD
}  // namespace Belle2
