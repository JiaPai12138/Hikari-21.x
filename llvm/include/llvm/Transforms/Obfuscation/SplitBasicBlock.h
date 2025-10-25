#ifndef OBFUSCATION_SPLIT_BASIC_BLOCK_H
#define OBFUSCATION_SPLIT_BASIC_BLOCK_H

namespace llvm {
  class FunctionPass;
  class PassRegistry;
  class ObfuscationOptions;

  FunctionPass* createSplitBasicBlockPass(ObfuscationOptions *argsOptions);
  void initializeSplitBasicBlockPass(PassRegistry &Registry);

}

#endif
