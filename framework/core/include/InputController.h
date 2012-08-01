#ifndef INPUTCONTROLLER_H
#define INPUTCONTROLLER_H

namespace Belle2 {
  /** A static class to control supported input modules.
   *
   *  You can use setNextEntry() to request loading of any event in 0..numEntries()-1,
   *  which will be done the next time the input module's event() function is called.
   *
   *  Use canControlInput() to check wether control is actually possible.
   */
  class InputController {
  public:
    /** Is there an input module to be controlled. */
    static bool canControlInput() { return s_canControlInput; }

    /** Call this function from supported input modules. */
    static void setCanControlInput(bool on) { s_canControlInput = on; }

    /** Set the file entry to be loaded the next time event() is called.
     *
     * This is mainly useful for interactive applications (e.g. event display).
     *
     * The input module should reset this to -1 once the entry was loaded.
     */
    static void setNextEntry(long entry) { s_nextEntry = entry; }

    //TODO:
    /** Set the file entry to be loaded the next time event() is called. */
    //static void setNextEntry(int exp, int run, int event);

    /** Return entry number set via setNextEntry(). */
    static long getNextEntry() { return s_nextEntry; }

    /** Returns total number of entries in the event tree.
     *
     * If no file is opened, zero is returned.
     */
    static long numEntries() { return s_numEntries; }

    /** set total number of entries in the opened file. */
    static void setNumEntries(long n) { s_numEntries = n; }


  private:
    InputController() { }
    ~InputController() { }

    /** Is there an input module to be controlled? */
    static bool s_canControlInput;

    /** entry to be loaded the next time event() is called in an input module.
     *
     *  -1 indicates that execution should continue normally.
     */
    static long s_nextEntry;

    /** total number of entries in event tree, or zero if none exists. */
    static long s_numEntries;
  };
}
#endif
