/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef YieldExpressionNode_h
#define YieldExpressionNode_h

#include "ExpressionNode.h"

namespace Escargot {

class YieldExpressionNode : public ExpressionNode {
public:
    YieldExpressionNode(RefPtr<Node> argument, bool isDelegate)
        : ExpressionNode()
        , m_argument(argument)
        , m_isDelegate(isDelegate)
    {
    }

    virtual ASTNodeType type() { return ASTNodeType::YieldExpression; }
    virtual void generateExpressionByteCode(ByteCodeBlock* codeBlock, ByteCodeGenerateContext* context, ByteCodeRegisterIndex dstRegister)
    {
        if (m_argument == nullptr) {
            codeBlock->pushCode(Yield(ByteCodeLOC(m_loc.index), REGISTER_LIMIT, dstRegister), context, this);
        } else {
            size_t argIdx = m_argument->getRegister(codeBlock, context);
            m_argument->generateExpressionByteCode(codeBlock, context, argIdx);

            if (m_isDelegate == true) {
                size_t iteratorIdx = context->getRegister();
                codeBlock->pushCode(GetIterator(ByteCodeLOC(m_loc.index), argIdx, iteratorIdx), context, this);
                size_t loopStart = codeBlock->currentCodeSize();

                size_t valueIdx = context->getRegister();
                codeBlock->pushCode(YieldDelegate(ByteCodeLOC(m_loc.index), iteratorIdx, valueIdx, dstRegister), context, this);
                size_t iterStepPos = codeBlock->lastCodePosition<YieldDelegate>();

                context->giveUpRegister(); // for drop valueIdx

                codeBlock->pushCode(Jump(ByteCodeLOC(m_loc.index), loopStart), context, this);
                codeBlock->peekCode<YieldDelegate>(iterStepPos)->m_endPosition = codeBlock->currentCodeSize();
                context->giveUpRegister(); // for drop iteratorIdx
            } else {
                codeBlock->pushCode(Yield(ByteCodeLOC(m_loc.index), argIdx, dstRegister), context, this);
            }

            context->giveUpRegister(); // for drop argIdx
        }
    }

private:
    RefPtr<Node> m_argument;
    bool m_isDelegate : 1;
};
}

#endif
