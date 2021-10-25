#ifndef ROBOTOC_CONTACT_SEQUENCE_HPP_
#define ROBOTOC_CONTACT_SEQUENCE_HPP_ 

#include <deque>
#include <iostream>
#include <memory>

#include "robotoc/robot/robot.hpp"
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
  /// @param[in] max_num_each_events Maximum number of each discrete events 
  /// (impulse and lift). Default is 0 (assumes that there are no discrete events).
  ///
  ContactSequence(const Robot& robot, const int max_num_each_events=0);

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
  void initContactSequence(const ContactStatus& contact_status);

  ///
  /// @brief Push back the discrete event. Contact status after discrete event 
  /// is also appended according to discrete_event. 
  /// @param[in] discrete_event Discrete event.
  /// @param[in] event_time Time of the discrete event.
  /// @param[in] sto if true, the switching time optimization (STO) is enabled
  /// for this discrete event. if false, it is disabled. Default is false.
  ///
  void push_back(const DiscreteEvent& discrete_event, const double event_time,
                 const bool sto=false);

  ///
  /// @brief Push back the discrete event that is automatically generated by
  /// last contact status of this object and contact_status.
  /// @param[in] contact_status Contact status.
  /// @param[in] event_time Time of the discrete event.
  /// @param[in] sto if true, the switching time optimization (STO) is enabled
  /// for this discrete event. if false, it is disabled. Default is false.
  ///
  void push_back(const ContactStatus& contact_status, const double event_time,
                 const bool sto=false);

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
  /// @brief Sets the contact points to contact statsus with specified contact  
  /// phase. Also set the contact points of the discrete event just before the  
  /// contact phase.
  /// @param[in] contact_phase Contact phase.
  /// @param[in] contact_points Contact points.
  ///
  void setContactPoints(const int contact_phase, 
                        const std::vector<Eigen::Vector3d>& contact_points);

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
  /// @brief Returns maximum number of each discrete events 
  /// (impulse and lift). 
  ///
  int maxNumEachEvents() const;

  ///
  /// @brief Returns maximum number of discrete events (sum of the maximum 
  /// numbers of impulse events and lift events).
  ///
  int maxNumEvents() const;

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
  int max_num_each_events_, max_num_events_;
  ContactStatus default_contact_status_;
  std::deque<ContactStatus> contact_statuses_;
  std::deque<DiscreteEvent> impulse_events_;
  std::deque<int> event_index_impulse_, event_index_lift_;
  std::deque<double> event_time_, impulse_time_, lift_time_;
  std::deque<bool> is_impulse_event_, sto_impulse_, sto_lift_;

  void clear_all();

};
 
} // namespace robotoc 

#include "robotoc/hybrid/contact_sequence.hxx"

#endif // ROBOTOC_CONTACT_SEQUENCE_HPP_ 