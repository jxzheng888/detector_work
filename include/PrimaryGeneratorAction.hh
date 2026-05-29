#ifndef PRIMARY_GENERATOR_ACTION_HH
#define PRIMARY_GENERATOR_ACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4String.hh"
#include "globals.hh"

class DetectorConstruction;
class G4Event;
class G4GenericMessenger;
class G4ParticleGun;

class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction {
 public:
  explicit PrimaryGeneratorAction(const DetectorConstruction* detector);
  ~PrimaryGeneratorAction() override;

  void GeneratePrimaries(G4Event* event) override;

 private:
  void DefineCommands();

  const DetectorConstruction* fDetector = nullptr;
  G4ParticleGun* fParticleGun = nullptr;
  G4GenericMessenger* fMessenger = nullptr;

  G4String fParticleName = "gamma";
  G4double fEnergy = 6.0;
  G4double fSourceX = 220.0;
  G4double fBeamHalfY = 8.0;
  G4double fBeamHalfZ = 18.0;
  G4double fDirectionX = -1.0;
  G4double fDirectionY = 0.0;
  G4double fDirectionZ = 0.0;
};

#endif
