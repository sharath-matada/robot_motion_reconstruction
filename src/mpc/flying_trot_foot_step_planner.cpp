#include "robotoc/mpc/flying_trot_foot_step_planner.hpp"
#include "robotoc/utils/rotation.hpp"

#include <stdexcept>
#include <iostream>
#include <cassert>


namespace robotoc {

FlyingTrotFootStepPlanner::FlyingTrotFootStepPlanner(const Robot& quadruped_robot)
  : ContactPlannerBase(),
    robot_(quadruped_robot),
    raibert_heuristic_(),
    enable_raibert_heuristic_(false),
    LF_foot_id_(quadruped_robot.pointContactFrames()[0]),
    LH_foot_id_(quadruped_robot.pointContactFrames()[1]),
    RF_foot_id_(quadruped_robot.pointContactFrames()[2]),
    RH_foot_id_(quadruped_robot.pointContactFrames()[3]),
    current_step_(0),
    contact_position_ref_(),
    com_ref_(),
    R_(),
    com_to_contact_position_local_(),
    v_com_(Eigen::Vector3d::Zero()),
    v_com_cmd_(Eigen::Vector3d::Zero()),
    step_length_(Eigen::Vector3d::Zero()),
    R_yaw_(Eigen::Matrix3d::Identity()),
    yaw_rate_cmd_(0) {
  try {
    if (quadruped_robot.maxNumPointContacts() < 4) {
      throw std::out_of_range(
          "invalid argument: robot is not a quadrupedal robot!\n robot.maxNumPointContacts() must be larger than 4!");
    }
  }
  catch(const std::exception& e) {
    std::cerr << e.what() << '\n';
    std::exit(EXIT_FAILURE);
  }
}


FlyingTrotFootStepPlanner::FlyingTrotFootStepPlanner() {
}


FlyingTrotFootStepPlanner::~FlyingTrotFootStepPlanner() {
}


void FlyingTrotFootStepPlanner::setGaitPattern(
    const Eigen::Vector3d& step_length, const double step_yaw) {
  step_length_ = step_length;
  R_yaw_<< std::cos(step_yaw), -std::sin(step_yaw), 0, 
           std::sin(step_yaw),  std::cos(step_yaw), 0,
           0, 0, 1;
  enable_raibert_heuristic_ = false;
}


void FlyingTrotFootStepPlanner::setGaitPattern(
    const Eigen::Vector3d& v_com_cmd, const double yaw_rate_cmd, 
    const double flying_time, const double stance_time, const double gain) {
  try {
    if (flying_time <= 0.0) {
      throw std::out_of_range("invalid argument: flying_time must be positive!");
    }
    if (stance_time <= 0.0) {
      throw std::out_of_range("invalid argument: stance_time must be positive!");
    }
    if (gain <= 0.0) {
      throw std::out_of_range("invalid argument: gain must be positive!");
    }
  }
  catch(const std::exception& e) {
    std::cerr << e.what() << '\n';
    std::exit(EXIT_FAILURE);
  }
  raibert_heuristic_.setParameters(2.0*stance_time, gain);
  v_com_cmd_ = v_com_cmd;
  const double yaw_cmd = yaw_rate_cmd * flying_time;
  R_yaw_<< std::cos(yaw_cmd), -std::sin(yaw_cmd), 0, 
           std::sin(yaw_cmd),  std::cos(yaw_cmd), 0,
           0, 0, 1;
  yaw_rate_cmd_ = yaw_rate_cmd;
  enable_raibert_heuristic_ = true;
}


void FlyingTrotFootStepPlanner::init(const Eigen::VectorXd& q) {
  Eigen::Matrix3d R = rotation::toRotationMatrix(q.template segment<4>(3));
  rotation::projectRotationMatrix(R, rotation::ProjectionAxis::Z);
  robot_.updateFrameKinematics(q);
  com_to_contact_position_local_ = { R.transpose() * (robot_.framePosition(LF_foot_id_)-robot_.CoM()), 
                                     R.transpose() * (robot_.framePosition(LH_foot_id_)-robot_.CoM()),
                                     R.transpose() * (robot_.framePosition(RF_foot_id_)-robot_.CoM()),
                                     R.transpose() * (robot_.framePosition(RH_foot_id_)-robot_.CoM()) };
  contact_position_ref_.clear();
  com_ref_.clear(),
  com_ref_.push_back(robot_.CoM());
  R_.clear();
  R_.push_back(R);
  current_step_ = 0;
}


bool FlyingTrotFootStepPlanner::plan(const double t, const Eigen::VectorXd& q,
                                     const Eigen::VectorXd& v,
                                     const ContactStatus& contact_status,
                                     const int planning_steps) {
  assert(planning_steps >= 0);
  if (enable_raibert_heuristic_) {
    v_com_.transpose() = R_.front().transpose() * v.template head<3>();
    raibert_heuristic_.planStepLength(v_com_.template head<2>(), 
                                      v_com_cmd_.template head<2>(), yaw_rate_cmd_);
    step_length_ = raibert_heuristic_.stepLength();
  }
  robot_.updateFrameKinematics(q);
  std::vector<Eigen::Vector3d> contact_position;
  for (const auto frame : robot_.pointContactFrames()) {
    contact_position.push_back(robot_.framePosition(frame));
  }
  Eigen::Vector3d com = com_ref_.front();
  Eigen::Matrix3d R = R_.front();
  if (contact_status.isContactActive(0) && contact_status.isContactActive(1) 
      && contact_status.isContactActive(2) && contact_status.isContactActive(3)) {
    current_step_ = 0;
    com.setZero();
    for (int i=0; i<4; ++i) {
      com.noalias() += contact_position[i];
      com.noalias() -= R * com_to_contact_position_local_[i];
    }
    com.array() /= 4.0;
  }
  else if (contact_status.isContactActive(0) && contact_status.isContactActive(3)) {
    if (current_step_%4 != 1) {
      ++current_step_;
      R = (R_yaw_ * R).eval();
    }
    com.setZero();
    com.noalias() += contact_position[0];
    com.noalias() -= R * com_to_contact_position_local_[0];
    com.noalias() += contact_position[3];
    com.noalias() -= R * com_to_contact_position_local_[3];
    com.array() /= 2.0;
    contact_position[1].noalias() = com + R * (com_to_contact_position_local_[1] - 0.5 * step_length_);
    contact_position[2].noalias() = com + R * (com_to_contact_position_local_[2] - 0.5 * step_length_);
  }
  else if (contact_status.isContactActive(1) && contact_status.isContactActive(2)) {
    if (current_step_%4 != 3) {
      ++current_step_;
      R = (R_yaw_ * R).eval();
    }
    com.setZero();
    com.noalias() += contact_position[1];
    com.noalias() -= R * com_to_contact_position_local_[1];
    com.noalias() += contact_position[2];
    com.noalias() -= R * com_to_contact_position_local_[2];
    com.array() /= 2.0;
    contact_position[0].noalias() = com + R * (com_to_contact_position_local_[0] - 0.5 * step_length_);
    contact_position[3].noalias() = com + R * (com_to_contact_position_local_[3] - 0.5 * step_length_);
  }
  else {
    if (current_step_%2 != 0) {
      ++current_step_;
    }
    for (int i=0; i<4; ++i) {
      contact_position[i] = contact_position_ref_[0][i];
    }
  } 
  com_ref_.clear();
  com_ref_.push_back(com);
  contact_position_ref_.clear();
  contact_position_ref_.push_back(contact_position);
  R_.clear();
  R_.push_back(R);
  for (int step=current_step_; step<=planning_steps+current_step_; ++step) {
    if (step == 0) {
      // do nothing
    }
    else if (current_step_ == 0 && step == 1) {
      // do nothing
    }
    else if (current_step_ == 0 && step == 2) {
      R = (R_yaw_ * R).eval();
      if (enable_raibert_heuristic_) {
        com.noalias() += 0.5 * R * step_length_;
      }
      else {
        com.noalias() += 0.25 * R * step_length_;
      }
      contact_position[1].noalias() = com + R * com_to_contact_position_local_[1];
      contact_position[2].noalias() = com + R * com_to_contact_position_local_[2];
    }
    else if (step%4 == 1) {
      // do nothing
    }
    else if (step%4 == 2) {
      R = (R_yaw_ * R).eval();
      com.noalias() += 0.5 * R * step_length_;
      contact_position[1].noalias() = com + R * com_to_contact_position_local_[1];
      contact_position[2].noalias() = com + R * com_to_contact_position_local_[2];
    }
    else if (step%4 == 3) {
      // do nothing
    }
    else {
      R = (R_yaw_ * R).eval();
      com.noalias() += 0.5 * R * step_length_;
      contact_position[0].noalias() = com + R * com_to_contact_position_local_[0];
      contact_position[3].noalias() = com + R * com_to_contact_position_local_[3];
    }

    com_ref_.push_back(com);
    contact_position_ref_.push_back(contact_position);
    R_.push_back(R);
  }
  com_ref_.push_back(com);
  contact_position_ref_.push_back(contact_position);
  R_.push_back(R);
  return true;
}


const aligned_vector<SE3>& FlyingTrotFootStepPlanner::contactPlacement(const int step) const {
  return contact_placement_ref_[step];
}


const aligned_vector<aligned_vector<SE3>>& FlyingTrotFootStepPlanner::contactPlacement() const {
  return contact_placement_ref_;
}


const std::vector<Eigen::Vector3d>& FlyingTrotFootStepPlanner::contactPosition(const int step) const {
  return contact_position_ref_[step];
}


const std::vector<std::vector<Eigen::Vector3d>>& FlyingTrotFootStepPlanner::contactPosition() const {
  return contact_position_ref_;
}


const Eigen::Vector3d& FlyingTrotFootStepPlanner::com(const int step) const {
  return com_ref_[step];
}
  

const std::vector<Eigen::Vector3d>& FlyingTrotFootStepPlanner::com() const {
  return com_ref_;
}


const Eigen::Matrix3d& FlyingTrotFootStepPlanner::R(const int step) const {
  return R_[step];
}
  

const std::vector<Eigen::Matrix3d>& FlyingTrotFootStepPlanner::R() const {
  return R_;
}


void FlyingTrotFootStepPlanner::disp(std::ostream& os) const {
  std::cout << "Flying trot foot step planner:" << std::endl;
  std::cout << "current_step:" << current_step_ << std::endl;
  const int planning_steps = contact_position_ref_.size();
  for (int i=0; i<planning_steps; ++i) {
    std::cout << "contact position[" << i << "]: ["  
              << contact_position_ref_[i][0].transpose() << "], [" 
              << contact_position_ref_[i][1].transpose() << "], [" 
              << contact_position_ref_[i][2].transpose() << "], [" 
              << contact_position_ref_[i][3].transpose() << "]" << std::endl;
    std::cout << "CoM position[" << i << "]: ["   << com_ref_[i].transpose() << "]" << std::endl;
    std::cout << "R[" << i << "]: ["   << R_[i] << "]" << std::endl;
  }
}


std::ostream& operator<<(std::ostream& os, 
                         const FlyingTrotFootStepPlanner& planner) {
  planner.disp(os);
  return os;
}


std::ostream& operator<<(std::ostream& os, 
                         const std::shared_ptr<FlyingTrotFootStepPlanner>& planner) {
  planner->disp(os);
  return os;
}

} // namespace robotoc 