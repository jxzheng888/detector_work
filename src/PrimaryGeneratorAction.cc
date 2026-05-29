#include "PrimaryGeneratorAction.hh"

#include "DetectorConstruction.hh"

#include "G4Event.hh"
#include "G4Exception.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction(const DetectorConstruction* detector)
    : fDetector(detector), fParticleGun(new G4ParticleGun(1))
{
  DefineCommands();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
  delete fMessenger;
}

void PrimaryGeneratorAction::DefineCommands()
{
  fMessenger = new G4GenericMessenger(this, "/therapy/source/",
                                      "Therapy beam controls");
  fMessenger->DeclareProperty("particle", fParticleName,
                              "Primary particle name: gamma, proton, neutron.");
  fMessenger->DeclarePropertyWithUnit("energy", "MeV", fEnergy,
                                      "Primary kinetic energy.");
  fMessenger->DeclarePropertyWithUnit("sourceX", "cm", fSourceX,
                                      "Source x position in world coordinates.");
  fMessenger->DeclarePropertyWithUnit("beamHalfY", "cm", fBeamHalfY,
                                      "Half-size of the rectangular field in y.");
  fMessenger->DeclarePropertyWithUnit("beamHalfZ", "cm", fBeamHalfZ,
                                      "Half-size of the rectangular field in z.");
  fMessenger->DeclareProperty("directionX", fDirectionX, "Beam direction x component.");
  fMessenger->DeclareProperty("directionY", fDirectionY, "Beam direction y component.");
  fMessenger->DeclareProperty("directionZ", fDirectionZ, "Beam direction z component.");
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  auto* particle = G4ParticleTable::GetParticleTable()->FindParticle(fParticleName);
  if (!particle) {
    G4ExceptionDescription msg;
    msg << "Unknown primary particle '" << fParticleName << "'.";
    G4Exception("PrimaryGeneratorAction::GeneratePrimaries", "Therapy001",
                FatalException, msg);
  }

  auto direction = G4ThreeVector(fDirectionX, fDirectionY, fDirectionZ);
  if (direction.mag2() == 0.0) {
    direction = {-1.0, 0.0, 0.0};
  }
  direction = direction.unit();

  const auto target = fDetector->GetTumorRegionCenterWorld();
  const auto y = target.y() + (2.0 * G4UniformRand() - 1.0) * fBeamHalfY;
  const auto z = target.z() + (2.0 * G4UniformRand() - 1.0) * fBeamHalfZ;

  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticleEnergy(fEnergy);
  fParticleGun->SetParticleMomentumDirection(direction);
  fParticleGun->SetParticlePosition({fSourceX, y, z});
  fParticleGun->GeneratePrimaryVertex(event);
}
