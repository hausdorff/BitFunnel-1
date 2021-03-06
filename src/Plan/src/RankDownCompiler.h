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

#include "BitFunnel/BitFunnelTypes.h"     // Rank used as a parameter.
#include "BitFunnel/NonCopyable.h"        // Inherits from NonCopyable.


namespace BitFunnel
{
    namespace Allocators
    {
        class IAllocator;
    }

    class CompileNode;
    class RowMatchNode;


    class RankDownCompiler : NonCopyable
    {
    public:
        RankDownCompiler(IAllocator& allocator);

        // DESIGN NOTE: Normally we would prefer a modeless class where all of
        // the work happened in the constructor and the CompileNode const & was
        // accessed via a const method. In this case we've broken out separate
        // Compile() and Create() tree methods because when compiling an Or
        // node, we must first compile the children and examine their ranks
        // before we can supply the initalRank parameter.

        void Compile(RowMatchNode const & root);

        CompileNode const & CreateTree(Rank initialRank);

    private:
        void CompileInternal(RowMatchNode const & root, bool leftMostChild);

        void CompileTraversal(RowMatchNode const & node,
                              bool leftMostChild);

        CompileNode const & RankUp(CompileNode const & node);

        IAllocator& m_allocator;
        Rank m_currentRank;
        CompileNode const * m_accumulator;
    };
}
