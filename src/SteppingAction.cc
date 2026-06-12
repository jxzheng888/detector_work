#include "SteppingAction.hh"

#include "EventAction.hh"
#include "SimulationConfig.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"

SteppingAction::SteppingAction(EventAction* eventAction)
    : fEventAction(eventAction)
{
}

G4int SteppingAction::ClassifyVolume(const G4String& volumeName) const
{
  if (volumeName == "TumorCellLV" ||
      volumeName == "TumorCellShellLV" ||
      volumeName == "TumorCellCoreLV") {
    return kTumorCell;
  }
  if (volumeName == "TumorRegionNormalCellLV") {
    return kNormalCellInTumorRegion;
  }
  if (volumeName == "HealthyNormalCellLV") {
    return kHealthyNormalCell;
  }
  return -1;
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  const auto* prePoint = step->GetPreStepPoint();
  const auto* volume = prePoint->GetTouchableHandle()->GetVolume();
  if (!volume) {
    return;
  }

  const auto* logical = volume->GetLogicalVolume();
  const auto cellId = ClassifyVolume(logical->GetName());
  if (cellId < 0) {
    return;
  }

  const auto edep = step->GetTotalEnergyDeposit();
  const auto stepLength = step->GetStepLength();
  if (edep <= 0.0) {
    return;
  }

  fEventAction->AddCellStep(cellId, edep, stepLength);

  const auto let = (stepLength > 0.0) ? (edep / keV) / (stepLength / um) : 0.0;
  auto* analysis = G4AnalysisManager::Instance();
  analysis->FillH1(5, let);

  const auto* track = step->GetTrack();
  const auto pdg = track->GetParticleDefinition()->GetPDGEncoding();
  const auto eventId = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

  analysis->FillNtupleIColumn(1, 0, eventId);
  analysis->FillNtupleIColumn(1, 1, cellId);
  analysis->FillNtupleIColumn(1, 2, pdg);
  analysis->FillNtupleDColumn(1, 3, edep / keV);
  analysis->FillNtupleDColumn(1, 4, stepLength / um);
  analysis->FillNtupleDColumn(1, 5, let);
  analysis->FillNtupleDColumn(1, 6, prePoint->GetKineticEnergy() / MeV);
  analysis->AddNtupleRow(1);
}
