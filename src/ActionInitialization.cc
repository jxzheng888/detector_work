#include "ActionInitialization.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"

ActionInitialization::ActionInitialization(const DetectorConstruction* detector)
    : fDetector(detector)
{
}

void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction(fDetector));
  SetUserAction(new RunAction(fDetector));

  auto* eventAction = new EventAction(fDetector);
  SetUserAction(eventAction);
  SetUserAction(new SteppingAction(eventAction));
}
