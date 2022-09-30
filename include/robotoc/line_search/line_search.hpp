#ifndef ROBOTOC_LINE_SEARCH_HPP_
#define ROBOTOC_LINE_SEARCH_HPP_

#include <memory>

#include "Eigen/Core"

#include "robotoc/robot/robot.hpp"
#include "robotoc/utils/aligned_vector.hpp"
#include "robotoc/core/solution.hpp"
#include "robotoc/core/direction.hpp"
#include "robotoc/core/kkt_residual.hpp"
#include "robotoc/ocp/ocp_def.hpp"
#include "robotoc/ocp/direct_multiple_shooting.hpp"
#include "robotoc/line_search/line_search_filter.hpp"
#include "robotoc/line_search/line_search_settings.hpp"


namespace robotoc {

///
/// @class LineSearch 
/// @brief Line search for optimal control problems.
///
class LineSearch {
public:
  ///
  /// @brief Construct a line search.
  /// @param[in] ocp Optimal control problem. 
  /// @param[in] nthreads Number of the threads in solving the optimal control 
  /// problem. Must be positive. Default is 1.
  /// @param[in] line_search_settings Line search settings.
  ///
  LineSearch(const OCPDef& ocp, 
             const LineSearchSettings& line_search_settings=LineSearchSettings());

  ///
  /// @brief Default constructor. 
  ///
  LineSearch();

  ///
  /// @brief Default destructor. 
  ///
  ~LineSearch() = default;

  ///
  /// @brief Default copy constructor. 
  ///
  LineSearch(const LineSearch&) = default;

  ///
  /// @brief Default copy assign operator. 
  ///
  LineSearch& operator=(const LineSearch&) = default;

  ///
  /// @brief Default move constructor. 
  ///
  LineSearch(LineSearch&&) noexcept = default;

  ///
  /// @brief Default move assign operator. 
  ///
  LineSearch& operator=(LineSearch&&) noexcept = default;

  ///
  /// @brief Compute primal step size by fliter line search method. 
  /// @param[in, out] ocp optimal control problem.
  /// @param[in] robots aligned_vector of Robot.
  /// @param[in] contact_sequence Shared ptr to the contact sequence. 
  /// @param[in] q Initial configuration.
  /// @param[in] v Initial generalized velocity.
  /// @param[in] s Solution. 
  /// @param[in] d Direction. 
  /// @param[in] max_primal_step_size Maximum primal step size. 
  ///
  double computeStepSize(
      DirectMultipleShooting& dms, aligned_vector<Robot>& robots,
      const TimeDiscretization& time_discretization,
      const Eigen::VectorXd& q, const Eigen::VectorXd& v, const Solution& s, 
      const Direction& d, const double max_primal_step_size);

  ///
  /// @brief Clear the line search filter. 
  ///
  void clearFilter();

  ///
  /// @brief Checks wheather the line search filter is empty or not. 
  /// @return true if the filter is empty. false if not.
  ///
  bool isFilterEmpty() const;
  
  ///
  /// @brief Set line search settings.
  ///
  void set(const LineSearchSettings& settings);

  ///
  /// @brief Reserve the internal data. 
  /// @param[in] ocp Optimal control problem.
  ///
  void reserve(const TimeDiscretization& time_discretization);

private:
  LineSearchFilter filter_;
  LineSearchSettings settings_;
  Solution s_trial_;
  KKTResidual kkt_residual_;

  void computeCostAndViolation(
      OCP& ocp, aligned_vector<Robot>& robots, 
      const std::shared_ptr<ContactSequence>& contact_sequence, 
      const Eigen::VectorXd& q, const Eigen::VectorXd& v, const Solution& s);

  void computeSolutionTrial(const aligned_vector<Robot>& robots, 
                            const TimeDiscretization& time_discretization,
                            const Solution& s, const Direction& d, 
                            const double step_size);

  static void computeSolutionTrial(const Robot& robot, const SplitSolution& s, 
                                   const SplitDirection& d, 
                                   const double step_size, 
                                   SplitSolution& s_trial,
                                   const bool impulse=false) {
    s_trial.setContactStatus(s);
    robot.integrateConfiguration(s.q, d.dq(), step_size, s_trial.q);
    s_trial.v = s.v + step_size * d.dv();
    if (!impulse) {
      s_trial.a = s.a + step_size * d.da();
      s_trial.u = s.u + step_size * d.du;
    }
    else {
      s_trial.dv = s.dv + step_size * d.ddv();
    }
    if (s.hasActiveContacts()) {
      s_trial.f_stack() = s.f_stack() + step_size * d.df();
      s_trial.set_f_vector();
    }
  }

  double lineSearchFilterMethod(
      DirectMultipleShooting& dms, aligned_vector<Robot>& robots,
      const TimeDiscretization& time_discretization,
      const Eigen::VectorXd& q, const Eigen::VectorXd& v, const Solution& s, 
      const Direction& d, const double max_primal_step_size);

  double meritBacktrackingLineSearch(
      DirectMultipleShooting& dms, aligned_vector<Robot>& robots,
      const TimeDiscretization& time_discretization,
      const Eigen::VectorXd& q, const Eigen::VectorXd& v, const Solution& s, 
      const Direction& d, const double max_primal_step_size);

  bool armijoCond(const double merit_now, const double merit_next, 
                  const double dd, const double step_size, 
                  const double armijo_control_rate) const;

  double penaltyParam(const TimeDiscretization time_discretization, 
                      const Solution& s) const;

};

} // namespace robotoc 

#endif // ROBOTOC_LINE_SEARCH_HPP_ 