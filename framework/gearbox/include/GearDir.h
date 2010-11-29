/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARDIR_H_
#define GEARDIR_H_

#include <framework/gearbox/GearboxIOAbs.h>

#include <string>
#include <TGeoMaterial.h>

namespace Belle2 {

  /**
   * The GearDir class.
   *
   * Follows the Decorator pattern of [1].
   *
   * References:
   * [1] Design Patterns: elements of reusable object-oriented software, E. Gamma et al.
   *     Addison Wesley professional computing series.
   */
  class GearDir : public GearboxIOAbs {

  public:

    /**
     * The GearDir constructor.
     *
     * @param path The prefix path of the GearDir.
     */
    GearDir(const std::string& path = "");

    /**
     * The GearDir constructor copying another GearDir and appending an additional path.
     *
     * @param gearDir The gearDir which should be copied.
     * @param path The path which should be appended to the path of the copied GearDir.
     */
    GearDir(GearDir& gearDir, const std::string& path);

    /**
     * The GearDir constructor copying another GearDir and appending an index value for iterating.
     *
     * @param gearDir The gearDir which should be copied.
     * @param index The index value which should be added to the path of the copied GearDir.
     */
    GearDir(GearDir& gearDir, const int index);

    /**
     * The GearGroup destructor.
     */
    virtual ~GearDir();

    /**
     * Closes the connection.
     *
     * @return True if the connection could be closed.
     */
    virtual bool close();

    /**
     * Returns true if the GearboxIO object is currently connected to a file/database etc.
     *
     * @return True if the GearboxIO object is currently connected to a file/database etc.
     */
    virtual bool isOpen() const;

    /**
     * Enables the validation of all paths and parameters.
     *
     * If set to true, a validity check of all paths and parameters is performed
     * each time they are accessed. By default the check is turned on.
     * Turn it off, in order to speed up the parameter access.
     *
     * @param pathCheck If set to true, a check of a path/parameter is performed each time it is accessed.
     */
    void enableParamCheck(bool paramCheck);

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
    int getNumberNodes(const std::string& path = "") const
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
    double getParamLength(const std::string& path = "") const
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
    double getParamAngle(const std::string& path = "") const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
          GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
          GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError);

    /**
     * Returns a parameter, given by the path, which describes an energy/mass/momentum.
     *
     * The path specifies the node, whose value should be returned. Instead of a single
     * node, a Path statement can also be used.
     * The attribute 'unit' of the node is used to convert the given angle parameter to [GeV].
     * The following units are currently supported: [eV], [keV], [MeV], [GeV], [TeV]
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
     * @return The value of the length parameter in [GeV]. If no unit or an unknown unit is given,
     *         the value is assumed to be in [GeV].
     */
    double getParamEnergy(const std::string& path = "") const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
          GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
          GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError);

    /**
     * Returns a parameter, given by the path, which describes a density.
     *
     * The path specifies the node, whose value should be returned. Instead of a single
     * node, a Path statement can also be used.
     * The attribute 'unit' of the node is used to convert the given angle parameter to [g/cm^3].
     * The following units are currently supported: [mg/cm^3], [kg/cm^3], [mg/mm^3], [g/mm^3], [kg/mm^3]
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
     * @return The value of the length parameter in [g/cm^3]. If no unit or an unknown unit is given,
     *         the value is assumed to be in [g/cm^3].
     */
    double getParamDensity(const std::string& path = "") const
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
    double getParamNumValue(const std::string& path = "") const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
          GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
          GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError);

    /**
     * Returns a parameter as a string.
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
    std::string getParamString(const std::string& path = "") const
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
          GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
          GearboxIOAbs::GearboxPathResultNotValidError);

    /**
     * Reads a material definition section and returns a pointer to the created TGeoMaterial object.
     * The material section can be either a <Material> or a <Mixture>.
     *
     * The following definitions for a <Material> are supported:
     * <Material name="">
     *   <Density unit="g/cm3"></Density>
     *   <RadLength></RadLength>
     *   <InterLength></InterLength>
     *   <AtomNumber> </AtomNumber>
     *   <MassNumber> </MassNumber>
     * </Material>
     *
     * <Material name="">
     *   <Density unit="g/cm3"></Density>
     *   <AtomNumber> </AtomNumber>
     *   <MassNumber> </MassNumber>
     * </Material>
     *
     * A Mixture consist of elements and a weight factor for each of them.
     * These elements can be an arbitrary combination of Elements, Materials
     * or other Mixtures. The basic structure is:
     * <Mixture name="">
     *   <Density unit="g/cm3"></Density>
     *   <Elements>
     *     add here the elements of the mixture.
     *   </Elements>
     * </Mixture>
     *
     * The elements of a mixture can be
     * 1) Elements
     *    <Element name="" weight="">
     *      <AtomNumber></AtomNumber>
     *      <MassNumber></MassNumber>
     *    </Element>
     *
     * 2) Materials
     *    Both material definitions given above are supported.
     *
     * 3) Mixtures
     *    As described here.
     *
     * 4) Existing Materials or Mixtures
     *    <Material name="" weight=""/>
     *    or
     *    <Mixture name="" weight=""/>
     *
     * @param path The path to the node which defines the TGeoMaterial or TGeoMixture.
     */
    TGeoMaterial* readMaterial(const std::string& path = "")
    throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
          GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
          GearboxIOAbs::GearboxPathResultNotValidError);

    /**
     * Sets the path which is added automatically as prefix to a given path.
     *
     * @param path The prefix path which is added in front of a given path.
     */
    void setDirPath(const std::string& path);

    /**
     * Returns the path which is added automatically as prefix to a given path.
     *
     * @return The prefix path which is added in front of a given path.
     */
    std::string getDirPath() const {return m_dirPath; };

    /**
     * Appends a new path to the current directory path. Useful to go into a deeper level of the hierarchy.
     *
     * @param path The path which is appended to the end of the current directory path.
    */
    void append(const std::string& path);

    /**
     * Appends a new path based on an index value to the current directory path.
     *
     * Useful to go into a deeper level of a hierarchy having the same identifier name.
     * For example:
     * <Ladders>
     *  <Ladder> 1 </Ladder>
     *  <Ladder> 2 </Ladder>
     *  <Ladder> 3 </Ladder>
     * </Ladders>
     *
     * Instead of having to write:
     *  laddersContent.append((format("Ladder[%1%]/") % (iLadder)).str());
     * this method allows having it much shorter:
     *  laddersContent.append("Ladder", iLadder);
     *
     * Please note: It is highly recommended to avoid having a trailing slash in the path parameter.
     *              The path of the GearDir after applying this method has a trailing slash.
     *
     * @param path The path which is appended together with an index to the end of the current directory path.
     * @param index The index value of the path.
    */
    void append(const std::string& path, int index);

    /**
     * Appends the path of the given GearDir to the current directory path. Useful to go into a deeper level of the hierarchy.
     *
     * @param gearDir The path of the this GearDir is appended to the end of the current directory path.
     */
    void append(GearDir& gearDir);


  protected:

    std::string m_dirPath;     /**< The prefix path which is added in front of a given path. */


  private:

  };

}

#endif /* GEARDIR_H_ */
