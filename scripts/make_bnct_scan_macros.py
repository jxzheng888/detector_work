#!/usr/bin/env python3
from pathlib import Path

TEMPLATE = """/control/verbose 1
/run/verbose 1
/tracking/verbose 0

/therapy/geometry/tumorCenterFromRight 5 cm
/therapy/geometry/tumorCenterFromTop 25 cm
/therapy/geometry/tumorCellFraction 0.80
/therapy/geometry/boronModel {model}
/therapy/geometry/boronPPM {ppm}

/therapy/source/particle neutron
/therapy/source/energy 0.5 eV
/therapy/source/sourceX 22 cm
/therapy/source/beamHalfY 0.8 cm
/therapy/source/beamHalfZ 1.8 cm
/therapy/source/directionX -1
/therapy/source/directionY 0
/therapy/source/directionZ 0

/run/setCut 0.1 um
/run/initialize
/therapy/output/fileName bnct_{model}_{ppm}ppm.root
/run/beamOn {events}
"""


def main() -> None:
    out = Path("macros/generated")
    out.mkdir(parents=True, exist_ok=True)
    for model in ("uniform", "shell"):
        for ppm in (0, 10, 20, 40, 80, 120):
            path = out / f"bnct_{model}_{ppm}ppm.mac"
            path.write_text(TEMPLATE.format(model=model, ppm=ppm, events=20000))
            print(path)


if __name__ == "__main__":
    main()
