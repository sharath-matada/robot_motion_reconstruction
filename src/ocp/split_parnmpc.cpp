#include "idocp/ocp/split_parnmpc.hpp"

#include <assert.h>


namespace idocp {

SplitParNMPC::SplitParNMPC(const Robot& robot, 
                           const std::shared_ptr<CostFunction>& cost, 
                           const std::shared_ptr<Constraints>& constraints) 
  : cost_(cost),
    cost_data_(robot),
    constraints_(constraints),
    constraints_data_(),
    kkt_residual_(robot),
    kkt_matrix_(robot),
    contact_dynamics_(robot),
    backward_correction_(robot),
    dimv_(robot.dimv()),
    dimx_(2*robot.dimv()),
    dimKKT_(kkt_residual_.dimKKT()),
    kkt_matrix_inverse_(Eigen::MatrixXd::Zero(kkt_residual_.dimKKT(), 
                                              kkt_residual_.dimKKT())),
    x_res_(Eigen::VectorXd::Zero(2*robot.dimv())),
    dx_(Eigen::VectorXd::Zero(2*robot.dimv())),
    use_kinematics_(false) {
  if (cost_->useKinematics() || constraints_->useKinematics() 
                             || robot.max_dimf() > 0) {
    use_kinematics_ = true;
  }
}


SplitParNMPC::SplitParNMPC() 
  : cost_(),
    cost_data_(),
    constraints_(),
    constraints_data_(),
    kkt_residual_(),
    kkt_matrix_(),
    contact_dynamics_(),
    backward_correction_(),
    dimv_(0),
    dimx_(0),
    dimKKT_(0),
    kkt_matrix_inverse_(),
    x_res_(),
    dx_(),
    use_kinematics_(false) {
}


SplitParNMPC::~SplitParNMPC() {
}


bool SplitParNMPC::isFeasible(Robot& robot, const SplitSolution& s) {
  return constraints_->isFeasible(robot, constraints_data_, s);
}


void SplitParNMPC::initConstraints(Robot& robot, const int time_step, 
                                   const double dtau, const SplitSolution& s) {
  assert(time_step >= 0);
  assert(dtau > 0);
  constraints_data_ = constraints_->createConstraintsData(robot, time_step);
  constraints_->setSlackAndDual(robot, constraints_data_, dtau, s);
}


void SplitParNMPC::linearizeOCP(Robot& robot, 
                                const ContactStatus& contact_status,
                                const double t, const double dtau, 
                                const Eigen::VectorXd& q_prev, 
                                const Eigen::VectorXd& v_prev,
                                const SplitSolution& s,
                                const SplitSolution& s_next) {
  assert(dtau > 0);
  assert(q_prev.size() == robot.dimq());
  assert(v_prev.size() == robot.dimv());
  assert(aux_mat_next_old.rows() == 2*robot.dimv());
  assert(aux_mat_next_old.cols() == 2*robot.dimv());
  setContactStatusForKKT(contact_status);
  if (use_kinematics_) {
    robot.updateKinematics(s.q, s.v, s.a);
  }
  kkt_residual_.setZero();
  kkt_matrix_.setZero();
  cost_->computeStageCostDerivatives(robot, cost_data_, t, dtau, s, 
                                     kkt_residual_);
  constraints_->augmentDualResidual(robot, constraints_data_, dtau, s,
                                    kkt_residual_);
  stateequation::LinearizeBackwardEuler(robot, dtau, q_prev, v_prev, s, s_next, 
                                        kkt_matrix_, kkt_residual_);
  cost_->computeStageCostHessian(robot, cost_data_, t, dtau, s, kkt_matrix_);
  constraints_->condenseSlackAndDual(robot, constraints_data_, dtau, s, 
                                     kkt_matrix_, kkt_residual_);
  contact_dynamics_.condenseRobotDynamics(robot, contact_status, dtau, s, 
                                          kkt_matrix_, kkt_residual_);
}


void SplitParNMPC::coarseUpdate(const Eigen::MatrixXd& aux_mat_next_old,
                                SplitDirection& d, 
                                SplitSolution& s_new_coarse) {
  assert(aux_mat_next_old.rows() == 2*robot.dimv());
  assert(aux_mat_next_old.cols() == 2*robot.dimv());
  kkt_matrix_.Qxx().noalias() += aux_mat_next_old;
  kkt_matrix_.symmetrize(); 
  kkt_matrix_.invert(kkt_matrix_inverse_);
  d.split_direction = kkt_matrix_inverse_ * kkt_residual_.KKT_residual;
  s_new_coarse.lmd = s.lmd - d.dlmd();
  s_new_coarse.gmm = s.gmm - d.dgmm();
  s_new_coarse.q = s.q - d.dq();
  s_new_coarse.v = s.v - d.dv();
  s_new_coarse.u = s.u - d.du();
}


void SplitParNMPC::getAuxiliaryMatrix(Eigen::MatrixXd& aux_mat) const {
  assert(aux_mat.rows() == dimx_);
  assert(aux_mat.cols() == dimx_);
  aux_mat = - kkt_matrix_inverse_.topLeftCorner(dimx_, dimx_);
}


void SplitParNMPC::backwardCorrectionSerial(const Robot& robot,
                                            const SplitSolution& s_next, 
                                            const SplitSolution& s_new_next,
                                            SplitSolution& s_new) {
  x_res_.head(robot.dimv()) = s_new_next.lmd - s_next.lmd;
  x_res_.tail(robot.dimv()) = s_new_next.gmm - s_next.gmm;
  dx_.noalias() 
      = kkt_matrix_inverse_.block(0, dimKKT_-dimx_, dimx_, dimx_) * x_res_;
  s_new.lmd.noalias() -= dx_.head(robot.dimv());
  s_new.gmm.noalias() -= dx_.tail(robot.dimv());
}


void SplitParNMPC::backwardCorrectionParallel(const Robot& robot, 
                                              SplitDirection& d,
                                              SplitSolution& s_new) const {
  d.split_direction().tail(dimKKT_-dimx_).noalias()
      = kkt_matrix_inverse_.block(dimx_, dimKKT_-dimx_, dimKKT_-dimx_, dimx_) 
          * x_res_;
  s_new.u.noalias() -= d.du();
  robot.integrateConfiguration(d.dq(), -1, s_new.q);
  s_new.v.noalias() -= d.dv();
}


void SplitParNMPC::forwardCorrectionSerial(const Robot& robot,
                                           const SplitSolution& s_prev,
                                           const SplitSolution& s_new_prev, 
                                           SplitSolution& s_new) {
  robot.subtractConfiguration(s_new_prev.q, s_prev.q, 
                              x_res_.head(robot.dimv()));
  x_res_.tail(robot.dimv()) = s_new_prev.v - s_prev.v;
  dx_.noalias() = kkt_matrix_inverse_.block(dimKKT_-dimx_, 0, dimx_, dimx_) 
                    * x_res_;
  robot.integrateConfiguration(dx_.head(robot.dimv()), -1, s_new.q);
  s_new.v.noalias() -= dx_.tail(robot.dimv());
}


void SplitParNMPC::forwardCorrectionParallel(const Robot& robot,
                                             SplitDirection& d,
                                             SplitSolution& s_new) const {
  d.split_direction().head(dimKKT_-dimx_).noalias()
      = kkt_matrix_inverse_.topLeftCorner(dimKKT_-dimx_, dimx_) * x_res_;
  s_new.lmd.noalias() -= d.dlmd();
  s_new.gmm.noalias() -= d.dgmm();
  s_new.u.noalias() -= d.du();
}


void SplitParNMPC::computePrimalAndDualDirection(Robot& robot, 
                                                 const double dtau, 
                                                 const SplitSolution& s, 
                                                 const SplitSolution& s_new,
                                                 SplitDirection& d) {
  d.dlmd() = s_new.lmd - s.lmd;
  d.dgmm() = s_new.gmm - s.gmm;
  d.dmu() = s_new.mu_stack() - s.mu_stack();
  d.da() = s_new.a - s.a;
  d.df() = s_new.f_stack() - s.f_stack();
  robot.subtractConfiguration(s_new.q, s.q, d.dq());
  d.dv() = s_new.v - s.v;
  robot_dynamics_.computeCondensedDirection(dtau, kkt_matrix_, kkt_residual_, d);
  constraints_->computeSlackAndDualDirection(robot, constraints_data_, dtau, s, d);
}

 
double SplitParNMPC::maxPrimalStepSize() {
  return constraints_->maxSlackStepSize(constraints_data_);
}


double SplitParNMPC::maxDualStepSize() {
  return constraints_->maxDualStepSize(constraints_data_);
}


void SplitParNMPC::updateDual(const double step_size) {
  assert(step_size > 0);
  assert(step_size <= 1);
  constraints_->updateDual(constraints_data_, step_size);
}


void SplitParNMPC::updatePrimal(Robot& robot, const double step_size, 
                                const double dtau, const SplitDirection& d, 
                                SplitSolution& s) {
  assert(step_size > 0);
  assert(step_size <= 1);
  assert(dtau > 0);
  s.lmd.noalias() += step_size * d.dlmd();
  s.gmm.noalias() += step_size * d.dgmm();
  s.mu_stack().noalias() += step_size * d.dmu();
  s.set_mu_contact();
  s.a.noalias() += step_size * d.da();
  s.f_stack().noalias() += step_size * d.df();
  s.set_f();
  robot.integrateConfiguration(d.dq(), step_size, s.q);
  s.v.noalias() += step_size * d.dv();
  s.u.noalias() += step_size * d.du;
  s.beta.noalias() += step_size * d.dbeta;
  constraints_->updateSlack(constraints_data_, step_size);
}


void SplitParNMPC::computeKKTResidual(Robot& robot, 
                                      const ContactStatus& contact_status, 
                                      const double t, const double dtau, 
                                      const Eigen::VectorXd& q_prev, 
                                      const Eigen::VectorXd& v_prev, 
                                      const SplitSolution& s, 
                                      const SplitSolution& s_next) {
  assert(dtau > 0);
  assert(q_prev.size() == robot.dimq());
  assert(v_prev.size() == robot.dimv());
  setContactStatusForKKT(contact_status);
  kkt_residual_.setZeroMinimum();
  if (use_kinematics_) {
    robot.updateKinematics(s.q, s.v, s.a);
  }
  cost_->computeStageCostDerivatives(robot, cost_data_, t, dtau, s, 
                                     kkt_residual_);
  constraints_->augmentDualResidual(robot, constraints_data_, dtau, s,
                                    kkt_residual_);
  stateequation::LinearizeBackwardEuler(robot, dtau, q_prev, v_prev, s, s_next,
                                        kkt_matrix_, kkt_residual_);
  contact_dynamics_.linearizeContactDynamics(robot, contact_status, dtau, s, 
                                             kkt_matrix_, kkt_residual_);
}


double SplitParNMPC::squaredNormKKTResidual(const double dtau) const {
  double error = 0;
  error += kkt_residual_.lx().squaredNorm();
  error += kkt_residual_.la.squaredNorm();
  error += kkt_residual_.lf().squaredNorm();
  if (has_floating_base_) {
    error += kkt_residual_.lu_passive.squaredNorm();
  }
  error += kkt_residual_.lu().squaredNorm();
  error += stateequation::SquaredNormStateEuqationResidual(kkt_residual_);
  error += contact_dynamics_.squaredNormContactDynamicsResidual(dtau);
  error += constraints_->squaredNormPrimalAndDualResidual(constraints_data_);
  return error;
}


double SplitParNMPC::stageCost(Robot& robot, const double t, const double dtau, 
                               const SplitSolution& s,
                               const double primal_step_size) {
  if (use_kinematics_) {
    robot.updateKinematics(s.q, s.v, s.a);
  }
  double cost = 0;
  cost += cost_->l(robot, cost_data_, t, dtau, s);
  if (primal_step_size > 0) {
    cost += constraints_->costSlackBarrier(constraints_data_, primal_step_size);
  }
  else {
    cost += constraints_->costSlackBarrier(constraints_data_);
  }
  return cost;
}


double SplitParNMPC::constraintViolation(Robot& robot, 
                                         const ContactStatus& contact_status, 
                                         const double t, const double dtau, 
                                         const Eigen::VectorXd& q_prev, 
                                         const Eigen::VectorXd& v_prev,
                                         const SplitSolution& s) {
  setContactStatusForKKT(contact_status);
  if (use_kinematics_) {
    robot.updateKinematics(s.q, s.v, s.a);
  }
  constraints_->computePrimalAndDualResidual(robot, constraints_data_, dtau, s);
  stateequation::ComputeBackwardEulerResidual(robot, dtau, q_prev, v_prev, s, 
                                              kkt_residual_);
  contact_dynamics_.computeContactDynamicsResidual(robot, contact_status, dtau, s);
  double violation = 0;
  violation += constraints_->l1NormPrimalResidual(constraints_data_);
  violation += stateequation::L1NormStateEuqationResidual(kkt_residual_);
  violation += contact_dynamics_.l1NormContactDynamicsResidual(dtau);
  return violation;
}


void SplitParNMPC::getStateFeedbackGain(Eigen::MatrixXd& Kq, 
                                        Eigen::MatrixXd& Kv) const {
  assert(Kq.rows() == riccati_gain_.Kq().rows());
  assert(Kq.cols() == riccati_gain_.Kq().cols());
  assert(Kv.rows() == riccati_gain_.Kv().rows());
  assert(Kv.cols() == riccati_gain_.Kv().cols());
  // if (fd_like_elimination_) {
  //   Kq = riccati_gain_.Kq();
  //   Kv = riccati_gain_.Kv();
  // }
  // else {
  //   // robot_dynamics_.getStateFeedbackGain(riccati_gain_.Kq(), riccati_gain_.Kv(), 
  //   //                                      Kq, Kv);
  // }
}

} // namespace idocp