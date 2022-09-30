#include "robotoc/robot/impulse_status.hpp"


namespace robotoc {

void ImpulseStatus::disp(std::ostream& os) const {
  os << "ImpulseStatus:" << "\n";
  os << "  impulse mode id: " << contact_status_.contactModeId() << "\n";
  os << "  active impulses: [";
  for (int i=0; i<maxNumContacts()-1; ++i) {
    if (isImpulseActive(i)) {
      os << i << ", ";
    }
  }
  if (isImpulseActive(maxNumContacts()-1)) {
    os << maxNumContacts()-1;
  }
  os << "]" << "\n";
  os << "  contact positions: [";
  for (int i=0; i<maxNumContacts()-1; ++i) {
    os << "[" << contactPosition(i).transpose() << "], ";
  }
  os << "[" << contactPosition(maxNumContacts()-1).transpose() << "]";
  os << "]" << "\n";
  os << "  contact rotations: [";
  for (int i=0; i<maxNumContacts()-1; ++i) {
    os << "[" << contactRotation(i).row(0) << "]  ";
  }
  os << "[" << contactRotation(maxNumContacts()-1).row(0) << "]" << "\n";
  os << "                               ";
  for (int i=0; i<maxNumContacts()-1; ++i) {
    os << "[" << contactRotation(i).row(1) << "]  ";
  }
  os << "[" << contactRotation(maxNumContacts()-1).row(1) << "]" << "\n";
  os << "                               ";
  for (int i=0; i<maxNumContacts()-1; ++i) {
    os << "[" << contactRotation(i).row(2) << "], ";
  }
  os << "[" << contactRotation(maxNumContacts()-1).row(2) << "]";
  os << "]" << "\n";
  os << "  friction coefficients: [";
  for (int i=0; i<maxNumContacts()-1; ++i) {
    os << "[" << frictionCoefficient(i) << "], ";
  }
  os << "[" << frictionCoefficient(maxNumContacts()-1) << "]";
  os << "]" << std::flush;
}


std::ostream& operator<<(std::ostream& os, 
                         const ImpulseStatus& impulse_status) {
  impulse_status.disp(os);
  return os;
}

} // namespace robotoc 