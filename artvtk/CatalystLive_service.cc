////////////////////////////////////////////////////////////////////////
// Class:       CatalystLive
// Plugin Type: service (art v2_02_01)
// File:        CatalystLive_service.cc
//
// Generated at Thu Aug 11 21:55:47 2016 by me using cetskelgen
// from cetlib version v1_19_00.
////////////////////////////////////////////////////////////////////////

#include "artvtk/artvtk/CatalystLive.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "cetlib/search_path.h"

#include "vtkCPDataDescription.h"
#include "vtkCPInputDataDescription.h"
#include "vtkCPProcessor.h"
#include "vtkCPPythonScriptPipeline.h"
#include "vtkTable.h"
#include "vtkIntArray.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"

#include <memory>
#include <cstdlib>
#include "messagefacility/MessageLogger/MessageLogger.h"

artvtk::CatalystLive::CatalystLive(fhicl::ParameterSet const & p, art::ActivityRegistry & areg) :
   catalystProcessor_( vtkCPProcessor::New() ),
   dataDescription_(nullptr),
   eventInfo_(nullptr),
   pythonPipelineFileSearchPaths_(p.get<std::string>("pythonPipelineFileSearchPaths", "STANDARD")),
   pythonPipelineFileName_( p.get<std::string>("pythonPipelineFileName", "catalystLiveMBDS_pipeline.py")),
   vizCounter_(0)
{
  areg.sPostBeginJob.watch(&CatalystLive::postBeginJob, *this);
  areg.sPostEndJob.watch(&CatalystLive::postEndJob, *this);
  areg.sPreProcessEvent.watch(&CatalystLive::preProcessEvent, *this);
  areg.sPostProcessEvent.watch(&CatalystLive::postProcessEvent, *this);

  if ( pythonPipelineFileSearchPaths_ == "STANDARD") {

    // Do we have ARTVTK_DIR?
    std::string baseSearch;
    char * artvtkdir = std::getenv("ARTVTK_DIR");
    if ( artvtkdir ) {
      baseSearch = std::string( artvtkdir );
    }
    else {
      artvtkdir = std::getenv("MRB_BUILDDIR");
      if ( ! artvtkdir ) {
        throw cet::exception("CatalystLive_service") << "Neither $ARTVTK_DIR nor $MRB_BUILDDIR are set";
      }
      baseSearch = std::string(artvtkdir) + std::string("/artvtk");
    }

    pythonPipelineFileSearchPaths_ = std::string(".:") + baseSearch + std::string("/pipelines");
  }

  LOG_DEBUG("CatalystLive_service") << "Using pipeline file " << pythonPipelineFileName_ << " from search path "
                                                                                         << pythonPipelineFileSearchPaths_;
}

void artvtk::CatalystLive::postEndJob() {
  catalystProcessor_->Finalize();
  catalystProcessor_->Delete();
}

void artvtk::CatalystLive::postBeginJob() {
  // Initialize the processor
  catalystProcessor_->Initialize();
  vtkCPPythonScriptPipeline* pipeline = vtkCPPythonScriptPipeline::New();

  // Determine the path to the python pipeline file
  LOG_INFO("CatalylstLive_service") << "Searching for " << pythonPipelineFileName_ << " in paths " << pythonPipelineFileSearchPaths_;
  cet::search_path searchPath { pythonPipelineFileSearchPaths_ };
  std::string pipelinePath = searchPath.find_file( pythonPipelineFileName_ );
  LOG_INFO("CatalylstLive_service") << "Found pipeline file " << pipelinePath;

  // Initialize the pipeline
  int ok = pipeline->Initialize(pipelinePath.c_str());
  if (! ok) {
    throw cet::exception("CatalystLive_service") << "Cannot find python pipeline file " << pipelinePath << ".\n";
  }

  // Add it to the processor
  catalystProcessor_->AddPipeline(pipeline);
}

bool artvtk::CatalystLive::doWeVizQuestionMark() {
  return catalystProcessor_->RequestDataDescription( dataDescription_ ) != 0;
}

void artvtk::CatalystLive::preProcessEvent(art::Event const &e) {

  // Make the event info table
  makeEventInfo(e);

  // Add it to the data
  dataDescription_ = vtkCPDataDescription::New();
  dataDescription_->AddInput("eventInfo");
  dataDescription_->AddInput("vizes");

  dataDescription_->SetTimeData(e.event(), vizCounter_);
}

void artvtk::CatalystLive::makeEventInfo(art::Event const &e) {
  eventInfo_ = vtkTable::New();

  vtkSmartPointer<vtkIntArray> eventNum = vtkSmartPointer<vtkIntArray>::New();
  eventNum->SetName("eventNum");
  eventNum->InsertNextValue(e.event());

  vtkSmartPointer<vtkIntArray> vizCounter = vtkSmartPointer<vtkIntArray>::New();
  vizCounter->SetName("vizCounter");
  vizCounter->InsertNextValue(vizCounter_);

  vtkSmartPointer<vtkIntArray> subRunNum = vtkSmartPointer<vtkIntArray>::New();
  subRunNum->SetName("subRunNum");
  subRunNum->InsertNextValue(e.subRun());

  vtkSmartPointer<vtkIntArray> runNum = vtkSmartPointer<vtkIntArray>::New();
  runNum->SetName("runNum");
  runNum->InsertNextValue(e.run());

  // Fill the table
  eventInfo_->AddColumn(runNum);
  eventInfo_->AddColumn(subRunNum);
  eventInfo_->AddColumn(eventNum);
  eventInfo_->AddColumn(vizCounter);
}

void artvtk::CatalystLive::postProcessEvent(art::Event const &e) {
  vizCounter_++;
  eventInfo_->Delete();
  dataDescription_->Delete();
}

void artvtk::CatalystLive::addVizesData( vtkDataObject* vtkData ) {
  dataDescription_->GetInputDescriptionByName("vizes")->SetGrid( vtkData );
}

void artvtk::CatalystLive::vizit() {
  // Add the event information table
  dataDescription_->GetInputDescriptionByName("eventInfo")->SetGrid( eventInfo_ );

  // Go!
  catalystProcessor_->CoProcess(dataDescription_);
}


DEFINE_ART_SERVICE(artvtk::CatalystLive)
