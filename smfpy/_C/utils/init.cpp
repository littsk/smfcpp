#include <pybind11/pybind11.h>

#include <init.hpp>

namespace py = pybind11;

PYBIND11_MODULE(C_init, m) {
    m.doc() = "Init Module"; // Optional module docstring

    m.def("init_glog", &InitGLOG, "Initialize Google Logging",
          py::arg("program_name") = "default_program",
          py::arg("log_dir") = "",
          py::arg("log_level") = google::INFO,
          py::arg("stderr_log_level") = google::INFO);
}
