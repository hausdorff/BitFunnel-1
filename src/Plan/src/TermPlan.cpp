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

#include "BitFunnel/Plan/TermPlan.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TermPlan
    //
    //*************************************************************************
    TermPlan::TermPlan(TermMatchNode const & matchTree,
                       IScoringEngine const & scoringEngine,
                       QueryPreferences const & queryPreferences)
        : m_matchTree(matchTree),
          m_scoringEngine(scoringEngine),
          m_queryPreferences(queryPreferences)
    {
    }


    TermPlan::~TermPlan()
    {
        // WARNING: TermPlan is designed to be allocated by an arena
        // allocator, so this destructor will never be called. Therefore,
        // TermPlan should hold no resources other than memory from
        // the arena allocator.
    }


    TermMatchNode const & TermPlan::GetMatchTree() const
    {
        return m_matchTree;
    }


    IScoringEngine const & TermPlan::GetScoringEngine() const
    {
        return m_scoringEngine;
    }


    QueryPreferences const & TermPlan::GetQueryPreferences() const
    {
        return m_queryPreferences;
    }
}
