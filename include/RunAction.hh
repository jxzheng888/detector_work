#ifndef RUN_ACTION_HH
#define RUN_ACTION_HH

#include "G4UserRunAction.hh"
#include "G4String.hh"

class DetectorConstruction;
class G4GenericMessenger;
class G4Run;

class RunAction final : public G4UserRunAction {
 public:
  explicit RunAction(const DetectorConstruction* detector);
  ~RunAction() override;

  void BeginOfRunAction(const G4Run* run) override;
  void EndOfRunAction(const G4Run* run) override;

 private:
  void DefineCommands();
  void BookAnalysis();

  const DetectorConstruction* fDetector = nullptr;
  G4GenericMessenger* fMessenger = nullptr;
  G4String fFileName = "therapy_output";
};

#endif
