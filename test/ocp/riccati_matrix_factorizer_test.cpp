#include <string>

#include <gtest/gtest.h>
#include "Eigen/Core"
#include "pinocchio/multibody/model.hpp"
#include "pinocchio/multibody/data.hpp"
#include "pinocchio/parsers/urdf.hpp"
#include "pinocchio/algorithm/joint-configuration.hpp"
#include "pinocchio/algorithm/crba.hpp"
#include "pinocchio/algorithm/rnea.hpp"
#include "pinocchio/algorithm/rnea-derivatives.hpp"

#include "robot/robot.hpp"
#include "ocp/riccati_matrix_factorizer.hpp"


namespace idocp {

class RiccatiMatrixFactorierTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    srand((unsigned int) time(0));
    std::random_device rnd;
    fixed_base_urdf_ = "../../urdf/iiwa14/iiwa14.urdf";
    floating_base_urdf_ = "../../urdf/anymal/anymal.urdf";
    fixed_base_robot_ = Robot(fixed_base_urdf_);
    floating_base_robot_ = Robot(floating_base_urdf_);
    dtau_ = std::abs(Eigen::VectorXd::Random(1)[0]);
  }

  virtual void TearDown() {
  }

  double dtau_;
  std::string fixed_base_urdf_, floating_base_urdf_;
  Robot fixed_base_robot_, floating_base_robot_;
};


TEST_F(RiccatiMatrixFactorierTest, fixed_base) {
  const int dimv = fixed_base_robot_.dimv();
  RiccatiMatrixFactorizer factorizer(fixed_base_robot_);
  Eigen::MatrixXd Pqq = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Pqv = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Pvq = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Pvv = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Qqq = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Qqv = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Qvq = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Qvv = Eigen::MatrixXd::Random(dimv, dimv);
  const Eigen::MatrixXd Qqq_ref = Qqq + Pqq;
  const Eigen::MatrixXd Qqv_ref = Qqv + dtau_ * Pqq + Pqv;
  const Eigen::MatrixXd Qvq_ref = Qvq + dtau_ * Pqq + Pvq;
  const Eigen::MatrixXd Qvv_ref = Qvv + dtau_ * dtau_ * Pqq 
                                      + dtau_ * (Pqv + Pvq) + Pvv;
  factorizer.factorize(dtau_, Pqq, Pqv, Pvq, Pvv, Qqq, Qqv, Qvq, Qvv);
  EXPECT_TRUE(Qqq.isApprox(Qqq_ref));
  EXPECT_TRUE(Qqv.isApprox(Qqv_ref));
  EXPECT_TRUE(Qvq.isApprox(Qvq_ref));
  EXPECT_TRUE(Qvv.isApprox(Qvv_ref));
  std::cout << "Qqq error:" << std::endl;
  std::cout << Qqq - Qqq_ref << std::endl;
  std::cout << std::endl;
  std::cout << "Qqv error:" << std::endl;
  std::cout << Qqv - Qqv_ref << std::endl;
  std::cout << std::endl;
  std::cout << "Qvq error:" << std::endl;
  std::cout << Qvq - Qvq_ref << std::endl;
  std::cout << std::endl;
  std::cout << "Qvv error:" << std::endl;
  std::cout << Qvv - Qvv_ref << std::endl;
  std::cout << std::endl;

  Eigen::MatrixXd Qqa = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Qva = Eigen::MatrixXd::Random(dimv, dimv);
  const Eigen::MatrixXd Qqa_ref = Qqa + dtau_ * Pqv;
  const Eigen::MatrixXd Qva_ref = Qva + dtau_ * dtau_ * Pqv + dtau_ * Pvv;
  factorizer.factorize(dtau_, Pqv, Pvv, Qqa, Qva);
  EXPECT_TRUE(Qqa.isApprox(Qqa_ref));
  EXPECT_TRUE(Qva.isApprox(Qva_ref));
  std::cout << "Qqa error:" << std::endl;
  std::cout << Qqa - Qqa_ref << std::endl;
  std::cout << std::endl;
  std::cout << "Qva error:" << std::endl;
  std::cout << Qva - Qva_ref << std::endl;
  std::cout << std::endl;

  Eigen::MatrixXd Qaa = Eigen::MatrixXd::Random(dimv, dimv);
  const Eigen::MatrixXd Qaa_ref = Qaa + dtau_ * dtau_ * Pvv;
  factorizer.factorize(dtau_, Pvv, Qaa);
  EXPECT_TRUE(Qaa.isApprox(Qaa_ref));
  std::cout << "Qaa error:" << std::endl;
  std::cout << Qaa - Qaa_ref << std::endl;
  std::cout << std::endl;
}


TEST_F(RiccatiMatrixFactorierTest, floating_base) {
  const int dimq = floating_base_robot_.dimq();
  const int dimv = floating_base_robot_.dimv();
  RiccatiMatrixFactorizer factorizer(floating_base_robot_);
  Eigen::MatrixXd Pqq = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Pqv = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Pvq = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Pvv = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Qqq = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Qqv = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Qvq = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Qvv = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::VectorXd q = Eigen::VectorXd::Zero(dimq);
  floating_base_robot_.generateRandomConfiguration(-Eigen::VectorXd::Ones(dimq), 
                                                   Eigen::VectorXd::Ones(dimq), 
                                                   q);
  Eigen::VectorXd v = Eigen::VectorXd::Random(dimv);
  Eigen::MatrixXd dintagrate_dq = Eigen::MatrixXd::Zero(dimv, dimv);
  Eigen::MatrixXd dintagrate_dv = Eigen::MatrixXd::Zero(dimv, dimv);
  floating_base_robot_.dIntegrateConfiguration(q, v, dtau_, dintagrate_dq,  
                                               dintagrate_dv);
  const Eigen::MatrixXd Qqq_ref 
      = Qqq + dintagrate_dq.transpose() * Pqq * dintagrate_dq;
  const Eigen::MatrixXd Qqv_ref
      = Qqv + dtau_ * dintagrate_dq.transpose() * Pqq * dintagrate_dv
            + dintagrate_dq.transpose() * Pqv;
  const Eigen::MatrixXd Qvq_ref
      = Qvq + dtau_ * dintagrate_dv.transpose() * Pqq * dintagrate_dq
            + Pvq * dintagrate_dq;
  const Eigen::MatrixXd Qvv_ref
      = Qvv + (dtau_*dtau_) * dintagrate_dv.transpose() * Pqq * dintagrate_dv
            + dtau_ * Pvq * dintagrate_dv 
            + dtau_ * dintagrate_dv.transpose() * Pqv + Pvv;
  factorizer.computeIntegrationSensitivities(floating_base_robot_, dtau_, q, v);
  factorizer.factorize(dtau_, Pqq, Pqv, Pvq, Pvv, Qqq, Qqv, Qvq, Qvv);
  EXPECT_TRUE(Qqq.isApprox(Qqq_ref));
  EXPECT_TRUE(Qqv.isApprox(Qqv_ref));
  EXPECT_TRUE(Qvq.isApprox(Qvq_ref));
  EXPECT_TRUE(Qvv.isApprox(Qvv_ref));
  std::cout << "Qqq error:" << std::endl;
  std::cout << Qqq - Qqq_ref << std::endl;
  std::cout << std::endl;
  std::cout << "Qqv error:" << std::endl;
  std::cout << Qqv - Qqv_ref << std::endl;
  std::cout << std::endl;
  std::cout << "Qvq error:" << std::endl;
  std::cout << Qvq - Qvq_ref << std::endl;
  std::cout << std::endl;
  std::cout << "Qvv error:" << std::endl;
  std::cout << Qvv - Qvv_ref << std::endl;
  std::cout << std::endl;

  Eigen::MatrixXd Qqa = Eigen::MatrixXd::Random(dimv, dimv);
  Eigen::MatrixXd Qva = Eigen::MatrixXd::Random(dimv, dimv);
  const Eigen::MatrixXd Qqa_ref 
      = Qqa + dtau_ * dintagrate_dq.transpose() * Pqv;
  const Eigen::MatrixXd Qva_ref 
      = Qva + dtau_ * dtau_ * dintagrate_dv.transpose() * Pqv + dtau_ * Pvv;
  factorizer.factorize(dtau_, Pqv, Pvv, Qqa, Qva);
  EXPECT_TRUE(Qqa.isApprox(Qqa_ref));
  EXPECT_TRUE(Qva.isApprox(Qva_ref));
  std::cout << "Qqa error:" << std::endl;
  std::cout << Qqa - Qqa_ref << std::endl;
  std::cout << std::endl;
  std::cout << "Qva error:" << std::endl;
  std::cout << Qva - Qva_ref << std::endl;
  std::cout << std::endl;

  Eigen::MatrixXd Qaa = Eigen::MatrixXd::Random(dimv, dimv);
  const Eigen::MatrixXd Qaa_ref = Qaa + dtau_ * dtau_ * Pvv;
  factorizer.factorize(dtau_, Pvv, Qaa);
  EXPECT_TRUE(Qaa.isApprox(Qaa_ref));
  std::cout << "Qaa error:" << std::endl;
  std::cout << Qaa - Qaa_ref << std::endl;
  std::cout << std::endl;
}


} // namespace idocp


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}