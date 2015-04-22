/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Lipp                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/TMVATeacher/TMVASPlotTeacherModule.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/logging/Logger.h>

#include <RooFit.h>
#include <RooStats/SPlot.h>
#include <RooDataSet.h>
#include <RooArgList.h>
#include <RooArgSet.h>
#include <RooGaussian.h>
#include <RooExponential.h>
#include <RooChebychev.h>
#include <RooProdPdf.h>
#include <RooAbsPdf.h>
#include <RooAddPdf.h>
#include <RooRealVar.h>
#include <RooAbsArg.h>
#include <RooConstVar.h>
#include <RooWorkspace.h>
#include <Rtypes.h>

#include <TIterator.h>
#include <TROOT.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TImage.h>
#include <TAxis.h>


namespace Belle2 {


  REG_MODULE(TMVASPlotTeacher)

  TMVASPlotTeacherModule::TMVASPlotTeacherModule() : Module(), m_discriminating_values("", DataStore::c_Persistent)
  {
    setDescription("Trains TMVA method with given particle lists as training samples. "
                   "The target variable has to be an integer valued variable which defines the clusters in the sample. "
                   "e.g. isSignal for signal and background cluster or abs_PDG to define different MC-PDGs as clusters. "
                   "The clusters are trained against each other. "
                   "See also https://belle2.cc.kek.jp/~twiki/bin/view/Software/TMVA for detailed instructions.");

    setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

    std::vector<std::string> empty;
    addParam("listNames", m_listNames,
             "Particles from these ParticleLists are used as input. If no name is given the teacher is applied to every event once, and one can only use variables which accept nullptr as Particle*",
             empty);
    addParam("methods", m_methods,
             "Vector of Tuples with (Name, Type, Config) of the methods. Valid types are: BDT, KNN, Fisher, Plugin. For type 'Plugin', the plugin matching the Name attribute will be loaded (e.g. NeuroBayes). The Config is passed to the TMVA Method and is documented in the TMVA UserGuide.");
    addParam("prefix", m_methodPrefix,
             "Prefix which is used by the TMVAInterface to store its configfile $prefix.config and by TMVA itself to write the files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml with additional information",
             std::string("TMVA"));
    addParam("workingDirectory", m_workingDirectory,
             "Working directory in which the config file and the weight file directory is created", std::string("."));
    addParam("variables", m_variables, "Input variables used by the TMVA method");
    addParam("spectators", m_spectators,
             "Input spectators used by the TMVA method. These variables are saved in the output file, but not used as training input.", empty);
    addParam("factoryOption", m_factoryOption, "Option passed to TMVA::Factory",
             std::string("!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification"));
    addParam("prepareOption", m_prepareOption, "Option passed to TMVA::Factory::PrepareTrainingAndTestTree",
             std::string("SplitMode=random:!V"));
    addParam("createMVAPDFs", m_createMVAPDFs,
             "Creates the MVA PDFs for signal and background. This is needed to transform the output of the trained method to a probability.",
             true);
    addParam("useExistingData", m_useExistingData, "Use existing data which is already stored in the $prefix.root file.", false);
    addParam("doNotTrain", m_doNotTrain,
             "Do not train, create only datafile with samples. Useful if you want to train outside of basf2 with the externTeacher tool.",
             false);
    addParam("doNotSPlot", m_doNotSPlot,
             "Do not run SPlot to calculate weights. Useful for debugging, for example in combination with doNotTrain to see how the data looks.",
             false);
    addParam("maxEventsPerClass", m_maxEventsPerClass, "Maximum number of events per class passed to TMVA. 0 means no limit.", 0u);
    addParam("discriminatingVariables", m_discriminatingVariables,
             "The discriminating variables used by sPlot to determine the weights.");
    addParam("modelFileName", m_modelFileName,
             "Path to the Root file containing the model which describes signal and background in the discriminating variable. This file will only be opened readonly.");
    addParam("modelObjectName", m_modelObjectName, "Name of the RooAbsPdf object in the Root file with path modelFileName.",
             std::string("model"));
    addParam("modelPlotComponentNames", m_modelPlotComponentNames,
             "Name of RooAbsPdf objects that are part of the model and should be plotted additionally.", empty);
    addParam("modelYieldsObjectNames", m_modelYieldsObjectNames,
             "Name of the RooRealVar objects that represent the yields of the event classes in the Root file with path modelFileName. There have to be as much yields as classes"
             "-- the number of yields is interpreted as the number of classes. In the case of two classes, the yield for the signal class has to be the second, because the largest classid is signal in TMVA.");
    addParam("setYieldRanges", m_setYieldRanges,
             "If True, the maximum value of the yield variables given with m_modelYieldsObjectNames will be set to the number of events. The minimum value will not be touched.",
             true);
    addParam("modelYieldsInitialFractions", m_modelYieldsInitialFractions,
             "If set, the initial value of yield i will be set to initialFraction[i]*numberOfEvents. If there are fewer initial fractions than yields, the values of the other yields will not be touched",
             std::vector<double>());
  }

  TMVASPlotTeacherModule::~TMVASPlotTeacherModule()
  {
  }

  void TMVASPlotTeacherModule::initialize()
  {
    for (auto& name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);
    }

    std::vector<TMVAInterface::Method> methods;
    for (auto& x : m_methods) {
      std::string config = std::get<2>(x);
      if (m_createMVAPDFs)
        config = std::string("CreateMVAPdfs:") + config;
      methods.push_back(TMVAInterface::Method(std::get<0>(x), std::get<1>(x), config, m_variables, m_spectators));
    }
    m_teacher = std::make_shared<TMVAInterface::Teacher>(m_methodPrefix, m_workingDirectory, "isSignal", "constant(1)", methods,
                                                         m_useExistingData);

    // Create a temporary workspace to manage the model and its variables
    auto workspace = getWorkspace();

    // Check that the yields are all RooRealVars
    // and are present in the model.
    // m_yields will be built in terminate()
    for (auto& yield : m_modelYieldsObjectNames) {
      // Check that yields are used in the model
      if (!this->isVariableInModel(yield, workspace)) {
        B2FATAL("TMVASPlotTeacher: Yield " << yield <<
                " is not at all used in the model. Thus, SPlot won't work. Please fix the model or remove the variable from the list of yield variables.");
      }
      // Check that the yields are RooRealVars
      if (!dynamic_cast<RooRealVar*>(workspace->var(yield.c_str()))) {
        B2FATAL("TMVASPlotTeacher: Yield " << yield << " in the model is not of type RooRealVar.");
      }
    }
    // Write number of yields to m_numberOfClasses
    m_numberOfClasses = m_modelYieldsObjectNames.size();

    // Prepare the discriminating variables and check that each of them is used inside the model.
    Variable::Manager& manager = Variable::Manager::Instance();
    RooArgSet vars;
    for (auto& variable : m_discriminatingVariables) {
      if (!this->isVariableInModel(variable, workspace)) {
        B2FATAL("TMVASPlotTeacher: Discriminating variable " << variable <<
                " is not at all used in the model. Thus, SPlot won't work. Please fix the model or remove the variable from the list of discriminating variables.");
      }
      m_discriminating_vars[variable] = manager.getVariable(variable);
      // TODO: save these pointers so the objects can be destroyed at some point, maybe the destructor of TMVASPlotTeacherModule.
      // Copy the RooRealVar, as the workspace gets destroyed at the end of initialize().
      // As the copy constructor needs a name, give it the same name.
      // The reason for extracting the variable from the model is, that this way,
      // the range and binning is just as the user set it.
      RooRealVar* variableInModel = new RooRealVar(*workspace->var(variable.c_str()), variable.c_str());
      vars.add(*variableInModel);
    }

    // Construct the header of the data set.
    m_discriminating_values.registerInDataStore(m_methodPrefix + "discriminating_values",
                                                DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
    m_discriminating_values.construct("discriminating_values", "discriminating_values", vars);

  }

  std::shared_ptr<RooWorkspace> TMVASPlotTeacherModule::getWorkspace()
  {

    auto workspace = std::make_shared<RooWorkspace>("SPlotWorkspace");
    // Get the model from the user
    TFile modelFile(m_modelFileName.c_str());
    // Check if it's derived from RooAbsPdf
    RooAbsPdf* model = dynamic_cast<RooAbsPdf*>(modelFile.Get(m_modelObjectName.c_str()));
    if (!model) {
      B2FATAL("TMVASPlotTeacher: The object " << m_modelObjectName << " is either not present in the file " << m_modelFileName <<
              " or not derived from RooAbsPdf.");
    }
    workspace->import(*model);
    return workspace;

  }

  bool TMVASPlotTeacherModule::isVariableInModel(const std::string variable, std::shared_ptr<RooWorkspace> wspace)
  {
    RooArgSet variables = wspace->allVars();
    TIterator* iter = variables.createIterator();
    RooAbsArg* arg;
    bool found = false;
    // static_cast works, because allVars returns a set of objects of a type that is derived from RooRealVar
    while ((arg = static_cast<RooRealVar*>(iter->Next()))) {
      if (std::string(arg->GetName()) == variable) {
        found = true;
        break;
      }
    }
    delete iter;
    return found;
  }

  void TMVASPlotTeacherModule::terminate()
  {
    if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {

      UInt_t numberOfEvents;
      if (m_discriminating_values->get().numEntries() < 0) {
        B2FATAL("TMVASPlotTeacher: Number of events is less than zero (" << m_discriminating_values->get().numEntries() << ")");
      } else {
        numberOfEvents = static_cast<UInt_t>(m_discriminating_values->get().numEntries());
      }
      UInt_t numberOfEntries = m_numberOfClasses * numberOfEvents;
      std::cout << "SPlot: number of events: " << numberOfEvents << ", number of entries: " << numberOfEntries << std::endl;

      // Create a new workspace to manage the project.
      auto workspace = getWorkspace();

      // The RooWorkspace will copy this RooDataSet. We grab a pointer to it immediately, to manipulate it more easily later
      workspace->import(m_discriminating_values->get());

      RooAddPdf* model = static_cast<RooAddPdf*>(workspace->pdf(m_modelObjectName.c_str()));
      RooDataSet* discriminating_values = static_cast<RooDataSet*>(workspace->data("discriminating_values"));

      // set yield maximum values
      if (m_setYieldRanges) {
        for (auto& yield : m_modelYieldsObjectNames) {
          std::cout << "TMVASPlotTeacher: Set maximum of yield variable " << yield << " to " << numberOfEvents << " (number of events)." <<
                    std::endl;
          workspace->var(yield.c_str())->setMax(numberOfEvents);
        }
      }

      // set yield initial values
      for (unsigned int i = 0; i < m_modelYieldsInitialFractions.size(); i++) {
        double initialValue = m_modelYieldsInitialFractions[i] * numberOfEvents;
        std::cout << "TMVASPlotTeacher: Set initial value of yield variable " << m_modelYieldsObjectNames[i] << " to " <<
                  m_modelYieldsInitialFractions[i] << "*" << numberOfEvents << " = " << initialValue << "." << std::endl;
        workspace->var(m_modelYieldsObjectNames[i].c_str())->setVal(initialValue);
      }

      // Build a RooArgList of the yields.
      // Build it here so the maximum and the initial value are already set.
      // It was already checked in initialize(), that they are all RooRealVars
      // and are present in the model.
      auto yields = new RooArgList("yields");
      for (auto& yield : m_modelYieldsObjectNames) {
        RooRealVar* variableInModel = static_cast<RooRealVar*>(workspace->var(yield.c_str()));
        yields->add(*variableInModel);
      }

      // fit the model to the data.
      std::cout << "SPlot: Fit data to model" << std::endl;
      model->Print("t");
      model->fitTo(*discriminating_values, RooFit::Extended());

      // Plot the fit if there is only one dimension
      if (m_discriminatingVariables.size() == 1) {
        RooRealVar* discriminating_variable = workspace->var(m_discriminatingVariables[0].c_str());
        std::cout << "SPlot: Generating plot for discriminating variable " << m_discriminatingVariables[0].c_str() << "." << std::endl;

        // Set ROOT batch mode to true, so it does not show the GUI
        // while the fit is plotted.
        bool batchMode = gROOT->IsBatch();
        bool batchModeChanged = false;
        if (batchMode == kFALSE) {
          std::cout << "TMVASPlotTeacher: ROOT batch mode is set to kFALSE, setting to kTRUE." << std::endl;
          gROOT->SetBatch(kTRUE);
          batchModeChanged = true;
        }

        RooPlot* xframe = discriminating_variable->frame(RooFit::Title("Fit of provided model"));
        auto canvas = new TCanvas("splot_fit", "splot_fit");

        discriminating_values->plotOn(xframe);
        model->plotOn(xframe);
        // Plot components of the model
        for (auto& component : m_modelPlotComponentNames) {
          std::cout << "TMVASPlotTeacher: Plotting model component " << component << " in addition to the model." << std::endl;
          model->plotOn(xframe, RooFit::Components(*(workspace->pdf(component.c_str()))), RooFit::LineStyle(kDashed)) ;
        }
        xframe->Draw();

        std::string pngFileName = m_methodPrefix + "_pre_splot_fit.png";
        TImage* img = TImage::Create();
        img->FromPad(canvas);
        img->WriteImage(pngFileName.c_str());

        // Reset ROOT batch mode to the previous value.
        if (batchModeChanged) {
          gROOT->SetBatch(batchMode);
          std::cout << "TMVASPlotTeacher: Setting ROOT batch mode back to kFALSE" << std::endl;
        }
      } else {
        std::cout << "SPlot: Skip generating plot, because there is more than one discriminating variable." << std::endl;
      } // End of plot

      std::cout << "SPlot: number of entries: " << discriminating_values->numEntries() << std::endl;

      //bool silentModeBefore = RooMsgService::instance().silentMode();
      //RooMsgService::instance().setSilentMode(true);

      if (not m_doNotSPlot) {
        std::cout << "SPlot: call SPlot" << std::endl;
        RooStats::SPlot* sData = new RooStats::SPlot("sData", "BASF2 SPlotTeacher", *discriminating_values, model, *yields);

        // This is used in the RooStat::SPlot tutorial.
        //std::cout << "SPlot: reset verbose status of RooMsgService" << std::endl;
        //RooMsgService::instance().setSilentMode(silentModeBefore);

        RooArgList sWeightedVars = sData->GetSWeightVars();
        std::cout << "For the following " << sWeightedVars.getSize() << " classes sWeights were calculated: ";
        for (Int_t i = 0; i < sWeightedVars.getSize(); i++) {
          std::cout << sWeightedVars.at(i)->GetName();
          if (i != sWeightedVars.getSize() - 1) {
            std::cout << ", ";
          }
        }
        std::cout << "." << std::endl;

        if (sWeightedVars.getSize() != static_cast<int>(m_numberOfClasses)) {
          B2FATAL("TMVASPlotTeacher: The SPlot algorithm did not succeed, the number of classes for which sWeights were calculated doesn't match with the number of classes.");
        }

        std::vector<float> weights(numberOfEntries);
        // For each event, a weight needs to be retrieved for each class
        for (UInt_t i = 0; i < numberOfEvents; i++) {
          for (UInt_t j = 0; j < m_numberOfClasses; j++) {
            // Get the name of the j-th coefficient from the model
            weights[2 * i + j] = sData->GetSWeight(i, yields->at(j)->GetName());
          }
        }

        m_teacher->setVariable("__weight__", weights);

      } else {
        std::cout << "TMVASPlotTeacher: SPlot skipped." << std::endl;
      }

      if (not m_doNotTrain) {
        m_teacher->train(m_factoryOption, m_prepareOption, m_maxEventsPerClass);
      }
      m_teacher->writeTree();
    }
    m_teacher.reset();
  }

  void TMVASPlotTeacherModule::event()
  {

    for (auto& listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate Signal Probability for Particles
      for (unsigned i = 0; i < list->getListSize(); ++i) {
        const Particle* particle = list->getParticle(i);

        // for each discriminating variable, remember the values, as this information is needed within the sPlot algorithm
        RooArgSet row;
        for (auto& variable : m_discriminatingVariables) {
          // TODO: memory leak, see above
          // If these RooRealVar objects are not allocated on the heap, they get destroyed at the end of the scope of this loop pass, and the data is lost, and appears as 0.
          // TODO: Investigate: Only the last variable would survive. Why!? What would SPlot do?
          RooRealVar* v = new RooRealVar(variable.c_str(), variable.c_str(), m_discriminating_vars[variable]->function(particle));
          row.add(*v);
          //RooRealVar v = RooRealVar(variable.c_str(), variable.c_str(), m_discriminating_vars[variable]->function(particle));
          //row.add(v);
        }
        m_discriminating_values->get().add(row);

        // TODO: unsigned int?
        for (unsigned int i = 0; i < m_numberOfClasses; i++) {
          m_teacher->addClassSample(particle, i);
        }
      }
    }

    if (m_listNames.empty()) {
      RooArgSet row;
      for (auto& variable : m_discriminatingVariables) {
        // TODO: memory leak, see initialize()
        RooRealVar* v = new RooRealVar(variable.c_str(), variable.c_str(), m_discriminating_vars[variable]->function(nullptr));
        row.add(*v);
        //RooRealVar v = RooRealVar(variable.c_str(), variable.c_str(), m_discriminating_vars[variable]->function(nullptr));
        //row.add(v);
      }
      m_discriminating_values->get().add(row);

      // TODO: unsigned int?
      for (unsigned int i = 0; i < m_numberOfClasses; i++) {
        m_teacher->addClassSample(nullptr, i);
      }
    }
  }

} // Belle2 namespace

