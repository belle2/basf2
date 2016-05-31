/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_WEIGHTFILE_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_WEIGHTFILE_HEADER

#include <mva/interface/Options.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TObject.h>
#include <TGenericClassInfo.h>

#include <boost/property_tree/ptree.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cerrno>

namespace Belle2 {

  namespace MVA {

    /**
     * The Weightfile class serializes all information about a training into an xml tree.
     * It can save and load this information from: a stream, a file, the database
     */
    class Weightfile {

    public:
      /**
       * Construct an empty weightfile
       */
      Weightfile() {};

      /**
       * Destructor (removes temporary files associated with this weightfiles)
       */
      ~Weightfile();

      /**
       * Add an Option object to the xml tree
       * @param options to add
       */
      void addOptions(const Options& options);

      /**
       * Fills an Option object from the xml tree
       * @param options
       */
      void getOptions(Options& options) const;

      /**
       * Saves the signal fraction in the xml tree
       * @param signal_fraction
       */
      void addSignalFraction(float signal_fraction);

      /**
       * Loads the signal fraction frm the xml tree
       * @return
       */
      float getSignalFraction() const;

      /**
       * Returns a temporary filename with the given suffix.
       * All temporary files are removed when the Weightfile class is destroyed
       * @param suffix of the temporary filename
       * @return
       */
      std::string getFileName(const std::string& suffix = "");

      /**
       * Add a file (mostly a weightfile from a MVA library) to our Weightfile
       * @param identifier of this weightfile
       * @param custom_weightfile the filename
       */
      void addFile(const std::string& identifier, const std::string& custom_weightfile);

      /**
       * Add a stream to our weightfile
       * @param identifier of the stream
       * @param in the stream
       */
      void addStream(const std::string& identifier, std::istream& in);

      /**
       * Add an element to the xml tree
       * @param identifier identifier of the element
       * @param element the value
       */
      template<class T>
      void addElement(const std::string& identifier, const T& element)
      {
        m_pt.put(identifier, element);
      }

      /**
       * Creates a file from our weightfile (mostly this will be a weightfile of an MVA library)
       * @param identifier of the file
       * @param custom_weightfile the filename which is created
       */
      void getFile(const std::string& identifier, const std::string& custom_weightfile);

      /**
       * Returns the content of a stored stream as string
       * @param identifier of the stored stream
       */
      std::string getStream(const std::string& identifier) const;

      /**
       * Returns a stored element from the xml tree
       * @param identifier of the stored element
       */
      template<class T>
      T getElement(const std::string& identifier) const
      {
        return m_pt.get<T>(identifier);
      }

      /**
       * Static function which saves a Weightfile to a file
       * @param weightfile to store
       * @param filename of the file or identifier of the database
       */
      static void save(Weightfile& weightfile, const std::string& filename,
                       const Belle2::IntervalOfValidity& iov = Belle2::IntervalOfValidity(0, 0, -1, -1));

      /**
       * Static function which saves a Weightfile to a ROOT file
       * @param weightfile to store
       * @param filename of the file
       */
      static void saveToROOTFile(Weightfile& weightfile, const std::string& filename);

      /**
       * Static function which saves a Weightfile to a XML file
       * @param weightfile to store
       * @param filename of the file
       */
      static void saveToXMLFile(Weightfile& weightfile, const std::string& filename);

      /**
       * Static function which serializes a Weightfile to a stream
       * @param weightfile to serialize
       * @param stream
       */
      static void saveToStream(Weightfile& weightfile, std::ostream& stream);

      /**
       * Static function which loads a Weightfile from a file
       * @param filename of the file containing our serialized Weightfile
       */
      static Weightfile load(const std::string& filename, const Belle2::EventMetaData& emd = Belle2::EventMetaData(0, 0, 0));

      /**
       * Static function which loads a Weightfile from a ROOT file
       * @param filename of the file containing our serialized Weightfile
       */
      static Weightfile loadFromROOTFile(const std::string& filename);

      /**
       * Static function which loads a Weightfile from a XML file
       * @param filename of the file containing our serialized Weightfile
       */
      static Weightfile loadFromXMLFile(const std::string& filename);

      /**
       * Static function which deserializes a Weightfile from a stream
       * @param stream containing our serialized Weightfile
       */
      static Weightfile loadFromStream(std::istream& stream);

      /**
       * Static function which saves a Weightfile in the basf2 condition database
       * @param weightfile to save
       * @param identifier in the database
       * @param iov interval of validity of this weightfile
       */
      static void saveToDatabase(Weightfile& weightfile, const std::string& identifier,
                                 const Belle2::IntervalOfValidity& iov = Belle2::IntervalOfValidity(0, 0, -1, -1));

      /**
       * Static function which loads a Weightfile from the basf2 condition database
       * @param identifier in the database
       * @param emd event meta data containing the current valid event which is checked against the interval of validity of the stored weightfiles.
       */
      static Weightfile loadFromDatabase(const std::string& identifier, const Belle2::EventMetaData& emd = Belle2::EventMetaData(0, 0,
                                         0));

    private:
      boost::property_tree::ptree m_pt; /**< xml tree containing all the saved information of this weightfile */
      std::vector<std::string> m_filenames; /**< generated temporary filenames, which will be removed in the destructor of this class */
    };

  }
}
#endif
