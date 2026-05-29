#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

#include <array>

class G4Element;
class G4GenericMessenger;
class G4LogicalVolume;
class G4Material;
class G4VPhysicalVolume;

class DetectorConstruction final : public G4VUserDetectorConstruction {
 public:
  DetectorConstruction();
  ~DetectorConstruction() override;

  G4VPhysicalVolume* Construct() override;

  G4double GetCellMass(G4int cellId) const;
  G4double GetTumorCellFraction() const { return fTumorCellFraction; }
  G4double GetBoronPPM() const { return fBoronPPM; }
  const G4String& GetBoronModel() const { return fBoronModel; }
  G4ThreeVector GetTumorRegionCenterWorld() const { return fTumorRegionCenterWorld; }

 private:
  void DefineCommands();
  void ConstructMaterials();
  G4Material* BuildBoronLoadedTissue(const G4String& name, G4double ppm);
  void ApplyVisAttributes();

  G4GenericMessenger* fMessenger = nullptr;

  G4double fTumorCellFraction = 0.80;
  G4double fBoronPPM = 0.0;
  G4String fBoronModel = "none";
  G4bool fCheckOverlaps = true;

  G4double fTumorCenterFromRight = 50.0;
  G4double fTumorCenterFromTop = 250.0;

  G4Material* fAir = nullptr;
  G4Material* fTissue = nullptr;
  G4Element* fB10Element = nullptr;

  G4LogicalVolume* fWorldLV = nullptr;
  G4LogicalVolume* fBodyLV = nullptr;
  G4LogicalVolume* fTumorRegionLV = nullptr;
  G4LogicalVolume* fTumorCellLV = nullptr;
  G4LogicalVolume* fTumorCellCoreLV = nullptr;
  G4LogicalVolume* fTumorCellShellLV = nullptr;
  G4LogicalVolume* fNormalInTumorLV = nullptr;
  G4LogicalVolume* fHealthyNormalLV = nullptr;

  std::array<G4double, 3> fCellMass{};
  G4ThreeVector fTumorRegionCenterWorld;
};

#endif
