/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

      //add an arrow to point to (0, 0, -2m)
      displayData.addArrow("z=-2m", TVector3(100, 100, -200),
              TVector3(0, 0, -200), kGray);
      \endcode
   *
   * You can then save the output of your module to a file and view it with 'b2display', or look at it directly
   * after execution by adding the Display module in your steering file.
   *
   * \sa Check display/examples/displaydata.py to see how to use most features from a python steering file.
   * \sa See display/examples/histogram_monitor.py for an example that generates and adds histograms in Python.
   */
  class DisplayData : public TObject {
  public:

    /** Constructor. */
    DisplayData() {}

    /** Destructor. */
    ~DisplayData();

    /** Add an arrow in the display (for axes, to point out some specific location).
     *
     * @param name  Text to be shown beside arrow.
     * @param start The arrow will start here.
     * @param end   The pointy end ends up here.
     * @param color Color_t to use for this object, default: random color.
     */
    void addArrow(const std::string& name, const TVector3& start, const TVector3& end, int color = -1);

    /** Add histogram with the given name.
     *
     * The histogram will show up in the 'Histograms' tab and can be drawn on the active canvas by double-clicking it.
     */
    void addHistogram(const std::string& name, const TH1* hist);

    /** Add a text label at the given position. */
    void addLabel(const std::string& text, const TVector3& pos);


    /** Add a point at the given position, as part of a collection specified by name.
     *
     * All points with the same name will be hilighted when clicked etc.
     */
    void addPoint(const std::string& name, const TVector3& pos);


    /** Select the given object in the display.
     *
     * Only has an effect if the object actually has a visualisation.
     * Can be called multiple times to select more than one object.
     *
     * @param object object to select, must be inside a StoreArray
     */
    void select(const TObject* object);

    /** Stores data associated with an arrow. */
    struct Arrow {
      std::string name; /**< label. */
      TVector3 start; /**< arrow starts here. */
      TVector3 end; /**< and ends here. */
      int color; /**< Color_t, e.g. kGreen. -1 for default */
    };
  private:

    std::map<std::string, std::vector<TVector3> > m_pointSets; /**< name -> points map */
    std::vector<std::pair<std::string, TVector3> > m_labels; /**< text labels (to be shown at a given position). */
    /** Histograms to be shown in Eve. */
    std::vector<TH1*> m_histograms; //->

    std::vector<std::pair<std::string, unsigned int> > m_selectedObjects; /**< List of selected objects (array name, index). */
    std::vector<Arrow> m_arrows; /**< List of arrows. */

    ClassDef(DisplayData, 4); /**< Add custom information to the display. */

    friend class DisplayUI;
    friend class EVEVisualization;

  }; //class
} // namespace Belle2
