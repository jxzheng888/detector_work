#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"

#include "G4PhysListFactory.hh"
#include "G4RunManager.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4Exception.hh"

int main(int argc, char** argv)
{
  auto* runManager = new G4RunManager;

  auto* detector = new DetectorConstruction;
  runManager->SetUserInitialization(detector);

  G4String physicsName = "QGSP_BIC_HP";
  if (argc > 2) {
    physicsName = argv[2];
  }

  G4PhysListFactory physicsFactory;
  auto* physicsList = physicsFactory.GetReferencePhysList(physicsName);
  if (!physicsList) {
    G4ExceptionDescription msg;
    msg << "Unknown Geant4 reference physics list '" << physicsName << "'.";
    G4Exception("main", "Therapy002", FatalException, msg);
  }
  physicsList->RegisterPhysics(new G4StepLimiterPhysics);
  runManager->SetUserInitialization(physicsList);

  runManager->SetUserInitialization(new ActionInitialization(detector));

  auto* visManager = new G4VisExecutive;
  visManager->Initialize();

  auto* uiManager = G4UImanager::GetUIpointer();
  if (argc == 1) {
    auto* ui = new G4UIExecutive(argc, argv);
    uiManager->ApplyCommand("/control/execute macros/vis.mac");
    ui->SessionStart();
    delete ui;
  } else {
    const G4String command = "/control/execute ";
    uiManager->ApplyCommand(command + argv[1]);
  }

  delete visManager;
  delete runManager;
  return 0;
}
