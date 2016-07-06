/// WARNING JKL: temporarily commented out until I have copied the relevant stuff into VXDTFModule again.
// // // // // // // // // /**************************************************************************
// // // // // // // // //  * BASF2 (Belle Analysis Framework 2)                                     *
// // // // // // // // //  * Copyright(C) 2015 - Belle II Collaboration                             *
// // // // // // // // //  *                                                                        *
// // // // // // // // //  * Author: The Belle II Collaboration                                     *
// // // // // // // // //  * Contributors: Eugenio Paoloni                                          *
// // // // // // // // //  *                                                                        *
// // // // // // // // //  * This software is provided "as is" without any warranty.                *
// // // // // // // // //  **************************************************************************/
// // // // // // // // //
// // // // // // // // // #pragma once
// // // // // // // // //
// // // // // // // // // //framework:
// // // // // // // // // #include <framework/core/Module.h>
// // // // // // // // //
// // // // // // // // // #include <vector>
// // // // // // // // // #include <string>
// // // // // // // // // #include <tuple>
// // // // // // // // //
// // // // // // // // // namespace Belle2 {
// // // // // // // // //
// // // // // // // // //   class VXDTFSecMap;
// // // // // // // // //
// // // // // // // // //   /** The SegmentFilterConverterModule is a module able to read the
// // // // // // // // //    *  cutoff values for filtering the Segments written in the
// // // // // // // // //    *  xml gear box and write it in the new redesigned format
// // // // // // // // //    */
// // // // // // // // //   class SegmentFilterConverterModule : public Module {
// // // // // // // // //
// // // // // // // // //   public:
// // // // // // // // //
// // // // // // // // //     //! Constructor
// // // // // // // // //     SegmentFilterConverterModule();
// // // // // // // // //
// // // // // // // // //     //! Destructor
// // // // // // // // //     virtual ~SegmentFilterConverterModule() { };
// // // // // // // // //
// // // // // // // // //     virtual void initialize()   ;
// // // // // // // // //     virtual void beginRun()     ;
// // // // // // // // //     virtual void event()        ;
// // // // // // // // //     virtual void endRun()       ;
// // // // // // // // //     virtual void terminate() { };
// // // // // // // // //
// // // // // // // // //   private:
// // // // // // // // //     void retrieveFromXML(void);
// // // // // // // // //     void initSectorMap(void);
// // // // // // // // //     void initSectorMapFilter(int i);
// // // // // // // // //
// // // // // // // // //     std::vector< std::string > m_PARAMsectorSetup;
// // // // // // // // //     double m_PARAMtuneCutoffs;
// // // // // // // // //
// // // // // // // // //     typedef std::tuple <
// // // // // // // // //     std::string,          // Name of the variable
// // // // // // // // //         std::vector<double>,  // special tune parameter
// // // // // // // // //         std::vector< std::tuple<bool> > // enables the filter
// // // // // // // // //         // Why putting a bool inside a std:tuple?
// // // // // // // // //         // Because the vector< bool > is a space-efficient specialization
// // // // // // // // //         // and we cannot expose to python as a parameter an element of
// // // // // // // // //         // of a vector< bool > since it cannot provide reference acces to
// // // // // // // // //         // its content.
// // // // // // // // //         > JakobVariable_t;
// // // // // // // // //
// // // // // // // // //     std::vector< JakobVariable_t > m_variables;
// // // // // // // // //
// // // // // // // // //     JakobVariable_t&      Variable(unsigned int);
// // // // // // // // //     std::string           VariableName(unsigned int);
// // // // // // // // //     std::vector<double>& VariableTunings(unsigned int);
// // // // // // // // //     double&   VariableTuning(unsigned int variableIndex,
// // // // // // // // //                              unsigned int setupIndex);
// // // // // // // // //
// // // // // // // // //     std::vector< std::tuple< bool > >&    VariableEnables(unsigned int);
// // // // // // // // //     bool& VariableEnable(unsigned int variableIndex,
// // // // // // // // //                          unsigned int setupIndex);
// // // // // // // // //
// // // // // // // // //
// // // // // // // // //     std::vector< const VXDTFSecMap* > m_SectorMaps;
// // // // // // // // //   };
// // // // // // // // // } // Belle2 namespace
