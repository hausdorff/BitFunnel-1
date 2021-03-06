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

#include "gtest/gtest.h"

#include "Allocator.h"
#include "BitFunnel/Plan/RowMatchNode.h"
#include "BitFunnel/Utilities/TextObjectFormatter.h"
#include "MatchTreeRewriter.h"
#include "SameExceptForWhitespace.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    namespace MatchTreeRewriterUnitTest
    {
        struct InputOutput
        {
        public:
            char const * m_input;
            char const * m_output;
            unsigned m_targetRowCount;
            unsigned m_targetCrossProductTermCount;
        };


        const InputOutput c_rewriteCases[] =
        {
            // Single row. Expect it to be copied verbatim.
            // There is no OR-tree in this input tree, so
            // the targetCrossProductTermCount is set to zero.
            {
                "Row(0, 0, 0, false)",
                "And {"
                "  Children: ["
                "    Row(0, 0, 0, false),"
                "    Report {"
                "      Child:"
                "    }"
                "  ]"
                "}",
                4,
                0
            },


            // Four rows. Expect higher rank rows first.
            // There is no OR-tree in this input tree, so
            // the targetCrossProductTermCount is set to zero.
            {
                "And {"
                "  Children: ["
                "    Row(0, 0, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Row(2, 6, 0, false),"
                "    Row(3, 6, 0, false)"
                "  ]"
                "}"
                ,
                "And {"
                "  Children: ["
                "    Row(3, 6, 0, false),"
                "    Row(2, 6, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Row(0, 0, 0, false),"
                "    Report {"
                "      Child:"
                "    }"
                "  ]"
                "}",
                4,
                0
            },


            // One row and one not. Expect not node to be at end.
            // There is no OR-tree in this input tree, so
            // the targetCrossProductTermCount is set to zero.
            {
                "And {"
                "  Children: ["
                "    Not {"
                "      Child: Row(2, 6, 0, false)"
                "    },"
                "    Row(0, 0, 0, false)"
                "  ]"
                "}"
                ,
                "And {"
                "  Children: ["
                "    Row(0, 0, 0, false),"
                "    Report {"
                "      Child: Not {"
                "        Child: Row(2, 0, 6, false)"
                "      }"
                "    }"
                "  ]"
                "}",
                4,
                0
            },


            // Simple or of two ands. Expect same or with rows in descending
            // rank order.
            // There is one OR-tree in this input tree, so
            // the targetCrossProductTermCount is set to two.
            {
                "Or {"
                "  Children: ["
                "    And {"
                "      Children: ["
                "        Row(0, 0, 0, false),"
                "        Row(1, 3, 0, false),"
                "        Row(2, 6, 0, false),"
                "        Row(3, 6, 0, false)"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(4, 0, 0, false),"
                "        Row(5, 3, 0, false),"
                "        Row(6, 6, 0, false),"
                "        Row(7, 6, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}"
                ,
                "Or {"
                "  Children: ["
                "    And {"
                "      Children: ["
                "        Row(3, 6, 0, false),"
                "        Row(2, 6, 0, false),"
                "        Row(1, 3, 0, false),"
                "        Row(0, 0, 0, false),"
                "        Report {"
                "          Child:"
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(7, 6, 0, false),"
                "        Row(6, 6, 0, false),"
                "        Row(5, 3, 0, false),"
                "        Row(4, 0, 0, false),"
                "        Report {"
                "          Child:"
                "        }"
                "      ]"
                "    }"
                "  ]"
                "}",
                4,
                2
            },


            // Distribute three rows and a not over the or of two ands.
            // Expect row 2 and 1 before the or. Expect row 0 and 3
            // at the end of each branch of the or.
            // There is one OR-tree in this input tree, so
            // the targetCrossProductTermCount is set to two.
            {
                "And {"
                "  Children: ["
                "    And {"
                "      Children: ["
                "        Row(0, 0, 0, false),"
                "        Row(1, 3, 0, false),"
                "        Row(2, 6, 0, false),"
                "        Not {"
                "          Child: Row(3, 6, 0, false)"
                "        }"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        And {"
                "          Children: ["
                "            Row(4, 0, 0, false),"
                "            Row(5, 3, 0, false),"
                "            Row(6, 6, 0, false),"
                "            Row(7, 6, 0, false)"
                "          ]"
                "        },"
                "        And {"
                "          Children: ["
                "            Row(8, 0, 0, false),"
                "            Row(9, 3, 0, false),"
                "            Row(10, 6, 0, false),"
                "            Row(11, 6, 0, false)"
                "          ]"
                "        }"
                "      ]"
                "    }"
                "  ]"
                "}"
                ,
                "And {"
                "  Children: ["
                "    Row(2, 6, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Or {"
                "      Children: ["
                "        And {"
                "          Children: ["
                "            Row(7, 3, 3, false),"
                "            Row(6, 3, 3, false),"
                "            Row(5, 3, 0, false),"
                "            Row(4, 0, 0, false),"
                "            Row(0, 0, 0, false),"
                "            Report {"
                "              Child: Not {"
                "                Child: Row(3, 0, 6, false)"
                "              }"
                "            }"
                "          ]"
                "        },"
                "        And {"
                "          Children: ["
                "            Row(11, 3, 3, false),"
                "            Row(10, 3, 3, false),"
                "            Row(9, 3, 0, false),"
                "            Row(8, 0, 0, false),"
                "            Row(0, 0, 0, false),"
                "            Report {"
                "              Child: Not {"
                "                Child: Row(3, 0, 6, false)"
                "              }"
                "            }"
                "          ]"
                "        }"
                "      ]"
                "    }"
                "  ]"
                "}",
                4,
                2
            },


            // Distribute three rows and a not over two ors of two rows.
            // There is two OR-trees in this input tree, so
            // the targetCrossProductTermCount is set to four.
            {
                "And {"
                "  Children: ["
                "    And {"
                "      Children: ["
                "        Row(0, 0, 0, false),"
                "        Row(1, 3, 0, false),"
                "        Row(2, 6, 0, false),"
                "        Not {"
                "          Child: Row(3, 6, 0, false)"
                "        }"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(4, 3, 0, false),"
                "        Row(5, 3, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(6, 3, 0, false),"
                "        Row(7, 3, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}"
                ,
                "And {"
                "  Children: ["
                "    Row(2, 6, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Or {"
                "      Children: ["
                "        And {"
                "          Children: ["
                "            Row(6, 3, 0, false),"
                "            Or {"
                "              Children: ["
                "                And {"
                "                  Children: ["
                "                    Row(4, 3, 0, false),"
                "                    Row(0, 0, 0, false),"
                "                    Report {"
                "                      Child: Not {"
                "                        Child: Row(3, 0, 6, false)"
                "                      }"
                "                    }"
                "                  ]"
                "                },"
                "                And {"
                "                  Children: ["
                "                    Row(5, 3, 0, false),"
                "                    Row(0, 0, 0, false),"
                "                    Report {"
                "                      Child: Not {"
                "                        Child: Row(3, 0, 6, false)"
                "                      }"
                "                    }"
                "                  ]"
                "                }"
                "              ]"
                "            }"
                "          ]"
                "        },"
                "        And {"
                "          Children: ["
                "            Row(7, 3, 0, false),"
                "            Or {"
                "              Children: ["
                "                And {"
                "                  Children: ["
                "                    Row(4, 3, 0, false),"
                "                    Row(0, 0, 0, false),"
                "                    Report {"
                "                      Child: Not {"
                "                        Child: Row(3, 0, 6, false)"
                "                      }"
                "                    }"
                "                  ]"
                "                },"
                "                And {"
                "                  Children: ["
                "                    Row(5, 3, 0, false),"
                "                    Row(0, 0, 0, false),"
                "                    Report {"
                "                      Child: Not {"
                "                        Child: Row(3, 0, 6, false)"
                "                      }"
                "                    }"
                "                  ]"
                "                }"
                "              ]"
                "            }"
                "          ]"
                "        }"
                "      ]"
                "    }"
                "  ]"
                "}",
                8,
                4
            },


            // Multiply out three ors of three rows.
            // Since the cross product term count is 4, only 4 out of 27
            // terms are generated.
            {
                "And {"
                "  Children: ["
                "    Or {"
                "      Children: ["
                "        Row(1, 0, 0, false),"
                "        Row(2, 0, 0, false),"
                "        Row(3, 0, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(4, 0, 0, false),"
                "        Row(5, 0, 0, false),"
                "        Row(6, 0, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(7, 0, 0, false),"
                "        Row(8, 0, 0, false),"
                "        Row(9, 0, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}"
                ,
                "Or {"
                "  Children: ["
                "    And {"
                "      Children: ["
                "        Row(1, 0, 0, false),"
                "        Row(4, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(2, 0, 0, false),"
                "        Row(4, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(3, 0, 0, false),"
                "        Row(4, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(1, 0, 0, false),"
                "        Row(5, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(5, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Or {"
                "          Children: ["
                "            Row(2, 0, 0, false),"
                "            Row(3, 0, 0, false)"
                "          ]"
                "        },"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(6, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Or {"
                "          Children: ["
                "            Row(1, 0, 0, false),"
                "            Row(2, 0, 0, false),"
                "            Row(3, 0, 0, false)"
                "          ]"
                "        },"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Or {"
                "          Children: ["
                "            Row(8, 0, 0, false),"
                "            Row(9, 0, 0, false)"
                "          ]"
                "        },"
                "        Or {"
                "          Children: ["
                "            Row(4, 0, 0, false),"
                "            Row(5, 0, 0, false),"
                "            Row(6, 0, 0, false)"
                "          ]"
                "        },"
                "        Or {"
                "          Children: ["
                "            Row(1, 0, 0, false),"
                "            Row(2, 0, 0, false),"
                "            Row(3, 0, 0, false)"
                "          ]"
                "        },"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    }"
                "  ]"
                "}",
                4,
                4
            },

            // Multiply out three ors of three rows.
            // The target fanout is 5. After 5 terms are generated,
            // we have to generated the 6th term, so finally 6 terms
            // are generated.
            {
                "And {"
                "  Children: ["
                "    Or {"
                "      Children: ["
                "        Row(1, 0, 0, false),"
                "        Row(2, 0, 0, false),"
                "        Row(3, 0, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(4, 0, 0, false),"
                "        Row(5, 0, 0, false),"
                "        Row(6, 0, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(7, 0, 0, false),"
                "        Row(8, 0, 0, false),"
                "        Row(9, 0, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}"
                ,
                "Or {"
                "  Children: ["
                "    And {"
                "      Children: ["
                "        Row(1, 0, 0, false),"
                "        Row(4, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(2, 0, 0, false),"
                "        Row(4, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(3, 0, 0, false),"
                "        Row(4, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(1, 0, 0, false),"
                "        Row(5, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(2, 0, 0, false),"
                "        Row(5, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(3, 0, 0, false),"
                "        Row(5, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(6, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Or {"
                "          Children: ["
                "            Row(1, 0, 0, false),"
                "            Row(2, 0, 0, false),"
                "            Row(3, 0, 0, false)"
                "          ]"
                "        },"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Or {"
                "          Children: ["
                "            Row(8, 0, 0, false),"
                "            Row(9, 0, 0, false)"
                "          ]"
                "        },"
                "        Or {"
                "          Children: ["
                "            Row(4, 0, 0, false),"
                "            Row(5, 0, 0, false),"
                "            Row(6, 0, 0, false)"
                "          ]"
                "        },"
                "        Or {"
                "          Children: ["
                "            Row(1, 0, 0, false),"
                "            Row(2, 0, 0, false),"
                "            Row(3, 0, 0, false)"
                "          ]"
                "        },"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    }"
                "  ]"
                "}",
                4,
                5
            },

            // Multiply out three ors of three rows.
            // The target cross product term count is 6,
            // so 6 of out 27 terms are generated.
            {
                "And {"
                "  Children: ["
                "    Or {"
                "      Children: ["
                "        Row(1, 0, 0, false),"
                "        Row(2, 0, 0, false),"
                "        Row(3, 0, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(4, 0, 0, false),"
                "        Row(5, 0, 0, false),"
                "        Row(6, 0, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(7, 0, 0, false),"
                "        Row(8, 0, 0, false),"
                "        Row(9, 0, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}"
                ,
                "Or {"
                "  Children: ["
                "    And {"
                "      Children: ["
                "        Row(1, 0, 0, false),"
                "        Row(4, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(2, 0, 0, false),"
                "        Row(4, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(3, 0, 0, false),"
                "        Row(4, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(1, 0, 0, false),"
                "        Row(5, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(2, 0, 0, false),"
                "        Row(5, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(3, 0, 0, false),"
                "        Row(5, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Row(6, 0, 0, false),"
                "        Row(7, 0, 0, false),"
                "        Or {"
                "          Children: ["
                "            Row(1, 0, 0, false),"
                "            Row(2, 0, 0, false),"
                "            Row(3, 0, 0, false)"
                "          ]"
                "        },"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    },"
                "    And {"
                "      Children: ["
                "        Or {"
                "          Children: ["
                "            Row(8, 0, 0, false),"
                "            Row(9, 0, 0, false)"
                "          ]"
                "        },"
                "        Or {"
                "          Children: ["
                "            Row(4, 0, 0, false),"
                "            Row(5, 0, 0, false),"
                "            Row(6, 0, 0, false)"
                "          ]"
                "        },"
                "        Or {"
                "          Children: ["
                "            Row(1, 0, 0, false),"
                "            Row(2, 0, 0, false),"
                "            Row(3, 0, 0, false)"
                "          ]"
                "        },"
                "        Report {"
                "          Child: "
                "        }"
                "      ]"
                "    }"
                "  ]"
                "}",
                4,
                6
            },


            // Distribute three rows and a not over two ors of two rows.
            // Since the target cross product term count is 2,
            // only R4 and R5 are multiplied out and combined with R0.
            {
                "And {"
                "  Children: ["
                "    And {"
                "      Children: ["
                "        Row(0, 0, 0, false),"
                "        Row(1, 3, 0, false),"
                "        Row(2, 6, 0, false),"
                "        Not {"
                "          Child: Row(3, 6, 0, false)"
                "        }"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(4, 3, 0, false),"
                "        Row(5, 3, 0, false)"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        Row(6, 3, 0, false),"
                "        Row(7, 3, 0, false)"
                "      ]"
                "    }"
                "  ]"
                "}"
                ,
                "And {"
                "  Children: ["
                "    Row(2, 6, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Or {"
                "      Children: ["
                "        And {"
                "          Children: ["
                "            Row(6, 3, 0, false),"
                "            Or {"
                "              Children: ["
                "                And {"
                "                  Children: ["
                "                    Row(4, 3, 0, false),"
                "                    Row(0, 0, 0, false),"
                "                    Report {"
                "                      Child: Not {"
                "                        Child: Row(3, 0, 6, false)"
                "                      }"
                "                    }"
                "                  ]"
                "                },"
                "                And {"
                "                  Children: ["
                "                    Row(5, 3, 0, false),"
                "                    Row(0, 0, 0, false),"
                "                    Report {"
                "                      Child: Not {"
                "                        Child: Row(3, 0, 6, false)"
                "                      }"
                "                    }"
                "                  ]"
                "                }"
                "              ]"
                "            }"
                "          ]"
                "        },"
                "        And {"
                "          Children: ["
                "            Row(7, 3, 0, false),"
                "            Row(0, 0, 0, false),"
                "            Or {"
                "              Children: ["
                "                Row(4, 0, 3, false),"
                "                Row(5, 0, 3, false)"
                "              ]"
                "            },"
                "            Report {"
                "              Child: Not {"
                "                Child: Row(3, 0, 6, false)"
                "              }"
                "            }"
                "          ]"
                "        }"
                "      ]"
                "    }"
                "  ]"
                "}",
                8,
                2
            },


            // A RowMatchTree with a Not node in Or tree.
            // Set targetCrossProductTermCount to zero to check
            // that the entire Or tree should be put under a report
            // node.
            {
                "And {"
                "  Children: ["
                "    And {"
                "      Children: ["
                "        Row(0, 0, 0, false),"
                "        Row(1, 3, 0, false),"
                "        Row(2, 6, 0, false),"
                "        Not {"
                "          Child: Row(3, 6, 0, false)"
                "        }"
                "      ]"
                "    },"
                "    Or {"
                "      Children: ["
                "        And {"
                "          Children: ["
                "            Row(4, 0, 0, false),"
                "            Row(5, 3, 0, false),"
                "            Row(6, 6, 0, false),"
                "            Not {"
                "              Child: Row(7, 6, 0, false)"
                "            }"
                "          ]"
                "        },"
                "        And {"
                "          Children: ["
                "            Row(8, 0, 0, false),"
                "            Row(9, 3, 0, false),"
                "            Row(10, 6, 0, false),"
                "            Not {"
                "              Child: Row(11, 6, 0, false)"
                "            }"
                "          ]"
                "        }"
                "      ]"
                "    }"
                "  ]"
                "}"
                ,
                "And {"
                "  Children: ["
                "    Row(2, 6, 0, false),"
                "    Row(1, 3, 0, false),"
                "    Row(0, 0, 0, false),"
                "    Report {"
                "      Child: And {"
                "        Children: ["
                "          Or {"
                "            Children: ["
                "              And {"
                "                Children: ["
                "                  Row(4, 0, 0, false),"
                "                  Row(5, 0, 3, false),"
                "                  Row(6, 0, 6, false),"
                "                  Not {"
                "                    Child: Row(7, 0, 6, false)"
                "                  }"
                "                ]"
                "              },"
                "              And {"
                "                Children: ["
                "                  Row(8, 0, 0, false),"
                "                  Row(9, 0, 3, false),"
                "                  Row(10, 0, 6, false),"
                "                  Not {"
                "                    Child: Row(11, 0, 6, false)"
                "                  }"
                "                ]"
                "              }"
                "            ]"
                "          },"
                "          Not {"
                "            Child: Row(3, 0, 6, false)"
                "          }"
                "        ]"
                "      }"
                "    }"
                "  ]"
                "}",
                4,
                0
            },
        };



        //void PrettyPrint(IPersistableObject const & object)
        //{
        //    std::stringstream output;
        //    TextObjectFormatter formatter(output);
        //    object.Format(formatter);
        //    std::cout << "\"" << output.str() << "\"" << std::endl;
        //}


        void VerifyCase(InputOutput const & testCase)
        {
            std::stringstream input(testCase.m_input);

            // TODO: is this size ok?
            Allocator allocator(1024*4);
            TextObjectParser parser(input, allocator, &RowPlanBase::GetType);
            RowMatchNode const & root = RowMatchNode::Parse(parser);

            RowMatchNode const & converted = MatchTreeRewriter::Rewrite(root,
                                                                        testCase.m_targetRowCount,
                                                                        testCase.m_targetCrossProductTermCount,
                                                                        allocator);

            std::stringstream output;
            TextObjectFormatter formatter(output);
            converted.Format(formatter);

            EXPECT_TRUE(SameExceptForWhitespace(output.str().c_str(), testCase.m_output));
        }


        TEST(MatchTreeRewriter, Basic)
        {
            for (unsigned i = 0; i < sizeof(c_rewriteCases) / sizeof(InputOutput); ++i)
            {
                VerifyCase(c_rewriteCases[i]);
            }
        }
    }
}
