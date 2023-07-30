#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()),
                           sheet_(sheet) {}
Cell::~Cell() = default;

void Cell::Set(std::string text) {

    std::unique_ptr<Impl> temp_impl;

    if (text.empty()) {
        temp_impl = std::make_unique<EmptyImpl>();

    } else if (text.size() >= 2 && text.at(0) == FORMULA_SIGN) {
        temp_impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);

    } else {
        temp_impl = std::make_unique<TextImpl>(std::move(text));
    }

    //начали искать циклические зависимости
    const Impl& temp_impl_ = *temp_impl;
    const auto temp_ref_cells = temp_impl_.GetReferencedCells();

    if (!temp_ref_cells.empty()) {

        std::set<const Cell*> ref_collection;
        std::set<const Cell*> enter_collection;
        std::vector<const Cell*> to_enter_collection;

        for (auto position : temp_ref_cells) {
            ref_collection.insert(sheet_.Get_Cell(position));
        }

        to_enter_collection.push_back(this);

        while (!to_enter_collection.empty()) {

            const Cell* ongoing = to_enter_collection.back();

            to_enter_collection.pop_back();
            enter_collection.insert(ongoing);

            if (ref_collection.find(ongoing) == ref_collection.end()) {

                for (const Cell* dependent : ongoing->dependent_cells_) {

                    if (enter_collection.find(dependent) == enter_collection.end()) {
                        to_enter_collection.push_back(dependent);
                    }
                }

            } else {
                throw CircularDependencyException("circular dependency detected");
            }
        }

        impl_ = std::move(temp_impl);

    } else {
        impl_ = std::move(temp_impl);
    }
    //закончили искать циклические зависимости

    //обновляем зависимости
    for (Cell* refrenced : referenced_cells_) {
        refrenced->dependent_cells_.erase(this);
    }

    referenced_cells_.clear();

    for (const auto& position : impl_->GetReferencedCells()) {

        Cell* refrenced = sheet_.Get_Cell(position);

        if (!refrenced) {
            sheet_.SetCell(position, "");
            refrenced = sheet_.Get_Cell(position);
        }

        referenced_cells_.insert(refrenced);
        refrenced->dependent_cells_.insert(this);
    }
    //обновили зависимости

    //инвалидация кеша
    InvalidateAllCache(true);
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {return impl_->GetValue();}
std::string Cell::GetText() const {return impl_->GetText();}
std::vector<Position> Cell::GetReferencedCells() const {return impl_->GetReferencedCells();}
bool Cell::IsReferenced() const {return !dependent_cells_.empty();}

void Cell::InvalidateAllCache(bool flag = false) {
    if (impl_->HasCache() || flag) {
        impl_->InvalidateCache();

        for (Cell* dependent : dependent_cells_) {
            dependent->InvalidateAllCache();
        }
    }
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {return {};}
bool Cell::Impl::HasCache() {return true;}
void Cell::Impl::InvalidateCache() {}

Cell::Value Cell::EmptyImpl::GetValue() const {return "";}
std::string Cell::EmptyImpl::GetText() const {return "";}

Cell::TextImpl::TextImpl(std::string text) : text_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {

    if (text_.empty()) {
        throw FormulaException("it is empty impl, not text");

    } else if (text_.at(0) == ESCAPE_SIGN) {
        return text_.substr(1);

    } else {
        return text_;
    }
}

std::string Cell::TextImpl::GetText() const {return text_;}

Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet) : formula_ptr_(ParseFormula(text.substr(1)))
        , sheet_(sheet) {}

Cell::Value Cell::FormulaImpl::GetValue() const {

    if (!cache_) {cache_ = formula_ptr_->Evaluate(sheet_);}
    return std::visit([](auto& helper){return Value(helper);}, *cache_);
}

std::string Cell::FormulaImpl::GetText() const {return FORMULA_SIGN + formula_ptr_->GetExpression();}
std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {return formula_ptr_->GetReferencedCells();}
bool Cell::FormulaImpl::HasCache() {return cache_.has_value();}
void Cell::FormulaImpl::InvalidateCache() {cache_.reset();}