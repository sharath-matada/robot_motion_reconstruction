#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/numpy.h>

#include "robotoc/cost/swing_foot_cost.hpp"


namespace robotoc {
namespace python {

namespace py = pybind11;

PYBIND11_MODULE(swing_foot_cost, m) {
  py::class_<SwingFootCost, CostFunctionComponentBase,
             std::shared_ptr<SwingFootCost>>(m, "SwingFootCost")
    .def(py::init<const Robot&, const int,
                  const std::shared_ptr<SwingFootRefBase>&>(),
          py::arg("robot"), py::arg("contact_index"), py::arg("x3d_ref"))
    .def(py::init<const Robot&, const std::string&,
                  const std::shared_ptr<SwingFootRefBase>&>(),
          py::arg("robot"), py::arg("contact_frame_name"), py::arg("x3d_ref"))
    .def("set_ref", &SwingFootCost::set_ref,
          py::arg("x3d_ref"))
    .def("set_weight", &SwingFootCost::set_weight,
          py::arg("x3d_weight"));
}

} // namespace python
} // namespace robotoc