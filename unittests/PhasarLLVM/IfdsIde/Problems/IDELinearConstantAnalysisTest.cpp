#include <gtest/gtest.h>
#include <phasar/DB/ProjectIRDB.h>
#include <phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h>
#include <phasar/PhasarLLVM/IfdsIde/Problems/IDELinearConstantAnalysis.h>
#include <phasar/PhasarLLVM/IfdsIde/Solver/LLVMIDESolver.h>
#include <phasar/PhasarLLVM/Passes/ValueAnnotationPass.h>
#include <phasar/PhasarLLVM/Pointer/LLVMTypeHierarchy.h>

using namespace psr;

/* ============== TEST FIXTURE ============== */
class IDELinearConstantAnalysisTest : public ::testing::Test {
protected:
  const std::string pathToLLFiles =
      PhasarConfig::getPhasarConfig().PhasarDirectory() +
      "build/test/llvm_test_code/linear_constant/";
  const std::vector<std::string> EntryPoints = {"main"};

  ProjectIRDB *IRDB;
  LLVMTypeHierarchy *TH;
  LLVMBasedICFG *ICFG;
  IDELinearConstantAnalysis *LCAProblem;

  IDELinearConstantAnalysisTest() = default;
  virtual ~IDELinearConstantAnalysisTest() = default;

  void Initialize(const std::vector<std::string> &IRFiles) {
    IRDB = new ProjectIRDB(IRFiles);
    IRDB->preprocessIR();
    TH = new LLVMTypeHierarchy(*IRDB);
    ICFG =
        new LLVMBasedICFG(*TH, *IRDB, CallGraphAnalysisType::OTF, EntryPoints);
    LCAProblem = new IDELinearConstantAnalysis(*ICFG, *TH, *IRDB, EntryPoints);
  }

  void SetUp() override {
    bl::core::get()->set_logging_enabled(false);
    ValueAnnotationPass::resetValueID();
  }

  void TearDown() override {
    delete IRDB;
    delete TH;
    delete ICFG;
    delete LCAProblem;
  }

  /**
   * We map instruction id to value for the ground truth. ID has to be
   * a string since Argument ID's are not integer type (e.g. main.0 for argc).
   * @param groundTruth results to compare against
   * @param solver provides the results
   */
  void compareResults(
      const std::map<std::string, int64_t> &groundTruth,
      LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> &solver) {
    std::map<std::string, int64_t> results;
    for (auto M : IRDB->getAllModules()) {
      for (auto &F : *M) {
        for (auto exit : ICFG->getExitPointsOf(&F)) {
          for (auto res : solver.resultsAt(exit, true)) {
            results.insert(std::pair<std::string, int64_t>(
                getMetaDataID(res.first), res.second));
          }
        }
      }
    }
    EXPECT_EQ(results, groundTruth);
  }
}; // Test Fixture

/* ============== BASIC TESTS ============== */
TEST_F(IDELinearConstantAnalysisTest, HandleBasicTest_01) {
  Initialize({pathToLLFiles + "basic_01_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {{"0", 0}, {"1", 13}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBasicTest_02) {
  Initialize({pathToLLFiles + "basic_02_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {{"0", 0}, {"1", 17}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBasicTest_03) {
  Initialize({pathToLLFiles + "basic_03_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {
      {"0", 0}, {"1", 14}, {"2", 14}, {"8", 14}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBasicTest_04) {
  Initialize({pathToLLFiles + "basic_04_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {
      {"0", 0}, {"1", 14}, {"2", 20}, {"10", 14}, {"11", 20}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBasicTest_05) {
  Initialize({pathToLLFiles + "basic_05_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {{"0", 0}, {"1", 3},  {"2", 14},
                                             {"7", 3}, {"8", 12}, {"9", 14}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBasicTest_06) {
  Initialize({pathToLLFiles + "basic_06_cpp_m2r_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {{"1", 16}};
  compareResults(gt, llvmlcasolver);
}

/* ============== BRANCH TESTS ============== */
TEST_F(IDELinearConstantAnalysisTest, HandleBranchTest_01) {
  Initialize({pathToLLFiles + "branch_01_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {
      {"1", 0}, {"2", LCAProblem->bottomElement()}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBranchTest_02) {
  // Probably a bad example/style, since variable i is possibly unitialized
  Initialize({pathToLLFiles + "branch_02_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {
      {"1", 0}, {"2", LCAProblem->bottomElement()}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBranchTest_03) {
  Initialize({pathToLLFiles + "branch_03_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {{"1", 0}, {"2", 30}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBranchTest_04) {
  Initialize({pathToLLFiles + "branch_04_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {{"1", 0},
                                             {"2", 10},
                                             {"3", LCAProblem->bottomElement()},
                                             {"12", 10},
                                             {"13", 20}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBranchTest_05) {
  Initialize({pathToLLFiles + "branch_05_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {
      {"1", 0},  {"2", 10},  {"3", LCAProblem->bottomElement()},
      {"8", 10}, {"13", 10}, {"14", 20}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBranchTest_06) {
  Initialize({pathToLLFiles + "branch_06_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {{"1", 0},
                                             {"2", 10},
                                             {"3", LCAProblem->bottomElement()},
                                             {"8", 10},
                                             {"9", 20}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleBranchTest_07) {
  Initialize({pathToLLFiles + "branch_07_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {
      {"1", 0},  {"2", 10}, {"3", LCAProblem->bottomElement()},
      {"8", 10}, {"9", 30}, {"14", 10},
      {"15", 12}};
  compareResults(gt, llvmlcasolver);
}

/* ============== CALL TESTS ============== */
TEST_F(IDELinearConstantAnalysisTest, HandleCallTest_01) {
  Initialize({pathToLLFiles + "call_01_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {
      {"0", 42}, {"1", 42},  {"5", 42},        {"8", 0},
      {"9", 42}, {"13", 42}, {"_Z3fooi.0", 42}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleCallTest_02) {
  Initialize({pathToLLFiles + "call_02_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {
      {"0", 2},  {"3", 2},   {"4", 42},       {"6", 0},
      {"7", 42}, {"10", 42}, {"_Z3fooi.0", 2}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleCallTest_03) {
  Initialize({pathToLLFiles + "call_03_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {{"1", 0}, {"2", 42}, {"5", 42}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleCallTest_04) {
  Initialize({pathToLLFiles + "call_04_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {{"1", 0}, {"2", 10}, {"6", 42}};
  compareResults(gt, llvmlcasolver);
}

TEST_F(IDELinearConstantAnalysisTest, HandleCallTest_05) {
  Initialize({pathToLLFiles + "call_05_cpp_dbg.ll"});
  LLVMIDESolver<const llvm::Value *, int64_t, LLVMBasedICFG &> llvmlcasolver(
      *LCAProblem);
  llvmlcasolver.solve();
  const std::map<std::string, int64_t> gt = {
      {"0", 0},
      {"1", LCAProblem->bottomElement()},
      {"3", LCAProblem->bottomElement()},
      {"10", LCAProblem->bottomElement()},
      {"main.0", LCAProblem->bottomElement()}};
  compareResults(gt, llvmlcasolver);
}

// main function for the test case
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
