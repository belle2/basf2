/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/Configuration.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/database/Downloader.h>
#include <framework/database/Database.h>
#include <framework/utilities/Utils.h>
#include <boost/python.hpp>
#include <framework/core/PyObjConvUtils.h>
#include <framework/core/PyObjROOTUtils.h>
#include <boost/algorithm/string.hpp>

#include <set>
#include <TPython.h>

// Current default globaltag when generating events.
#define CURRENT_DEFAULT_TAG "master_2021-06-25"

namespace py = boost::python;

namespace {
  /** extract a list of strings from any iterable python object
   * This function is much more lenient than what we usually do: It will use `str()`
   * on each object in the list and use the string representation. So it should
   * work with basically any python object.
   */
  std::vector<std::string> extractStringList(const py::object& obj)
  {
    std::vector<std::string> result;
    Belle2::PyObjConvUtils::iteratePythonObject(obj, [&result](const boost::python::object & item) {
      py::object str(py::handle<>(PyObject_Str(item.ptr()))); // convert to string
      py::extract<std::string> extract(str); // and extract
      result.emplace_back(extract()); // and push back
      return true;
    });
    // done, return
    return result;
  }
}

namespace Belle2::Conditions {
  boost::python::list& CppOrPyList::ensurePy()
  {
    // convert to python list ...
    if (m_value.index() == 0) {
      boost::python::list tmp;
      for (const auto& e : std::get<0>(m_value)) { tmp.append(e); }
      m_value.emplace<boost::python::list>(std::move(tmp));
    }
    return std::get<1>(m_value);
  }

  std::vector<std::string>& CppOrPyList::ensureCpp()
  {
    // or convert to std::vector ...
    if (m_value.index() == 1) {
      std::vector<std::string> tmp = extractStringList(std::get<1>(m_value));
      m_value.emplace<std::vector<std::string>>(std::move(tmp));
    }
    return std::get<0>(m_value);
  }

  void CppOrPyList::append(const std::string& element)
  {
    std::visit(Utils::VisitOverload{
      [&element](std::vector<std::string>& list) {list.emplace_back(element);},
      [&element](boost::python::list & list) {list.append(element);}
    }, m_value);
  }

  void CppOrPyList::prepend(const std::string& element)
  {
    std::visit(Utils::VisitOverload{
      [&element](std::vector<std::string>& list) {list.emplace(list.begin(), element);},
      [&element](boost::python::list & list) {list.insert(0, element);}
    }, m_value);
  }

  void CppOrPyList::shallowCopy(const boost::python::object& source)
  {
    ensurePy().slice(boost::python::_, boost::python::_) = source;
  }

  Configuration& Configuration::getInstance()
  {
    static Configuration instance;
    return instance;
  }

  Configuration::Configuration()
  {
    // Backwards compatibility with the existing BELLE2_CONDB_GLOBALTAG
    // environment variable: If it is set disable replay
    if (EnvironmentVariables::isSet("BELLE2_CONDB_GLOBALTAG")) {
      fillFromEnv(m_globalTags, "BELLE2_CONDB_GLOBALTAG", "");
      overrideGlobalTags();
    }
    std::string serverList = EnvironmentVariables::get("BELLE2_CONDB_SERVERLIST", "http://belle2db.sdcc.bnl.gov/b2s/rest/");
    fillFromEnv(m_metadataProviders, "BELLE2_CONDB_METADATA", serverList + " /cvmfs/belle.cern.ch/conditions/database.sqlite");
    fillFromEnv(m_payloadLocations, "BELLE2_CONDB_PAYLOADS", "/cvmfs/belle.cern.ch/conditions");
  }

  void Configuration::reset()
  {
    if (m_databaseInitialized) {
      Database::Instance().reset(true);
    }
    *this = Configuration();
  }

  std::vector<std::string> Configuration::getDefaultGlobalTags() const
  {
    // currently the default globaltag can be overwritten by environment variable
    // so keep that
    return EnvironmentVariables::getOrCreateList("BELLE2_CONDB_GLOBALTAG", CURRENT_DEFAULT_TAG);
  }

  py::tuple Configuration::getDefaultGlobalTagsPy() const
  {
    // same as above but as a python tuple ...
    py::list list;
    fillFromEnv(list, "BELLE2_CONDB_GLOBALTAG", CURRENT_DEFAULT_TAG);
    return py::tuple(list);
  }

  void Configuration::setInputMetadata(const std::vector<FileMetaData>& inputMetadata)
  {
    ensureEditable();
    m_inputMetadata = inputMetadata;
    // make sure the list of globaltags to be used is created but empty
    m_inputGlobaltags.emplace();
    // now check for compatibility: make sure all metadata have the same globaltag
    // setting. Unless we don't have metadata ...
    if (inputMetadata.empty()) return;

    std::optional<std::string> inputGlobaltags;
    for (const auto& metadata : inputMetadata) {
      if (!inputGlobaltags) {
        inputGlobaltags = metadata.getDatabaseGlobalTag();
      } else {
        if (inputGlobaltags != metadata.getDatabaseGlobalTag()) {
          B2WARNING("Input files metadata contain incompatible globaltag settings, globaltag replay not possible");
          // no need to set anything
          return;
        }
      }
    }
    // if it's still set and empty we have an empty input list ... warn specifically.
    if (inputGlobaltags and inputGlobaltags->empty()) {
      B2WARNING("Input files metadata all have empty globaltag setting, globaltag replay not possible");
      return;
    }
    // set the list of globaltags from the string containing the globaltags
    boost::split(*m_inputGlobaltags, *inputGlobaltags, boost::is_any_of(","));

    // HACK: So, we successfully set the input globaltags from the input file,
    // however we also decided that we want to add new payloads for
    // boost/invariant mass/beam spot. So if any of the files was created
    // before the first of October 2019 we assume their globaltag might be
    // missing these new payloads and we append an extra globaltag containing
    // just this information with lowest priority. If the files actually had
    // all payloads these legacy payloads will never be used as they have
    // lowest priority. Otherwise this should enable running over old files.
    //
    // TODO: Once we're sure all files being used contain all payloads remove this.
    std::optional<std::string> youngest;
    for (const auto& metadata : inputMetadata) {
      // Skip release 4 or later files.
      const std::string& release = metadata.getRelease();
      if (release.substr(0, 8) == "release-" and
          release.compare(8, 2, "04", 2) >= 0)
        continue;
      // Otherwise, get the date of the youngest file.
      if (!youngest or * youngest > metadata.getDate()) {
        youngest = metadata.getDate();
      }
    }
    if (youngest and youngest->compare("2019-12-31") < 0) {
      B2DEBUG(30, "Enabling legacy IP information globaltag in tag replay");
      m_inputGlobaltags->emplace_back("Legacy_IP_Information");
    }
    // END TODO/HACK
  }

  std::vector<std::string> Configuration::getBaseTags() const
  {
    // return the list of base tags to be used: Either the default tag
    // or the list of globaltags from the input files
    if (not m_inputGlobaltags) return getDefaultGlobalTags();
    return *m_inputGlobaltags;
  }

  std::vector<std::string> Configuration::getFinalListOfTags()
  {
    if (m_overrideEnabled) {
      B2INFO("Global tag override is in effect: input globaltags and default globaltag will be ignored");
      return m_globalTags.ensureCpp();
    }

    auto baseList = getBaseTags();
    if (m_callback) {
      // Create a dictionary of keyword arguments for the callback
      py::dict arguments;
      // we want a python list of the base tags
      {
        py::list baseListPy;
        for (const auto& tag : baseList) baseListPy.append(tag);
        arguments["base_tags"] = baseListPy;
      }
      // and set the user tags from our list.
      arguments["user_tags"] = m_globalTags.ensurePy();
      // and prepare list of metadata. It's None when no replay has been
      // requested which should mean that we generate events
      arguments["metadata"] = py::object();
      // otherwise it's a list of file metadata instances
      if (m_inputGlobaltags) {
        py::list metaDataList;
        for (const auto& m : m_inputMetadata) metaDataList.append(createROOTObjectPyCopy(m));
        arguments["metadata"] = metaDataList;
      }
      // arguments ready, call callback function, python will handle the exceptions
      py::object retval = (*m_callback)(*py::tuple(), **arguments);
      // If the return value is not None it should be an iterable
      // containing the final tag list
      if (retval != py::object()) {
        return extractStringList(retval);
      }
      // callback returned None so fall back to default
    }
    // Default tag replay ... bail if list of globaltags is empty
    if (baseList.empty()) {
      if (m_inputGlobaltags) {
        B2FATAL(R"(No baseline globaltags available.
    The input files you selected don't have compatible globaltags or an empty
    globaltag setting. As such globaltag configuration cannot be determined
    automatically.

    If you really sure that it is a good idea to process these files together
    you have to manually override the list of globaltags:

    >>> basf2.conditions.override_globaltags()
)");
      }else{
        B2FATAL(R"(No default globaltags available.
    There is no default globaltag available for processing. This usually means
    you set the environment variable BELLE2_CONDB_GLOBALTAG to an empty value.

    As this is unlikely to work for even the most basic functionality this is not
    directly supported anymore. If you really want to disable any access to the
    conditions database please configure this explicitly

    >>> basf2.conditions.metadata_providers = []
    >>> basf2.conditions.override_globaltags([])
)");
      }
    }
    // We have base tags and possibly user tags, so return both
    std::vector finalList = m_globalTags.ensureCpp();
    for (const auto& tag : baseList) { finalList.emplace_back(tag); }
    return finalList;
  }

  namespace {
    /** Configure the network settings for the Conditions database downloads */
    boost::python::dict expertSettings(const boost::python::tuple& args, boost::python::dict kwargs)
    {
      if (py::len(args) != 1) {
        // keyword only function: raise typerror on non-keyword arguments
        PyErr_SetString(PyExc_TypeError, ("expert_settings() takes one positional argument but " +
                                          std::to_string(len(args)) + " were given").c_str());
        py::throw_error_already_set();
      }
      Configuration& self = py::extract<Configuration&>(args[0]);

      py::dict result;
      // We want to check for a list of names if they exist in the input keyword
      // arguments. If so, set the new value. In any case, add to output
      // dictionary. Simplest way: create a variadic lambda with references to the
      // dictionary and arguments for name, getter and setter.
      auto checkValue = [&kwargs, &result](const std::string & name, auto setter, auto getter) {
        using value_type = decltype(getter());
        if (kwargs.has_key(name)) {
          value_type value{};
          py::object object = kwargs[name];
          try {
            value = PyObjConvUtils::convertPythonObject(object, value);
          } catch (std::runtime_error&) {
            std::stringstream error;
            error << "Cannot convert argument '" << name << "' to " << PyObjConvUtils::Type<value_type>::name();
            PyErr_SetString(PyExc_TypeError, error.str().c_str());
            py::throw_error_already_set();
          }
          setter(value);
          // remove key from kwargs so we can easily check for ones we don't understand later
          py::delitem(kwargs, py::object(name));
        }
        result[name] = PyObjConvUtils::convertToPythonObject(getter());
      };
      auto& downloader = Downloader::getDefaultInstance();
      // That was all the heavy lifting, now just declare all known options :D
      // I would love to indent this a bit better but astyle has different opinions
      checkValue("save_payloads",
      [&self](const std::string & path) { self.setNewPayloadLocation(path);},
      [&self]() {return self.getNewPayloadLocation();});
      checkValue("download_cache_location",
      [&self](const std::string & path) { self.setDownloadCacheDirectory(path);},
      [&self]() {return self.getDownloadCacheDirectory();});
      checkValue("download_lock_timeout",
      [&self](size_t timeout) { self.setDownloadLockTimeout(timeout);},
      [&self]() { return self.getDownloadLockTimeout();});
      checkValue("usable_globaltag_states",
      [&self](const auto & states) { self.setUsableTagStates(states); },
      [&self]() { return self.getUsableTagStates(); });
      checkValue("connection_timeout",
      [&downloader](unsigned int timeout) {downloader.setConnectionTimeout(timeout);},
      [&downloader]() { return downloader.getConnectionTimeout();});
      checkValue("stalled_timeout",
      [&downloader](unsigned int timeout) {downloader.setStalledTimeout(timeout);},
      [&downloader]() { return downloader.getStalledTimeout();});
      checkValue("max_retries",
      [&downloader](unsigned int retries) {downloader.setMaxRetries(retries);},
      [&downloader]() { return downloader.getMaxRetries();});
      checkValue("backoff_factor",
      [&downloader](unsigned int factor) { downloader.setBackoffFactor(factor);},
      [&downloader]() { return downloader.getBackoffFactor();});
      // And lastly check if there is something in the kwargs we don't understand ...
      if (py::len(kwargs) > 0) {
        std::string message = "Unrecognized keyword arguments: ";
        auto keys = kwargs.keys();
        for (int i = 0; i < len(keys); ++i) {
          if (i > 0) message += ", ";
          message += py::extract<std::string>(keys[i]);
        }
        PyErr_SetString(PyExc_TypeError, message.c_str());
        py::throw_error_already_set();
      }
      return result;
    }
  }

  void Configuration::overrideGlobalTagsPy(const boost::python::list& globalTags)
  {
    setGlobalTagsPy(globalTags);
    m_overrideEnabled = true;
  }

  void Configuration::exposePythonAPI()
  {
    //don't show c++ signature in python doc to keep it simple
    py::docstring_options options(true, false, false);

    void (Configuration::*overrideGTFlag)() = &Configuration::overrideGlobalTags;
    void (Configuration::*overrideGTList)(const py::list&) = &Configuration::overrideGlobalTagsPy;
    py::object expert = raw_function(expertSettings);
    py::class_<Configuration>("ConditionsConfiguration", R"DOC(
This class contains all configurations for the conditions database service

* which globaltags to use
* where to look for payload information
* where to find the actual payload files
* which temporary testing payloads to use

But for most users the only thing they should need to care about is to set the
list of additional `globaltags` to use.
)DOC")
    .add_property("override_enabled", &Configuration::overrideEnabled, R"DOC(
Indicator whether or not the override of globaltags is enabled. If true then
globaltags present in input files  will be ignored and only the ones given in
`globaltags` will be considered.
)DOC")
    .def("reset", &Configuration::reset, R"DOC(reset()

Reset the conditions database configuration to its original state.
)DOC")
    .add_property("default_globaltags", &Configuration::getDefaultGlobalTagsPy, R"DOC(
A tuple containing the default globaltags to be used if events are generated without an input file.
)DOC")
    .add_property("globaltags", &Configuration::getGlobalTagsPy, &Configuration::setGlobalTagsPy, R"DOC(
List of globaltags to be used. These globaltags will be the ones with highest
priority but by default the globaltags used to create the input files or the
default globaltag will also be used.

The priority of the globaltags in this list is highest first. So the first in
the list will be checked first and all other globaltags will only be checked for
payloads not found so far.

Warning:
    By default this list contains the globaltags to be used **in addition** to
    the ones from the input file or the default one if no input file is present.
    If this is not desirable you need to call `override_globaltags()` to disable
    any addition or modification of this list.
)DOC")
    .def("append_globaltag", &Configuration::appendGlobalTag, py::args("name"), R"DOC(append_globaltag(name)

Append a globaltag to the end of the `globaltags` list. That means it will be
the lowest priority of all tags in the list.
)DOC")
    .def("prepend_globaltag", &Configuration::prependGlobalTag, py::args("name"), R"DOC(prepend_globaltag(name)

Add a globaltag to the beginning of the `globaltags` list. That means it will be
the highest priority of all tags in the list.
)DOC")
    .def("override_globaltags", overrideGTFlag)
    .def("override_globaltags", overrideGTList, py::args("globaltags"), R"DOC(override_globaltags(list=None)

Enable globaltag override. This disables all modification of the globaltag list at the beginning of processing:

* the default globaltag or the input file globaltags will be ignored.
* any callback set with `set_globaltag_callback` will be ignored.
* the list of `globaltags` will be used exactly as it is.

Parameters:
    list (list(str) or None) if given this list will replace the current content of `globaltags`

Warning:
    it's still possible to modify `globaltags` after this call.
)DOC")
    .def("disable_globaltag_replay", &Configuration::disableGlobalTagReplay, R"DOC("disable_globaltag_replay()

Disable global tag replay and revert to the old behavior that the default
globaltag will be used if no other globaltags are specified.

This is a shortcut to just calling

    >>> conditions.override_globaltags()
    >>> conditions.globaltags += list(conditions.default_globaltags)

)DOC")
    .def("append_testing_payloads", &Configuration::appendTestingPayloadLocation, py::args("filename"), R"DOC(append_testing_payloads(filename)

Append a text file containing local test payloads to the end of the list of
`testing_payloads`. This will mean they will have lower priority than payloads
in previously defined text files but still higher priority than globaltags.

Parameters:
    filename (str): file containing a local definition of payloads and their
        intervals of validity for testing

Warning:
    This functionality is strictly for testing purposes. Using local payloads
    leads to results which cannot be reproduced by anyone else and thus cannot
    be published.
)DOC")
    .def("prepend_testing_payloads", &Configuration::prependTestingPayloadLocation, py::args("filename"), R"DOC(prepend_testing_payloads(filename)

Insert a text file containing local test payloads in the beginning of the list
of `testing_payloads`. This will mean they will have higher priority than payloads in
previously defined text files as well as higher priority than globaltags.

Parameters:
    filename (str): file containing a local definition of payloads and their
        intervals of validity for testing

Warning:
    This functionality is strictly for testing purposes. Using local payloads
    leads to results which cannot be reproduced by anyone else and thus cannot
    be published.
)DOC")
    .add_property("testing_payloads", &Configuration::getTestingPayloadLocationsPy, &Configuration::setTestingPayloadLocationsPy, R"DOC(
List of text files to look for local testing payloads. Each entry should be a
text file containing local payloads and their intervals of validity to be used
for testing.

Payloads found in these files and valid for the current run will have a higher
priority than any of the `globaltags`. If a valid payload is present in multiple
files the first one in the list will have higher priority.

Warning:
    This functionality is strictly for testing purposes. Using local payloads
    leads to results which cannot be reproduced by anyone else and thus cannot
    be published.
)DOC")
    .add_property("metadata_providers", &Configuration::getMetadataProvidersPy, &Configuration::setMetadataProvidersPy, R"DOC(
List of metadata providers to use when looking for payload metadata. There are currently two supported providers:

1. Central metadata provider to look for payloads in the central conditions database.
   This provider is used for any entry in this list which starts with ``http(s)://``.
   The URL should point to the top level of the REST api endpoints on the server

2. Local metadata provider to look for payloads in a local SQLite snapshot taken
   from the central server. This provider will be assumed for any entry in this
   list not starting with a protocol specifier or if the protocol is given as ``file://``

This list should rarely need to be changed. The only exception is for users who
want to be able to use the software without internet connection after they
downloaded a snapshot of the necessary globaltags with ``b2conditionsdb download``
to point to this location.
)DOC")
    .add_property("payload_locations", &Configuration::getPayloadLocationsPy, &Configuration::setPayloadLocationsPy, R"DOC(
List of payload locations to search for payloads which have been found by any of
the configured `metadata_providers`. This can be a local directory or a
``http(s)://`` url pointing to the payload directory on a server.

For remote locations starting with ``http(s)://`` we assume that the layout of
the payloads on the server is the same as on the main payload server:
The combination of given location and the relative url in the payload metadata
field ``payloadUrl`` should point to the correct payload on the server.

For local directories, two layouts are supported and will be auto detected:

flat
    All payloads are in the same directory without any substructure with the name
    ``dbstore_{name}_rev_{revision}.root``
hashed
    All payloads are stored in subdirectories in the form ``AB/{name}_r{revision}.root``
    where ``A`` and ``B`` are the first two characters of the md5 checksum of the
    payload file.

Example:
  Given ``payload_locations = ["payload_dir/", "http://server.com/payloads"]``
  the framework would look for a payload with name ``BeamParameters`` in revision
  ``45`` (and checksum ``a34ce5...``) in the following places


  1. ``payload_dir/a3/BeamParameters_r45.root``
  2. ``payload_dir/dbstore_BeamParameters_rev_45.root``
  3. ``http://server.com/payloads/dbstore/BeamParameters/dbstore_BeamParameters_rev_45.root``
     given the usual pattern of the ``payloadUrl`` metadata. But this could be
     changed on the central servers so mirrors should not depend on this convention
     but copy the actual structure of the central server.

If the payload cannot be found in any of the given locations the framework will
always attempt to download it directly from the central server and put it in a
local cache directory.
)DOC")
    .def("expert_settings", expert, R"DOC(expert_settings(**kwargs)

Set some additional settings for the conditions database.

You can supply any combination of keyword-only arguments defined below. The
function will return a dictionary containing all current settings.

    >>> conditions.expert_settings(connection_timeout=5, max_retries=1)
    {'save_payloads': 'localdb/database.txt',
     'download_cache_location': '',
     'download_lock_timeout': 120,
     'usable_globaltag_states': {'PUBLISHED', 'RUNNING', 'TESTING', 'VALIDATED'},
     'connection_timeout': 5,
     'stalled_timeout': 60,
     'max_retries': 1,
     'backoff_factor': 5}

Warning:
    Modification of these parameters should not be needed, in rare
    circumstances this could be used to optimize access for many jobs at once
    but should only be set by experts.

Parameters:
  save_payloads (str): Where to store new payloads created during processing.
      This should be a filename to contain the payload information and the payload
      files will be placed in the same directory as the file.
  download_cache_location (str): Where to store payloads which have been downloaded
      from the central server. This could be a user defined directory, otherwise
      empty string defaults to ``$TMPDIR/basf2-conditions`` where ``$TMPDIR`` is the
      temporary directories defined in the system. Newly downloaded payloads will
      be stored in this directory in a hashed structure, see `payload_locations`
  download_lock_timeout (int): How many seconds to wait for a write lock when
      concurrently downloading the same payload between different processes.
      If locking fails the payload will be downloaded to a temporary file
      separately for each process.
  usable_globaltag_states (set(str)): Names of globaltag states accepted for
      processing. This can be changed to make sure that only fully published
      globaltags are used or to enable running on an open tag. It is not possible
      to allow usage of 'INVALID' tags, those will always be rejected.
  connection_timeout (int): timeout in seconds before connection should be
      aborted. 0 sets the timeout to the default (300s)
  stalled_timeout (int): timeout in seconds before a download should be
      aborted if the speed stays below 10 KB/s, 0 disables this timeout
  max_retries (int): maximum amount of retries if the server responded with
      an HTTP response of 500 or more. 0 disables retrying
  backoff_factor (int): backoff factor for retries in seconds. Retries are
      performed using something similar to binary backoff: For retry :math:`n`
      and a ``backoff_factor`` :math:`f` we wait for a random time chosen
      uniformely from the interval :math:`[1, (2^{n} - 1) \times f]` in
      seconds.
)DOC")
    .def("set_globaltag_callback", &Configuration::setGlobaltagCallbackPy, R"DOC(set_globaltag_callback(function)

Set a callback function to be called just before processing.

This callback can be used to further customize the globaltags to be used during
processing. It will be called after the input files have been opened and checked
with three keyword arguments:

base_tags
    The globaltags determined from either the input files or, if no input files
    are present, the default globaltags

user_tags
    The globaltags provided by the user

metadata
    If there are not input files (e.g. generating events) this argument is None.
    Otherwise it is a list of all the ``FileMetaData`` instances from all input files.
    This list can be empty if there is no metadata associated with the input files.

From this information the callback function should then compose the final list
of globaltags to be used for processing and return this list. If ``None`` is
returned the default behavior is applied as if there were no callback function.
If anything else is returned the processing is aborted.

If no callback function is specified the default behavior is equivalent to ::

    def callback(base_tags, user_tags, metadata):
      if not base_tags:
        basf2.B2FATAL("No baseline globaltags available. Please use override")

      return user_tags + base_tags

If `override_enabled` is ``True`` then the callback function will not be called.

Warning:
  If a callback is set it is responsible to select the correct list of globaltags
  and also make sure that all files are compatible. No further checks will be
  done by the framework but any list of globaltags which is returned will be used
  exactly as it is.

  If the list of ``base_tags`` is empty that usually means that the input files
  had different globaltag settings but it is the responsibility of the callback
  to then verify if the list of globaltags is usable or not.

  If the callback function determines that no working set of globaltags can be
  determined then it should abort processing using a FATAL error or an exception
)DOC")
    ;

    py::scope().attr("conditions") = py::ptr(&Configuration::getInstance());
  }
} // Belle2::Conditions namespace
