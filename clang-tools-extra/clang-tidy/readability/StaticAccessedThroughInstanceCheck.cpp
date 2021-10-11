//===--- StaticAccessedThroughInstanceCheck.cpp - clang-tidy---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "StaticAccessedThroughInstanceCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/ADT/StringRef.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace readability {

static unsigned getNameSpecifierNestingLevel(QualType T) {
  unsigned NestingLevel = 0;
  if (const auto *ET = T->getAs<ElaboratedType>())
    for (const NestedNameSpecifier *NNS = ET->getQualifier(); NNS;
         NNS = NNS->getPrefix())
      ++NestingLevel;
  return NestingLevel;
}

void StaticAccessedThroughInstanceCheck::storeOptions(
    ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "NameSpecifierNestingThreshold",
                NameSpecifierNestingThreshold);
}

void StaticAccessedThroughInstanceCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      memberExpr(hasDeclaration(anyOf(cxxMethodDecl(isStaticStorageClass()),
                                      varDecl(hasStaticStorageDuration()))))
          .bind("memberExpression"),
      this);
}

void StaticAccessedThroughInstanceCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *MemberExpression =
      Result.Nodes.getNodeAs<MemberExpr>("memberExpression");

  if (MemberExpression->getBeginLoc().isMacroID())
    return;

  const Expr *BaseExpr = MemberExpression->getBase();

  // Do not warn for overloaded -> operators.
  if (isa<CXXOperatorCallExpr>(BaseExpr))
    return;

  QualType BaseType =
      BaseExpr->getType()->isPointerType()
          ? BaseExpr->getType()->getPointeeType().getUnqualifiedType()
          : BaseExpr->getType().getUnqualifiedType();

  const ASTContext *AstContext = Result.Context;
  PrintingPolicy PrintingPolicyWithSupressedTag(AstContext->getLangOpts());
  PrintingPolicyWithSupressedTag.SuppressTagKeyword = true;
  PrintingPolicyWithSupressedTag.SuppressUnwrittenScope = true;
  std::string BaseTypeName =
      BaseType.getAsString(PrintingPolicyWithSupressedTag);

  // Do not warn for CUDA built-in variables.
  if (StringRef(BaseTypeName).startswith("__cuda_builtin_"))
    return;

  SourceLocation MemberExprStartLoc = MemberExpression->getBeginLoc();
  auto Diag =
      diag(MemberExprStartLoc, "static member accessed through instance");

  if (BaseExpr->HasSideEffects(*AstContext) ||
      getNameSpecifierNestingLevel(BaseType) > NameSpecifierNestingThreshold)
    return;

  Diag << FixItHint::CreateReplacement(
      CharSourceRange::getCharRange(MemberExprStartLoc,
                                    MemberExpression->getMemberLoc()),
      BaseTypeName + "::");
}

} // namespace readability
} // namespace tidy
} // namespace clang
