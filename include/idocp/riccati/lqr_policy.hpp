#ifndef IDOCP_LQR_POLICY_HPP_
#define IDOCP_LQR_POLICY_HPP_

#include "Eigen/Core"

#include "idocp/robot/robot.hpp"


namespace idocp {

///
/// @class LQRPolicy
/// @brief The state feedback and feedforward policy of LQR subproblem at 
/// a time stage.
///
class LQRPolicy {
public:
  using MatrixXdRowMajor 
      = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

  ///
  /// @brief Constructs LQR gain and feedforward term.
  /// @param[in] robot Robot model. 
  ///
  LQRPolicy(const Robot& robot)
    : K(MatrixXdRowMajor::Zero(robot.dimu(), 2*robot.dimv())),
      k(Eigen::VectorXd::Zero(robot.dimu())),
      dimv_(robot.dimv()),
      dimu_(robot.dimu()) {
  }

  ///
  /// @brief Default constructor. 
  ///
  LQRPolicy() 
    : K(),
      k(),
      dimv_(0),
      dimu_(0) {
  }

  ///
  /// @brief Destructor. 
  ///
  ~LQRPolicy() {
  }

  ///
  /// @brief Default copy constructor. 
  ///
  LQRPolicy(const LQRPolicy&) = default;

  ///
  /// @brief Default copy operator. 
  ///
  LQRPolicy& operator=(const LQRPolicy&) = default;

  ///
  /// @brief Default move constructor. 
  ///
  LQRPolicy(LQRPolicy&&) noexcept = default;

  ///
  /// @brief Default move assign operator. 
  ///
  LQRPolicy& operator=(LQRPolicy&&) noexcept = default;

  ///
  /// @brief State feedback gain matrix. Size is 
  /// Robot::dimu() x 2 * Robot::dimv().
  ///
  MatrixXdRowMajor K;

  ///
  /// @brief Feedforward term. Size is Robot::dimu().
  ///
  Eigen::VectorXd k;

  ///
  /// @brief State feedback gain matrix w.r.t. the configuration q. Size is 
  /// Robot::dimu() x Robot::dimv().
  /// @return const reference to the gain matrix.
  ///
  const Eigen::Block<const MatrixXdRowMajor> Kq() const {
    return K.topLeftCorner(dimu_, dimv_);
  }

  ///
  /// @brief State feedback gain matrix w.r.t. the velocity v. Size is 
  /// Robot::dimu() x Robot::dimv().
  /// @return const reference to the gain matrix.
  ///
  const Eigen::Block<const MatrixXdRowMajor> Kv() const {
    return K.topRightCorner(dimu_, dimv_);
  }

  ///
  /// @brief Checks the equivalence of two LQRPolicy.
  /// @param[in] other object.
  /// @return true if this and other is same. false otherwise.
  ///
  bool isApprox(const LQRPolicy& other) const {
    if (!K.isApprox(other.K)) return false;
    if (!k.isApprox(other.k)) return false;
    return true;
  }

private:
  int dimv_, dimu_;

};

} // namespace idocp 

#endif // IDOCP_LQR_POLICY_HPP_ 