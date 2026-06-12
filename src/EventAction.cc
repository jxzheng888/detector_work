#include "EventAction.hh"

#include "DetectorConstruction.hh"
#include "SimulationConfig.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"

EventAction::EventAction(const DetectorConstruction* detector)
    : fDetector(detector)
{
  fEdep.fill(0.0);
  fTrackLength.fill(0.0);
}

void EventAction::BeginOfEventAction(const G4Event*)
{
  fEdep.fill(0.0);
  fTrackLength.fill(0.0);
}

void EventAction::AddCellStep(G4int cellId, G4double edep, G4double stepLength)
{
  if (cellId < 0 || cellId >= static_cast<G4int>(fEdep.size())) {
    return;
  }
  fEdep[cellId] += edep;
  fTrackLength[cellId] += stepLength;
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  auto* analysis = G4AnalysisManager::Instance();
  const auto eventId = event->GetEventID();
  const auto meanChord = (4.0 * 5.0 * um) / 3.0;
  const auto modelId = BoronModelId(fDetector->GetBoronModel());
  const auto tumorFraction = fDetector->GetTumorCellFraction();

  std::array<G4double, 3> dose{};
  std::array<G4double, 3> lineal{};

  for (G4int cellId = 0; cellId < 3; ++cellId) {
    const auto mass = fDetector->GetCellMass(cellId);
    dose[cellId] = (mass > 0.0) ? (fEdep[cellId] / mass) / gray : 0.0;
    lineal[cellId] = (meanChord > 0.0) ? (fEdep[cellId] / keV) / (meanChord / um) : 0.0;

    analysis->FillNtupleIColumn(0, 0, eventId);
    analysis->FillNtupleIColumn(0, 1, cellId);
    analysis->FillNtupleDColumn(0, 2, fEdep[cellId] / MeV);
    analysis->FillNtupleDColumn(0, 3, dose[cellId]);
    analysis->FillNtupleDColumn(0, 4, fTrackLength[cellId] / um);
    analysis->FillNtupleDColumn(0, 5, lineal[cellId]);
    analysis->FillNtupleDColumn(0, 6, fDetector->GetBoronPPM());
    analysis->FillNtupleIColumn(0, 7, modelId);
    analysis->FillNtupleDColumn(0, 8, tumorFraction);
    analysis->AddNtupleRow(0);
  }

  const auto normalFraction = 1.0 - tumorFraction;
  const auto mixedEdep =
      tumorFraction * fEdep[kTumorCell] + normalFraction * fEdep[kNormalCellInTumorRegion];
  const auto mixedDose =
      tumorFraction * dose[kTumorCell] + normalFraction * dose[kNormalCellInTumorRegion];
  const auto mixedTrack =
      tumorFraction * fTrackLength[kTumorCell] +
      normalFraction * fTrackLength[kNormalCellInTumorRegion];
  const auto mixedLineal =
      tumorFraction * lineal[kTumorCell] + normalFraction * lineal[kNormalCellInTumorRegion];

  analysis->FillNtupleIColumn(0, 0, eventId);
  analysis->FillNtupleIColumn(0, 1, kTumorRegionMixture);
  analysis->FillNtupleDColumn(0, 2, mixedEdep / MeV);
  analysis->FillNtupleDColumn(0, 3, mixedDose);
  analysis->FillNtupleDColumn(0, 4, mixedTrack / um);
  analysis->FillNtupleDColumn(0, 5, mixedLineal);
  analysis->FillNtupleDColumn(0, 6, fDetector->GetBoronPPM());
  analysis->FillNtupleIColumn(0, 7, modelId);
  analysis->FillNtupleDColumn(0, 8, tumorFraction);
  analysis->AddNtupleRow(0);

  analysis->FillH1(1, dose[kTumorCell]);
  analysis->FillH1(2, dose[kNormalCellInTumorRegion]);
  analysis->FillH1(3, dose[kHealthyNormalCell]);
  analysis->FillH1(4, mixedDose);
}
