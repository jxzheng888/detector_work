#ifndef EVENT_ACTION_HH
#define EVENT_ACTION_HH

#include "G4UserEventAction.hh"
#include "globals.hh"

#include <array>

class DetectorConstruction;
class G4Event;

class EventAction final : public G4UserEventAction {
 public:
  explicit EventAction(const DetectorConstruction* detector);
  ~EventAction() override = default;

  void BeginOfEventAction(const G4Event* event) override;
  void EndOfEventAction(const G4Event* event) override;

  void AddCellStep(G4int cellId, G4double edep, G4double stepLength);

 private:
  const DetectorConstruction* fDetector = nullptr;
  std::array<G4double, 3> fEdep{};
  std::array<G4double, 3> fTrackLength{};
};

#endif
