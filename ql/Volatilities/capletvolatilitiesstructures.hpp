/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006  Fran�ois du Vignaud

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

/*! \file CapletVolatilitiesStructures.hpp
    \brief Caplet Volatilities Structures used during bootstrapping procedure
*/

#ifndef caplet_volatilities_structures_hpp
#define caplet_volatilities_structures_hpp

#include <ql/capvolstructures.hpp>
#include <ql/Volatilities/smilesection.hpp>
#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    typedef std::vector<Handle<InterpolatedSmileSection> > \
        SmileSectionInterfaceVector;
    typedef std::vector<std::vector<boost::shared_ptr<CapFloor> > > CapMatrix;

    class ImpliedVolHelper{
    public:
        ImpliedVolHelper(boost::shared_ptr<CapFloor> cap,
                         Real targetValue,
                         Real& volatilityParameter):
                         targetValue_(targetValue), cap_(cap),
                         volatilityParameter_(volatilityParameter){};

        Real operator()(Real x) const {
            volatilityParameter_ = x;
            cap_->update();
            return cap_->NPV() - targetValue_;
        };
    private:
        Real targetValue_;
        boost::shared_ptr<CapFloor> cap_;
        Real& volatilityParameter_;
    };

    inline void fitVolatilityParameter(boost::shared_ptr<CapFloor> mkData,
                                    Real& volatilityParameter,
                                    Real targetValue,
                                    Real accuracy = 1e-5,
                                    Size maxEvaluations = 1000,
                                    Volatility minVol = 1e-4,
                                    Volatility maxVol = 4){
        ImpliedVolHelper f(mkData, targetValue, volatilityParameter);
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        volatilityParameter = solver.solve(f, accuracy, volatilityParameter, minVol, maxVol);
    }

   class SmileSectionsVolStructure: public CapletVolatilityStructure{
    public:
        SmileSectionsVolStructure(const Date& referenceDate,
                           const DayCounter& dayCounter,
                           const SmileSectionInterfaceVector& smileSections);

        Volatility volatilityImpl(Time length,
            Rate strike) const;

        void setClosestTenors(Time time, Time& nextLowerTenor,
            Time& nextHigherTenor) const;

        Time maxTime() const{ return tenorTimes_.back();}

        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        DayCounter dayCounter() const;
        //@}

        //! \name CapletVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
    private:
        SmileSectionInterfaceVector smileSections_;
        DayCounter dayCounter_;
        std::vector<Time> tenorTimes_;
        Rate minStrike_, maxStrike_;
        Date maxDate_;
    };

    class ParametrizedCapletVolStructure:
       public CapletVolatilityStructure{
    public:
        ParametrizedCapletVolStructure(Date referenceDate):
          CapletVolatilityStructure(referenceDate){};

       virtual Matrix& volatilityParameters() const = 0;
    };

    class BilinInterpCapletVolStructure:
        public ParametrizedCapletVolStructure{
    public:
        BilinInterpCapletVolStructure(
            const Date& referenceDate,
            const DayCounter dayCounter,
            const CapMatrix& referenceCaps,
            const std::vector<Rate>& strikes);

        Volatility volatilityImpl(Time length, Rate strike) const;

        void setClosestTenors(Time time,
            Time& nextLowerTenor, Time& nextHigherTenor);

        Time minTime() const{ return tenorTimes_.front();}

        Real& volatilityParameter(Size i, Size j) const {
            return volatilities_[i][j];
        }

        Matrix& volatilityParameters() const {
            return volatilities_;
        }

        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        DayCounter dayCounter() const;
        //@}

        //! \name CapletVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
    private:
        DayCounter dayCounter_;
        LinearInterpolation firstRowInterpolator_;
        std::vector<Time> tenorTimes_;
        std::vector<Rate> strikes_;
        mutable Matrix volatilities_;
        boost::shared_ptr<BilinearInterpolation> bilinearInterpolation_;
        Date maxDate_;
        Rate maxStrike_, minStrike_;
    };


    class HybridCapletVolatilityStructure: 
        public ParametrizedCapletVolStructure{
    public:
        HybridCapletVolatilityStructure(
            const Date& referenceDate,
            const DayCounter dayCounter,
            const CapMatrix& referenceCaps, 
            const std::vector<Rate>& strikes,
            const SmileSectionInterfaceVector& smileSections);

        Volatility volatilityImpl(Time length,
                                  Rate strike) const;
        
        Matrix& volatilityParameters() const {
            return volatilitiesFromCaps_->volatilityParameters();
        }

        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        DayCounter dayCounter() const;
        //@}

        //! \name CapletVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
    private:
        DayCounter dayCounter_;
        Time overlapStart, overlapEnd;
        boost::shared_ptr<BilinInterpCapletVolStructure>
            volatilitiesFromCaps_;
        boost::shared_ptr<SmileSectionsVolStructure>
            volatilitiesFromFutureOptions_;
        Date maxDate_;
        Rate minStrike_, maxStrike_;
    };

}

#endif