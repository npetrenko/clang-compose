// Declares clang::SyntaxOnlyAction.
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

// Declares llvm::cl::extrahelp.
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Lex/Lexer.h>

#include <llvm/Support/raw_ostream.h>

#include <vector>
#include <iostream>
#include <unordered_set>

#include <clang/Rewrite/Core/Rewriter.h>
#include <compose/exceptions.hpp>

namespace ct = clang::tooling;

static llvm::cl::OptionCategory my_tool_category("Style-checker options");

class ProjectLocation {
public:
    /**
     * A placeholder for a better implementation
     */
    bool IsInProject(clang::StringRef path) const {
        return path.contains("/home");
    }
};

class IncludesFinder : public clang::PPCallbacks {
public:
    struct Include {
        Include(bool ia, clang::StringRef cont) : is_angled(ia), content(cont) {
        }
        bool is_angled;
        clang::StringRef content;

	bool operator<(const Include& other) const {
	    return content < other.content;
	}
    };

    IncludesFinder(clang::CompilerInstance* compiler) : compiler_(compiler) {
    }

    std::set<Include> found_includes;

    void InclusionDirective(clang::SourceLocation hash_loc, const clang::Token&,
                            clang::StringRef filename, bool is_angled, clang::CharSourceRange,
                            const clang::FileEntry* file, clang::StringRef, clang::StringRef,
                            const clang::Module*, clang::SrcMgr::CharacteristicKind) override {
        if (!project_filter_(compiler_->getSourceManager().getFilename(hash_loc))) {
            return;
        }

        if (file) {
            if (project_filter_(file->getName())) {
                return;
            }

            found_includes.insert({is_angled, filename});
        } else {
            std::cerr << "Could not locate file from include";
        }
    }

    template <class T>
    void SetProjectFilter(T&& exclude_filter) {
        project_filter_ = std::forward<T>(exclude_filter);
    }

private:
    std::function<bool(clang::StringRef)> project_filter_;
    clang::CompilerInstance* compiler_;
};

class IncludesFinderAction : public clang::PreprocessOnlyAction {
public:
    IncludesFinderAction(ProjectLocation loc) : proj_loc_(loc) {
    }

    bool BeginSourceFileAction(clang::CompilerInstance& compiler) override {
        // compiler.getDiagnostics().setClient(new clang::IgnoringDiagConsumer());

        std::unique_ptr find_includes_callback = std::make_unique<IncludesFinder>(&compiler);
        find_includes_callback->SetProjectFilter(
            [proj_loc = proj_loc_](llvm::StringRef file) { return proj_loc.IsInProject(file); });

        clang::Preprocessor& pp = compiler.getPreprocessor();
        pp.addPPCallbacks(std::move(find_includes_callback));
        return true;
    }

    void EndSourceFileAction() override {
        clang::CompilerInstance& ci = getCompilerInstance();
        clang::Preprocessor& pp = ci.getPreprocessor();
        auto* find_includes_callback = static_cast<IncludesFinder*>(pp.getPPCallbacks());

        llvm::raw_fd_ostream stream(1, false);
        for (auto& inc : find_includes_callback->found_includes) {
            stream << "#include ";
            stream << (inc.is_angled ? "<" : "\"");
            stream << inc.content;
            stream << (inc.is_angled ? ">" : "\"");
            stream << "\n";
        }

        stream << "\n";
    }

private:
    ProjectLocation proj_loc_;
};

class FindNamedClassAction : public clang::ASTFrontendAction {
public:
    FindNamedClassAction(ProjectLocation loc) : proj_loc_(loc) {
    }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& compiler,
                                                          llvm::StringRef infile) override;

private:
    ProjectLocation proj_loc_;
};

class FindNamedClassConsumer : public clang::ASTConsumer {
public:
    FindNamedClassConsumer(clang::CompilerInstance* compiler, std::string infile,
                           ProjectLocation loc)
        : compiler_(compiler), infile_(infile), proj_loc_(loc) {
    }

    void HandleTranslationUnit(clang::ASTContext& context) override;
    virtual ~FindNamedClassConsumer() = default;

private:
    void TraverseTUDecl(clang::TranslationUnitDecl* decl, clang::ASTContext* context,
                        clang::Rewriter* rewriter);
    clang::CompilerInstance* compiler_;
    llvm::StringRef infile_;
    ProjectLocation proj_loc_;
};

std::unique_ptr<clang::ASTConsumer> FindNamedClassAction::CreateASTConsumer(
    clang::CompilerInstance& compiler, llvm::StringRef infile) {
    // compiler.getDiagnostics().setClient(new clang::IgnoringDiagConsumer());

    return std::make_unique<FindNamedClassConsumer>(&compiler, infile, proj_loc_);
}

void FindNamedClassConsumer::HandleTranslationUnit(clang::ASTContext& context) {
    clang::Rewriter rewriter;
    rewriter.setSourceMgr(compiler_->getSourceManager(), compiler_->getLangOpts());

    TraverseTUDecl(context.getTranslationUnitDecl(), &context, &rewriter);

    rewriter.InsertText(context.getTranslationUnitDecl()->getEndLoc(), "Hello, world!");

    const clang::RewriteBuffer* buf =
        rewriter.getRewriteBufferFor(compiler_->getSourceManager().getMainFileID());
    if (buf) {
        std::cout << std::string(buf->begin(), buf->end()) << "\n";
    }
}

void FindNamedClassConsumer::TraverseTUDecl(clang::TranslationUnitDecl* tu_decl,
                                            clang::ASTContext* context, clang::Rewriter* rewriter) {
    clang::SourceManager& sm = compiler_->getSourceManager();
    llvm::raw_fd_ostream output_stream(1, false);

    for (const auto* subdecl : tu_decl->decls()) {
        const auto& loc = subdecl->getLocation();
        auto fname = context->getSourceManager().getFilename(loc);
        if (!proj_loc_.IsInProject(fname)) {
            continue;
        }

        if (auto declkind = subdecl->getKind(); declkind == clang::Decl::CXXRecord) {
            const auto* cxxrec = static_cast<const clang::CXXRecordDecl*>(subdecl);
            if (cxxrec->getName().empty()) {
                loc.print(output_stream, sm);
                output_stream << "\n";
                throw AnonymousCXXRecordDecl();
            }
        }

        subdecl->print(output_stream);
        if (clang::Lexer::findLocationAfterToken(subdecl->getEndLoc(), clang::tok::semi, sm,
                                                 compiler_->getLangOpts(), true)
                .isValid()) {
            // found a semicolon
            output_stream << ";";
        } else {
            if (auto declkind = subdecl->getKind();
                declkind == clang::Decl::CXXRecord || declkind == clang::Decl::ClassTemplate) {
                output_stream << "\n";
                loc.print(output_stream, sm);
                output_stream << "\n";
                throw CXXRecPlusVar();
            }
        }
        output_stream << "\n\n";
    }
}

template <class ActionType>
class ProjectFrontendActionFactory : public ct::FrontendActionFactory {
public:
    ProjectFrontendActionFactory(ProjectLocation loc) : proj_loc_(loc) {
    }

    clang::FrontendAction* create() override {
        return new ActionType(proj_loc_);
    }

private:
    ProjectLocation proj_loc_;
};

int main(int argc, const char** argv) {
    clang::tooling::CommonOptionsParser parser(argc, argv, my_tool_category);
    if (parser.getSourcePathList().size() > 1) {
        std::cerr << "Can only work with a single source\n";
        return 1;
    }

    clang::tooling::ClangTool tool(parser.getCompilations(), parser.getSourcePathList());

    ProjectLocation project_location;

    {
        std::unique_ptr factory =
            std::make_unique<ProjectFrontendActionFactory<IncludesFinderAction>>(project_location);
        if (int ret_code = tool.run(factory.get())) {
            // return ret_code;
        }
    }
    {
        std::unique_ptr factory =
            std::make_unique<ProjectFrontendActionFactory<FindNamedClassAction>>(project_location);
        if (int ret_code = tool.run(factory.get())) {
            return ret_code;
        }
    }
    return 0;
}
