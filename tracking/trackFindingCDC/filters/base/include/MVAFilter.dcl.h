/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.dcl.h>
#include <tracking/trackFindingCDC/filters/base/Filter.fwd.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <framework/database/DBObjPtr.h>
#include <tracking/dbobjects/TrackingMVAFilterParameters.h>

#include <memory>
#include <string>
#include <cmath>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class MVAExpert;

    /// Filter based on a mva method.
    template<class AFilter>
    class MVA : public OnVarSet<AFilter> {

    private:
      /// Type of the super class
      using Super = OnVarSet<AFilter>;

    public:
      /// Type of the object to be analysed.
      using Object = typename AFilter::Object;

    private:
      /// Type of the variable set
      using AVarSet = BaseVarSet<Object>;

    public:
      /// Constructor of the filter.
      explicit MVA(std::unique_ptr<AVarSet> varSet,
                   const std::string& identifier = "",
                   double defaultCut = NAN,
                   const std::string& dbObjectName = "");

      /// Default destructor
      virtual ~MVA();

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Initialize the expert before event processing.
      void initialize() override;

      /// Signal to load new run parameters
      void beginRun() override;

    public:
      /// Function to object for its signalness
      Weight operator()(const Object& obj) override;

      /// Evaluate the mva method
      virtual double predict(const Object& obj);

      /// Evaluate the MVA method over several inputs simultaneously
      std::vector<float> predict(const std::vector<Object*>& objs);

      /// Evaluate the MVA method over a vector of objects
      virtual std::vector<float> operator()(const std::vector <Object*>& objs) override;
    private:
      /// Database identifier of the expert or weight file name
      std::string m_identifier = "";

      /// The cut on the MVA output.
      double m_cutValue;

      /// Name of the DB payload
      std::string m_DBPayloadName = "";

      /// MVA Expert to examine the object
      std::unique_ptr<MVAExpert> m_mvaExpert;

      /// named variables, ordered as in the weightFile:
      std::vector<Named<Float_t*>> m_namedVariables;
    };

    /// Convience template to create a mva filter for a set of variables.
    template<class AVarSet>
    class MVAFilter: public MVA<Filter<typename AVarSet::Object> > {

    private:
      /// Type of the super class
      using Super = MVA<Filter<typename AVarSet::Object> >;

    public:
      /// Type of the object to be analysed.
      using Object = typename AVarSet::Object;

      /// Constructor of the filter.
      explicit MVAFilter(const std::string& defaultTrainingName = "",
                         double defaultCut = NAN,
                         const std::string& defaultDBObjectName = "");

      /// Default destructor
      ~MVAFilter();
    };
  }
}
