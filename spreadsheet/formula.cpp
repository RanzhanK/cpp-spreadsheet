#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
    class Formula : public FormulaInterface {
    public:

        explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression)) {}
        catch (...) {
            throw FormulaException("formula is syntactically incorrect");
        }

        Value Evaluate(const SheetInterface& sheet) const override {

            try {

                std::function<double(Position)> args = [&sheet](const Position pos)->double {

                    if (pos.IsValid()) {

                        const auto* cell = sheet.GetCell(pos);
                        if (cell) {

                            if (std::holds_alternative<double>(cell->GetValue())) {
                                return std::get<double>(cell->GetValue());

                            } else if (std::holds_alternative<std::string>(cell->GetValue())) {

                                auto str_value = std::get<std::string>(cell->GetValue());
                                if (str_value != "") {
                                    std::istringstream input(str_value);
                                    double num = 0.0;

                                    if (input.eof() && input >> num) {
                                        return num;
                                    } else {
                                        throw FormulaError(FormulaError::Category::Value);
                                    }

                                } else {
                                    return 0.0;
                                }

                            } else {
                                throw FormulaError(std::get<FormulaError>(cell->GetValue()));
                            }

                        } else {
                            return 0.0;
                        }

                    } else {
                        throw FormulaError(FormulaError::Category::Ref);
                    }
                };

                return ast_.Execute(args);

            } catch (const FormulaError& evaluate_error) {
                return evaluate_error;
            }
        }

        std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);

            return out.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            std::vector<Position> cells;
            for (const auto& cell : ast_.GetCells()) {

                if (cell.IsValid()) {
                    cells.push_back(cell);
                } else {
                    continue;
                }
            }
            return cells;
        }

    private:
        FormulaAST ast_;
    };

}//end namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}