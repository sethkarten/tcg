# setup.py

from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy as np

extensions = [
    Extension(
        "cy_ptcg",
        ["cy_ptcg.pyx"],
        include_dirs=[np.get_include(), "simulator"],
        library_dirs=["simulator"],
        libraries=["ptcg"],
    )
]

setup(
    ext_modules=cythonize(extensions),
)
