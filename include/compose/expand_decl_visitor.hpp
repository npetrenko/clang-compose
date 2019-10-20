#pragma once

#include <clang/AST/RecursiveASTVisitor.h>

class ExpandDeclRange : public clang::RecursiveASTVisitor<ExpandDeclRange> {
public:
    ExpandDeclRange(const clang::SourceManager* sm, const clang::LangOptions* lang_opts) noexcept
        : sm_ptr_(sm), lang_opts_ptr_(lang_opts) {
    }
    bool VisitDecl(clang::Decl* decl);
    bool VisitType(clang::Type* type);
    clang::SourceRange GetExpandedSourceRange() const;

private:
    void MaybeExpand(clang::SourceRange new_range);
    std::optional<clang::SourceLocation> begin_, end_;
    const clang::SourceManager* sm_ptr_;
    const clang::LangOptions* lang_opts_ptr_;
};
