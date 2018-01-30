/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/

#pragma once

#include <framework/core/Module.h>
// #include <fstream>
#include <string>
#include <vector>
#include <array> // C++11 array
#include <iostream>


namespace Belle2 {

  /** The event counter module
   *
   * this module simply counts the number of events (and prints every x-th event onto the screen, where x is user-defined). Useful when executing modules which do not provide this feature themselves
   *
   */
  class EventCounterModule : public Module {

  public:



    /** constants used for this module - TODO find a better way to get this information (global variables?) */
    enum {
      c_nPXDLayers = 2, /**< constant defining the total number of pxdLayers */
      c_nSVDLayers = 4, /**< constant defining the total number of svdLayers */
      c_nInnermostPXDLayer = 1, /**< constant defining layer ID of the innermost PXD layer */
      c_nInnermostSVDLayer = 3, /**< constant defining layer ID of the innermost SVD layer */
    };



    /** a template class to count some event-specific stuff */
    template <class StoreType> class CountStuff {
    public:
      /** standard constructor setting all members to 0 */
      CountStuff()
      {
        m_pxdPixelCounter.fill(0);
        m_pxdClusterCounter.fill(0);
        m_svdStripCounter4U.fill(0);
        m_svdStripCounter4V.fill(0);
        m_svdStripCounterTotal.fill(0);
        m_svdClusterCounter.fill(0);
        m_svdClusterCombinationsCounter.fill(0);
      }



      template<std::size_t SIZE>
      CountStuff(
        std::array<StoreType, SIZE> pxdPixelCounter,
        std::array<StoreType, SIZE> pxdClusterCounter,
        std::array<StoreType, SIZE> svdStripCounter4U,
        std::array<StoreType, SIZE> svdStripCounter4V,
        std::array<StoreType, SIZE> svdStripCounterTotal,
        std::array<StoreType, SIZE> svdClusterCounter,
        std::array<StoreType, SIZE> svdClusterCombinationsCounter) :
        m_pxdPixelCounter(pxdPixelCounter),
        m_pxdClusterCounter(pxdClusterCounter),
        m_svdStripCounter4U(svdStripCounter4U),
        m_svdStripCounter4V(svdStripCounter4V),
        m_svdStripCounterTotal(svdStripCounterTotal),
        m_svdClusterCounter(svdClusterCounter),
        m_svdClusterCombinationsCounter(svdClusterCombinationsCounter) {} /**< constructor setting all members to values given */



      /** assignment operator */
      CountStuff& operator= (const CountStuff& other)
      {
        for (unsigned int i = 0; i < c_nPXDLayers; i++) {
          m_pxdPixelCounter[i] = other.m_pxdPixelCounter[i];
          m_pxdClusterCounter[i] = other.m_pxdClusterCounter[i];
        }
        for (unsigned int i = 0; i < c_nSVDLayers; i++) {
          m_svdStripCounter4U[i] = other.m_svdStripCounter4U[i];
          m_svdStripCounter4V[i] = other.m_svdStripCounter4V[i];
          m_svdStripCounterTotal[i] = other.m_svdStripCounterTotal[i];
          m_svdClusterCounter[i] = other.m_svdClusterCounter[i];
          m_svdClusterCombinationsCounter[i] = other.m_svdClusterCombinationsCounter[i];
        }
        return *this;
      }



      /** '+' operator */
      CountStuff operator+ (const CountStuff& other)
      {
        CountStuff result;
        for (unsigned int i = 0; i < c_nPXDLayers; i++) {
          result.m_pxdPixelCounter[i] = m_pxdPixelCounter[i] + other.m_pxdPixelCounter[i];
          result.m_pxdClusterCounter[i] = m_pxdClusterCounter[i] + other.m_pxdClusterCounter[i];
        }
        for (unsigned int i = 0; i < c_nSVDLayers; i++) {
          result.m_svdStripCounter4U[i] = m_svdStripCounter4U[i] + other.m_svdStripCounter4U[i];
          result.m_svdStripCounter4V[i] = m_svdStripCounter4V[i] + other.m_svdStripCounter4V[i];
          result.m_svdStripCounterTotal[i] = m_svdStripCounterTotal[i] + other.m_svdStripCounterTotal[i];
          result.m_svdClusterCounter[i] = m_svdClusterCounter[i] + other.m_svdClusterCounter[i];
          result.m_svdClusterCombinationsCounter[i] = m_svdClusterCombinationsCounter[i] + other.m_svdClusterCombinationsCounter[i];
        }
        return result;
      }



      /** '*=' operator for a simple type*/
      template<class T>
      CountStuff& operator*= (const T& value)
      {
        for (unsigned int i = 0; i < c_nPXDLayers; i++) {
          m_pxdPixelCounter[i] *= value;
          m_pxdClusterCounter[i] *= value;
        }
        for (unsigned int i = 0; i < c_nSVDLayers; i++) {
          m_svdStripCounter4U[i] *= value;
          m_svdStripCounter4V[i] *= value;
          m_svdStripCounterTotal[i] *= value;
          m_svdClusterCounter[i] *= value;
          m_svdClusterCombinationsCounter[i] *= value;
        }
        return *this;
      }



      /** '+=' operator */
      CountStuff& operator+= (const CountStuff& other)
      {
        for (unsigned int i = 0; i < c_nPXDLayers; i++) {
          m_pxdPixelCounter[i] += other.m_pxdPixelCounter[i];
          m_pxdClusterCounter[i] += other.m_pxdClusterCounter[i];
        }
        for (unsigned int i = 0; i < c_nSVDLayers; i++) {
          m_svdStripCounter4U[i] += other.m_svdStripCounter4U[i];
          m_svdStripCounter4V[i] += other.m_svdStripCounter4V[i];
          m_svdStripCounterTotal[i] += other.m_svdStripCounterTotal[i];
          m_svdClusterCounter[i] += other.m_svdClusterCounter[i];
          m_svdClusterCombinationsCounter[i] += other.m_svdClusterCombinationsCounter[i];
        }
        return *this;
      }



      /** '/' operator */
      CountStuff operator/ (const CountStuff& other)
      {
        CountStuff result;
        for (unsigned int i = 0; i < c_nPXDLayers; i++) {
          result.m_pxdPixelCounter[i] = m_pxdPixelCounter[i] / other.m_pxdPixelCounter[i];
          result.m_pxdClusterCounter[i] = m_pxdClusterCounter[i] / other.m_pxdClusterCounter[i];
        }
        for (unsigned int i = 0; i < c_nSVDLayers; i++) {
          result.m_svdStripCounter4U[i] = m_svdStripCounter4U[i] / other.m_svdStripCounter4U[i];
          result.m_svdStripCounter4V[i] = m_svdStripCounter4V[i] / other.m_svdStripCounter4V[i];
          result.m_svdStripCounterTotal[i] = m_svdStripCounterTotal[i] / other.m_svdStripCounterTotal[i];
          result.m_svdClusterCounter[i] = m_svdClusterCounter[i] / other.m_svdClusterCounter[i];
          result.m_svdClusterCombinationsCounter[i] = m_svdClusterCombinationsCounter[i] / other.m_svdClusterCombinationsCounter[i];
        }
        return result;
      }



      /** overloaded NewType cast */
      template < class NewType >
      operator CountStuff< NewType >()
      {
        CountStuff< NewType > result;
        for (unsigned int i = 0; i < c_nPXDLayers; i++) {
          result.m_pxdPixelCounter[i] = NewType(m_pxdPixelCounter[i]);
          result.m_pxdClusterCounter[i] = NewType(m_pxdClusterCounter[i]);
        }
        for (unsigned int i = 0; i < c_nSVDLayers; i++) {
          result.m_svdStripCounter4U[i] = NewType(m_svdStripCounter4U[i]);
          result.m_svdStripCounter4V[i] = NewType(m_svdStripCounter4V[i]);
          result.m_svdStripCounterTotal[i] = NewType(m_svdStripCounterTotal[i]);
          result.m_svdClusterCounter[i] = NewType(m_svdClusterCounter[i]);
          result.m_svdClusterCombinationsCounter[i] = NewType(m_svdClusterCombinationsCounter[i]);
        }
        return result;
      }



      /** for readable output of the info if printEverything == true, the whole object will be printed. If == false, only Strip/Pixel-Infos are printed, if calcOccupancy == false: nStripsTotal = nUStrips + nVStrips, if == true: nStripsTotal = 0.5*(nUStrips + nVStrips) */
      std::string PrintStuff(bool printEverything = true)
      {
        std::stringstream output;
        CountStuff<double> overallResult; // only innermost entry filled. Used for mean calculation

        output << "PXD:\n";

        if (printEverything == true) {
          for (unsigned short layerMinusOne = 0; layerMinusOne < c_nPXDLayers; layerMinusOne++) {
            output << "L" << layerMinusOne + c_nInnermostPXDLayer
                   << ": nClusters: " << m_pxdClusterCounter.at(layerMinusOne)
                   << ", nPixels: " << m_pxdPixelCounter.at(layerMinusOne)
                   << std::endl;
            overallResult.m_pxdClusterCounter[0] += double(m_pxdClusterCounter.at(layerMinusOne));
            overallResult.m_pxdPixelCounter[0] += double(m_pxdPixelCounter.at(layerMinusOne));
          }
          output << "PXD total" << ": nClusters: " << overallResult.m_pxdClusterCounter[0]
                 << ", nPixels: " << overallResult.m_pxdPixelCounter[0]
                 << std::endl;
          overallResult.m_pxdClusterCounter[0] /= c_nPXDLayers;
          overallResult.m_pxdPixelCounter[0] /= c_nPXDLayers;
          output << "meanPXD per Layer" << ": nClusters: " << overallResult.m_pxdClusterCounter[0]
                 << ", nPixels: " << overallResult.m_pxdPixelCounter[0]
                 << std::endl;
        } else {
          for (unsigned short layerMinusOne = 0; layerMinusOne < c_nPXDLayers; layerMinusOne++) {
            output << "L" << layerMinusOne + c_nInnermostPXDLayer
                   << ", pixels: " << m_pxdPixelCounter.at(layerMinusOne)
                   << std::endl;
            overallResult.m_pxdPixelCounter[0] += double(m_pxdPixelCounter.at(layerMinusOne));
          }
          output << "PXD total: " << "nPixels: " << overallResult.m_pxdPixelCounter[0] << std::endl;
          overallResult.m_pxdPixelCounter[0] /= c_nPXDLayers;
          output << "meanPXD per Layer: " << "nPixels: " << overallResult.m_pxdPixelCounter[0] << std::endl;
        }

        output << "SVD:\n";

        if (printEverything == true) {

          for (unsigned short layerCounter = 0; layerCounter < c_nSVDLayers; layerCounter++) {
            output << "L" << layerCounter + c_nInnermostSVDLayer
                   << ": nClusters: " << m_svdClusterCounter.at(layerCounter)
                   << ", nClusterCombinations: " << m_svdClusterCombinationsCounter.at(layerCounter)
                   << ", nUStrips: " << m_svdStripCounter4U.at(layerCounter)
                   << ", nVStrips: " << m_svdStripCounter4V.at(layerCounter)
                   << ", nStripsTotal: " << m_svdStripCounterTotal.at(layerCounter)
                   << std::endl;
            overallResult.m_svdClusterCounter[0] += double(m_svdClusterCounter.at(layerCounter));
            overallResult.m_svdClusterCombinationsCounter[0] += double(m_svdClusterCombinationsCounter.at(layerCounter));
            overallResult.m_svdStripCounter4U[0] += double(m_svdStripCounter4U.at(layerCounter));
            overallResult.m_svdStripCounter4V[0] += double(m_svdStripCounter4V.at(layerCounter));
            overallResult.m_svdStripCounterTotal[0] += double(m_svdStripCounterTotal.at(layerCounter));
          }
          output << "SVD total" << ": nClusters: " << overallResult.m_svdClusterCounter[0]
                 << ", nClusterCombinations: " << overallResult.m_svdClusterCombinationsCounter[0]
                 << ", nUStrips: " << overallResult.m_svdStripCounter4U[0]
                 << ", nVStrips: " << overallResult.m_svdStripCounter4V[0]
                 << ", nStripsTotal: " << overallResult.m_svdStripCounterTotal[0]
                 << std::endl;
          overallResult.m_svdClusterCounter[0] /= c_nSVDLayers;
          overallResult.m_svdClusterCombinationsCounter[0] /= c_nSVDLayers;
          overallResult.m_svdStripCounter4U[0] /= c_nSVDLayers;
          overallResult.m_svdStripCounter4V[0] /= c_nSVDLayers;
          overallResult.m_svdStripCounterTotal[0] /= c_nSVDLayers;
          output << "meanSVD per Layer" << ": nClusters: " << overallResult.m_svdClusterCounter[0]
                 << ", nClusterCombinations: " << overallResult.m_svdClusterCombinationsCounter[0]
                 << ", nUStrips: " << overallResult.m_svdStripCounter4U[0]
                 << ", nVStrips: " << overallResult.m_svdStripCounter4V[0]
                 << ", nStripsTotal: " << overallResult.m_svdStripCounterTotal[0]
                 << std::endl;

        } else {

          for (unsigned short layerCounter = 0; layerCounter < c_nSVDLayers; layerCounter++) {
            output << "L" << layerCounter + c_nInnermostSVDLayer
                   << ", nUStrips: " << m_svdStripCounter4U.at(layerCounter)
                   << ", nVStrips: " << m_svdStripCounter4V.at(layerCounter)
                   << ", nStripsTotal: " << m_svdStripCounterTotal.at(layerCounter)
                   << std::endl;
            overallResult.m_svdStripCounter4U[0] += double(m_svdStripCounter4U.at(layerCounter));
            overallResult.m_svdStripCounter4V[0] += double(m_svdStripCounter4V.at(layerCounter));
            overallResult.m_svdStripCounterTotal[0] += double(m_svdStripCounterTotal.at(layerCounter));
          }
          output << "SVD total" << ": nUStrips: " << overallResult.m_svdStripCounter4U[0]
                 << ", nVStrips: " << overallResult.m_svdStripCounter4V[0]
                 << ", nStripsTotal: " << overallResult.m_svdStripCounterTotal[0]
                 << std::endl;
          overallResult.m_svdStripCounter4U[0] /= c_nSVDLayers;
          overallResult.m_svdStripCounter4V[0] /= c_nSVDLayers;
          overallResult.m_svdStripCounterTotal[0] /= c_nSVDLayers;
          output << "meanSVD per Layer" << ": nUStrips: " << overallResult.m_svdStripCounter4U[0]
                 << ", nVStrips: " << overallResult.m_svdStripCounter4V[0]
                 << ", nStripsTotal: " << overallResult.m_svdStripCounterTotal[0]
                 << std::endl;

        }

        return output.str();
      }



      template<std::size_t SIZE>
      static StoreType sumUpArray(const std::array<StoreType, SIZE>& aContainer)
      {
        StoreType sumTotal = 0;
        for (const StoreType entry : aContainer) { sumTotal += entry; }
        return sumTotal;
      }/**< summing up all entries of given container */


/// WARNING compiles but can't be used:
      template < typename EntryType, template <typename...> class ContainerType >
      EntryType sumUpVT(ContainerType<EntryType>& aContainer)
      {
        EntryType sumTotal = 0;
        for (EntryType entry : aContainer) { sumTotal += entry; }
        return sumTotal;
      } /**< summing up all entries of given container */


      /// WARNING compiles but can't be used:
      template < typename EntryType, typename...OtherArguments, template <typename, typename...> class ContainerType >
      EntryType sumUpVT2(ContainerType<EntryType, OtherArguments...>& aContainer)
      {
        EntryType sumTotal = 0;
        for (EntryType entry : aContainer) { sumTotal += entry; }
        return sumTotal;
      } /**< summing up all entries of given container */


      // member variables
      std::array<StoreType, c_nPXDLayers> m_pxdPixelCounter; /**< counts total number of illuminated pixels in the pxd */
      std::array<StoreType, c_nPXDLayers> m_pxdClusterCounter; /**< counts total number of pxd clusters occured */
      std::array<StoreType, c_nSVDLayers> m_svdStripCounter4U; /**< counts total number of illuminated u strips in the svd */
      std::array<StoreType, c_nSVDLayers> m_svdStripCounter4V; /**< counts total number of illuminated v strips in the svd */
      std::array<StoreType, c_nSVDLayers> m_svdStripCounterTotal; /**< counts total number of illuminated strips in the svd */
      std::array<StoreType, c_nSVDLayers> m_svdClusterCounter; /**< counts total number of svd clusters occured */
      std::array<StoreType, c_nSVDLayers>
      m_svdClusterCombinationsCounter; /**< counts total number of svd clusters combinations occured */
    };



    /**
     * Constructor of the module.
     */
    EventCounterModule();

    /** Initializes the Module.
     */
    void initialize() override;

    /**
     * Prints a header for each new run.
     */
    void beginRun() override;

    /** Prints the full information about the event, run and experiment number. */
    void event() override;

    /**
     * Prints a footer for each run which ended.
     */
    void endRun() override;


    /** initialize variables to avoid nondeterministic behavior */
    void InitializeCounters();

  protected:

    // parameters for the module
    int m_PARAMstepSize; /**< Informes the user that event: (eventCounter-modulo(stepSize)) is currently executed */
    bool m_PARAMshowClusterStatistics; /**< if activated, some statistics for SVD and PXD Clusters are measured */
    bool m_PARAMallowMultiThreaded; /**< if false, the module suppresses multithreaded execution of this and its following modules in the module-path */

    // member variables
    int m_eventCounter; /**< knows current event number */
    int m_pxdClusterCounter; /**< counts total number of pxd clusters occured */
    int m_svdStripCounter4U; /**< counts total number of illuminated u strips in the svd */
    int m_svdStripCounter4V; /**< counts total number of illuminated v strips in the svd */
    int m_svdClusterCounter; /**< counts total number of svd clusters occured */
    int m_svdClusterCombinationsCounter; /**< counts total number of svd clusters combinations occured */

    std::vector<CountStuff<unsigned int> > m_allEventsCounted; /**< collects detailed info of each event */
    CountStuff<double>
    m_countedPixelsAndStrips; /**< store total number of pixels, uStrips and vStrips in a CountStuff-container, unrecognized detector types are ignored (like Tel) */
  private:

  };
}
