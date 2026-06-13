#include "RunAction.hh"

#include "DetectorConstruction.hh"

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"

RunAction::RunAction(const DetectorConstruction* detector)
    : fDetector(detector)
{
  DefineCommands();
  BookAnalysis();
}

RunAction::~RunAction()
{
  delete fMessenger;
}

void RunAction::DefineCommands()
{
  fMessenger = new G4GenericMessenger(this, "/therapy/output/",
                                      "Output controls");
  fMessenger->DeclareProperty("fileName", fFileName,
                              "Output ROOT file name, including .root extension.");
}

void RunAction::BookAnalysis()
{
  auto* analysis = G4AnalysisManager::Instance();
  analysis->SetVerboseLevel(1);
  analysis->SetFirstHistoId(1);
  analysis->SetDefaultFileType("root");
  analysis->SetFileName(fFileName);

  analysis->CreateH1("tumor_cell_dose", "Tumor cell dose per event;dose (Gy);events",
                     200, 0.0, 5.0e-3);
  analysis->CreateH1("normal_in_region_dose",
                     "Normal cell in tumor region dose per event;dose (Gy);events",
                     200, 0.0, 5.0e-3);
  analysis->CreateH1("healthy_normal_dose",
                     "Healthy normal cell dose per event;dose (Gy);events",
                     200, 0.0, 5.0e-3);
  analysis->CreateH1("tumor_region_mixed_dose",
                     "Cell-fraction weighted tumor-region dose per event;dose (Gy);events",
                     200, 0.0, 5.0e-3);
  analysis->CreateH1("step_let", "Step LET proxy in scored cells;keV/um;steps",
                     200, 0.0, 1000.0);

  analysis->CreateNtuple("events", "Per-event cell-level scores");
  analysis->CreateNtupleIColumn("event_id");
  analysis->CreateNtupleIColumn("cell_id");
  analysis->CreateNtupleDColumn("edep_MeV");
  analysis->CreateNtupleDColumn("dose_Gy");
  analysis->CreateNtupleDColumn("track_um");
  analysis->CreateNtupleDColumn("lineal_keV_um");
  analysis->CreateNtupleDColumn("boron_ppm");
  analysis->CreateNtupleIColumn("boron_model_id");
  analysis->CreateNtupleDColumn("tumor_cell_fraction");
  analysis->FinishNtuple();

  analysis->CreateNtuple("steps", "Energy-depositing steps in scored cells");
  analysis->CreateNtupleIColumn("event_id");
  analysis->CreateNtupleIColumn("cell_id");
  analysis->CreateNtupleIColumn("pdg");
  analysis->CreateNtupleDColumn("edep_keV");
  analysis->CreateNtupleDColumn("step_um");
  analysis->CreateNtupleDColumn("let_keV_um");
  analysis->CreateNtupleDColumn("kinetic_MeV");
  analysis->FinishNtuple();
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  auto* analysis = G4AnalysisManager::Instance();
  analysis->SetFileName(fFileName);
  analysis->OpenFile();
}

void RunAction::EndOfRunAction(const G4Run*)
{
  auto* analysis = G4AnalysisManager::Instance();
  analysis->Write();
  analysis->CloseFile();
}
