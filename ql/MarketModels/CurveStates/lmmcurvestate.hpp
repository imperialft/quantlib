/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
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


#ifndef quantlib_lmmcurvestate_hpp
#define quantlib_lmmcurvestate_hpp

#include <ql/MarketModels/newcurvestate.hpp>

namespace QuantLib {

    /*! This class stores the state of the yield curve associated to the
        fixed calendar times within the simulation.
        This is the workhorse discounting object associated to the rate times
        of the simulation. It's important to pass the rates via an object like
        this to the product rather than directly to make it easier to switch
        to other engines such as a coterminal swap rate engine.
        Many products will not need expired rates and others will only require
        the first rate.
    */
    class LMMCurveState : public NewCurveState {
    /* There will n+1 rate times expressing payment and reset times
        of forward rates.

                |-----|-----|-----|-----|-----|      (size = 6)
                t0    t1    t2    t3    t4    t5     rateTimes
                f0    f1    f2    f3    f4           forwardRates
                d0    d1    d2    d3    d4    d5     discountBonds
                d0/d0 d1/d0 d2/d0 d3/d0 d4/d0 d5/d0  discountRatios
                sr0   sr1   sr2   sr3   sr4          cotSwaps
    */
      public:
        LMMCurveState(const std::vector<Time>& rateTimes);
        //! \name Modifiers
        //@{
        void setOnForwardRates(const std::vector<Rate>& fwdRates,
                               Size firstValidIndex = 0);

        template <class ForwardIterator>
        void setOnDiscountRatios(ForwardIterator begin,
                                 ForwardIterator end,
                                 Size firstValidIndex = 0) {
            QL_REQUIRE(end-begin==nRates_+1,
                       "too many discount ratios: " <<
                       nRates_+1 << " required, " <<
                       end-begin << " provided");
            QL_REQUIRE(firstValidIndex<nRates_,
                       "first valid index must be less than " <<
                       nRates_+1 << ": " <<
                       firstValidIndex << " not allowed");

            // discount ratios
            first_ = firstValidIndex;
            std::copy(begin+first_, end, discRatios_.begin()+first_);

            // fwd rates
            for (Size i=first_; i<nRates_; ++i)
                forwardRates_[i] = (discRatios_[i]/discRatios_[i+1]-1.0) /
                                                                    taus_[i];

            // lazy evaluation of coterminal swap rates and annuities
            firstCotSwap_ = nRates_;
        }

        //@}

        //! \name Inspectors
        //@{
        Real discountRatio(Size i,
                          Size j) const;
        Rate forwardRate(Size i) const;

        Rate coterminalSwapRate(Size i) const;
        Rate coterminalSwapAnnuity(Size numeraire,
                                   Size i) const;

        Rate cmSwapRate(Size i,
                        Size spanningForwards) const;
        Rate cmSwapAnnuity(Size numeraire,
                           Size i,
                           Size spanningForwards) const;
        //@}
      private:
        Size first_;
        std::vector<DiscountFactor> discRatios_;
        std::vector<Rate> forwardRates_;
        mutable std::vector<Rate> cmSwapRates_;
        mutable std::vector<Real> cmSwapAnnuities_;
        mutable std::vector<Rate> cotSwapRates_;
        mutable std::vector<Real> cotAnnuities_;
    };

}

#endif