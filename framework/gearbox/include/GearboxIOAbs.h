/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARBOXIOABS_H
#define GEARBOXIOABS_H

#include <framework/core/FrameworkExceptions.h>

#include <string>

namespace Belle2 {

  /**
   * The GearboxIOAbs class.
   *
   * Defines an abstract interface class for reading parameters from an arbitrary source.
   *
   * The parameters of the detector geometry are organized as a tree, where nodes
   * represent subdetectors, subdetector components and component parameters.
   * The GearboxIO provides a common node-level interface to the detector parameters,
   * without having to implement a specific type of the persistency.
   */
  class GearboxIOAbs {

  public:

    //Define exceptions
    /** Exception is thrown if the GearboxIO is not connected to a storage medium. */
    BELLE2_DEFINE_EXCEPTION(GearboxIONotConnectedError, "The GearboxIO object is not available. Please make sure you have the Gearbox module in your path !");
    /** Exception is thrown if the path statement is not valid. */
    BELLE2_DEFINE_EXCEPTION(GearboxPathNotValidError, "The path \"%1%\" is not valid !");
    /** Exception is thrown if the returned result of the path query is empty. */
    BELLE2_DEFINE_EXCEPTION(GearboxPathEmptyResultError, "The result of the path \"%1%\" is empty !");
    /** Exception is thrown if the returned type of the path query is not supported. */
    BELLE2_DEFINE_EXCEPTION(GearboxPathResultNotValidError, "The result of the path \"%1%\" is not valid !");
    /** Exception is thrown if the parameter does not exist and the parameter check is enabled. */
    BELLE2_DEFINE_EXCEPTION(GearboxParamNotExistsError, "The requested gearbox parameter %1% does not exist !");
    /** Exception is thrown if the conversion of a string to a numerical value failed. */
    BELLE2_DEFINE_EXCEPTION(GearboxStringNumConversionError, "The conversion of the string \"%1%\" to a number failed !");

    /** The GearboxIOAbs constructor. */
    GearboxIOAbs() {};

    /** The GearboxIOAbs destructor. */
    virtual ~GearboxIOAbs() {};

    /**
     * Closes the connection.
     *
     * @return True if the connection could be closed.
     */
    virtual bool close()  = 0;

    /**
     * Returns true if the GearboxIO object is currently connected to a file/database etc.
     *
     * @return True if the GearboxIO object is currently connected to a file/database etc.
     */
    virtual bool isOpen() const = 0;

    /**
     * Enables the validation of all paths and parameters.
     *
     * If set to true, a validity check of all paths and parameters is performed
     * each time they are accessed. By default the check is turned on.
     * Turn it off, in order to speed up the parameter access.
     *
     * @param pathCheck If set to true, a check of a path/parameter is performed each time it is accessed.
     */
    virtual void enableParamCheck(bool paramCheck) = 0;

    /**
     * Checks if the given path is a valid path.
     *
     * Different types of exceptions can be thrown:
     * GearboxIONotConnectedError: if the GearboxIO is not connected to a storage medium.
     *
     * @param path The path which should be validated.
     * @return True if the path is valid.
     */
    virtual bool isPathValid(const std::string& path) const
    throw(GearboxIONotConnectedError) = 0;

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
    virtual bool isParamAvailable(const std::string& path) const
    throw(GearboxIONotConnectedError, GearboxPathNotValidError) = 0;

    /**
     * Returns the name of the node the path is pointing to.
     *
     * Different types of exceptions can be thrown:
     * GearboxIONotConnectedError: if the GearboxIO is not connected to a storage medium.
     * GearboxPathNotValidError: if the path statement is not valid.
     * GearboxParamNotExistsError: if the parameter does not exist and the parameter check is enabled.
     * GearboxPathEmptyResultError: if the returned result of the path query is empty.
     * GearboxPathResultNotValidError: if the returned type of the path query is not supported.
     *
     * @param path The path to the node whose name should be returned.
     * @return The name of the node the path is pointing to.
     */
    virtual std::string getNodeName(const std::string& path = "") const
    throw(GearboxIONotConnectedError, GearboxPathNotValidError, GearboxParamNotExistsError, GearboxPathEmptyResultError,
          GearboxPathResultNotValidError) = 0;

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
    virtual int getNumberNodes(const std::string& path = "") const
    throw(GearboxIONotConnectedError, GearboxPathNotValidError, GearboxPathEmptyResultError, GearboxPathResultNotValidError) = 0;

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
    virtual double getParamLength(const std::string& path = "") const
    throw(GearboxIONotConnectedError, GearboxPathNotValidError, GearboxParamNotExistsError, GearboxPathEmptyResultError,
          GearboxPathResultNotValidError, GearboxStringNumConversionError) = 0;

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
    virtual double getParamAngle(const std::string& path = "") const
    throw(GearboxIONotConnectedError, GearboxPathNotValidError, GearboxParamNotExistsError, GearboxPathEmptyResultError,
          GearboxPathResultNotValidError, GearboxStringNumConversionError) = 0;

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
    virtual double getParamEnergy(const std::string& path = "") const
    throw(GearboxIONotConnectedError, GearboxPathNotValidError, GearboxParamNotExistsError, GearboxPathEmptyResultError,
          GearboxPathResultNotValidError, GearboxStringNumConversionError) = 0;

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
    virtual double getParamDensity(const std::string& path = "") const
    throw(GearboxIONotConnectedError, GearboxPathNotValidError, GearboxParamNotExistsError, GearboxPathEmptyResultError,
          GearboxPathResultNotValidError, GearboxStringNumConversionError) = 0;

    /**
     * Returns a parameter, given by the path, which describes a general floating point numerical value.
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
     * @return The floating point numerical value.
     */
    virtual double getParamNumValue(const std::string& path = "") const
    throw(GearboxIONotConnectedError, GearboxPathNotValidError, GearboxParamNotExistsError, GearboxPathEmptyResultError,
          GearboxPathResultNotValidError, GearboxStringNumConversionError) = 0;

    /**
     * Returns a parameter, given by the path, which describes a general integer numerical value.
     *
     * If the value specified by the path is a floating point value, the value is rounded (like floor()).
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
     * @return The integer numerical value.
     */
    virtual int getParamIntValue(const std::string& path = "") const
    throw(GearboxIONotConnectedError, GearboxPathNotValidError, GearboxParamNotExistsError, GearboxPathEmptyResultError,
          GearboxPathResultNotValidError, GearboxStringNumConversionError) = 0;

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
    virtual std::string getParamString(const std::string& path = "") const
    throw(GearboxIONotConnectedError, GearboxPathNotValidError, GearboxParamNotExistsError, GearboxPathEmptyResultError,
          GearboxPathResultNotValidError) = 0;

  protected:

  private:

  };

} //end of namespace Belle2

#endif /* GEARBOXIOABS_H */
