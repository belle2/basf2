#ifndef DISPLAYUI_H
#define DISPLAYUI_H

#include <TQObject.h>

#include <string>
#include <utility>
#include <vector>

class TEveBox;
class TEveElement;
class TEveElementList;
class TGButton;
class TGPictureButton;
class TGLabel;
class TGNumberEntry;
class TGTextEntry;
class TTimer;

namespace Belle2 {
  //forward declaration needed because CINT balks at inclusion of python headers in ModuleParam...
  template <class T> class ModuleParam;
  class SplitGLView;
  class DisplayData;
  class VisualRepMap;

  /** Control TEve browser user interface.
   *
   *  Mostly responsible for interactive elements like buttons etc.
   *
   *  @sa DisplayModule
   */
  class DisplayUI : public TQObject {
  public:
    /** Constructor.
     *
     * @param automatic if true, hide window and save events using automaticEvent()
     */
    DisplayUI(bool automatic = false);

    /** Destructor. */
    ~DisplayUI();

    /** Generate UI elements so the given module parameter can be changed at run time.
     *
     * Will result in a checkbox with given label indented by the amount in level
     * (0 being leftmost). Clicking the checkbox will toggle the parameter and
     * reload the event.
     */
    void addParameter(const std::string& label, ModuleParam<bool>& param, int level);

    /** Go to next event. */
    void next();

    /** Go to previous event. */
    void prev();

    /** Go to event with index id.*/
    void goToEvent(Long_t id);

    /** Go to the event specified, using the input file's index. */
    void goToEvent(Long_t event, Long_t run, Long_t experiment);

    /** go to the event given by m_eventNumberWidget. */
    void goToEventWidget();

    /** m_autoAdvanceDelay was changed, update m_timer if enabled. */
    void autoAdvanceDelayChanged();

    /** Handle Play/Pause button clicks. */
    void togglePlayPause();

    /** Show a dialog to to enter exp, run, event numbers. */
    void showJumpToEventDialog();

    /** remove all event data in current event. */
    void clearEvent();

    /** switch to automatic mode, where visualisations are saved for each event, with no interactive control. */
    void startAutomaticRun();

    /** The actual per-event functionality for automatic saving. */
    void automaticEvent();

    /** Start interactive display for current event.
     *
     *  Returns only after user presses prev/next, or closes the window.
     *
     *  @returns wether to reprocess the current event, e.g. when visualisation options changed
     */
    bool startDisplay();

    /** Set title of Eve window. Add fileName, if given. */
    void setTitle(const std::string& fileName = "");

    /** Toggle between light and dark color scheme for viewers. */
    void toggleColorScheme();

    /** Called when one of the module parameters is changed via UI. */
    void handleParameterChange(int id);

    /** Save the current view to a user-defined filename
     *
     * @param highres save picture with 4000px width instead of screen size
     **/
    void savePicture(bool highres = false);

    /** alias for savePicture(true). */
    void saveHiResPicture() { savePicture(true); }

    /** return right-side pane with viewers. */
    SplitGLView* getViewPane() { return m_viewPane; }

    /** Check if new events are available, and go to next event.
     *
     * Only useful for AsyncDisplay.
     */
    void pollNewEvents();

    /** Close window and continue execution. */
    void closeAndContinue();

    /** Close window and exit immediately. */
    void exit();

    /** Add user-defined data (histograms, etc.). */
    void showUserData(const DisplayData& displayData);

    /** If true, DisplayModule shouldn't clear previous data (i.e. we want to show multiple events) */
    bool cumulativeIsOn() const { return m_cumulative; }

    /** toggle cumulative mode. */
    void toggleCumulative() { m_cumulative = !m_cumulative; }

    /** Set the TObject <-> TEveElement map (to be filled by EVEVisualization). */
    void setVisualRepMap(const VisualRepMap* visualRepMap) { m_visualRepMap = visualRepMap; }

    /** Handle special actions when objects are selected. */
    void selectionHandler(TEveElement* eveObj);


  private:
    /** Wraps a module parameter that can be toggled from the UI. */
    struct Parameter {
      std::string m_label; /**< Label shown in UI. */
      ModuleParam<bool>* m_param; /**< wrapped parameter. */
      int m_level; /**< Level this parameter is shown at (0 is highest). */
    };

    /** Build the buttons for event navigation.*/
    void makeGui();

    /** Update UI after a new event was loaded, as well as m_currentEntry. */
    void updateUI();

    /** Current entry id */
    long m_currentEntry;

    /** Rescale errors with this factor to ensure visibility. */
    double fErrorScale;

    /** Was GUI already built? */
    bool m_guiInitialized;

    /** Show current event again after startDisplay() returns? */
    bool m_reshowCurrentEvent;

    /** If true, disable interactive control and call automaticEvent() instead. */
    bool m_automatic;

    /** If true, DisplayModule shouldn't clear previous data (i.e. we want to show multiple events) */
    bool m_cumulative;

    /** List of run time configurable module parameters. */
    std::vector<Parameter> m_paramList;

    /** Button to switch to previous event. */
    TGButton* m_prevButton;

    /** Button to switch to next event. */
    TGButton* m_nextButton;

    /** Event switcher with numeric entry. */
    TGNumberEntry* m_eventNumberWidget;

    /** Delay for automatic advance, in seconds. */
    TGNumberEntry* m_autoAdvanceDelay;

    /** Play / Pause button. */
    TGPictureButton* m_playPauseButton;

    /** show event/run/exp number for current event. */
    TGLabel* m_eventLabel;

    /** File name prefix (prefix + #event + "_" + projection + ".png"). */
    TGTextEntry* m_autoFileNamePrefix;

    /** width of saved PNGs. */
    TGNumberEntry* m_autoPictureWidth;

    /** List of event data, including projections. */
    TEveElementList* m_eventData;

    /** pointer to right-side pane with viewers. */
    SplitGLView* m_viewPane;

    /** Polling/auto-advance timer. */
    TTimer* m_timer;

    /** Map TObject <-> TEveElement */
    const VisualRepMap* m_visualRepMap;


    /** Dictionary needed for signal/slot mechanism (0 to disable I/O). */
    ClassDef(DisplayUI, 0)
  };
}
#endif
