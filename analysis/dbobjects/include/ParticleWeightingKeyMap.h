/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <map>
#include <vector>
#include <string>
//#include <analysis/dataobjects/Particle.h>
//#include <framework/dataobjects/EventMetaData.h>
//#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  // Just pair of numners - min and max values of bin border
  typedef std::pair<double, double> BinLimits;

  // N-dim bin: pairs of bin limits with name of the axis variable.
  typedef std::map<std::string, BinLimits> NDBin;

  // Map of bin limits with bin ID
  typedef std::map<BinLimits, double> Nameless1DMap;

  // Vector of axis names paired with 1D map.
  // Vector is left for consistency:
  // It doesn't have any sense for 1D, but have sense for 2D and 3D
  typedef std::pair< std::vector<std::string>, Nameless1DMap> Named1DMap;

  // Map of 2D bins with bin ID
  typedef std::map<BinLimits, Nameless1DMap> Nameless2DMap;

  // Vector of axis names paired with 2D map.
  typedef std::pair< std::vector<std::string>, Nameless2DMap> Named2DMap;

  // Map of 3D bins with bin ID
  typedef std::map<BinLimits, Nameless2DMap> Nameless3DMap;

  // Vector of axis names paired with 3D map.
  typedef std::pair< std::vector<std::string>, Nameless3DMap> Named3DMap;

  /**
   * Class for handling LookUp tables
   */
  class ParticleWeightingKeyMap {

    /**
     * 3D key table
     * std::pair< std::vector<std::string>, std::map<std::pair<double, double>, std::map<std::pair<double, double>, std::map<std::pair<double, double>, double>>>>
     */
    Named3DMap m_3DkeyTable;

  public:

    /**
     * Constructor
     */
    ParticleWeightingKeyMap() {};

    /**
    * Binlimits for unused dimensions
    */
    BinLimits unusedBin();

    /**
    * Check if bin in unused bin
    */
    bool isUnusedBin(BinLimits bin);

    /**
     * Adding predefined ID to the table.
     * @param bin contains bin ranges and names of te variables
     * @param key_ID predefined bin ID
     * @return added key ID
     */
    double addKey(NDBin bin, double key_ID);

    /**
     * Adding predefined ID to 1D key table.
     * @param var1_minimax pair of min. and max. values of the observable for the given bin
     * @param key_ID predefined bin ID
     * @return added key ID
     */
    double addKey(BinLimits var1_minimax, double key_ID);

    /**
     * Adding predefined ID to 2D key table.
     * @param var1_minimax pair of min. and max. values of the 1st observable for the given bin
     * @param var2_minimax pair of min. and max. values of the 2nd observable for the given bin
     * @param key_ID predefined bin ID
     * @return added key ID
     */
    double addKey(BinLimits var1_minimax, BinLimits var2_minimax, double key_ID);

    /**
     * Adding predefined ID to 3D key table.
     * @param var1_minimax pair of min. and max. values of the 1st observable for the given bin
     * @param var2_minimax pair of min. and max. values of the 2nd observable for the given bin
     * @param var3_minimax pair of min. and max. values of the 3rd observable for the given bin
     * @param key_ID predefined bin ID
     * @return added key ID
     */
    double addKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax, double key_ID);

    /**
     * Adding bin to the table.
     * @param bin contains bin ranges and names of the variables
     * @return added key ID
     */
    double addKey(NDBin bin);

    /**
     * Adding new entry to 1D key table.
     * @param var1_minimax pair of min. and max. values of the observable for the new bin
     * @return added key ID
     */
    double addKey(BinLimits var1_minimax);

    /**
     * Adding new entry to 1D key table.
     * @param var1_minimax pair of min. and max. values of the 1st observable for the new bin
     * @param var2_minimax pair of min. and max. values of the 2nd observable for the new bin
     * @return added key ID
     */
    double addKey(BinLimits var1_minimax, BinLimits var2_minimax);

    /**
     * Adding new entry to 1D key table.
     * @param var1_minimax pair of min. and max. values of the 1st observable for the new bin
     * @param var2_minimax pair of min. and max. values of the 2nd observable for the new bin
     * @param var3_minimax pair of min. and max. values of the 3rd observable for the new bin
     * @return added key ID
     */
    double addKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax);

    /**
     * Getting key ID from the 1D table
     * @param var1_minimax pair of min. and max. values of the observable of the bin
     * @return key ID
     */
    double getKey(BinLimits var1_minimax);

    /**
     * Getting key ID from the 2D table
     * @param var1_minimax pair of min. and max. values of the 1st observable of the bin
     * @param var2_minimax pair of min. and max. values of the 2nd observable of the bin
     * @return added key ID
     */
    double getKey(BinLimits var1_minimax, BinLimits var2_minimax);

    /**
     * Getting key ID from the 3D table
     * @param var1_minimax pair of min. and max. values of the 1st observable of the bin
     * @param var2_minimax pair of min. and max. values of the 2nd observable of the bin
     * @param var3_minimax pair of min. and max. values of the 3rd observable of the bin
     * @return added key ID
     */
    double getKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax);

    /**
     * Getting key ID from the 1D table for given variable value
     * @param var1_val value of the variable
     * @return key ID
     */
    double getKey(double var1_val);

    /**
     * Getting key ID from the 2D table for given variables value
     * @param var1_val value of the variable 1
     * @param var2_val value of the variable 2
     * @return key ID
     */
    double getKey(double var1_val, double var2_val);

    /**
     * Getting key ID from the 3D table for given variables value
     * @param var1_val value of the variable 1
     * @param var2_val value of the variable 2
     * @param var3_val value of the variable 3
     * @return key ID
     */
    double getKey(double var1_val, double var2_val, double var3_val);

    /**
     * Check if a certain bin is a key for 1D table
     * @param var1_minimax pair of min. and max. values of the 1st observable of the bin
     */
    bool isKey(BinLimits var1_minimax);

    /**
     * Check if a certain bin is a key for 2D table
     * @param var1_minimax pair of min. and max. values of the 1st observable of the bin
     * @param var2_minimax pair of min. and max. values of the 2nd observable of the bin
     */
    bool isKey(BinLimits var1_minimax, BinLimits var2_minimax);

    /**
     * Check if a certain bin is a key for 3D table
     * @param var1_minimax pair of min. and max. values of the 1st observable of the bin
     * @param var2_minimax pair of min. and max. values of the 2nd observable of the bin
     * @param var3_minimax pair of min. and max. values of the 3rd observable of the bin
     */
    bool isKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax);

    /**
     * Return bin info for given bin
     * @param bin key
     * @return bin info
     */
    NDBin getNDBin(double key_ID);

    /**
     * Check if a certain ID is in the table
     * @param key subjected ID
     */
    bool isKey(double key);

    /**
     * @return name of the first variable
     */
    std::string getVar1();

    /**
     * @return name of the second variable
     */
    std::string getVar2();

    /**
     * @return name of the third variable
     */
    std::string getVar3();

    /**
     * Set the first variable
     * @param name name of the variable
     */
    void setVar1(std::string name);

    /**
     * Set the second variable
     * @param name name of the variable
     */
    void setVar2(std::string name);

    /**
     * Set the third variable
     * @param name name of the variable
     */
    void setVar3(std::string name);

    /**
     * Set names from the given bin. Generate Error if trying change predefined names.
     * @param bin bin with names.
     * @return vector of variable names
     */
    std::vector<std::string> addNames(NDBin bin);

    ///**
    // * Get key ID for given particle. If particle is out of range, -1 is returned.
    // * @param particle
    // * @return key ID
    // */
    //double getKey(const Particle* p);

    /**
    * Check if key table is of expected dimensions.
    * Generate error if number of dimensions is wrong.
    * @param ndim number of dimensions
    */
    void dimensionsCheck(int ndim);

    /**
    * Return number of dimesions of te key table.
    * Generate error is more than one key map is defined.
    * @return number of dimensions of the key table.
    */
    int numberOfDimensions();

    /**
    * Return true if ParticleWeightingKeyMap is empty.
    */
    bool isNewParticleWeightingKeyMap();

    /**
    * Return true if ParticleWeightingKeyMap axes are unnamed.
    */
    bool isUnnamed();

    /**
    * Get vector of names from N-dim bin
    */
    std::vector<std::string> getBinNameVector(NDBin bin);

    /**
    * Get vector of names ParticleWeightingKeyMap
    */
    std::vector<std::string> getNames();

    /**
    * Get vector variable names as in variablemanager
    */
    std::vector<std::string> getVarManagerNames();


    /**
    * Check that bin limits are defined correctly (min, max)
    * Issue error if not.
    * @param var_minimax testsed limits
    */
    void checkLimits(BinLimits var_minimax);


  };

} // Belle2 namespace


