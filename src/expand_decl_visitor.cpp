#include <compose/expand_decl_visitor.hpp>
#include <clang/Lex/Lexer.h>
#include <iostream>

bool ExpandDeclRange::VisitDecl(clang::Decl* decl) {
    MaybeExpand(decl->getSourceRange());
    return true;
}

void ExpandDeclRange::MaybeExpand(clang::SourceRange range) {
    assert(range.isValid());
    auto is_before = [&](auto&& first, auto&& second) {
        return clang::FullSourceLoc(first, *sm_ptr_).isBeforeInTranslationUnitThan(second);
    };

    if (!begin_ || !(is_before(*begin_, range.getBegin()))) {
        begin_ = range.getBegin();
    }

    if (!end_ || is_before(*end_, range.getEnd())) {
        end_ = range.getEnd();
    }
}

bool ExpandDeclRange::VisitType(clang::Type* type) {
    if (type->isDecltypeType()) {
        // decltypes are kinda special in handling
        clang::DecltypeType* declt = static_cast<clang::DecltypeType*>(type);
        auto sr = declt->getUnderlyingExpr()->getSourceRange();
        while (true) {
            auto token = clang::Lexer::findNextToken(sr.getEnd(), *sm_ptr_, *lang_opts_ptr_);
            if (token.getValue().getKind() == clang::tok::TokenKind::semi) {
                break;
            }
            auto loc = clang::Lexer::getLocForEndOfToken(sr.getEnd(), 0, *sm_ptr_, *lang_opts_ptr_);
            sr.setEnd(loc);
        }
        sr.setEnd(sr.getEnd().getLocWithOffset(1));
        MaybeExpand(sr);
    }

    return true;
}

clang::SourceRange ExpandDeclRange::GetExpandedSourceRange() const {
    return {begin_.value(), end_.value()};
}
