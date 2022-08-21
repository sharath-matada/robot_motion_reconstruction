#ifndef ROBOTOC_CONTACT_SEQUENCE_HPP_
#define ROBOTOC_CONTACT_SEQUENCE_HPP_ 

#include <deque>
#include <iostream>
#include <memory>

#include "robotoc/robot/robot.hpp"
#include "robotoc/robot/se3.hpp"
#include "robotoc/utils/aligned_vector.hpp"
#include "robotoc/robot/contact_status.hpp"
#include "robotoc/robot/impulse_status.hpp"
#include "robotoc/hybrid/discrete_event.hpp"


namespace robotoc {

///
/// @class ContactSequence
/// @brief The sequence of contact status and discrete events (impulse and lift). 
///
class ContactSequence {
public:
  ///
  /// @brief Constructor. 
  /// @param[in] robot Robot model. 
  /// @param[in] reserved_num_discrete_events Reserved number of each discrete 
  /// events (impulse and lift) to avoid dynamic memory allocation. Must be 
  /// non-negative. Default is 0.
  ///
  ContactSequence(const Robot& robot, const int reserved_num_discrete_events=0);

  ///
  /// @brief Default constructor. 
  ///
  ContactSequence();

  ///
  /// @brief Destructor. 
  ///
  ~ContactSequence();

  ///
  /// @brief Default copy constructor. 
  ///
  ContactSequence(const ContactSequence&) = default;

  ///
  /// @brief Default copy assign operator. 
  ///
  ContactSequence& operator=(const ContactSequence&) = default;

  ///
  /// @brief Default move constructor. 
  ///
  ContactSequence(ContactSequence&&) noexcept = default;

  ///
  /// @brief Default move assign operator. 
  ///
  ContactSequence& operator=(ContactSequence&&) noexcept = default;

  ///
  /// @brief Sets the contact status over all of the time stages uniformly. Also, 
  /// disable discrete events over all of the time stages.
  /// @param[in] contact_status Contact status.
  ///
  void init(const ContactStatus& contact_status);

  ///
  /// @brief Push back the discrete event. Contact status after discrete event 
  /// is also appended according to discrete_event. 
  /// @param[in] discrete_event Discrete event.
  /// @param[in] event_time Time of the discrete event.
  /// @param[in] sto if true, the switching time optimization (STO) is enabled
  /// for this discrete event. if false, it is disabled. Default is false.
  /// @note If event_time is larger than the terminal time of the optimal 
  /// control problem (t+T), then the discrete event and the contact status
  /// after the discrete event is not considered in the optimization problem.
  ///
  void push_back(const DiscreteEvent& discrete_event, const double event_time,
                 const bool sto=false);

  ///
  /// @brief Push back the contact sequence. A discrete event is automatically 
  /// generated by last contact status of this contact sequence and input 
  /// contact_status.
  /// @param[in] contact_status Contact status.
  /// @param[in] switching_time Time of the switch of the last contact status of 
  /// this contact sequence and the input contact status. 
  /// @param[in] sto if true, the switching time optimization (STO) is enabled
  /// for this discrete event. if false, it is disabled. Default is false.
  /// @note If switching_time is larger than the terminal time of the optimal 
  /// control problem (t+T), then the discrete event and the contact status
  /// after the discrete event is not considered in the optimization problem.
  ///
  void push_back(const ContactStatus& contact_status, 
                 const double switching_time, const bool sto=false);

  ///
  /// @brief Pop back the discrete event. Contact status after discrete event 
  /// is also removed. 
  ///
  void pop_back();

  ///
  /// @brief Pop front the discrete event. Contact status before the front 
  /// discrete event is also removed. 
  ///
  void pop_front();

  ///
  /// @brief Sets the time of the impulse event. 
  /// @param[in] impulse_index Index of the impulse event. Must be non-negative
  /// and less than numImpulseEvents().
  /// @param[in] impulse_time Impulse time.
  ///
  void setImpulseTime(const int impulse_index, const double impulse_time);

  ///
  /// @brief Sets the time of the lift event. 
  /// @param[in] lift_index Index of the lift event. Must be non-negative
  /// and less than numLiftEvents().
  /// @param[in] lift_time Lift time.
  ///
  void setLiftTime(const int lift_index, const double lift_time);

  ///
  /// @brief Checks wheather the STO is enabled for the specified impulse event. 
  /// @param[in] impulse_index Index of the impulse of interest. 
  /// @return true if the STO is enabled. false if not.
  ///
  bool isSTOEnabledImpulse(const int impulse_index) const;

  ///
  /// @brief Checks wheather the STO is enabled for the specified lift event. 
  /// @param[in] lift_index Index of the lift of interest. 
  /// @return true if the STO is enabled. false if not.
  ///
  bool isSTOEnabledLift(const int lift_index) const;

  ///
  /// @brief Checks wheather the event times are consistent. 
  /// @return true if there is no problem. false if not.
  /// 
  bool isEventTimeConsistent() const;

  ///
  /// @brief Sets the contact placements (positions and rotations) to contact 
  /// statsus with specified contact phase. The rotations are set to 
  /// Eigen::Matrix3d::Identity(). Also sets the contact placement of 
  /// the discrete event just before the contact phase.
  /// @param[in] contact_phase Contact phase.
  /// @param[in] contact_positions Contact positions.
  ///
  void setContactPlacements(
      const int contact_phase, 
      const std::vector<Eigen::Vector3d>& contact_positions);

  ///
  /// @brief Sets the contact placements (positions and rotations) to contact 
  /// statsus with specified contact phase. Also set the contact placement of 
  /// the discrete event just before the contact phase.
  /// @param[in] contact_phase Contact phase.
  /// @param[in] contact_positions Contact positions.
  /// @param[in] contact_rotations Contact rotations.
  ///
  void setContactPlacements(
      const int contact_phase, 
      const std::vector<Eigen::Vector3d>& contact_positions,
      const std::vector<Eigen::Matrix3d>& contact_rotations);

  ///
  /// @brief Sets the contact placements (positions and rotations) to contact 
  /// statsus with specified contact phase. Also set the contact placement of 
  /// the discrete event just before the contact phase.
  /// @param[in] contact_phase Contact phase.
  /// @param[in] contact_placements Contact placements.
  ///
  void setContactPlacements(const int contact_phase, 
                            const aligned_vector<SE3>& contact_placements);

  ///
  /// @brief Sets the friction coefficients. Also sets the friction coefficients 
  /// of the discrete event just before the contact phase.
  /// @param[in] contact_phase Contact phase.
  /// @param[in] friction_coefficients Friction coefficients.
  ///
  void setFrictionCoefficients(const int contact_phase, 
                               const std::vector<double>& friction_coefficients);

  ///
  /// @brief Returns number of impulse events. 
  /// @return Number of impulse events.
  ///
  int numImpulseEvents() const;

  ///
  /// @brief Returns number of lift events. 
  /// @return Number of lift events.
  ///
  int numLiftEvents() const;

  ///
  /// @brief Returns number of discrete events, i.e., sum of 
  /// numImpulseEvents() and numLiftEvents().
  /// @return Number of discrete events.
  ///
  int numDiscreteEvents() const;

  ///
  /// @brief Returns number of contact phases. 
  /// @return Number of contact phases.
  ///
  int numContactPhases() const;

  ///
  /// @brief Gets the contact status. 
  /// @param[in] contact_phase Index of contact status phase.
  /// @return const reference to the contact status.
  ///
  const ContactStatus& contactStatus(const int contact_phase) const;

  ///
  /// @brief Gets the impulse status. 
  /// @param[in] impulse_index Index of impulse event.
  /// @return const reference to the impulse status.
  ///
  const ImpulseStatus& impulseStatus(const int impulse_index) const;

  ///
  /// @brief Returns impulse event time. 
  /// @return Impulse event time.
  ///
  double impulseTime(const int impulse_index) const;

  ///
  /// @brief Returns lift event time. 
  /// @return Lift event time.
  ///
  double liftTime(const int lift_index) const;

  ///
  /// @brief Returns the event type of the specified discrete event. 
  /// @param[in] event_index Index of the discrete event. Must be less than
  /// ContactSequence::numDiscreteEvents().
  /// @return The event type of the specified discrete event.
  ///
  DiscreteEventType eventType(const int event_index) const;

  ///
  /// @brief Returns the event times of each event. 
  /// @return const reference to the event times.
  ///
  const std::deque<double>& eventTimes() const;

  ///
  /// @brief Reserves each discrete events (impulse and lift) to avoid dynamic 
  /// memory allocation.
  /// @param[in] reserved_num_discrete_events The reserved size.
  ///
  void reserve(const int reserved_num_discrete_events);

  ///
  /// @brief Returns reserved size of container of each discrete events.
  ///
  int reservedNumDiscreteEvents() const;

  ///
  /// @brief Displays the contact sequence onto a ostream.
  ///
  void disp(std::ostream& os) const;

  friend std::ostream& operator<<(std::ostream& os, 
                                  const ContactSequence& contact_sequence);

  friend std::ostream& operator<<(
      std::ostream& os, 
      const std::shared_ptr<ContactSequence>& contact_sequence);

private:
  int reserved_num_discrete_events_;
  ContactStatus default_contact_status_;
  std::deque<ContactStatus> contact_statuses_;
  std::deque<DiscreteEvent> impulse_events_;
  std::deque<int> event_index_impulse_, event_index_lift_;
  std::deque<double> event_time_, impulse_time_, lift_time_;
  std::deque<bool> is_impulse_event_, sto_impulse_, sto_lift_;

  void clear_all();

  template <typename T> 
  static void reserveDeque(std::deque<T>& deq, const int size) {
    if (deq.empty()) {
      deq.resize(size);
    }
    else {
      const int current_size = deq.size();
      if (current_size < size) {
        while (deq.size() < size) {
          deq.push_back(deq.back());
        }
        while (deq.size() > current_size) {
          deq.pop_back();
        }
      }
    }
  }

};
 
} // namespace robotoc 

#include "robotoc/hybrid/contact_sequence.hxx"

#endif // ROBOTOC_CONTACT_SEQUENCE_HPP_ 