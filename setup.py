import os
import re
import sys
import subprocess
from pathlib import Path
from glob import glob

from setuptools import setup, Extension, find_packages
from pybind11.setup_helpers import Pybind11Extension
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion
import shutil

# # Convert distutils Windows platform specifiers to CMake -A arguments
# PLAT_TO_CMAKE = {
#     "win32": "Win32",
#     "win-amd64": "x64",
#     "win-arm32": "ARM",
#     "win-arm64": "ARM64",
# }

# # A CMakeExtension needs a sourcedir instead of a file list.
# # The name must be the _single_ output extension from the CMake build.
# # If you need multiple extensions, see scikit-build.
# class CMakeExtension(Extension):
#     def __init__(self, name: str, sourcedir: str = "") -> None:
#         super().__init__(name, sources=[])
#         self.sourcedir = os.fspath(Path(sourcedir).resolve())

#         print("\n\n\n", self.sourcedir, "\n\n\n")

# class CMakeBuild(build_ext):
#     def build_extension(self, ext: CMakeExtension) -> None:
#         # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
#         ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)

#         extdir = ext_fullpath.parent.resolve()

#         # Using this requires trailing slash for auto-detection & inclusion of
#         # auxiliary "native" libs

#         debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
#         cfg = "Debug" if debug else "Release"

#         # CMake lets you override the generator - we need to check this.
#         # Can be set with Conda-Build, for example.
#         cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

#         # Set Python_EXECUTABLE instead if you use PYBIND11_FINDPYTHON
#         # EXAMPLE_VERSION_INFO shows you how to pass a value into the C++ code
#         # from Python.
#         cmake_args = [
#             "-DCOMPILE_CLIENT=ON"
#             f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}",
#             f"-DPYTHON_EXECUTABLE={sys.executable}",
#             f"-DCMAKE_BUILD_TYPE={cfg}",  # not used on MSVC, but no harm
#         ]
#         build_args = []
#         # Adding CMake arguments set as environment variable
#         # (needed e.g. to build for ARM OSx on conda-forge)
#         if "CMAKE_ARGS" in os.environ:
#             cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

#         # In this example, we pass in the version to C++. You might not need to.
#         cmake_args += [f"-DEXAMPLE_VERSION_INFO={self.distribution.get_version()}"]

#         if self.compiler.compiler_type != "msvc":
#             # Using Ninja-build since it a) is available as a wheel and b)
#             # multithreads automatically. MSVC would require all variables be
#             # exported for Ninja to pick it up, which is a little tricky to do.
#             # Users can override the generator with CMAKE_GENERATOR in CMake
#             # 3.15+.
#             if not cmake_generator or cmake_generator == "Ninja":
#                 try:
#                     import ninja

#                     ninja_executable_path = Path(ninja.BIN_DIR) / "ninja"
#                     cmake_args += [
#                         "-GNinja",
#                         f"-DCMAKE_MAKE_PROGRAM:FILEPATH={ninja_executable_path}",
#                     ]
#                 except ImportError:
#                     pass

#         else:
#             # Single config generators are handled "normally"
#             single_config = any(x in cmake_generator for x in {"NMake", "Ninja"})

#             # CMake allows an arch-in-generator style for backward compatibility
#             contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})

#             # Specify the arch if using MSVC generator, but only if it doesn't
#             # contain a backward-compatibility arch spec already in the
#             # generator name.
#             if not single_config and not contains_arch:
#                 cmake_args += ["-A", PLAT_TO_CMAKE[self.plat_name]]

#             # Multi-config generators have a different way to specify configs
#             if not single_config:
#                 cmake_args += [
#                     f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"
#                 ]
#                 build_args += ["--config", cfg]

#         if sys.platform.startswith("darwin"):
#             # Cross-compile support for macOS - respect ARCHFLAGS if set
#             archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
#             if archs:
#                 cmake_args += ["-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))]

#         # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
#         # across all generators.
#         if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
#             # self.parallel is a Python 3 only way to set parallel jobs by hand
#             # using -j in the build_ext call, not supported by pip or PyPA-build.
#             if hasattr(self, "parallel") and self.parallel:
#                 # CMake 3.12+ only.
#                 build_args += [f"-j{self.parallel}"]

#         build_temp = Path(self.build_temp) / ext.name
#         if not build_temp.exists():
#             build_temp.mkdir(parents=True)

#         subprocess.run(
#             ["cmake", ext.sourcedir, *cmake_args], cwd=build_temp, check=True
#         )
#         subprocess.run(
#             ["cmake", "--build", ".", "-j4", *build_args], cwd=build_temp, check=True
#         )

def install_smfpy_file(target_dir="./build"):
    packages = find_packages()
    for package in packages:
        package_dir = os.path.join(package.replace(".", os.path.sep))
        install_package_dir = os.path.join(target_dir, package_dir)
        shutil.copytree(package_dir, install_package_dir, dirs_exist_ok=True)

def copy_so_files_recursive(source_dir, target_dir):
    for root, dirs, files in os.walk(source_dir):
        # print([root, dirs, files])
        for file in files:
            if file.endswith(".so"):
                source_file = os.path.join(root, file)
                relative_path = os.path.relpath(source_file, source_dir)
                target_file = os.path.join(target_dir, relative_path)
                print([source_file, relative_path, target_file])
                os.makedirs(os.path.dirname(target_file), exist_ok=True)
                shutil.copy(source_file, target_file)

def get_setup_parameters():
    packages = find_packages()
    
    package_data = {}
    
    for package in packages:
        package_dir = os.path.join(package.replace(".", os.path.sep))
        package_files = []
        
        for file in os.listdir(package_dir):
            if file.endswith(".so"):
                package_files.append(file)
        
        if package_files:
            package_data[package] = package_files
    
    return packages, package_data

if __name__ == "__main__":
    pacakge_name = "smfpy"
    build_dir = "./build"
    install_smfpy_file(build_dir)
    os.chdir(build_dir)
    copy_so_files_recursive(os.path.join(".", pacakge_name, "_C"), os.path.join(".", pacakge_name))
    packages, package_data = get_setup_parameters()
    print(packages, package_data)

    setup(
        name='smfpy',
        version='0.1',
        packages=packages,
        package_data=package_data,
        entry_points={
            'console_scripts': [
                'my_script = smfpy.client_app.console:main',  # 指定要运行的脚本或模块
            ],
        },
    )
