
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sad Rejeb
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file vanillaengines.hpp
    \brief Vanilla option engines

    \fullpath
    ql/PricingEngines/%vanillaengines.hpp
*/

// $Id$

#ifndef quantlib_vanilla_engines_h
#define quantlib_vanilla_engines_h

#include <ql/exercise.hpp>
#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>
//#include <ql/TermStructures/quantotermstructure.hpp>
#include <ql/MonteCarlo/mctypedefs.hpp>
#include <ql/MonteCarlo/europeanpathpricer.hpp>
#include <ql/TermStructures/drifttermstructure.hpp>
#include <ql/PricingEngines/genericengine.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! arguments for vanilla option calculation
        class VanillaOptionArguments : public virtual Arguments {
          public:
            VanillaOptionArguments() : type(Option::Type(-1)),
                                       underlying(Null<double>()),
                                       strike(Null<double>()) {}
            void validate() const;
            Option::Type type;
            double underlying, strike;
            RelinkableHandle<TermStructure> riskFreeTS, dividendTS;
            RelinkableHandle<BlackVolTermStructure> volTS;
            // ??
            Exercise exercise;
        };

        inline void VanillaOptionArguments::validate() const {
            QL_REQUIRE(type != Option::Type(-1),
                       "VanillaOptionArguments::validate() : "
                       "no option type given");
            QL_REQUIRE(underlying != Null<double>(),
                       "VanillaOptionArguments::validate() : "
                       "null underlying given");
            QL_REQUIRE(underlying > 0.0,
                       "VanillaOptionArguments::validate() : "
                       "negative or zero underlying given");
            QL_REQUIRE(strike != Null<double>(),
                       "VanillaOptionArguments::validate() : "
                       "null strike given");
            QL_REQUIRE(strike >= 0.0,
                       "VanillaOptionArguments::validate() : "
                       "negative strike given");
            QL_REQUIRE(!dividendTS.isNull(),
                       "VanillaOptionArguments::validate() : "
                       "null dividend term structure");
            QL_REQUIRE(!riskFreeTS.isNull(),
                       "VanillaOptionArguments::validate() : "
                       "null risk free term structure");
            QL_REQUIRE(!exercise.isNull(),
                       "VanillaOptionArguments::validate() : "
                       "null exercise data");
            QL_REQUIRE(!volTS.isNull(),
                       "VanillaOptionArguments::validate() : "
                       "null vol term structure");
        }

        //! %results from vanilla option calculation
        class VanillaOptionResults : public OptionValue, 
                                     public OptionGreeks {};


        //! Vanilla engine base class
        class VanillaEngine : public GenericEngine<VanillaOptionArguments,
                                                   VanillaOptionResults> {};

        //! Pricing engine for Vanilla options using analytical formulae
        class AnalyticalVanillaEngine : public VanillaEngine {
          public:
            void calculate() const;
        };

        //! Pricing engine for Vanilla options using Finite Differences
        class FDVanillaEngine : public VanillaEngine {
          public:
            void calculate() const;
        };

        
        
        //! Pricing engine for Vanilla options using binomial trees
        class BinomialVanillaEngine : public VanillaEngine {
          public:
            enum Type { CoxRossRubinstein, JarrowRudd, LeisenReimer };

            BinomialVanillaEngine(Type type, Size steps)
            : type_(type), steps_(steps) {}
            void calculate() const;
          private:
            Type type_;
            Size steps_;
        };

    }

}

#endif
