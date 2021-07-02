/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>

#include <vector>
#include <string>
//#include <algorithm>

namespace Belle2 {
  namespace CurlTagger {

    /** class to contain particles identified to come from the same actual/mc particle */
    class Bundle {

    public:
      /** Constructor */
      explicit Bundle(bool isTruthBundle);

      /** Destructor */
      ~Bundle();

      /** adds Particle to Bundle */
      void addParticle(Particle* particle);

      /** gets Particle from Bundle */
      Particle* getParticle(unsigned int i);

      /** gets Bundle size */
      unsigned int size();

      /** sets curl extra info for particles in Bundle */
      void tagCurlInfo();

      /** sets extraInfo with size of Bundle for particles in Bundle */
      void tagSizeInfo();

    private:

      /** scaled impact parameter for selecting best track */
      float scaledImpactParam(Particle* particle);

      /** dr scale factor */
      double m_Gamma;

      /** used to rank particles by scaledImpactParam - currently unused*/
      bool compareParticles(Particle* iPart, Particle* jPart);

      /** flag for if the bundle is based on truth info, changes which labels are used */
      bool m_IsTruthBundle;

      /** label used to tag particles with curl info */
      std::string m_CurlLabel;

      /** label used to tag particles with Bundle size */
      std::string m_SizeLabel;

      /** particles in bundle */
      std::vector<Particle*> m_Particles;

    };//class

  } // Curl Tagger namespace
} // Belle 2 namespace

