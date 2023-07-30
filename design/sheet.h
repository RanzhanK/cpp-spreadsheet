#pragma once

#include "cell.h"
#include "common.h"

#include <vector>
#include <functional>

class Sheet : public SheetInterface {

public:

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface *GetCell(Position pos) const override;

    CellInterface *GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream &output) const override;

    void PrintTexts(std::ostream &output) const override;

private:
    bool IsCircularDependency(const CellInterface *cell, const Position &pos) const;

    std::pair <Position, Position> GetArea() const;

    struct PosHasher {
        size_t operator()(const Position &p) const;
    };

    struct Visitor {
        std::string operator()(std::string val) const;

        std::string operator()(double val) const;

        std::string operator()(FormulaError val) const;
    };

    std::unordered_map <Position, std::unique_ptr<CellInterface>, PosHasher> cell_map_;
};