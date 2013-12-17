/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>

//for dictionaries
#include <TH1.h>
#include <TVector3.h>

#include <vector>
#include <map>
#include <string>
#include <utility>


namespace Belle2 {
  /** Add custom information to the display.
   *
   * To use it, simply create a DisplayData object using StoreObjPtr and call
   * the functions to add custom data. For example:

      \code
      StoreObjPtr<DisplayData> displayData;
      displayData.create();

      //let's label the interaction point
      displayData.addLabel("IP", TVector3(0, 0, 0));

      //add some dots along the x axis
      for (int i = 0; i < 20; i++) {
        displayData.addPoint("pointset", TVector3(i*10.0, 0, 0));
      }
      \endcode
   *
   * You can then save the output of your module to a file and view it with 'b2display', or look at it directly
   * after execution by adding the Display module in your steering file.
   *
   * See display/examples/histogram_monitor.py for an example that generates and adds histograms in Python.
   */
  class DisplayData : public TObject {
  public:

    /** Constructor. */
    DisplayData() {}

    /** Destructor. */
    ~DisplayData();

    /** Add a point at the given position, as part of a collection specified by name.
     *
     * All points with the same name will be hilighted when clicked etc.
     */
    void addPoint(const std::string& name, const TVector3& pos);

    /** Add a text label at the given position. */
    void addLabel(const std::string& text, const TVector3& pos);

    /** Add histogram with the given name.
     *
     * The histogram will show up in the 'Histograms' tab and can be drawn on the active canvas by double-clicking it.
     */
    void addHistogram(const std::string& name, const TH1* hist);

  private:

    std::map<std::string, std::vector<TVector3> > m_pointSets; /**< name -> points map */
    std::vector<std::pair<std::string, TVector3> > m_labels; /**< text labels (to be shown at a given position). */
    /** Histograms to be shown in Eve. */
    std::vector<TH1*> m_histograms; //->

    ClassDef(DisplayData, 1); /**< Add custom information to the display. */

    friend class DisplayUI;
    friend class EVEVisualization;

  }; //class
} // namespace Belle2
