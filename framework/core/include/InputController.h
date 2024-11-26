/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>
#include <utility>

class TChain;

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

    /** Get if we are merging events from two paths. */
    static bool getEventMerging() { return s_doEventMerging; }

    /** Set that we are merging events from two paths. */
    static void enableEventMerging(Module* steerRootInputModule);

    /** Set the file entry to be loaded the next time event() is called.
     *
     * This is mainly useful for interactive applications (e.g. event display).
     *
     * The input module should call eventLoaded() after the entry was loaded.
     */
    static void setNextEntry(long entry, bool independentPath = false) { (!independentPath) ? s_nextEntry.first = entry : s_nextEntry.second = entry; }

    /** Return entry number set via setNextEntry(). */
    static long getNextEntry(bool independentPath = false) { return (!independentPath) ? s_nextEntry.first : s_nextEntry.second;};

    /** Set the file entry to be loaded the next time event() is called, by evt/run/exp number.
     *
     * The input module should call eventLoaded() after the entry was loaded.
     */
    static void setNextEntry(long exp, long run, long event) { s_nextExperiment = exp; s_nextRun = run; s_nextEvent = event; }

    /** set the number of entries skipped by the RootInputModule. */
    static void setSkippedEntries(long entries, bool independentPath = false) { (!independentPath) ? s_skippedEntries.first = entries : s_skippedEntries.second = entries; }

    /** Return experiment number set via setNextEntry(). */
    static long getNextExperiment() { return s_nextExperiment; }

    /** Return run number set via setNextEntry(). */
    static long getNextRun() { return s_nextRun; }

    /** Return event number set via setNextEntry(). */
    static long getNextEvent() { return s_nextEvent; }

    /** returns the entry number currently loaded. */
    static long getCurrentEntry(bool independentPath = false) { return (!independentPath) ? s_currentEntry.first : s_currentEntry.second; }

    /** returns the number of entries skipped by the RootInputModule. */
    static long getSkippedEntries(bool independentPath = false) { return (!independentPath) ? s_skippedEntries.first : s_skippedEntries.second; }

    /** Returns total number of entries in the event tree.
     *
     * If no file is opened, zero is returned.
     */
    static long numEntries(bool independentPath = false);

    /** Return name of current file in loaded chain (or empty string if none loaded). */
    static std::string getCurrentFileName(bool independentPath = false);

    /** Indicate that an event (in the given entry) was loaded and reset all members related to the next entry. */
    static void eventLoaded(long entry, bool independentPath = false);

    /** Set the loaded TChain (event durability). */
    static void setChain(const TChain* chain, bool independentPath = false);

    /** Reset InputController (e.g. after forking a thread) */
    static void resetForChildProcess();

    /** Necessary to make sure the ProgressModule shows reasonable output. */
    static long getNumEntriesToProcess();

  private:
    InputController() { }
    ~InputController() { }

    /** Is there an input module to be controlled? */
    static bool s_canControlInput;

    /** Are we merging events from two paths? */
    static bool s_doEventMerging;

    /** Explicit pointer to steerRootInput */
    static Module* s_steerRootInputModule;

    /** entry to be loaded the next time event() is called in an input module.
     *
     *  Storing two values (second one if independent path is executed)
     *  -1 indicates that execution should continue normally.
     */
    static std::pair<long, long> s_nextEntry;

    /** Experiment number to load next.
     *
     * -1 by default.
     */
    static long s_nextExperiment;

    /** Run number to load next. */
    static long s_nextRun;

    /** Event (not entry!) to load next. */
    static long s_nextEvent;

    /** number of events in paths if two input modules are used (independent paths) */
    static std::pair<long, long> s_eventNumbers;

    /** current entry in file.
     *  Storing two values (second one if independent path is executed)
     */
    static std::pair<long, long> s_currentEntry;

    /** entries skipped by RootInputModule (if any)
     *  Storing two values (second one if independent path is executed)
     */
    static std::pair<long, long> s_skippedEntries;

    /** Opened TChain (event durability).
     *  Storing two values (second one if independent path is executed)
     */
    static std::pair<const TChain*, const TChain*> s_chain;
  };
}
