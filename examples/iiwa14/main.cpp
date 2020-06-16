#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "Eigen/Core"

#include "ocp/OCP.hpp"
#include "robot/robot.hpp"
#include "cost_function.hpp"
#include "constraints.hpp"


int main() {
  srand((unsigned int) time(0));
  const std::string urdf_file_name = "../urdf/iiwa14.urdf";
  idocp::Robot robot(urdf_file_name);
  const Eigen::VectorXd q_ref = Eigen::VectorXd::Constant(robot.dimq(), 2);
  idocp::iiwa14::CostFunction cost(robot, q_ref);
  idocp::iiwa14::Constraints constraints(robot);
  const double T = 2;
  const unsigned int N = 100;
  const unsigned int num_proc = 4;
  idocp::OCP ocp_(robot, &cost, &constraints, T, N, num_proc);
  const double t = 0;
  Eigen::VectorXd q = Eigen::VectorXd::Random(robot.dimq());
  Eigen::VectorXd v = Eigen::VectorXd::Random(robot.dimv());
  q.fill(-2);
  v.fill(0);

  // ocp_.setStateTrajectory(q, v, q_ref, Eigen::VectorXd::Zero(robot.dimv()));
  ocp_.setStateTrajectory(q, v);
  std::chrono::system_clock::time_point start_clock, end_clock;
  start_clock = std::chrono::system_clock::now();
  const int num_iteration = 100;
  // std::cout << ocp_.optimalityError(t, q, v) << std::endl;
  for (int i=0; i<num_iteration; ++i) {
    ocp_.solveLQR(t, q, v);
    std::cout << ocp_.optimalityError(t, q, v) << std::endl;
  }
  end_clock = std::chrono::system_clock::now();
  ocp_.printSolution();
  // std::cout << "q0: " << q.transpose() << std::endl;
  // std::cout << "v0: " << v.transpose() << std::endl;
  std::cout << "total CPU time: " << 1e-03 * std::chrono::duration_cast<std::chrono::microseconds>(end_clock-start_clock).count() << "[ms]" << std::endl;
  std::cout << "CPU time per update: " << 1e-03 * std::chrono::duration_cast<std::chrono::microseconds>(end_clock-start_clock).count() / num_iteration << "[ms]" << std::endl;

  return 0;
}