#include "DetectorConstruction.hh"

#include "SimulationConfig.hh"

#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4Element.hh"
#include "G4GenericMessenger.hh"
#include "G4Isotope.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PhysicalConstants.hh"
#include "G4PVPlacement.hh"
#include "G4Sphere.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"

#include <algorithm>
#include <cmath>

DetectorConstruction::DetectorConstruction()
{
  fCellMass.fill(0.0);
  DefineCommands();
}

DetectorConstruction::~DetectorConstruction()
{
  delete fMessenger;
}

void DetectorConstruction::DefineCommands()
{
  fMessenger = new G4GenericMessenger(this, "/therapy/geometry/",
                                      "Geometry and cell-composition controls");
  fMessenger->DeclareProperty("tumorCellFraction", fTumorCellFraction,
                              "Tumor-cell fraction inside the red tumor region.");
  fMessenger->DeclareProperty("boronPPM", fBoronPPM,
                              "10B mass concentration in tumor cells, in ppm.");
  fMessenger->DeclareProperty("boronModel", fBoronModel,
                              "10B distribution in tumor cells: none, uniform, or shell.");
  fMessenger->DeclareProperty("checkOverlaps", fCheckOverlaps,
                              "Enable Geant4 overlap checks for placements.");
  fMessenger->DeclarePropertyWithUnit("tumorCenterFromRight", "cm",
                                      fTumorCenterFromRight,
                                      "Distance from body right edge to tumor-region center.");
  fMessenger->DeclarePropertyWithUnit("tumorCenterFromTop", "cm",
                                      fTumorCenterFromTop,
                                      "Distance from body top edge to tumor-region center.");
}

void DetectorConstruction::ConstructMaterials()
{
  auto* nist = G4NistManager::Instance();
  fAir = nist->FindOrBuildMaterial("G4_AIR");
  fTissue = nist->FindOrBuildMaterial("G4_WATER");

  auto* b10 = new G4Isotope("B10", 5, 10, 10.012937 * g / mole);
  fB10Element = new G4Element("Boron10", "B10", 1);
  fB10Element->AddIsotope(b10, 100.0 * perCent);
}

G4Material* DetectorConstruction::BuildBoronLoadedTissue(const G4String& name, G4double ppm)
{
  const auto clampedPPM = std::max<G4double>(0.0, ppm);
  if (clampedPPM <= 0.0) {
    return fTissue;
  }

  const auto massFraction = std::min<G4double>(clampedPPM * 1.0e-6, 0.05);
  auto* material = new G4Material(name, fTissue->GetDensity(), 2,
                                  kStateSolid, 293.15 * kelvin);
  material->AddMaterial(fTissue, 1.0 - massFraction);
  material->AddElement(fB10Element, massFraction);
  return material;
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  fCellMass.fill(0.0);
  ConstructMaterials();

  const G4double worldHalf = 1.5 * m;
  auto* worldSolid = new G4Box("WorldSolid", worldHalf, worldHalf, worldHalf);
  fWorldLV = new G4LogicalVolume(worldSolid, fAir, "WorldLV");
  auto* worldPV = new G4PVPlacement(nullptr, {}, fWorldLV, "WorldPV",
                                    nullptr, false, 0, fCheckOverlaps);

  const G4double bodyHalfX = 130.0 * mm;
  const G4double bodyHalfY = 60.0 * mm;
  const G4double bodyHalfZ = 250.0 * mm;
  auto* bodySolid = new G4Box("BodySolid", bodyHalfX, bodyHalfY, bodyHalfZ);
  fBodyLV = new G4LogicalVolume(bodySolid, fTissue, "BodyLV");
  new G4PVPlacement(nullptr, {}, fBodyLV, "BodyPV",
                    fWorldLV, false, 0, fCheckOverlaps);

  auto* neckSolid = new G4Tubs("NeckSolid", 0.0, 50.0 * mm, 45.0 * mm,
                               0.0, twopi);
  auto* neckLV = new G4LogicalVolume(neckSolid, fTissue, "NeckLV");
  new G4PVPlacement(nullptr, {0.0, 0.0, bodyHalfZ + 45.0 * mm}, neckLV,
                    "NeckPV", fWorldLV, false, 0, fCheckOverlaps);

  auto* headSolid = new G4Sphere("HeadSolid", 0.0, 90.0 * mm,
                                 0.0, twopi, 0.0, pi);
  auto* headLV = new G4LogicalVolume(headSolid, fTissue, "HeadLV");
  new G4PVPlacement(nullptr, {0.0, 0.0, bodyHalfZ + 90.0 * mm + 90.0 * mm},
                    headLV, "HeadPV", fWorldLV, false, 0, fCheckOverlaps);

  auto* legSolid = new G4Tubs("LegSolid", 0.0, 55.0 * mm, 410.0 * mm,
                              0.0, twopi);
  auto* legLV = new G4LogicalVolume(legSolid, fTissue, "LegLV");
  new G4PVPlacement(nullptr, {-65.0 * mm, 0.0, -bodyHalfZ - 410.0 * mm},
                    legLV, "LeftLegPV", fWorldLV, false, 0, fCheckOverlaps);
  new G4PVPlacement(nullptr, {65.0 * mm, 0.0, -bodyHalfZ - 410.0 * mm},
                    legLV, "RightLegPV", fWorldLV, false, 1, fCheckOverlaps);

  const auto tumorX = bodyHalfX - fTumorCenterFromRight;
  const auto tumorZ = bodyHalfZ - fTumorCenterFromTop;
  fTumorRegionCenterWorld = {tumorX, 0.0, tumorZ};

  auto* tumorRegionSolid = new G4Box("TumorRegionSolid",
                                     10.0 * mm, 5.0 * mm, 15.0 * mm);
  fTumorRegionLV = new G4LogicalVolume(tumorRegionSolid, fTissue,
                                       "TumorRegionLV");
  new G4PVPlacement(nullptr, fTumorRegionCenterWorld, fTumorRegionLV,
                    "TumorRegionPV", fBodyLV, false, 0, fCheckOverlaps);

  const G4double cellRadius = 5.0 * um;
  const G4double coreRadius = 4.0 * um;
  const G4double fullVolume = 4.0 * pi * std::pow(cellRadius, 3) / 3.0;
  const G4double coreVolume = 4.0 * pi * std::pow(coreRadius, 3) / 3.0;
  const G4double shellVolume = fullVolume - coreVolume;
  const auto shellVolumeFraction = shellVolume / fullVolume;

  G4Material* tumorUniformMaterial = fTissue;
  G4Material* tumorShellMaterial = fTissue;
  if (fBoronModel == "uniform") {
    tumorUniformMaterial = BuildBoronLoadedTissue("TumorCell_B10_uniform", fBoronPPM);
  } else if (fBoronModel == "shell") {
    tumorShellMaterial = BuildBoronLoadedTissue("TumorCell_B10_shell",
                                                fBoronPPM / shellVolumeFraction);
  }

  auto* fullCellSolid = new G4Sphere("CellSolid", 0.0, cellRadius,
                                     0.0, twopi, 0.0, pi);
  auto* coreCellSolid = new G4Sphere("CellCoreSolid", 0.0, coreRadius,
                                     0.0, twopi, 0.0, pi);

  if (fBoronModel == "shell") {
    fTumorCellShellLV = new G4LogicalVolume(fullCellSolid, tumorShellMaterial,
                                            "TumorCellShellLV");
    fTumorCellCoreLV = new G4LogicalVolume(coreCellSolid, fTissue,
                                           "TumorCellCoreLV");
    new G4PVPlacement(nullptr, {}, fTumorCellCoreLV, "TumorCellCorePV",
                      fTumorCellShellLV, false, 0, fCheckOverlaps);
    new G4PVPlacement(nullptr, {0.0, -2.0 * mm, 0.0}, fTumorCellShellLV,
                      "TumorCellPV", fTumorRegionLV, false, 0, fCheckOverlaps);
    fCellMass[kTumorCell] =
        tumorShellMaterial->GetDensity() * shellVolume + fTissue->GetDensity() * coreVolume;
  } else {
    fTumorCellLV = new G4LogicalVolume(fullCellSolid, tumorUniformMaterial,
                                       "TumorCellLV");
    new G4PVPlacement(nullptr, {0.0, -2.0 * mm, 0.0}, fTumorCellLV,
                      "TumorCellPV", fTumorRegionLV, false, 0, fCheckOverlaps);
    fCellMass[kTumorCell] = tumorUniformMaterial->GetDensity() * fullVolume;
  }

  auto* normalCellSolid = new G4Sphere("NormalCellSolid", 0.0, cellRadius,
                                       0.0, twopi, 0.0, pi);
  fNormalInTumorLV = new G4LogicalVolume(normalCellSolid, fTissue,
                                         "TumorRegionNormalCellLV");
  new G4PVPlacement(nullptr, {0.0, 2.0 * mm, 0.0}, fNormalInTumorLV,
                    "TumorRegionNormalCellPV", fTumorRegionLV,
                    false, 0, fCheckOverlaps);

  auto* healthyCellSolid = new G4Sphere("HealthyNormalCellSolid", 0.0, cellRadius,
                                        0.0, twopi, 0.0, pi);
  fHealthyNormalLV = new G4LogicalVolume(healthyCellSolid, fTissue,
                                         "HealthyNormalCellLV");
  new G4PVPlacement(nullptr, {tumorX, 0.0, tumorZ + 60.0 * mm},
                    fHealthyNormalLV, "HealthyNormalCellPV", fBodyLV,
                    false, 0, fCheckOverlaps);

  fCellMass[kNormalCellInTumorRegion] = fTissue->GetDensity() * fullVolume;
  fCellMass[kHealthyNormalCell] = fTissue->GetDensity() * fullVolume;

  auto* cellLimit = new G4UserLimits(1.0 * um);
  if (fTumorCellLV) fTumorCellLV->SetUserLimits(cellLimit);
  if (fTumorCellShellLV) fTumorCellShellLV->SetUserLimits(cellLimit);
  if (fTumorCellCoreLV) fTumorCellCoreLV->SetUserLimits(cellLimit);
  fNormalInTumorLV->SetUserLimits(cellLimit);
  fHealthyNormalLV->SetUserLimits(cellLimit);

  ApplyVisAttributes();
  return worldPV;
}

G4double DetectorConstruction::GetCellMass(G4int cellId) const
{
  if (cellId < 0 || cellId >= static_cast<G4int>(fCellMass.size())) {
    return 0.0;
  }
  return fCellMass[cellId];
}

void DetectorConstruction::ApplyVisAttributes()
{
  fWorldLV->SetVisAttributes(G4VisAttributes::GetInvisible());

  auto* bodyVis = new G4VisAttributes(G4Colour(0.0, 0.7, 0.25, 0.25));
  bodyVis->SetForceSolid(true);
  fBodyLV->SetVisAttributes(bodyVis);

  auto* tumorRegionVis = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 0.35));
  tumorRegionVis->SetForceSolid(true);
  fTumorRegionLV->SetVisAttributes(tumorRegionVis);

  auto* tumorCellVis = new G4VisAttributes(G4Colour(0.8, 0.0, 0.0, 1.0));
  tumorCellVis->SetForceSolid(true);
  if (fTumorCellLV) fTumorCellLV->SetVisAttributes(tumorCellVis);
  if (fTumorCellShellLV) fTumorCellShellLV->SetVisAttributes(tumorCellVis);

  auto* coreVis = new G4VisAttributes(G4Colour(1.0, 0.7, 0.7, 1.0));
  coreVis->SetForceSolid(true);
  if (fTumorCellCoreLV) fTumorCellCoreLV->SetVisAttributes(coreVis);

  auto* normalCellVis = new G4VisAttributes(G4Colour(0.2, 0.7, 1.0, 1.0));
  normalCellVis->SetForceSolid(true);
  fNormalInTumorLV->SetVisAttributes(normalCellVis);
  fHealthyNormalLV->SetVisAttributes(normalCellVis);
}
