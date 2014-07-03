#ifndef __VTL_OPERATOR_HPP__
#define __VTL_OPERATOR_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2003

Module Name:
    Generic Operator definition for vectors, or other containers
    operations assumes:
        1. item is accessed by at(size_t)
        2. items are in continuous memory block begin from &at(0);

Abstract:
    
Notes:
     These MACROs requires T has following:
        T::ValueType           // the type of each item
        T::ConstValue          // the type of const item type
        T::LengthType          // the type of vector length
        T::AccumType           // the type of accumulation of items
        T::at(size_t k)         // the reference to each item in mem-block
        size_t T::size()const   // the size of the vector
        T::resize(const T&)     // the allocator to copy size from another

NOTE:
    1. We do not use operator[](size_t) here
       because Vector and Matrix may define different operator []
       On the other hand, at(size_t k) is defined to mem-block wise item
        
       e.g. for vector, at(size_t) is same as operator[],
       however, for matrix they are not same. matrix[] returns row pointer.

    2. We do not use resize(size_t c) here
        because vector and matrix may have different resize function.
        One is resize(size_t), and the other is resize(size_t, size_t)
        All we need in operators is to copy size from another.

Usage:
        
History:
    Created  on 2003 Aug 18 by oliver_liyin
          
\*************************************************************************/

#include "Vtl_Utility.hpp"

namespace vtl{

// ------------------------------------------------
// REPEAT MACROs
// ------------------------------------------------

    /// [these repeat macro is to expande vector operation, by give number N].
    #define REPEAT_1(STEP) STEP(0)
    #define REPEAT_2(STEP) REPEAT_1(STEP); STEP(1)
    #define REPEAT_3(STEP) REPEAT_2(STEP); STEP(2)
    #define REPEAT_4(STEP) REPEAT_3(STEP); STEP(3)
    #define REPEAT_5(STEP) REPEAT_4(STEP); STEP(4)
    #define REPEAT_6(STEP) REPEAT_5(STEP); STEP(5)

    /// [this macro takes number 0 , hence it use vector::size() as counter].
    /// if the size is small, it can be optimized by compiler
    #define REPEAT_0(STEP)                      \
        const size_t _N_ = size();              \
        for (size_t _k_ = 0; _k_ < _N_; _k_ ++) \
        {                                       \
            STEP(_k_);                          \
        }

    // REPEAT_##N##N is for friend functions, takes _V_ as given size
    #define REPEAT_00(STEP)                     \
        const size_t _N_ = _V_.size();          \
        for (size_t _k_ = 0; _k_ < _N_; _k_ ++) \
        {                                       \
            STEP(_k_);                          \
        }
    
    #define REPEAT_11(STEP) REPEAT_1(STEP)
    #define REPEAT_22(STEP) REPEAT_2(STEP)
    #define REPEAT_33(STEP) REPEAT_3(STEP)
    #define REPEAT_44(STEP) REPEAT_4(STEP)
    #define REPEAT_55(STEP) REPEAT_5(STEP)
    #define REPEAT_66(STEP) REPEAT_6(STEP)


// ------------------------------------------------
// VECTOR_OP_BASIC
//  Vector operator basic
//      1. Copy(SRC& src);  // allow across type copy
//      2. Sum(void) const;     // sum all elements
//      3. Fill(ConstValue s)  // fill in scalar value
//      4. SetZero(void);       // fill bit field zero
//      5. bool EqualsTo(const T& v) const;
// ------------------------------------------------

    /// [These cmd_xxx functions are one single step in iteration].
    #define cmd_fill(N)     { at(N) = xtl::RealCast<U>(_V_);}
    #define cmd_equalsto(N) { if(!vtl::EqualsTo(at(N), _V_.at(N), _T_)) return false;}
    #define cmd_iszero(N)   { if(!vtl::IsZero(at(N), _T_)) return false;}

    // this macro is for vectors with basic operators, repeated version
    #define VECTOR_OP_BASIC(T, N)                                   \
        T& SetZero(void)                                            \
        {                                                           \
            memset(&at(0), 0, size() * sizeof(ValueType));          \
            return *this;                                           \
        }                                                           \
        T& SetValue(ConstValue _V_)                                 \
        {                                                           \
            typedef T::ValueType U;                                 \
            REPEAT_##N(cmd_fill);                                   \
            return *this;                                           \
        }                                                           \
        bool EqualsTo(const T& _V_, ValueType _T_                   \
            = xtl::Limits<ValueType>::denorm_min()) const           \
        {                                                           \
            REPEAT_##N(cmd_equalsto);                               \
            return true;                                            \
        }                                                           \
        bool IsZero(ValueType _T_                                   \
            = xtl::Limits<ValueType>::denorm_min()) const           \
        {                                                           \
            REPEAT_##N(cmd_iszero);                                 \
            return true;                                            \
        }                                                           \
        
// ------------------------------------------------
// VECTOR_OP_ARITHMATIC
// Vector OP Arithmetic, include
//  1. +, -, *, /
//  2. +=, -=, *=, /=
//  3. ++, --
//  4. -,
// ------------------------------------------------


    // a += b;
    #define cmd_op_eq_add(N)   { at(N) = xtl::RealCast<U>(at(N) + _V_.at(N));}
    #define cmd_op_eq_sub(N)   { at(N) = xtl::RealCast<U>(at(N) - _V_.at(N));}
    #define cmd_op_eq_mul(N)   { at(N) = xtl::RealCast<U>(at(N) * _V_.at(N));}
    #define cmd_op_eq_div(N)   { at(N) = xtl::RealCast<U>((_V_.at(N) == 0) ? 0 : at(N) / _V_.at(N));}

    // a += 6;
    #define cmd_op_eq_s_add(N)   { at(N) = xtl::RealCast<U>(at(N) + _S_);}
    #define cmd_op_eq_s_sub(N)   { at(N) = xtl::RealCast<U>(at(N) - _S_);}
    #define cmd_op_eq_s_mul(N)   { at(N) = xtl::RealCast<U>(at(N) * _S_);}
    #define cmd_op_eq_s_div(N)   { at(N) = (_S_ == 0) ? 0 : xtl::RealCast<U>(at(N) / _S_);}

    // c = 6 + a;
    #define cmd_op_s_v_add(N)    { _R_.at(N) = xtl::RealCast<U>(_S_ + _V_.at(N));}
    #define cmd_op_s_v_sub(N)    { _R_.at(N) = xtl::RealCast<U>(_S_ - _V_.at(N));}
    #define cmd_op_s_v_mul(N)    { _R_.at(N) = xtl::RealCast<U>(_S_ * _V_.at(N));}
    #define cmd_op_s_v_div(N)    { _R_.at(N) = xtl::RealCast<U>((_V_.at(N) == 0) ? 0 : _S_ / _V_.at(N));}

    // a ++; ++ a;
    #define cmd_op_self_add(N)         { _R_.at(N) ++;}
    #define cmd_op_self_sub(N)         { _R_.at(N) --;}
    #define cmd_op_self2_add(N)        { ++ at(N);}
    #define cmd_op_self2_sub(N)        { -- at(N);}

    // c = -a;
    #define cmd_op_single_negative(N)  { _R_.at(N) = xtl::RealCast<U>(- at(N));}
    #define cmd_op_single_not(N)       { _R_.at(N) = xtl::RealCast<U>(! at(N));}

    // A += B;
    #define M_OP_OPEQ(T, N, NAME, OP)                               \
        T& operator OP##= (const T& _V_)                            \
        {                                                           \
            typedef T::ValueType U;                                 \
            REPEAT_##N(cmd_op_eq_##NAME);                           \
            return *this;                                           \
        }                                                           \
        const T operator OP (const T& _V_) const                    \
        {                                                           \
            return T(*this) OP##= _V_;                              \
        }                                                           \

    // A *= 3.f;
    // A = B / 6;
    #define M_OP_SCALAR(T, N, NAME, OP, S)                          \
        T& operator OP##= (S _S_)                                   \
        {                                                           \
            typedef T::ValueType U;                                 \
            REPEAT_##N(cmd_op_eq_s_##NAME);                         \
            return *this;                                           \
        }                                                           \
        const T operator OP (const S _S_) const                     \
        {                                                           \
            return T(*this) OP##= _S_;                              \
        }                                                           \
        friend const T operator OP(                                 \
            const S _S_, const T& _V_)                              \
        {                                                           \
            T _R_;                                                  \
            _R_.resize(_V_);                                        \
            typedef T::ValueType U;                                 \
            REPEAT_##N##N(cmd_op_s_v_##NAME);                       \
            return _R_;                                             \
        }

    // A = -A;
    #define M_OP_SINGLE(T, N, NAME, OP)                             \
        const T operator OP() const                                 \
        {                                                           \
            T _R_;                                                  \
            _R_.resize(*this);                                      \
            typedef T::ValueType U;                                 \
            REPEAT_##N(cmd_op_single_##NAME);                       \
            return _R_;                                             \
        }                                                           \

    // A++; --B;
    #define M_OP_SELF(T, N, NAME, OP)                               \
        const T operator OP(int) const                              \
        {                                                           \
            T _R_(*this);                                           \
            REPEAT_##N(cmd_op_self_##NAME);                         \
            return _R_;                                             \
        }                                                           \
        T& operator OP()                                            \
        {                                                           \
            REPEAT_##N(cmd_op_self2_##NAME);                        \
            return *this;                                           \
        }                                                           \


    #define VECTOR_OP_ARITHMATIC(T, N)                           \
        M_OP_OPEQ(T, N, add, +)                                  \
        M_OP_OPEQ(T, N, sub, -)                                  \
        M_OP_OPEQ(T, N, mul, *)                                  \
        M_OP_OPEQ(T, N, div, /)                                  \
        M_OP_SCALAR(T, N, add, +, int)                           \
        M_OP_SCALAR(T, N, sub, -, int)                           \
        M_OP_SCALAR(T, N, mul, *, int)                           \
        M_OP_SCALAR(T, N, div, /, int)                           \
        M_OP_SCALAR(T, N, add, +, unsigned int)                  \
        M_OP_SCALAR(T, N, sub, -, unsigned int)                  \
        M_OP_SCALAR(T, N, mul, *, unsigned int)                  \
        M_OP_SCALAR(T, N, div, /, unsigned int)                  \
        M_OP_SCALAR(T, N, add, +, float)                         \
        M_OP_SCALAR(T, N, sub, -, float)                         \
        M_OP_SCALAR(T, N, mul, *, float)                         \
        M_OP_SCALAR(T, N, div, /, float)                         \
        M_OP_SCALAR(T, N, sub, -, double)                        \
        M_OP_SCALAR(T, N, mul, *, double)                        \
        M_OP_SCALAR(T, N, add, +, double)                        \
        M_OP_SCALAR(T, N, div, /, double)                        \
        M_OP_SELF(T, N, add, ++)                                 \
        M_OP_SELF(T, N, sub, --)                                 \
        M_OP_SINGLE(T, N, negative, -)                           \
        M_OP_SINGLE(T, N, not, !)                                \


// ------------------------------------------------
// VECTOR_OP_GEOMETRIC
// Vector OP Geometric, include
//  1. LengthType Norm(int) const; i == 0, 1, 2; Must be non-negative
//  2. void Normalize(), and T Normalized() const;
//  3. AbsMaxElement, [NOT_IMPL] MaxElement, MinElement
//  2. SquareLength()const, Length()const, AbsLength() const
//  4. ApplyLength();
//  5. Dot(const V& v);
//  6. Lerp(const V& v, alpha) const;
//  7. Clamp(const V& a, const V& b);
// ------------------------------------------------

    #define cmd_sum(N)     { _sum_ = xtl::RealCast<U>(_sum_ + at(N));}
    #define cmd_abs_max(N) { if(_max_ < abs(at(N))) _max_ = xtl::RealCast<U>(abs(at(N)));}
    #define cmd_abs_sum(N) { _sum_ = xtl::RealCast<U>(_sum_ + abs(at(N)));}
    #define cmd_dot(N)     { _sum_ = xtl::RealCast<U>(_sum_ + at(N) * _V_.at(N));}
    #define cmd_clamp(N)   { at(N) = xtl::RealCast<U>(max(_A_.at(N), min(_B_.at(N), at(N))));}

    #define VECTOR_OP_GEOMETRIC(T, N)                               \
        const AccumType Sum(void) const                             \
        {                                                           \
            AccumType _sum_ = 0;                                    \
            typedef T::ValueType U;                                 \
            REPEAT_##N(cmd_sum);                                    \
            return _sum_;                                           \
        }                                                           \
        AccumType Dot(const T& _V_) const                           \
        {                                                           \
            AccumType _sum_ = 0;                                    \
            typedef T::ValueType U;                                 \
            REPEAT_##N(cmd_dot);                                    \
            return _sum_;                                           \
        }                                                           \
        LengthType SquareLength() const                             \
        {                                                           \
            return xtl::RealCast<LengthType>(Dot(*this));           \
        }                                                           \
        LengthType Length() const                                   \
        {                                                           \
            return xtl::RealCast<LengthType>(sqrt(SquareLength())); \
        }                                                           \
        AccumType AbsLength() const                                 \
        {                                                           \
            AccumType _sum_ = 0;                                    \
            typedef T::ValueType U;                                 \
            REPEAT_##N(cmd_abs_sum);                                \
            return _sum_;                                           \
        }                                                           \
        ValueType AbsMaxElement() const                             \
        {                                                           \
            ValueType _max_ = 0;                                    \
            typedef T::ValueType U;                                 \
            REPEAT_##N(cmd_abs_max);                                \
            return _max_;                                           \
        }                                                           \
        LengthType Norm(int i = 2) const                            \
        {                                                           \
            assert(i == 0 || i == 1 || i == 2);                     \
            return xtl::RealCast<LengthType>                        \
                   ((i == 0) ? AbsMaxElement() :                    \
                   ((i == 1) ? AbsLength() :                        \
                   ((i == 2) ? Length() : 0)));                     \
        }                                                           \
        void Normalize(int i = 2)                                   \
        {                                                           \
            const LengthType norm = Norm(i);                        \
            if (norm <= xtl::Limits<LengthType>::denorm_min())      \
            {                                                       \
                SetZero();                                          \
            }                                                       \
            else                                                    \
            {                                                       \
                (*this) *= LengthType(1) / norm;                    \
            }                                                       \
        }                                                           \
        const T Normalized(int i = 2) const                         \
        {                                                           \
            T _R_(*this);                                           \
            _R_.Normalize(i);                                       \
            return _R_;                                             \
        }                                                           \
        T& ApplyLength(LengthType newLength)                        \
        {                                                           \
            const LengthType len = Length();                        \
            if (len <= xtl::Limits<LengthType>::denorm_min())       \
            {                                                       \
                SetZero();                                          \
            }                                                       \
            else                                                    \
            {                                                       \
                (*this) *= LengthType(newLength) / len;             \
            }                                                       \
            return *this;                                           \
        }                                                           \
        const T Lerp(const T& _V_, ValueType _A_) const             \
        {                                                           \
            return vtl::Lerp(*this, _V_, _A_);                      \
        }                                                           \
        T& Clamp(const T& _A_, const T& _B_)                        \
        {                                                           \
            typedef T::ValueType U;                                 \
            REPEAT_##N(cmd_clamp);                                  \
            return *this;                                           \
        }                                                           \

// ------------------------------------------------
// VECTOR_OP_COMPARISON
// String lexicon comparison of two vectors, including:
// 1. == and !=
// 2. >, <, >=, <=
// ------------------------------------------------

    #define cmd_compare_equal(N)  {if(at(N) != Rhs.at(N)) return false;}

    #define M_OP_ALPHABETA_COMPARE(T, OP, OPS, EQUAL, SHORTER)      \
        bool operator OP (const T& Rhs) const                       \
        {                                                           \
            const size_t min_size = min(size(), Rhs.size());        \
            for (size_t i = 0; i < min_size; i ++)                  \
            {                                                       \
                if (at(i) OPS Rhs.at(i))    return true;            \
                if (Rhs.at(i) OPS at(i))    return false;           \
            }                                                       \
            return (size() == Rhs.size()) ? EQUAL :                 \
                   (size() <  Rhs.size()) ? SHORTER : !SHORTER;     \
        }                                                           \

    #define VECTOR_OP_COMPARISON(T, N)                              \
        M_OP_ALPHABETA_COMPARE(T, <=, <, true, true)                \
        M_OP_ALPHABETA_COMPARE(T, >=, >, true, false)               \
        M_OP_ALPHABETA_COMPARE(T, < , <, false, true)               \
        M_OP_ALPHABETA_COMPARE(T, > , >, false, false)              \
        bool operator == (const T& Rhs) const                       \
        {                                                           \
            REPEAT_##N(cmd_compare_equal);                          \
            return true;                                            \
        }                                                           \
        bool operator != (const T& Rhs) const                       \
        {                                                           \
            return !operator==(Rhs);                                \
        }                                                           \


} /// namespace vtl

#endif//__VTL_OPERATOR_HPP__