#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/numpy.h>

#include "Eigen/Core"

#include "robotoc/utils/rotation.hpp"


namespace robotoc {
namespace rotation {
namespace python {

namespace py = pybind11;

PYBIND11_MODULE(rotation, m) {
  py::enum_<ProjectionAxis>(m, "ProjectionAxis", py::arithmetic())
    .value("X", ProjectionAxis::X)
    .value("Y", ProjectionAxis::Y)
    .value("Z", ProjectionAxis::Z)
    .export_values();

  m.def("rotation_matrix_from_quaternion", [](const Eigen::Vector4d& quat_xyzw) {
      return rotation::RotationMatrixFromQuaternion(quat_xyzw);
    },  py::arg("quat_xyzw"));
  m.def("rotation_matrix_from_normal_vector", [](const Eigen::Vector3d& normal_vector) {
      return rotation::RotationMatrixFromNormalVector(normal_vector);
    },  py::arg("normal_vector"));
  m.def("quaternion_from_rotation_matrix", [](const Eigen::Matrix3d& R) {
      return rotation::QuaternionFromRotationMatrix(R);
    },  py::arg("R"));
  m.def("quaternion_from_normal_vector", [](const Eigen::Vector3d& normal_vector) {
      return rotation::QuaternionFromNormalVector(normal_vector);
    },  py::arg("normal_vector"));
  m.def("project_rotation_matrix", [](const Eigen::Matrix3d& R, const ProjectionAxis axis) {
      Eigen::Matrix3d ret = R;
      rotation::ProjectRotationMatrix(ret, axis);
      return ret;
    },  py::arg("R"), py::arg("axis"));
}

} // namespace python
} // namespace rotation
} // namespace robotoc