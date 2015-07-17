/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Object which can store pointers to hits while processing FastHogh algorithm
 *
 * TODO: check if it's possible to store in each hit list of nodes in which we can meet it.
 *
 */


#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorTemplate.h>

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>
#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

#include <framework/logging/Logger.h>

#include <boost/math/constants/constants.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <boost/multi_array.hpp>

#include <set>
#include <vector>
#include <array>
#include <algorithm>

#include <cstdlib>
#include <iomanip>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <functional>


namespace Belle2 {
  namespace TrackFindingCDC {

    class FastHough;
    class TrackHit;
    class CDCRecoSegment2D;

    class QuadTreeProcessor {

    public:

      QuadTreeProcessor(unsigned char lastLevel) : m_lastLevel(lastLevel) {}

      // version with a threshold on r
      void fillGivenTree(QuadTreeLegendre* node, QuadTreeLegendre::CandidateProcessorLambda& lmdProcessor,
                         unsigned int nHitsThreshold, double rThreshold) const
      {
        this->fillGivenTree(node, lmdProcessor, nHitsThreshold, rThreshold, true);
      }

      // version without a threshold on r
      void fillGivenTree(QuadTreeLegendre* node, QuadTreeLegendre::CandidateProcessorLambda& lmdProcessor,
                         unsigned int nHitsThreshold) const
      {
        this->fillGivenTree(node, lmdProcessor, nHitsThreshold, 0.0f, false);
      }


      void fillChildren(QuadTreeLegendre* m_node, std::vector<TrackHit*>& m_hits) const
      {

        double dist_1[3][3];
        double dist_2[3][3];
        const size_t neededSize = 2 * m_hits.size();
        m_node->getChildren()->apply([neededSize](QuadTreeLegendre * qt) {qt->reserveHitsVector(neededSize);});



        //Voting within the four bins
        for (TrackHit* hit : m_hits) {
          //B2DEBUG(100, "PROCCESSING hit " << hit_counter << " of " << nhitsToReserve);
          if (hit->getHitUsage() != TrackHit::c_notUsed)
            continue;

          for (int t_index = 0; t_index < m_sizeX; ++t_index) {
            for (int r_index = 0; r_index < m_sizeY; ++r_index) {

              float r_temp_min = hit->getConformalX()
                                 * TrigonometricalLookupTable::Instance().cosTheta(m_node->getChildren()->get(t_index, r_index)->getXMin())
                                 + hit->getConformalY()
                                 * TrigonometricalLookupTable::Instance().sinTheta(m_node->getChildren()->get(t_index, r_index)->getXMin());
              float r_temp_max = hit->getConformalX()
                                 * TrigonometricalLookupTable::Instance().cosTheta(m_node->getChildren()->get(t_index, r_index)->getXMax())
                                 + hit->getConformalY()
                                 * TrigonometricalLookupTable::Instance().sinTheta(m_node->getChildren()->get(t_index, r_index)->getXMax());

              float r_min1 = r_temp_min - hit->getConformalDriftLength();
              float r_min2 = r_temp_min + hit->getConformalDriftLength();
              float r_max1 = r_temp_max - hit->getConformalDriftLength();
              float r_max2 = r_temp_max + hit->getConformalDriftLength();

              float m_rMin = m_node->getChildren()->get(t_index, r_index)->getYMin();
              float m_rMax = m_node->getChildren()->get(t_index, r_index)->getYMax();

              dist_1[0][0] = m_rMin - r_min1;
              dist_1[0][1] = m_rMin - r_max1;
              dist_1[1][0] = m_rMax - r_min1;
              dist_1[1][1] = m_rMax - r_max1;

              dist_2[0][0] = m_rMin - r_min2;
              dist_2[0][1] = m_rMin - r_max2;
              dist_2[1][0] = m_rMax - r_min2;
              dist_2[1][1] = m_rMax - r_max2;


              //curves are assumed to be straight lines, might be a reasonable assumption locally
              if (!FastHough::sameSign(dist_1[0][0], dist_1[0][1],
                                       dist_1[1][0], dist_1[1][1])) {
                //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
                m_node->getChildren()->get(t_index, r_index)->insertItem(hit);
              } else if (!FastHough::sameSign(dist_2[0][0], dist_2[0][1],
                                              dist_2[1][0], dist_2[1][1])) {
                //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
                m_node->getChildren()->get(t_index, r_index)->insertItem(hit);
              }



            }
          }
          //B2DEBUG(100, "MOVING to next hit");
        }
      }


      void fillNodeWithRespectToGivenPoint(QuadTreeTemplate<float, float, TrackFindingCDC::TrackHit>* m_node,
                                           std::vector<TrackHit*>& m_hits, std::pair<double, double>& ref_point) const
      {

        double dist_1[3][3];
        double dist_2[3][3];
        const size_t neededSize = 2 * m_hits.size();
        m_node->reserveHitsVector(neededSize);



        //Voting within the four bins
        for (TrackHit* hit : m_hits) {
          //B2DEBUG(100, "PROCCESSING hit " << hit_counter << " of " << nhitsToReserve);
          if (hit->getHitUsage() != TrackHit::c_notUsed)
            continue;

          std::tuple<double, double, double> confCoords = hit->performConformalTransformWithRespectToPoint(ref_point.first, ref_point.second);


          for (int t_index = 0; t_index < m_sizeX; ++t_index) {
            for (int r_index = 0; r_index < m_sizeY; ++r_index) {

              float r_temp_min = std::get<0>(confCoords) * cos(m_node->getXMin())
                                 + std::get<1>(confCoords) * sin(m_node->getXMin());
              float r_temp_max = std::get<0>(confCoords) * cos(m_node->getXMax())
                                 + std::get<1>(confCoords) * sin(m_node->getXMax());

              float r_min1 = r_temp_min - std::get<2>(confCoords);
              float r_min2 = r_temp_min + std::get<2>(confCoords);
              float r_max1 = r_temp_max - std::get<2>(confCoords);
              float r_max2 = r_temp_max + std::get<2>(confCoords);

              float m_rMin = m_node->getYMin();
              float m_rMax = m_node->getYMax();

              dist_1[0][0] = m_rMin - r_min1;
              dist_1[0][1] = m_rMin - r_max1;
              dist_1[1][0] = m_rMax - r_min1;
              dist_1[1][1] = m_rMax - r_max1;

              dist_2[0][0] = m_rMin - r_min2;
              dist_2[0][1] = m_rMin - r_max2;
              dist_2[1][0] = m_rMax - r_min2;
              dist_2[1][1] = m_rMax - r_max2;


              //curves are assumed to be straight lines, might be a reasonable assumption locally
              if (!FastHough::sameSign(dist_1[0][0], dist_1[0][1],
                                       dist_1[1][0], dist_1[1][1])) {
                //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
                m_node->insertItem(hit);
              } else if (!FastHough::sameSign(dist_2[0][0], dist_2[0][1],
                                              dist_2[1][0], dist_2[1][1])) {
                //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
                m_node->insertItem(hit);
              }



            }
          }
          //B2DEBUG(100, "MOVING to next hit");
        }

      }


      void initializeChildren(QuadTreeLegendre* node, QuadTreeLegendre::Children* m_children) const
      {

        for (int i = 0; i < 2; ++i) {
          //m_children[i] = new QuadTreeTemplate*[m_nbins_r];
          for (int j = 0; j < 2; ++j) {
            if ((node->getLevel() > (m_lastLevel - 7)) && (fabs(node->getYMean()) > 0.005)) {
              if (node->getLevel() < (m_lastLevel - 5)) {
                float r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 4.;
                float r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j))  / 4.;
                int theta1 = node->getXBin(i) - std::abs(pow(2, m_lastLevel + 0 - node->getLevel()) / 4.);
                int theta2 = node->getXBin(i + 1) + std::abs(pow(2, m_lastLevel + 0 - node->getLevel()) / 4.);

                if (theta1 < 0)
                  theta1 = node->getXBin(i);
                if (theta2 >= TrigonometricalLookupTable::Instance().getNBinsTheta())
                  theta2 = node->getXBin(i + 1);

                m_children->set(i, j, new QuadTreeTemplate<int, float, TrackHit>(theta1, theta2, r1, r2, node->getLevel() + 1,
                                node));
              } else {
                float r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 8.;
                float r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j))  / 8.;
                int theta1 = node->getXBin(i) - std::abs(pow(2, m_lastLevel + 0 - node->getLevel()) / 8.);
                int theta2 = node->getXBin(i + 1) + std::abs(pow(2, m_lastLevel + 0 - node->getLevel()) / 8.);

                if (theta1 < 0)
                  theta1 = node->getXBin(i);
                if (theta2 >= TrigonometricalLookupTable::Instance().getNBinsTheta())
                  theta2 = node->getXBin(i + 1);

                m_children->set(i, j, new QuadTreeTemplate<int, float, TrackHit>(theta1, theta2, r1, r2, node->getLevel() + 1,
                                node));
              }
            } else {
              m_children->set(i, j, new QuadTreeTemplate<int, float, TrackHit>(node->getXBin(i), node->getXBin(i + 1), node->getYBin(j),
                              node->getYBin(j + 1), node->getLevel() + 1, node));
            }
          }
        }
      }

      void provideItemsSet(const std::set<TrackHit*>& hits_set, std::vector<TrackHit*>& hits_vector) const
      {
        hits_vector.clear();
        std::copy_if(hits_set.begin(), hits_set.end(), std::back_inserter(hits_vector),
        [&](TrackHit * hit) {return (hit->getHitUsage() == TrackHit::c_notUsed);});
      };

      void cleanUpItems(std::vector<TrackHit*>& hits) const
      {
        hits.erase(
          std::remove_if(hits.begin(), hits.end(),
        [&](TrackHit * hit) {return hit->getHitUsage() != TrackHit::c_notUsed;}),
        hits.end());
      } ;

    private:

      // internal version
      void fillGivenTree(QuadTreeLegendre* node, QuadTreeLegendre::CandidateProcessorLambda& lmdProcessor,
                         unsigned int nHitsThreshold, double rThreshold, bool checkThreshold) const
      {
        B2DEBUG(100, "startFillingTree with " << node->getItemsVector().size() << " hits at level " << static_cast<unsigned int>
                (node->getLevel()));
        if (node->getItemsVector().size() < nHitsThreshold)
          return;
        if (checkThreshold) {
          if ((node->getYMin() * node->getYMax() >= 0) && (fabs(node->getYMin()) > rThreshold)  && (fabs(node->getYMax()) > rThreshold))
            return;
        }

        unsigned char level_diff = 0;
        if (fabs(node->getYMean()) > 0.07) level_diff = 2;
        else if ((fabs(node->getYMean()) < 0.07) && (fabs(node->getYMean()) > 0.04))
          level_diff = 1;
        if (node->getLevel() >= (m_lastLevel - level_diff)) {
          lmdProcessor(node);
          return;
        }



        if (node->getChildren() == nullptr)
          node->initializeChildren(*this);

        if (!node->checkFilled()) {
          fillChildren(node, node->getItemsVector());
          node->setFilled();
        }

        constexpr int m_nbins_theta = m_sizeX;
        constexpr int m_nbins_r = m_sizeY;

        bool binUsed[m_nbins_theta][m_nbins_r];
        for (int ii = 0; ii < m_nbins_theta; ii++)
          for (int jj = 0; jj < m_nbins_r; jj++)
            binUsed[ii][jj] = false;

        //Processing, which bins are further investigated
        for (int bin_loop = 0; bin_loop < m_nbins_theta * m_nbins_r; bin_loop++) {
          int t_index = 0;
          int r_index = 0;
          float max_value_temp = 0;
          for (int t_index_temp = 0; t_index_temp < m_nbins_theta; ++t_index_temp) {
            for (int r_index_temp = 0; r_index_temp < m_nbins_r; ++r_index_temp) {
              if ((max_value_temp < node->getChildren()->get(t_index_temp, r_index_temp)->getNItems())
                  && (!binUsed[t_index_temp][r_index_temp])) {
                max_value_temp = node->getChildren()->get(t_index_temp, r_index_temp)->getNItems();
                t_index = t_index_temp;
                r_index = r_index_temp;
              }
            }
          }

          binUsed[t_index][r_index] = true;

          node->getChildren()->get(t_index, r_index)->cleanUpItems(*this);
          this->fillGivenTree(node->getChildren()->get(t_index, r_index), lmdProcessor, nHitsThreshold, rThreshold, checkThreshold);
        }
      }

      unsigned char m_lastLevel;

      // size of the x and y component of the quad tree children
      // obviously this will be fixed to 2 for a quad tree.
      constexpr static int m_sizeX = 2;
      constexpr static int m_sizeY = 2;
    };

  }
}
