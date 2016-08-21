////////////////////////////////////////////////////////////////////////
// Class:       WriteVtkVizData
// Plugin Type: analyzer (art v2_02_01)
// File:        writeVtkVizData_module.cc
//
// Generated at Thu Aug 11 16:42:58 2016 by me using cetskelgen
// from cetlib version v1_19_00.
//
// Analyzer to write out the VtkVizData objects in the event to .vtm files
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#include <boost/format.hpp>

#include "vtkMultiBlockDataSet.h"
#include "vtkSmartPointer.h"
#include "vtkXMLMultiBlockDataWriter.h"

#include "artvtk/artvtk/VtkVizData.hh"


namespace artvtk {
  class WriteVtkVizData;
}

class artvtk::WriteVtkVizData : public art::EDAnalyzer {
public:

  explicit WriteVtkVizData(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  WriteVtkVizData(WriteVtkVizData const &) = delete;
  WriteVtkVizData(WriteVtkVizData &&) = delete;
  WriteVtkVizData & operator = (WriteVtkVizData const &) = delete;
  WriteVtkVizData & operator = (WriteVtkVizData &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

private:

  std::string makeFilename(const art::Event &e) const;

  std::string baseFileName_;
  std::string writeMode_; // ascii, binary, appended
  bool zlibCompression_;  // only for appended
  bool encodeAppended_;   // if true, use base64 encoding (true XML, but slower to read)

};


artvtk::WriteVtkVizData::WriteVtkVizData(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p),
  baseFileName_( p.get<std::string>("baseFileName") ),
  writeMode_( p.get<std::string>("writeMode", "appended")),
  zlibCompression_( p.get<bool>("zlibCompression", true)),
  encodeAppended_( p.get<bool>("encodeAppended", false))
{}

void artvtk::WriteVtkVizData::analyze(art::Event const & e)
{
  // Construct the file name
  auto fileName = makeFilename(e);

  // Make the multiblock dataset to fill
  vtkSmartPointer<vtkMultiBlockDataSet> mb = vtkSmartPointer<vtkMultiBlockDataSet>::New();

  // Get the vtk objects in the event
  std::vector<art::Handle<artvtk::VtkVizData> > vizes;
  e.getManyByType(vizes);

  // We know how many blocks we'll have, so fill it
  mb->SetNumberOfBlocks( vizes.size() );

  unsigned blockno = 0;
  for ( auto& aViz : vizes) {
    aViz->insertIntoMultiBlock(mb, blockno);
    blockno++;
  }

  // Write it out
  vtkSmartPointer<vtkXMLMultiBlockDataWriter> mbw = vtkSmartPointer<vtkXMLMultiBlockDataWriter>::New();
  mbw->SetFileName(fileName.c_str());

  // Determine the data mode
  if ( writeMode_ == "appended" ) {
    mbw->SetDataModeToAppended();
    mbw->SetEncodeAppendedData(encodeAppended_ ? 1 : 0);

    if ( zlibCompression_ ) {
      mbw->SetCompressorTypeToZLib();
    }
    else {
      mbw->SetCompressorTypeToNone();
    }
  }
  else if ( writeMode_ == "ascii" ) {
    mbw->SetDataModeToAscii();
  }
  else if ( writeMode_ == "binary" ) {
    mbw->SetDataModeToBinary();
  }
  else {
    throw cet::exception("WriteVtkVizData") << "Unknown write mode; choose ascii, binary, or appended (default)" << "\n";
  }

  mbw->SetInputData(mb);
  mbw->Write();
}

/// Construct the filename as basename_run_subrun_event
/// \param e The event object
/// \return The filename as a string
std::string artvtk::WriteVtkVizData::makeFilename(const art::Event &e) const {
  // Construct the file name based on run, subrun and event number
  std::stringstream fileNameS;
  auto eid = e.id();
  fileNameS << boost::format("%1$s_r%2$06d_s%3$06d_e%4$06d.vtm") % baseFileName_ % eid.run() % eid.subRun() % eid.event();
  return fileNameS.str();
}

DEFINE_ART_MODULE(artvtk::WriteVtkVizData)
