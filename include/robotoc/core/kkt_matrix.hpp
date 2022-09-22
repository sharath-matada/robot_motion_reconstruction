#ifndef ROBOTOC_KKT_MATRIX_HPP_
#define ROBOTOC_KKT_MATRIX_HPP_

#include <iostream>

#include "robotoc/core/split_kkt_matrix.hpp"
#include "robotoc/core/switching_constraint_jacobian.hpp"
#include "robotoc/core/hybrid_container.hpp"


namespace robotoc {

///
/// @typedef KKTMatrix 
/// @brief The KKT matrix of the (hybrid) optimal control problem. 
///
using KKTMatrix = hybrid_container<SplitKKTMatrix, SplitKKTMatrix, 
                                   SwitchingConstraintJacobian>;

std::ostream& operator<<(std::ostream& os, const KKTMatrix& kkt_matrix);

} // namespace robotoc

#endif // ROBOTOC_KKT_MATRIX_HPP_ 