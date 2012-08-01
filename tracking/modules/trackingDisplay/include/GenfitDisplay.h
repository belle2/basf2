/* Copyright 2011, Technische Universitaet Muenchen,
   Author: Karl Bicker

   This file is part of GENFIT.

   GENFIT is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GENFIT is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GENFIT.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef GenfitDisplay_H
#define GenfitDisplay_H

#include <GFTrack.h>

#include <TQObject.h>
#include <TGButton.h>
#include <TVector3.h>

#include <string>
#include <vector>

class TEveBox;

namespace Belle2 {
  /** @brief Event display designed to run with Genfit.
   *
   *  @author Karl Bicker (Technische Universit&auml;t M&uuml;nchen, original author)
   *  @author Christian Pulvermacher (adapted for Belle II)
   *
   *  Hands over TGeo objects, GFTracks and raw hits to TEve for display.
   *
   *  @sa TrackingDisplayModule
   */
  class GenfitDisplay : public TQObject {
  public:
    /** Constructor. */
    GenfitDisplay();
    /** Destructor. */
    ~GenfitDisplay();

    /** @brief Drop all events.*/
    void reset();

    /** @brief Add new event
     *
     * Add a new event. An event is a collection of GFTracks which are displayed at the
     * the same time.
     */
    void addEvent(const std::vector<GFTrack*>& tr);

    /** @brief Add a vector of space point hits.
     *
     * The format is (x, y, z)
     * Use setHits at the same time like addEvent, because Hits and Events
     * must have the same size.
     *
     *  */
    void setHits(const std::vector< std::vector<double> >& hits);

    /** @brief Go to the next event or step a certain number of events ahead.*/
    void next(unsigned int stp = 1);

    /** @brief Go to the previous event or step a certain number of events back.*/
    void prev(unsigned int stp = 1);

    /** @brief Go to event with index id.*/
    void goToEvent(unsigned int id);

    /** @brief Set the display options.
     *
     * The option string lets you steer the way the events are displayed. The following
     * options are available:\n
     * \n
     * 'A': Autoscale errors. The representation of hits are scaled with the error found in
     *      their covariance matrix. This can lead to hits not being displayed beause the
     *      errors are too small. Autoscaling ensures that the errors are scaled up
     *      sufficiently to ensure all hits are displayed. However, this can lead to unwanted
     *      results if there are only a few hits with very small errors, as all hits are scaled
     *      by the same factor to ensure consistency.\n\n
     * 'D': Draw detectors. This causes a simple representation for all detectors to be drawn. For
     *      planar detectors, this is a plane with the same position and orientation of the real
     *      detector plane, but with different size. For wires, this is a tube whose diameter
     *      is equal to the value measured by the wire. Spacepoint hits are not affected by this
     *      option.\n\n
     * 'H': Draw hits. This causes the hits to be visualized. Normally, the size of the hit
     *      representation is connected to the covariance matrix of the hit, scaled by the value
     *      set in setErrScale which is normally 1. See also option 'A' and 'S'. Normally used in
     *      connection with 'D'.\n\n
     * 'R': Draw Hits added via the function setHits. Then option 'H' shouldn't be used, otherwise
     *      the hits belonging to a track will be plotted twice. This feature is in a beta stage.\n\n
     * 'G': Draw geometry. Also draw the geometry in the gGeoManager. This feature is experimental
     *      and may lead to strange things being drawn.\n\n
     * 'M': Draw track markers. Draw the intersection points between the track and the virtual
     *      (and/or real) detector planes. Can only be used in connection with 'T'.\n\n
     * 'P': Draw detector planes. Draws the virtual (and/or real) detector planes.\n\n
     * 'S': Scale manually. This leads to the spacepoint hits (and only them up to now!) being drawn
     *      as spheres with radius 0.5 scaled with the error scale factor. Can be used if the scaling
     *      with errors leads to problems.\n\n
     * 'T': Draw Track. Draw the track as straight lines between the virtual (and/or real) detector
     *      planes.\n\n
     * 'X': Draw silent. Does not run the TApplication.
     *
     */
    void setOptions(const std::string& opts = "ADHT");

    /** @brief Set the scaling factor for the visualization of the errors.*/
    void setErrScale(double errScale = 1.);

    /** @brief Get the error scaling factor.*/
    double getErrScale() const;

    /** @brief Open the event display.*/
    void open();

    /** Change projection to r-phi. */
    void setRPhiProjection();

    /** Change projection to 3D (default). */
    void setDefaultProjection();

    /** Save the current view to a user-defined filename
     *
     * @param highres save picture with 4000px width instead of screen size
     **/
    void savePicture(bool highres = false);

    void saveHiResPicture() { savePicture(true); }


  private:
    /** @brief Build the buttons for event navigation.*/
    void makeGui();

    /** @brief Draw current event.*/
    void drawEvent();

    /** @brief Create a box around o, orientet along u and v with widths ud, vd and depth and
     *  return a pointer to the box object.
     */
    TEveBox* boxCreator(const TVector3& o, TVector3 u, TVector3 v, float ud, float vd, float depth);


    /** Current event id */
    long fEventId;

    /** Rescale errors with this factor to ensure visibility. */
    double fErrorScale;

    /** Option string.
     *
     * @sa setOptions
     */
    std::string fOption;

    /** Was GUI already built? */
    bool m_guiInitialized;

    /** List of GFTracks for current event. */
    std::vector<GFTrack*> m_tracks;

    /** Hits that are not part of GFTracks, stored as x/y/z tuple of doubles. */
    std::vector< std::vector<double> > fHits;

    /** Button to switch to previous event. */
    TGButton* m_prevButton;

    /** Button to switch to next event. */
    TGButton* m_nextButton;

    /** Dictionary needed for signal/slot mechanism. */
    ClassDef(GenfitDisplay, 1)
  };
}
#endif
