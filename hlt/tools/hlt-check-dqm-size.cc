/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Basf2 headers. */
#include <framework/logging/Logger.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

/* ROOT headers. */
#include <TClass.h>
#include <TCollection.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TList.h>
#include <TObject.h>
#include <TKey.h>
#include <TSystem.h>

/* C++ headers. */
#include <iostream>
#include <memory>
#include <string>
#include <vector>

/* LZ4 headers. */
#include <lz4.h>

/* ZeroMQ headers. */
#include <zmq.hpp>

namespace {

  /** Stream the histograms. */
  unsigned int streamHistograms(TDirectory* directory, Belle2::MsgHandler& msgHandler, const std::string& directoryName = "")
  {
    TList* keylist{directory->GetListOfKeys()};
    TIter nextkey{keylist};
    TKey* key{nullptr};
    unsigned int counter{0};
    while ((key = dynamic_cast<TKey*>(nextkey()))) {
      TObject* object{directory->Get(key->GetName())};
      TClass* objectClass{object->IsA()};
      std::string objectName{directoryName};
      if (not objectName.empty()) {
        objectName += "/";
      }
      objectName += object->GetName();
      if (objectClass->InheritsFrom(TH1::Class())) {
        TH1* histogram{dynamic_cast<TH1*>(object)};
        msgHandler.add(histogram, objectName);
        counter++;
      } else if (objectClass->InheritsFrom(TDirectory::Class())) {
        TDirectory* subDirectory{dynamic_cast<TDirectory*>(object)};
        // Apparently the dqm server does not understand multi-layer directory structures,
        // therefore the original author broke this down to only show the last directory
        counter += streamHistograms(subDirectory, msgHandler, object->GetName());
      }
    }
    return counter;
  }
}

int main(int argc, char* argv[])
{
  if (argc == 1 or std::string(argv[1]) == "--help" or std::string(argv[1]) == "-h") {
    std::cout << "Usage: " << argv[0] << " INPUT_FILE\n\n"
              "   This tool checks if the online systems can handle the size of the DQM histograms\n"
              "   by compressing and decompressing them with LZ4 as done within our ZeroMQ framework.\n";
    return 1;
  }
  std::string inputFileName{argv[1]};
  if (inputFileName.find(".root") == std::string::npos) {
    B2ERROR("The input file is not a .root file!");
    return 1;
  }
  if (gSystem->AccessPathName(inputFileName.c_str())) {
    B2ERROR("The input file does not exist!");
    return 1;
  }
  std::unique_ptr<TFile> inputFile{
    std::unique_ptr<TFile>(TFile::Open(inputFileName.c_str(), "READ"))
  };
  if (!inputFile or !inputFile->IsOpen() or inputFile->IsZombie()) {
    B2ERROR("The input file is not working!");
    return 1;
  }
  Belle2::MsgHandler msgHandler;
  unsigned int streamedHistograms{streamHistograms(gDirectory, msgHandler)};
  std::unique_ptr<Belle2::EvtMessage> evtMessage{
    std::unique_ptr<Belle2::EvtMessage>(msgHandler.encode_msg(Belle2::ERecordType::MSG_EVENT))
  };
  size_t maximalCompressedSize{100000000};
  std::vector<char> compressedBuffer;
  compressedBuffer.resize(maximalCompressedSize, 0);
  int compressedSize{
    LZ4_compress_default(evtMessage->buffer(), &compressedBuffer[0],
                         evtMessage->size(), maximalCompressedSize)
  };
  if (compressedSize <= 0) {
    B2ERROR("LZ4_compress_default failed"
            << LogVar("file name", inputFileName)
            << LogVar("streamed histograms", streamedHistograms)
            << LogVar("original size", evtMessage->size())
            << LogVar("compressed size", compressedSize));
    inputFile->Close();
    return 1;
  }
  zmq::message_t message{&compressedBuffer[0], static_cast<size_t>(compressedSize)};
  size_t maximalUncompressedSize{128000000};
  std::vector<char> uncompressedBuffer;
  uncompressedBuffer.reserve(maximalUncompressedSize);
  int uncompressedSize{
    LZ4_decompress_safe(message.data<char>(), &uncompressedBuffer[0],
                        message.size(), maximalUncompressedSize)
  };
  if (uncompressedSize <= 0) {
    B2ERROR("LZ4_decompress_safe failed"
            << LogVar("file name", inputFileName)
            << LogVar("streamed histograms", streamedHistograms)
            << LogVar("original size", evtMessage->size())
            << LogVar("compressed size", compressedSize)
            << LogVar("uncompressed size", uncompressedSize));
    inputFile->Close();
    return 1;
  }
  if (evtMessage->size() != uncompressedSize) {
    B2ERROR("Original size and decompressed size differ"
            << LogVar("file name", inputFileName)
            << LogVar("streamed histograms", streamedHistograms)
            << LogVar("original size", evtMessage->size())
            << LogVar("compressed size", compressedSize)
            << LogVar("uncompressed size", uncompressedSize));
    inputFile->Close();
    return 1;
  }
  B2INFO("The compression/decompression cycle with LZ4 is successfully completed"
         << LogVar("file name", inputFileName)
         << LogVar("streamed histograms", streamedHistograms)
         << LogVar("original size", evtMessage->size())
         << LogVar("compressed size", compressedSize)
         << LogVar("uncompressed size", uncompressedSize)
         << LogVar("compressed/uncompressed ratio", (compressedSize * 1.) / uncompressedSize));
  inputFile->Close();
  return 0;
}
