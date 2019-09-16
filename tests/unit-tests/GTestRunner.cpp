#include <gtest/gtest.h>

class NoOutputListener : public ::testing::EmptyTestEventListener {
  // Called before a test starts.
  virtual void OnTestStart(const ::testing::TestInfo &test_info) {
  }

  // Called after a failed assertion or a SUCCESS().
  virtual void OnTestPartResult(const ::testing::TestPartResult &test_part_result) {
  }

  virtual void OnTestEnd(const ::testing::TestInfo &test_info) {
  }
};

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
//  ::testing::TestEventListeners& listeners =
//      ::testing::UnitTest::GetInstance()->listeners();
//  delete listeners.Release(listeners.default_result_printer());
//  listeners.Append(new NoOutputListener);
  return RUN_ALL_TESTS();
}
