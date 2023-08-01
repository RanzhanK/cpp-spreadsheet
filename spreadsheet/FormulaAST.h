#pragma once

#include "common.h"
#include "FormulaLexer.h"

#include <stdexcept>
#include <functional>
#include <forward_list>

namespace ASTImpl {
    class Expr;
}

class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

using SheetArgs = std::function<double(Position)>;

class FormulaAST {
public:
    explicit FormulaAST(std::unique_ptr <ASTImpl::Expr> root_expr,
                        std::forward_list <Position> cells);

    FormulaAST(FormulaAST &&) = default;

    FormulaAST &operator=(FormulaAST &&) = default;

    ~FormulaAST();

    double Execute(const SheetArgs &) const;

    void PrintCells(std::ostream &out) const;

    void Print(std::ostream &out) const;

    void PrintFormula(std::ostream &out) const;

    std::forward_list <Position> &GetCells() {
        return cells_;
    }

    const std::forward_list <Position> &GetCells() const {
        return cells_;
    }

private:
    std::unique_ptr <ASTImpl::Expr> root_expr_;
    std::forward_list <Position> cells_;
};

FormulaAST ParseFormulaAST(std::istream &in);

FormulaAST ParseFormulaAST(const std::string &in_str);