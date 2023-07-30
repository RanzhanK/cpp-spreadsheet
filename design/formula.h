#pragma once

#include "common.h"
#include "FormulaAST.h"

#include <memory>
#include <vector>

class FormulaInterface {

public:
    using Value = std::variant<double, FormulaError>;

    virtual Value Evaluate(const SheetInterface &sheet) const = 0;

    virtual std::string GetExpression() const = 0;

    virtual std::vector <Position> GetReferencedCells() const = 0;

    virtual ~FormulaInterface() = default;

};

std::unique_ptr <FormulaInterface> ParseFormula(std::string expression);
