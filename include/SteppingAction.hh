#ifndef STEPPING_ACTION_HH
#define STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"

class EventAction;
class G4Step;

class SteppingAction final : public G4UserSteppingAction {
 public:
  explicit SteppingAction(EventAction* eventAction);
  ~SteppingAction() override = default;

  void UserSteppingAction(const G4Step* step) override;

 private:
  G4int ClassifyVolume(const G4String& volumeName) const;

  EventAction* fEventAction = nullptr;
};

#endif
