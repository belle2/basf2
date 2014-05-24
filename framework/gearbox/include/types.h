#ifndef GEARBOX_TYPES_H_
#define GEARBOX_TYPES_H_

#include <string>
#include <map>
#include <list>

#include <boost/variant.hpp>
#include <boost/property_tree/ptree.hpp>


namespace Belle2 {
  namespace gearbox {

    class Backend;
    struct BackendMountHandle;


    /** one backend configuration parameter */
    typedef boost::variant<int, double, std::string> BackendConfigParam;

    /** a set of backend configuration parameters */
    typedef std::map<std::string, BackendConfigParam> BackendConfigParamSet;

    /** Pointer to a existing Backend */
    // we need boost::shared_ptr here instead of std::shared_ptr because of
    // this problem: https://svn.boost.org/trac/boost/ticket/6545
    //// typedef boost::shared_ptr<gearbox::Backend> BackendPtr;
    // unfortunately, the shared pointer does not correctly remember the use
    // count, when passed from c++ to python
    // and the raw pointer needs a return value policy to be defined, e.g. using
    // ", return_value_policy<manage_new_object>())"
    // but this results in "'manage_new_object' was not declared in this scope"
    // -> everything very nice.
    typedef gearbox::Backend* BackendPtr;

    /** Pointer to a backend mount Handle */
    typedef std::list<BackendMountHandle>::iterator BackendMountHandlePtr;

    /** type of value of a result set node */
    typedef boost::variant<int, double, std::string> nodeValue_t;

    /** type of unique node ID */
    typedef std::string nodeId_t;

    /** type of query result node */
    typedef std::pair<nodeId_t, nodeValue_t> QryResultNode;

    /** type of a query result set, to which a node is attached */
    typedef boost::property_tree::basic_ptree<std::string, QryResultNode> QryResultSet;

    /** the existing mount modes */
    enum class EMountMode {
      overlay,//!< overlay
      merge   //!< merge
    };

    /** the existing node exitence states */
    enum class nodeExistenceState {
      add,
      modify,
      remove,
      remove_all,
      purge,
      purge_all
    };


    // TODO: temporary solution - ordinary functions for that task
    // Boost::uuid could be interesting, but length and charset can not be adjusted
    /** Seed the random number generator */
    void randomSeed();
    /**
     * Generate a random string of a certain length
     * @param length the length of the string that is to be generated
     * @return the resulting random string
     */
    std::string genRandomString(size_t length);


    /**
     * This struct represents a backend mount handle
     */
    struct BackendMountHandle {

      /** Pointer to the mounted backend */
      BackendPtr backend;

      /** Path within the backend, that is mounted */
      std::string mountPath;

      /** Mount point within the global hierarchy */
      std::string mountPoint;

      /** The mount mode **/
      EMountMode mountMode;

      /** Constructor accepting all members */
      BackendMountHandle(BackendPtr backend, std::string mountPath, std::string mountPoint, EMountMode mountMode)
        : backend(backend), mountPath(std::move(mountPath)), mountPoint(std::move(mountPoint)), mountMode(mountMode) {

      }

    };


  }  // namespace gearbox
}

#endif  // GEARBOX_TYPES_H_

