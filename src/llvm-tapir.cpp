// This file is a part of Julia. License is MIT: https://julialang.org/license
#define DEBUG_TYPE "julia_tapir"

#include "llvm-version.h"

#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include "llvm/Transforms/Tapir/LoweringUtils.h"
#include <llvm/Support/Debug.h>

#include "codegen_shared.h"
#include "julia.h"
#include "julia_internal.h"

#include "julia_assert.h"

namespace llvm {

class JuliaTapir : public TapirTarget {
    public:
        JuliaTapir() {}
        ~JuliaTapir() {}
        Value *lowerGrainsizeCall(CallInst *GrainsizeCall) override final;
        void lowerSync(SyncInst &inst) override final;

        void preProcessFunction(Function &F) override final;
        void postProcessFunction(Function &F) override final;
        void postProcessHelper(Function &F) override final;

        void processOutlinedTask(Function &F) override final;
        void processSpawner(Function &F) override final;
        void processSubTaskCall(TaskOutlineInfo &TOI, DominatorTree &DT)
            override final;
};


Value *JuliaTapir::lowerGrainsizeCall(CallInst *GrainsizeCall) {
    Value *Limit = GrainsizeCall->getArgOperand(0);
    Module *M = GrainsizeCall->getModule();
    IRBuilder<> Builder(GrainsizeCall);

    // get jl_n_threads (extern global variable)
    GlobalVariable *proto = new GlobalVariable(*M, Type::getInt32Ty(M->getContext()),
                           false, GlobalVariable::ExternalLinkage,
                           NULL, "jl_n_threads");
    M->getGlobalList().push_back(proto);

    Value *Workers = Builder.CreateLoad(proto);

    // Choose 8xWorkers as grainsize
    Value *WorkersX8 = Builder.CreateIntCast(
        Builder.CreateMul(Workers, ConstantInt::get(Workers->getType(), 8)),
        Limit->getType(), false);

    // Compute ceil(limit / 8 * workers) =
    //           (limit + 8 * workers - 1) / (8 * workers)
    Value *SmallLoopVal =
      Builder.CreateUDiv(Builder.CreateSub(Builder.CreateAdd(Limit, WorkersX8),
                                           ConstantInt::get(Limit->getType(), 1)),
                         WorkersX8);
    // Compute min
    Value *LargeLoopVal = ConstantInt::get(Limit->getType(), 2048);
    Value *Cmp = Builder.CreateICmpULT(LargeLoopVal, SmallLoopVal);
    Value *Grainsize = Builder.CreateSelect(Cmp, LargeLoopVal, SmallLoopVal);

    // Replace uses of grainsize intrinsic call with this grainsize value.
    GrainsizeCall->replaceAllUsesWith(Grainsize);
    return Grainsize;
}

void JuliaTapir::lowerSync(SyncInst &inst) {
}

void JuliaTapir::preProcessFunction(Function &F) {
}

void JuliaTapir::postProcessFunction(Function &F) {
}

void JuliaTapir::postProcessHelper(Function &F) {
}

void JuliaTapir::processOutlinedTask(Function &F) {
}

void JuliaTapir::processSpawner(Function &F) {
}

void JuliaTapir::processSubTaskCall(TaskOutlineInfo &TOI, DominatorTree &DT) {
}

} // namespace LLVM
