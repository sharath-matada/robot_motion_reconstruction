#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/numpy.h>

#include "robotoc/cost/time_varying_com_cost.hpp"


namespace robotoc {
namespace python {

class PyTimeVaryingCoMRefBase : public TimeVaryingCoMRefBase {
public:
  // Inherit the constructors
  using TimeVaryingCoMRefBase::TimeVaryingCoMRefBase;

  void update_com_ref(const double t, Eigen::VectorXd& com_ref) const override {
    PYBIND11_OVERRIDE_PURE(void, TimeVaryingCoMRefBase, 
                           update_com_ref, t, com_ref);
  }

  bool isActive(const double t) const override {
    PYBIND11_OVERRIDE_PURE(bool, TimeVaryingCoMRefBase, 
                           isActive, t);
  }
};


namespace py = pybind11;

PYBIND11_MODULE(time_varying_com_ref_base, m) {
  py::class_<TimeVaryingCoMRefBase, PyTimeVaryingCoMRefBase,
             std::shared_ptr<TimeVaryingCoMRefBase>>(m, "TimeVaryingCoMRefBase")
    .def(py::init<>())
    .def("update_com_ref", &TimeVaryingCoMRefBase::update_com_ref,
          py::arg("t"), py::arg("com_ref"))
    .def("is_active", &TimeVaryingCoMRefBase::isActive,
          py::arg("t"));
}

} // namespace python
} // namespace robotoc