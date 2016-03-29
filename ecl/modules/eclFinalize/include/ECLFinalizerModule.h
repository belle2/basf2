/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module converts the ecl dataobject(s) in the mdst dataobect(s)    *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLFINALIZERMODULE_H_
#define ECLFINALIZERMODULE_H_

#include <framework/core/Module.h>

namespace Belle2 {
  namespace ECL {

    /** Class to perform the shower correction */
    class ECLFinalizerModule : public Module {

    public:
      /** Constructor. */
      ECLFinalizerModule();

      /** Destructor. */
      ~ECLFinalizerModule();

      /** Initialize. */
      virtual void initialize();

      /** Begin run. */
      virtual void beginRun();

      /** Event. */
      virtual void event();

      /** End run. */
      virtual void endRun();

      /** Terminate. */
      virtual void terminate();

      template <typename T> std::vector<T> make_vector(T const& t1, T const& t2);  /**< make_vector. */
      template <typename T> std::pair<T, T> vectorToPair(std::vector<T>& vec, const std::string& name = "");   /**< vectorToPair. */

    private:

      std::vector<double> m_clusterEnergyCuts; /**< Min [0] and Max [1] value for the cluster energy cut. */
      std::vector<double> m_highestEnergyCuts; /**< Min [0] and Max [1] value for the highest energy cut. */
      std::vector<double> m_clusterTimeCuts; /**< Min [0] and Max [1] value for the cluster time cut. */

    public:
      /** We need names for the data objects to differentiate between PureCsI and default*/

      /** Default name ECLShower */
      virtual const char* eclShowerArrayName() const
      { return "ECLShowers" ; }

      /** Default name ECLCluster */
      virtual const char* eclClusterArrayName() const
      { return "ECLClusters"; }

    }; // end of ECLFinalizerModule


    /** The very same module but for PureCsI */
    class ECLFinalizerPureCsIModule : public ECLFinalizerModule {
    public:

      /** PureCsI name ECLShower */
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }

      /** PureCsI name ECLCluster */
      virtual const char* eclClusterArrayName() const override
      { return "ECLClustersPureCsI"; }

    }; // end of ECLFinalizerPureCsIModule

    template <typename T>
    inline std::vector<T> ECLFinalizerModule::make_vector(T const& t1, T const& t2)
    {
      std::vector<T> v;
      v.push_back(t1);
      v.push_back(t2);
      return v; /**< make_vector */
    }

    template <typename T>
    inline std::pair<T, T> ECLFinalizerModule::vectorToPair(std::vector<T>& vec, const std::string& name)
    {
      std::pair<T, T> p;
      if (vec.size() != 2) {
        B2ERROR("The parameter " + name + " has to have exactly 2 elements!")
      } else {
        p.first  = vec[0];
        p.second = vec[1];
      }
      return p;  /**< vectorToPair. */
    }


  } // end of ECL namespace
} // end of Belle2 namespace

#endif
