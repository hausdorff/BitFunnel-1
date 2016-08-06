// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <vector>               // std::vector member.

#include "ITermTreatment.h"     // Base class.


namespace BitFunnel
{
    class TreatmentPrivateRank0 : public ITermTreatment
    {
    public:
        TreatmentPrivateRank0();

        //
        // ITermTreatment methods.
        //

        virtual RowConfiguration GetTreatment(Term term) const override;

    private:
        RowConfiguration m_configuration;
    };


    class TreatmentPrivateSharedRank0 : public ITermTreatment
    {
    public:
        TreatmentPrivateSharedRank0(double density, double snr);

        //
        // ITermTreatment methods.
        //

        virtual RowConfiguration GetTreatment(Term term) const override;

    private:
        std::vector<RowConfiguration> m_configurations;
    };


    class TreatmentPrivateSharedRank0and3 : public ITermTreatment
    {
    public:
        TreatmentPrivateSharedRank0and3(double density, double snr);

        //
        // ITermTreatment methods.
        //

        virtual RowConfiguration GetTreatment(Term term) const override;

    private:
        std::vector<RowConfiguration> m_configurations;
    };
}
