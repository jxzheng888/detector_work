# detector_work

Geant4 project for the detector simulation homework on radiotherapy comparison and BNCT.

## Physics question

The assignment is treated as two related Monte Carlo studies:

1. Compare gamma and proton irradiation for a simplified body phantom. The scored quantities are cell-level energy deposition, dose, and lineal-energy/LET-like spectra for tumor cells and normal cells.
2. Simulate BNCT with a 0.5 eV thermal neutron beam. Study how the expected cell dose changes with the intracellular `10B` distribution: uniform in the tumor cell, or concentrated in the outer 1 um shell.

This is an educational Geant4 model, not a clinical treatment-planning model.

## Geometry and assumptions

The phantom follows the homework sketch:

- head: sphere, diameter 180 mm
- neck: cylinder, diameter 100 mm, height 90 mm
- body: box, `X = 260 mm`, `Y = 120 mm`, `Z = 500 mm`
- legs: two cylinders, diameter 110 mm, height 820 mm
- red tumor region: box, `X = 2 cm`, `Y = 1 cm`, `Z = 3 cm`

The two distances in the sketch are interpreted as center-to-edge distances:

- tumor-region center is 5 cm from the right side of the body
- tumor-region center is 25 cm from the top of the body

With the body centered at the world origin, this places the tumor-region center at approximately `(x, y, z) = (8 cm, 0, 0)`.

The red region is not treated as pure tumor. It is a mixed tissue region:

- default tumor-region mixture: 80 percent tumor cells and 20 percent normal cells
- normal region: 100 percent normal cells

The simulation places representative 10 um diameter spherical cells:

- one tumor cell inside the tumor region
- one normal cell inside the tumor region
- one healthy normal cell outside the tumor region

The mixed tumor-region score is computed as a weighted result:

```text
Dose_mixed = f_tumor * Dose_tumor_cell + (1 - f_tumor) * Dose_normal_cell_in_tumor_region
```

For BNCT, `10B` is added only to the tumor cell. Normal cells are kept as tissue-equivalent material. The shell model keeps the total `10B` amount equal to the uniform model by increasing the shell concentration according to the shell volume fraction.

## Build

Run in a Geant4 environment:

```bash
cmake -S . -B build
cmake --build build -j
```

The default reference physics list is `QGSP_BIC_HP`, chosen so low-energy neutron interactions are available for BNCT. You can pass a different Geant4 reference list as the second executable argument.

## Run

```bash
./build/detector_work macros/gamma.mac QGSP_BIC_HP
./build/detector_work macros/proton.mac QGSP_BIC_HP
./build/detector_work macros/bnct_uniform.mac QGSP_BIC_HP
./build/detector_work macros/bnct_shell.mac QGSP_BIC_HP
```

Or run everything:

```bash
bash scripts/run_all.sh
```

For a BNCT concentration scan:

```bash
python3 scripts/make_bnct_scan_macros.py
cmake --build build -j
./build/detector_work macros/generated/bnct_uniform_40ppm.mac QGSP_BIC_HP
./build/detector_work macros/generated/bnct_shell_40ppm.mac QGSP_BIC_HP
```

## Output

Each macro writes a ROOT file. The event tree contains:

- `cell_id = 0`: tumor cell
- `cell_id = 1`: normal cell inside tumor region
- `cell_id = 2`: healthy normal cell
- `cell_id = 3`: weighted tumor-region mixture

Main columns:

- `edep_MeV`
- `dose_Gy`
- `track_um`
- `lineal_keV_um`
- `boron_ppm`
- `boron_model_id`: 0 none, 1 uniform, 2 shell
- `tumor_cell_fraction`

The step tree stores energy-depositing steps in scored cells and includes an LET proxy:

```text
LET_step = edep / step_length
```

The event-level lineal energy uses the mean chord length of a 10 um diameter sphere:

```text
mean chord = 4R/3 = 6.67 um
lineal energy = event energy imparted / mean chord
```

## Quick ROOT summary

```bash
root -l 'analysis/plot_results.C("gamma_6MeV.root")'
```

The macro prints mean scores and saves dose/LET plots as PNG files.

## Notes for the report

Expected qualitative comparison:

- gamma irradiation is penetrating, so the healthy-normal-cell dose is expected to be less localized relative to the tumor score
- proton irradiation should show better localization if the beam energy is tuned so the Bragg peak is near the tumor depth
- BNCT selectivity should come mainly from `10B` being inside tumor cells; concentrating `10B` near the cell surface changes where the short-range alpha and lithium fragments deposit their energy

For a stronger report, scan:

- proton energy around 65 to 85 MeV
- `10B` concentration, for example 0, 10, 20, 40, 80, 120 ppm
- tumor-cell fraction in the red region, for example 0.7, 0.8, 0.9
