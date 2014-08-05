#pragma once

#include "CodeGenHelpers.h"
#include "Node.h"
#include "NativeJIT/X64CodeGenerator.h"     // OpCode type.


namespace NativeJIT
{
    template <OpCode OP, typename L, typename R>
    class BinaryNode : public Node<L>
    {
    public:
        BinaryNode(ExpressionTree& tree, Node<L>& left, Node<R>& right);

        virtual Storage<L> CodeGenValue(ExpressionTree& tree) override;

        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        static unsigned ComputeRegisterCount(unsigned leftCount, unsigned rightCount);

        Node<L>& m_left;
        Node<R>& m_right;
    };


    //*************************************************************************
    //
    // Template definitions for BinaryNode
    //
    //*************************************************************************
    template <OpCode OP, typename L, typename R>
    BinaryNode<OP, L, R>::BinaryNode(ExpressionTree& tree,
                                     Node<L>& left,
                                     Node<R>& right)
        : Node(tree),
          m_left(left),
          m_right(right)
    {
        left.IncrementParentCount();
        right.IncrementParentCount();
    }


    template <OpCode OP, typename L, typename R>
    typename Storage<L> BinaryNode<OP, L, R>::CodeGenValue(ExpressionTree& tree)
    {
        unsigned l = m_left.GetRegisterCount();
        unsigned r = m_right.GetRegisterCount();
        unsigned a = tree.GetAvailableRegisterCount<RegisterType>();

        if (r <= l && r < a)
        {
            // Evaluate left first. Once evaluation completes, left will use one register,
            // leaving at least a-one register for right.
            auto sLeft = m_left.CodeGen(tree);
            sLeft.ConvertToValue(tree, true);
            auto sRight = m_right.CodeGen(tree);

            CodeGenHelpers::Emit<OP>(tree.GetCodeGenerator(), sLeft.GetDirectRegister(), sRight);
            return sLeft;
        }
        else if (l < r && l < a)
        {
            // Evaluate right first. Once evaluation completes, right will use one register,
            // leaving at least one register for left.
            auto sRight = m_right.CodeGen(tree);
            auto sLeft = m_left.CodeGen(tree);
            sLeft.ConvertToValue(tree, true);

            CodeGenHelpers::Emit<OP>(tree.GetCodeGenerator(), sLeft.GetDirectRegister(), sRight);
            return sLeft;
        }
        else
        {
            // The smaller of l and r is greater than a, therefore
            // both l and r are greater than a. Since there are not
            // enough registers available, need to spill to memory.
            auto sRight = m_right.CodeGen(tree);
            sRight.Spill(tree);

            auto sLeft = m_left.CodeGen(tree);
            sLeft.ConvertToValue(tree, true);

            CodeGenHelpers::Emit<OP>(tree.GetCodeGenerator(), sLeft.GetDirectRegister(), sRight);

            return sLeft;
        }
    }


    template <OpCode OP, typename L, typename R>
    unsigned BinaryNode<OP, L, R>::LabelSubtree(bool /*isLeftChild*/)
    {
        unsigned left = m_left.LabelSubtree(true);
        unsigned right = m_right.LabelSubtree(false);

        SetRegisterCount(ComputeRegisterCount(left, right));

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return GetRegisterCount();
    }


    template <OpCode OP, typename L, typename R>
    void BinaryNode<OP, L, R>::Print() const
    {
        std::cout << "Operation (" << X64CodeGenerator::OpCodeName(OP) << ") id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", left = " << m_left.GetId();
        std::cout << ", right = " << m_right.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <OpCode OP, typename L, typename R>
    unsigned BinaryNode<OP, L, R>::ComputeRegisterCount(unsigned left, unsigned right)
    {
        if (left > right)
        {
            return left;
        }
        else if (right > left)
        {
            return right;
        }
        else
        {
            return left + 1;
        }
    }
}