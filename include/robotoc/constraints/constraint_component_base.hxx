#ifndef ROBOTOC_CONSTRAINT_COMPONENT_BASE_HXX_
#define ROBOTOC_CONSTRAINT_COMPONENT_BASE_HXX_

#include "robotoc/constraints/pdipm.hpp"
#include <cassert>
#include <stdexcept>
#include <iostream>


namespace robotoc {

inline ConstraintComponentBase::ConstraintComponentBase(
    const double barrier_param, const double fraction_to_boundary_rule) 
  : barrier_(barrier_param),
    fraction_to_boundary_rule_(fraction_to_boundary_rule) {
  if (barrier_param <= 0) {
    throw std::out_of_range(
        "[ConstraintComponentBase] invalid argment: 'barrier_param' must be positive!");
  }
  if (fraction_to_boundary_rule <= 0) {
    throw std::out_of_range(
        "[ConstraintComponentBase] invalid argment: 'fraction_to_boundary_rule' must be positive!");
  }
  if (fraction_to_boundary_rule >= 1) {
    throw std::out_of_range(
        "[ConstraintComponentBase] invalid argment: 'fraction_to_boundary_rule' must be less than 1!");
  }
}


inline double ConstraintComponentBase::maxSlackStepSize(
    const ConstraintComponentData& data) const {
  return pdipm::fractionToBoundarySlack(fraction_to_boundary_rule_, data);
}


inline double ConstraintComponentBase::maxDualStepSize(
    const ConstraintComponentData& data) const {
  return pdipm::fractionToBoundaryDual(fraction_to_boundary_rule_, data);
}


inline void ConstraintComponentBase::updateSlack(ConstraintComponentData& data, 
                                                 const double step_size) {
  assert(step_size > 0);
  data.slack.noalias() += step_size * data.dslack;
}


inline void ConstraintComponentBase::updateDual(ConstraintComponentData& data, 
                                                const double step_size) {
  assert(step_size > 0);
  data.dual.noalias() += step_size * data.ddual;
}


inline double ConstraintComponentBase::getBarrierParam() const {
  return barrier_;
}


inline double ConstraintComponentBase::getFractionToBoundaryRule() const {
  return fraction_to_boundary_rule_;
}


inline void ConstraintComponentBase::setBarrierParam(const double barrier_param) {
  if (barrier_param <= 0) {
    throw std::out_of_range(
        "[ConstraintComponentBase] invalid argment: 'barrier_param' must be positive");
  }
  barrier_ = barrier_param;
}


inline void ConstraintComponentBase::setFractionToBoundaryRule(
    const double fraction_to_boundary_rule) {
  if (fraction_to_boundary_rule <= 0) {
    throw std::out_of_range(
        "[ConstraintComponentBase] invalid argment: 'fraction_to_boundary_rule' must be positive");
  }
  if (fraction_to_boundary_rule >= 1) {
    throw std::out_of_range(
        "[ConstraintComponentBase] invalid argment: 'fraction_to_boundary_rule' must be less than 1");
  }
  fraction_to_boundary_rule_ = fraction_to_boundary_rule;
}


inline void ConstraintComponentBase::setSlackAndDualPositive(
    ConstraintComponentData& data) const {
  pdipm::setSlackAndDualPositive(barrier_, data);
}


inline void ConstraintComponentBase::computeComplementarySlackness(
    ConstraintComponentData& data) const {
  pdipm::computeComplementarySlackness(barrier_, data);
}


inline void ConstraintComponentBase::computeComplementarySlackness(
    ConstraintComponentData& data, const int start, const int size) const {
  pdipm::computeComplementarySlackness(barrier_, data, start, size);
}


template <int Size>
inline void ConstraintComponentBase::computeComplementarySlackness(
    ConstraintComponentData& data, const int start) const {
  pdipm::computeComplementarySlackness<Size>(barrier_, data, start);
}


inline double ConstraintComponentBase::computeComplementarySlackness(
    const double slack, const double dual) const {
  return pdipm::computeComplementarySlackness(barrier_, slack, dual);
}


inline void ConstraintComponentBase::computeCondensingCoeffcient(
    ConstraintComponentData& data) {
  pdipm::computeCondensingCoeffcient(data);
}


inline void ConstraintComponentBase::computeCondensingCoeffcient(
    ConstraintComponentData& data, const int start, const int size) {
  pdipm::computeCondensingCoeffcient(data, start, size);
}


template <int Size>
inline void ConstraintComponentBase::computeCondensingCoeffcient(
    ConstraintComponentData& data, const int start) {
  pdipm::computeCondensingCoeffcient<Size>(data, start);
}


inline double ConstraintComponentBase::computeCondensingCoeffcient(
    const double slack, const double dual, const double residual, 
    const double cmpl) {
  return pdipm::computeCondensingCoeffcient(slack, dual, residual, cmpl);
}


inline void ConstraintComponentBase::computeDualDirection(
    ConstraintComponentData& data) {
  pdipm::computeDualDirection(data);
}


inline void ConstraintComponentBase::computeDualDirection(
    ConstraintComponentData& data, const int start, const int size) {
  pdipm::computeDualDirection(data, start, size);
}


template <int Size>
inline void ConstraintComponentBase::computeDualDirection(
    ConstraintComponentData& data, const int start) {
  pdipm::computeDualDirection<Size>(data, start);
}


inline double ConstraintComponentBase::computeDualDirection(
    const double slack, const double dual, const double dslack, 
    const double cmpl) {
  return pdipm::computeDualDirection(slack, dual, dslack, cmpl);
}


template <typename VectorType>
inline double ConstraintComponentBase::logBarrier(
    const Eigen::MatrixBase<VectorType>& slack) const {
  return pdipm::logBarrier(barrier_, slack);
}

} // namespace robotoc

#endif // ROBOTOC_CONSTRAINT_COMPONENT_BASE_HXX_