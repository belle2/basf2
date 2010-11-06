/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARBOXIOXML_H_
#define GEARBOXIOXML_H_

#include <libxml/xpath.h>

#include <framework/gearbox/GearboxIOAbs.h>

#include <string>
#include <map>
#include <cmath>


namespace Belle2 {

  /**
   * The GearboxIOXML class.
   *
   * This class implements a XML persistency for the detector parameters.
   */
  class GearboxIOXML : public GearboxIOAbs {

  public:

    /** The GearboxIOXML constructor. */
    GearboxIOXML();

    /** The GearboxIOXML destructor. */
    ~GearboxIOXML();

    /**
     * Opens the connection to the XML file where the parameters are stored.
     *
     * @param filename The filename (+path) to the XML file should be opened.
     * @return True if the connection could be opened.
     */
    bool open(const std::string& filename);

    /**
     * Closes the connection.
     *
     * @return True if the connection could be closed.
     */
    bool close();

    /**
     * Returns true if the GearboxIO object is currently connected to a file/database etc.
     *
     * @return True if the GearboxIO object is currently connected to a file/database etc.
     */
    bool isOpen() const;

    /**
     * Enables the validation of all paths and parameters.
     *
     * If set to true, a validity check of all paths and parameters is performed
     * each time they are accessed. By default the check is turned on.
     * Turn it off, in order to speed up the parameter access.
     *
     * @param paramCheck If set to true, a check of a path/parameter is performed each time it is accessed.
     */
    void enableParamCheck(bool paramCheck) {m_enableParamCheck = paramCheck; };

    /**
     * Checks if the given path is a valid path.
     *
     * Different types of exceptions can be thrown:
     * GearboxIONotConnectedError: if the GearboxIO is not connected to a storage medium.
     *
     * @param path The path which should be validated.
     * @return True if the path is valid.
     */
    bool isPathValid(const std::string& path) const
    throw(GearboxIOAbs::GearboxIONotConnectedError);

    /**
     * Checks if a parameter given by the path is available.
     *
     * Different types of exceptions can be thrown:
     * GearboxIONotConnectedError: if the GearboxIO is not connected to a storage medium.
     * GearboxPathNotValidError: if the path statement is not valid.
     *
     * @param path The path to the node which should be checked for existence.
     * @return True if the path to the node and the node (parameter) itself exists.
     */
    bool isParamAvailable(const std::string& path) const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError);

    /**
     * Returns the number of nodes given by the last node in the path.
     *
     * Different types of exceptions can be thrown:
     * GearboxIONotConnectedError: if the GearboxIO is not connected to a storage medium.
     * GearboxPathNotValidError: if the path statement is not valid.
     * GearboxPathEmptyResultError: if the returned result of the path query is empty.
     * GearboxPathResultNotValidError: if the returned type of the path query is not supported.
     *
     * @param path The path to the nodes which are counted.
     * @return The number of nodes.
     */
    int getNumberNodes(const std::string& path) const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
          GearboxIOAbs::GearboxPathEmptyResultError, GearboxIOAbs::GearboxPathResultNotValidError);

    /**
     * Returns a parameter, given by the path, which describes a length.
     *
     * The path specifies the node, whose value should be returned. Instead of a single
     * node, a Path statement can also be used.
     * The attribute 'unit' of the node is used to convert the given length parameter to [cm].
     * The following units are currently supported: [mum], [mm], [cm], [m], [km]
     *
     * Different types of exceptions can be thrown:
     * GearboxIONotConnectedError: if the GearboxIO is not connected to a storage medium.
     * GearboxPathNotValidError: if the path statement is not valid.
     * GearboxParamNotExistsError: if the parameter does not exist and the parameter check is enabled.
     * GearboxPathEmptyResultError: if the returned result of the path query is empty.
     * GearboxPathResultNotValidError: if the returned type of the path query is not supported.
     * GearboxStringNumConversionError: if the conversion of a string to a numerical value failed.
     *
     * @param path The path to the node which should be returned.
     * @return The value of the length parameter in [cm]. If no unit or an unknown unit is given,
     *         the value is assumed to be in [cm].
     */
    double getParamLength(const std::string& path) const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
          GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
          GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError);

    /**
     * Returns a parameter, given by the path, which describes an angle.
     *
     * The path specifies the node, whose value should be returned. Instead of a single
     * node, a Path statement can also be used.
     * The attribute 'unit' of the node is used to convert the given angle parameter to [rad].
     * The following units are currently supported: [deg], [rad], [mrad]
     *
     * Different types of exceptions can be thrown:
     * GearboxIONotConnectedError: if the GearboxIO is not connected to a storage medium.
     * GearboxPathNotValidError: if the path statement is not valid.
     * GearboxParamNotExistsError: if the parameter does not exist and the parameter check is enabled.
     * GearboxPathEmptyResultError: if the returned result of the path query is empty.
     * GearboxPathResultNotValidError: if the returned type of the path query is not supported.
     * GearboxStringNumConversionError: if the conversion of a string to a numerical value failed.
     *
     * @param path The path to the node which should be returned.
     * @return The value of the length parameter in [rad]. If no unit or an unknown unit is given,
     *         the value is assumed to be in [rad].
     */
    double getParamAngle(const std::string& path) const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
          GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
          GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError);

    /**
     * Returns a parameter, given by the path, which describes a general numerical value.
     *
     * The path specifies the node, which value should be returned. Instead of a single
     * node, a Path statement can also be used.
     *
     * Different types of exceptions can be thrown:
     * GearboxIONotConnectedError: if the GearboxIO is not connected to a storage medium.
     * GearboxPathNotValidError: if the path statement is not valid.
     * GearboxParamNotExistsError: if the parameter does not exist and the parameter check is enabled.
     * GearboxPathEmptyResultError: if the returned result of the path query is empty.
     * GearboxPathResultNotValidError: if the returned type of the path query is not supported.
     * GearboxStringNumConversionError: if the conversion of a string to a numerical value failed.
     *
     * @param path The path to the node which should be returned.
     * @return The numerical value.
     */
    double getParamNumValue(const std::string& path) const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
          GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
          GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError);

    /** Returns a parameter as a string.
     *
     * The path specifies the node, which value should be returned. Instead of a single
     * node, a Path statement can also be used.
     *
     * Different types of exceptions can be thrown:
     * GearboxIONotConnectedError: if the GearboxIO is not connected to a storage medium.
     * GearboxPathNotValidError: if the path statement is not valid.
     * GearboxParamNotExistsError: if the parameter does not exist and the parameter check is enabled.
     * GearboxPathEmptyResultError: if the returned result of the path query is empty.
     * GearboxPathResultNotValidError: if the returned type of the path query is not supported.
     *
     * @param path The path to the node which should be returned.
     * @return The string value.
     */
    std::string getParamString(const std::string& path) const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
          GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
          GearboxIOAbs::GearboxPathResultNotValidError);


  protected:

    /** Definition of the supported length units. */
    enum ELengthUnitTypes { c_UM,  /**< length unit [micrometer]. */
                            c_MM,  /**< length unit [mm]. */
                            c_CM,  /**< length unit [cm]. */
                            c_M,   /**< length unit [m]. */
                            c_KM   /**< length unit [km]. */
                          };

    /** Definition of the supported length units. */
    enum EAngleUnitTypes { c_Deg,  /**< angle unit [deg]. */
                           c_Rad,  /**< angle unit [rad]. */
                           c_MRad  /**< angle unit [mrad]. */
                         };

    xmlDocPtr m_xmlDocument;                    /**< The XML document. */
    std::map<std::string, int> m_lengthUnitMap; /**< Maps a string representing a length unit to the unit type. */
    std::map<std::string, int> m_angleUnitMap;  /**< Maps a string representing an angle unit to the unit type. */

    bool m_enableParamCheck; /**< If set to true, performs a check of the path/parameter each time it is accessed. */


  private:

    /** Returns the result set of a XPath query.
     *
     * @param document Pointer to the XML document.
     * @param xpath The XPath query which should be carried out.
     * @return The result set of the XPath query.
     */
    xmlXPathObjectPtr getNodeSet(xmlDocPtr document, xmlChar *xpath) const;

    /**
     * Returns a parameter as double value and its unit.
     *
     * Different types of exceptions can be thrown:
     * GearboxIONotConnectedError: if the GearboxIO is not connected to a storage medium.
     * GearboxPathNotValidError: if the path statement is not valid.
     * GearboxPathEmptyResultError: if the returned result of the path query is empty.
     * GearboxPathResultNotValidError: if the returned type of the path query is not supported.
     * GearboxStringNumConversionError: if the conversion of a string to a numerical value failed.
     *
     * @param value The found value is returned in this variable.
     * @param unit The found unit is returned in this variable.
     * @param xpath The XPath statement defining the requested node.
     * @param defaultUnit The default unit which is used if no unit is found.
     * @param unitMap The map which links unit string representations to unit types.
     */
    void getDoubleWithUnit(double& value, int& unit, const std::string& xpath,
                           int defaultUnit, const std::map<std::string, int>& unitMap) const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError, GearboxIOAbs::GearboxPathEmptyResultError,
          GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError);

    /** Fills the map which links the string representing of a length unit to the type if the unit. */
    void setLengthUnitMap();

    /** Fills the map which links the string representing of an angle unit to the type if the unit. */
    void setAngleUnitMap();

  };

} //end of namespace Belle2


#endif /* GEARBOXXML_H_ */
