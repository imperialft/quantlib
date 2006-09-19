/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MarketModels/Products/MultiStep/callspecifiedmultiproduct.hpp>
#include <ql/MarketModels/Products/MultiStep/cashrebate.hpp>

namespace QuantLib {

    namespace {

        void mergeTimes(const std::vector<std::vector<Time> >& times,
                        const std::vector<Time>& mergedTimes,
                        std::vector<std::vector<bool> >& isPresent);

    }

    CallSpecifiedMultiProduct::CallSpecifiedMultiProduct(
              const boost::shared_ptr<MarketModelMultiProduct>& underlying,
              const boost::shared_ptr<ExerciseStrategy<CurveState> >& strategy,
              const boost::shared_ptr<MarketModelMultiProduct>& rebate)
    : underlying_(underlying), strategy_(strategy), rebate_(rebate) {
        
        Size products = underlying_->numberOfProducts();
        EvolutionDescription d1 = underlying->suggestedEvolution();
        const std::vector<Time>& rateTimes1 = d1.rateTimes();
        const std::vector<Time>& evolutionTimes1 = d1.evolutionTimes();
        const std::vector<Time>& exerciseTimes = strategy->exerciseTimes();

        if (rebate_) {
            EvolutionDescription d2 = rebate_->suggestedEvolution();
            const std::vector<Time>& rateTimes2 = d2.rateTimes();
            QL_REQUIRE(rateTimes1.size() == rateTimes2.size() &&
                       std::equal(rateTimes1.begin(), rateTimes1.end(),
                                  rateTimes2.begin()),
                       "incompatible rate times");
        } else {
            EvolutionDescription description(rateTimes1, exerciseTimes);
            Matrix amounts(products, exerciseTimes.size(), 0.0);

            rebate_ = boost::shared_ptr<MarketModelMultiProduct>(
                    new MarketModelCashRebate(description, exerciseTimes,
                                              amounts, products));
        }

        std::vector<Time> mergedEvolutionTimes;
        std::vector<std::vector<Time> > allEvolutionTimes(3);
        allEvolutionTimes[0] = evolutionTimes1;
        allEvolutionTimes[1] = exerciseTimes;
        allEvolutionTimes[2] = rebate_->suggestedEvolution().evolutionTimes();

        mergeTimes(allEvolutionTimes,
                   mergedEvolutionTimes,
                   isPresent_);

        evolution_ = EvolutionDescription(rateTimes1, mergedEvolutionTimes,
                                          d1.numeraires()); // TODO: add
                                                            // relevant rates
        cashFlowTimes_ = underlying_->possibleCashFlowTimes();
        rebateOffset_ = cashFlowTimes_.size();
        const std::vector<Time> rebateTimes = rebate_->possibleCashFlowTimes();
        std::copy(rebateTimes.begin(), rebateTimes.end(),
                  std::back_inserter(cashFlowTimes_));

        dummyCashFlowsThisStep_ = std::vector<Size>(products, 0);
        Size n = rebate_->maxNumberOfCashFlowsPerProductPerStep();
        dummyCashFlowsGenerated_ =
            std::vector<std::vector<CashFlow> >(products,
                                                std::vector<CashFlow>(n));
    }

    EvolutionDescription
    CallSpecifiedMultiProduct::suggestedEvolution() const {
        return evolution_;
    }

    std::vector<Time>
    CallSpecifiedMultiProduct::possibleCashFlowTimes() const {
        return cashFlowTimes_;
    }

    Size CallSpecifiedMultiProduct::numberOfProducts() const {
        return underlying_->numberOfProducts();
    }

    Size
    CallSpecifiedMultiProduct::maxNumberOfCashFlowsPerProductPerStep() const {
        return std::max(underlying_->maxNumberOfCashFlowsPerProductPerStep(),
                        rebate_->maxNumberOfCashFlowsPerProductPerStep());
    }

    void CallSpecifiedMultiProduct::reset() {
        underlying_->reset();
        rebate_->reset();
        strategy_->reset();
        currentIndex_ = 0;
        wasCalled_ = false;
    }


    bool CallSpecifiedMultiProduct::nextTimeStep(
            const CurveState& currentState, 
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<CashFlow> >& cashFlowsGenerated) {

        bool isUnderlyingTime = isPresent_[0][currentIndex_];
        bool isExerciseTime = isPresent_[1][currentIndex_];
        bool isRebateTime = isPresent_[2][currentIndex_];

        bool done = false;

        if (!wasCalled_ && isExerciseTime)
            wasCalled_ = strategy_->nextExercise(currentState);

        if (wasCalled_) {
            if (isRebateTime)
                done = rebate_->nextTimeStep(currentState,
                                             numberCashFlowsThisStep,
                                             cashFlowsGenerated);
        } else {
            if (isRebateTime)
                rebate_->nextTimeStep(currentState,
                                      dummyCashFlowsThisStep_,
                                      dummyCashFlowsGenerated_);
            if (isUnderlyingTime)
                done = underlying_->nextTimeStep(currentState,
                                                 numberCashFlowsThisStep,
                                                 cashFlowsGenerated);
        }

        ++currentIndex_;
        return done || currentIndex_ == evolution_.evolutionTimes().size();
    }

}
