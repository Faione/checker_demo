//===- CallTargetMemberLValueChecker.cpp ------------------------*- C++ -*-===//
//
// Demo checker for Clang Static Analyzer.
//
// Reports matched function declaration locations and prints record/member
// information when the call result is assigned to a struct/union member.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/Basic/SourceManager.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ento;
using llvm::raw_ostream;
using llvm::StringRef;

namespace {

class CallTargetMemberLValueChecker
    : public Checker<check::PostStmt<CallExpr>> {
  llvm::StringSet<> FunctionNames;
  const BugType BT{this, "Matched configured function call",
                   "Call target member lvalue demo"};

public:
  void setFunctionNames(StringRef Names);
  void checkPostStmt(const CallExpr *CE, CheckerContext &C) const;

private:
  bool shouldTrack(const FunctionDecl *FD) const;
  void reportMatchedCall(const FunctionDecl *FD, const CallExpr *CE,
                         CheckerContext &C) const;
  void describeAssignedMemberIfAny(llvm::raw_ostream &OS, const CallExpr *CE,
                                   CheckerContext &C) const;
  const Expr *getAssignedLHS(const CallExpr *CE, ASTContext &ACtx) const;
  const Expr *climbThroughTransparentParents(const Expr *E,
                                             ASTContext &ACtx) const;
};

} // namespace

void CallTargetMemberLValueChecker::setFunctionNames(StringRef Names) {
  llvm::SmallVector<StringRef, 16> Parts;
  Names.split(Parts, ",");
  for (StringRef Part : Parts) {
    Part = Part.trim();
    if (!Part.empty())
      FunctionNames.insert(Part);
  }
}

bool CallTargetMemberLValueChecker::shouldTrack(const FunctionDecl *FD) const {
  if (!FD)
    return false;

  if (FunctionNames.empty())
    return false;

  return FunctionNames.contains(FD->getName());
}

static void describeLocation(raw_ostream &OS, SourceLocation Loc,
                             const SourceManager &SM) {
  PresumedLoc PLoc = SM.getPresumedLoc(SM.getExpansionLoc(Loc));
  if (!PLoc.isValid()) {
    OS << "<invalid location>";
    return;
  }

  OS << PLoc.getFilename() << ':' << PLoc.getLine() << ':' << PLoc.getColumn();
}

void CallTargetMemberLValueChecker::reportMatchedCall(const FunctionDecl *FD,
                                                      const CallExpr *CE,
                                                      CheckerContext &C) const {
  const SourceManager &SM = C.getSourceManager();

  ExplodedNode *N = C.generateNonFatalErrorNode();
  if (!N)
    return;

  llvm::SmallString<256> Message;
  llvm::raw_svector_ostream OS(Message);
  OS << "matched configured function call '" << FD->getQualifiedNameAsString()
     << "'";
  OS << "; callee declaration: ";
  describeLocation(OS, FD->getLocation(), SM);
  OS << "; call site: ";
  describeLocation(OS, CE->getExprLoc(), SM);
  describeAssignedMemberIfAny(OS, CE, C);

  auto Report = std::make_unique<PathSensitiveBugReport>(BT, OS.str(), N);
  Report->addRange(CE->getSourceRange());
  C.emitReport(std::move(Report));
}

const Expr *CallTargetMemberLValueChecker::climbThroughTransparentParents(
    const Expr *E, ASTContext &ACtx) const {
  const Expr *Current = E;

  while (true) {
    DynTypedNodeList Parents = ACtx.getParents(*Current);
    if (Parents.size() != 1)
      return Current;

    const Expr *ParentExpr = Parents[0].get<Expr>();
    if (!ParentExpr)
      return Current;

    if (isa<ImplicitCastExpr>(ParentExpr) || isa<ParenExpr>(ParentExpr) ||
        isa<FullExpr>(ParentExpr) || isa<ExprWithCleanups>(ParentExpr)) {
      Current = ParentExpr;
      continue;
    }

    return Current;
  }
}

const Expr *
CallTargetMemberLValueChecker::getAssignedLHS(const CallExpr *CE,
                                              ASTContext &ACtx) const {
  const Expr *Current = climbThroughTransparentParents(CE, ACtx);
  DynTypedNodeList Parents = ACtx.getParents(*Current);
  if (Parents.size() != 1)
    return nullptr;

  if (const auto *BO = Parents[0].get<BinaryOperator>()) {
    if (BO->isAssignmentOp() &&
        BO->getRHS()->IgnoreParenImpCasts() == CE->IgnoreParenImpCasts())
      return BO->getLHS()->IgnoreParenImpCasts();
  }

  return nullptr;
}

void CallTargetMemberLValueChecker::describeAssignedMemberIfAny(
    llvm::raw_ostream &OS, const CallExpr *CE, CheckerContext &C) const {
  ASTContext &ACtx = C.getASTContext();
  const Expr *LHS = getAssignedLHS(CE, ACtx);
  if (!LHS)
    return;

  const auto *ME = dyn_cast<MemberExpr>(LHS);
  if (!ME)
    return;

  const auto *Field = dyn_cast<FieldDecl>(ME->getMemberDecl());
  if (!Field)
    return;

  const RecordDecl *Record = Field->getParent();
  if (!Record)
    return;

  OS << "; assigned struct member: ";
  if (Record->getIdentifier())
    OS << Record->getName();
  else
    OS << "<anonymous record>";

  OS << "." << Field->getName();

  if (ME->isArrow())
    OS << " (via ->)";
  else
    OS << " (via .)";
}

void CallTargetMemberLValueChecker::checkPostStmt(const CallExpr *CE,
                                                  CheckerContext &C) const {
  const FunctionDecl *FD = CE->getDirectCallee();
  if (!shouldTrack(FD))
    return;

  reportMatchedCall(FD, CE, C);
}

void ento::registerCallTargetMemberLValueChecker(CheckerManager &Mgr) {
  auto *Checker = Mgr.registerChecker<CallTargetMemberLValueChecker>();
  Checker->setFunctionNames(Mgr.getAnalyzerOptions().getCheckerStringOption(
      Checker, "FunctionNames"));
}

bool ento::shouldRegisterCallTargetMemberLValueChecker(
    const CheckerManager &Mgr) {
  (void)Mgr;
  return true;
}
