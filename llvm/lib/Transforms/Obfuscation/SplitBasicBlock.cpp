#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Obfuscation/ObfuscationOptions.h"
#include "llvm/Transforms/Obfuscation/Utils.h"
#include "llvm/CryptoUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>
#include <algorithm>

#define DEBUG_TYPE "split"

using namespace llvm;

namespace {

struct SplitBasicBlockPass : public FunctionPass {
  static char ID;
  ObfuscationOptions *ArgsOptions;
  CryptoUtils RandomEngine;
  bool Changed = false;

  SplitBasicBlockPass(ObfuscationOptions *argsOptions)
    : FunctionPass(ID), ArgsOptions(argsOptions) {}

  StringRef getPassName() const override { return "SplitBasicBlockPass"; }

  bool runOnFunction(Function &F) override {
    auto opt = ArgsOptions->toObfuscate(ArgsOptions->splitOpt(), &F);
    if (!opt.isEnabled())
      return false;

    int splitNum = std::max(1, opt.level() ? opt.level() : 3);
    splitFunction(F, splitNum);
    return Changed;
  }

  void splitFunction(Function &F, int splitNum) {
    std::vector<BasicBlock *> origBBs;
    for (auto &BB : F)
      origBBs.push_back(&BB);

    for (auto *BB : origBBs) {
      if (BB->size() < 2 || containsPHI(BB))
        continue;

      int splitN = std::min<int>(splitNum, BB->size() - 1);
      std::vector<int> points;
      for (unsigned i = 1; i < BB->size(); ++i)
        points.push_back(i);

      if (points.size() > 1) {
        shuffle(points);
        std::sort(points.begin(), points.begin() + splitN);
      }

      BasicBlock::iterator it = BB->begin();
      BasicBlock *curr = BB;
      int last = 0;

      for (int i = 0; i < splitN; ++i) {
        if (curr->size() < 2)
          break;

        for (int j = 0; j < points[i] - last; ++j)
          ++it;

        last = points[i];
        curr = curr->splitBasicBlock(it, curr->getName() + ".split");
        Changed = true;
      }
    }
  }

  bool containsPHI(BasicBlock *BB) {
    for (auto &I : *BB)
      if (isa<PHINode>(&I))
        return true;
    return false;
  }

  void shuffle(std::vector<int> &vec) {
    int n = vec.size();
    for (int i = n - 1; i > 0; --i)
      std::swap(vec[i], vec[RandomEngine.get_uint32_t() % (i + 1)]);
  }
};

} // namespace

char SplitBasicBlockPass::ID = 0;

FunctionPass *llvm::createSplitBasicBlockPass(ObfuscationOptions *argsOptions) {
  return new SplitBasicBlockPass(argsOptions);
}

INITIALIZE_PASS(SplitBasicBlockPass, "split",
                "Split Basic Block Obfuscation Pass", false, false)
