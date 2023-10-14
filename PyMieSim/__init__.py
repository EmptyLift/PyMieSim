from .tools import measure
import numpy

from PyMieSim.tools.directories import lp_mode_path


def load_lp_mode(mode_number: str, structure_type: str = 'unstructured', sampling: int = 500):
    structure_type = structure_type.lower()

    lp_file_directory = lp_mode_path.joinpath(mode_number)

    if not lp_file_directory.exists():
        available_modes = [path.name for path in lp_mode_path.iterdir() if path.name.startswith('LP')]
        raise ValueError(f"This LP mode: {mode_number} is not avaible.\nAvailable are {available_modes}")

    lp_sampling_file_directory = lp_file_directory.joinpath(f"{structure_type}_{sampling}.npy")

    print(lp_sampling_file_directory)
    if not lp_sampling_file_directory.exists():
        available_sampling = [path.name for path in lp_file_directory.iterdir() if path.name.startswith(structure_type)]
        raise ValueError(f"This sampling: {sampling} is not avaible.\nAvailable are {available_sampling}")

    return numpy.load(lp_sampling_file_directory).astype(complex)

# -
