#pragma once

#include <exception>

class AnonymousCXXRecordDecl : public std::runtime_error {
public:
    AnonymousCXXRecordDecl()
        : std::runtime_error(
              "Anonymous CXXRecord declarations in non-stl code cannot be handled correctly") {
    }
};

class CXXRecPlusVar : public std::runtime_error {
public:
    CXXRecPlusVar() : std::runtime_error("CXXRecord + VarDecl is not supported") {
    }
};
