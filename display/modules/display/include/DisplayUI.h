#ifndef DISPLAYUI_H
#define DISPLAYUI_H

#include <TQObject.h>

#include <string>
#include <utility>
#include <vector>

class TEveBox;
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

    /** Generate UI elements so the given module parameter can be changed at run time. */
    void addParameter(const std::string& label, ModuleParam<bool> &param);

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

    /** return viewer class. */
    SplitGLView* getViewer() { return m_viewer; }

    /** Check if new events are available, and go to next event.
     *
     * Only useful for AsyncDisplay.
     */
    void pollNewEvents();

    /** Close window and continue execution. */
    void closeAndContinue();

    /** Close window and exit immediately. */
    void exit();


  private:
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

    /** List of run time configurable module parameters. */
    std::vector< std::pair<std::string, ModuleParam<bool>* > > m_paramList;

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

    /** pointer to viewer class. */
    SplitGLView* m_viewer;

    /** Polling/auto-advance timer. */
    TTimer* m_timer;


    /** Dictionary needed for signal/slot mechanism (0 to disable I/O). */
    ClassDef(DisplayUI, 0)
  };
}
#endif
