#ifndef SIMULATION_CONFIG_HH
#define SIMULATION_CONFIG_HH

#include "G4String.hh"

enum CellScoreId {
  kTumorCell = 0,
  kNormalCellInTumorRegion = 1,
  kHealthyNormalCell = 2,
  kTumorRegionMixture = 3
};

inline int BoronModelId(const G4String& model)
{
  if (model == "uniform") return 1;
  if (model == "shell") return 2;
  return 0;
}

#endif
