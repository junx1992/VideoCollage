#ifndef __VTL_LUD_HPP__
#define __VTL_LUD_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2003

Module Name:
  VTL numeric functions: LU decomposition
  
Abstract:
    1. for matrix invert, LU decomposition
    
Notes:

Usage:

History:
    Created  on 2003 Feb 14 by oliver_liyin
          
\*************************************************************************/

#include "vtl/Vtl_Matrix.hpp"

namespace vtl{

    template<class M>
    class Ludcmp
    {
    public:
        typedef M MatrixType;
        typedef typename MatrixType::ValueType ValueType;
        typedef vtl::VectorDynamic<ValueType> VectorType;
        typedef vtl::VectorDynamic<int> PivotType;

        COMPILE_TIME_ASSERT(IS_SAME_TYPE(ValueType, float) || IS_SAME_TYPE(ValueType, ValueType));

    public:
        /// Decompose a square matrix, A = LU
        /// where L is lower triangle matrix and U is upper triangle matrix
        /// This can always be done when A is a square matrix.
        HRESULT Decompose(const MatrixType& matA);

        /// Solve the linear equation Ax = B;
        /// the parameter vecBX serves both input and output.
        template<class B>
        HRESULT SolveEquation(vtl::Vector<ValueType, B>& vecBX);

        /// Get the inverse of the matrix A
        HRESULT GetInverse(MatrixType& matInverted);

        /// Get the determinant of matrix A
        ValueType GetDeterminant();

        /// Get the L matrix, lower triangle matrix
        const MatrixType& GetL();
        
        /// Get the L matrix, upper triangle matrix
        const MatrixType& GetU();

    private:
        HRESULT SetLU();

    private:
        MatrixType m_matL;
        MatrixType m_matU;
        MatrixType m_matLU;
        MatrixType m_matP;

        PivotType m_pivot;

        bool m_singular;
        int m_rows, m_cols;
        int m_pivotSign;
    };

    template<class M>
    HRESULT Ludcmp<M>::Decompose(const MatrixType& matA)
    {
        m_rows = (int) matA.nRows();
        m_cols = (int) matA.nCols();

        if (m_rows != m_cols) return E_INVALIDARG;
        if (m_rows == 0 || m_cols == 0) return E_INVALIDARG;

        m_matLU.Copy(matA);
        m_pivot.resize(m_rows);

        for (int i = 0; i < m_rows; i++)
        {
            m_pivot[i] = i;
        }
        m_pivotSign = 1;

        VectorType columnValue;
        columnValue.resize(m_rows);

        //Outer loop
        assert(m_rows == m_cols);
        for(int k = 0; k < m_cols; k++)
        {
            for (int i = 0; i < m_rows; i ++)
            {
                columnValue[i] = m_matLU[i][k];
            }

            int p = k;
            ValueType maxColumnValue = 0;
            for (int i = k; i < m_rows; i++)
            {
                ValueType absValue = abs(columnValue[i]);
                if (abs(columnValue[i]) > maxColumnValue)
                {
                    maxColumnValue = absValue;
                    p = i;
                }
            }

            if (p != k)
            {
                //Exchange rows[p] value and rows[i] value
                for (int j = 0; j < m_cols; j++) 
                {
                    ValueType tempValue = m_matLU[p][j];
                    m_matLU[p][j] = m_matLU[k][j];
                    m_matLU[k][j] = tempValue;
                }

                //Exchange m_pivot[p] and m_pivot[i]
                int v = m_pivot[p]; 
                m_pivot[p] = m_pivot[k]; 
                m_pivot[k] = v; 

                /// change the sign							
                m_pivotSign = - m_pivotSign;
            }

            //Compute the m_matLU value
            if ((m_matLU[k][k] != 0.0))
            {
                for (int i = k + 1; i < m_rows; i++)
                {	
                    m_matLU[i][k] /= m_matLU[k][k];
                    for(int j = k + 1; j < m_cols; j++)
                    {
                        m_matLU[i][j] -= m_matLU[i][k] * m_matLU[k][j];
                    }

                }
            }
        }

        SetLU();

        m_singular = false;
        for (int i = 0; i < m_rows; i++) 
        {
            if (m_matU[i][i] == 0) 
            {
                m_singular = true;
                break;
            }
        }   

        return S_OK;
    }

    template<class M>
    HRESULT Ludcmp<M>::SetLU()
    {
        if (m_rows < m_cols)
        {
            m_matL.resize(m_rows, m_rows);
            m_matU.resize(m_rows, m_cols);
        }
        else
        {
            m_matL.resize(m_rows, m_cols);
            m_matU.resize(m_cols, m_cols);
        }

        for (int i = 0; i < m_rows; i++)
        {
            for (int j = 0; j < m_cols; j++)
            {
                if (i > j)
                {
                    m_matL[i][j] = m_matLU[i][j];
                }
                else
                {
                    m_matU[i][j] = m_matLU[i][j];
                }

                if (i < m_cols)
                {
                    m_matL[i][i] = 1.0;
                }
            }
        }

        return S_OK;
    }

    template<class M>
    HRESULT Ludcmp<M>::GetInverse(MatrixType& matInverted)
    {
        /// must be non-empty square matrix to compute inverse
        if (m_rows != m_cols) return E_INVALIDARG;
        if (m_rows == 0 || m_cols == 0) return E_INVALIDARG;

        /// Matrix has not inverse matrix;
        if (m_singular) return E_FAIL;

        const int n = m_rows;
        ValueType tempValue = 0;
        ValueType sumValue;

        matInverted.resize(n, n);
        matInverted.SetZero();

        matInverted[n - 1][n - 1] = 1 / m_matU[n - 1][n - 1];

        for (int k = n - 2; k >= 0; k--)
        {
            //X(k+1:n, k) = -X(k+1:n, k+1:n)L(k+1:n,k)
            sumValue = 0.0;
            for (int i = k + 1; i < n; i++)
            {
                for (int j = k + 1; j < n; j++)
                {
                    sumValue += matInverted[i][j] * m_matL[j][k];
                }
                matInverted[i][k] = -sumValue;
                sumValue = 0.0;
            }

            //X(k, k+1:n) = -U(k, k+1:n)X(k+1:n, k+1:n)/U(k,k)
            tempValue = 1 / m_matU[k][k];
            for (int i = k + 1; i < n; i++)
            {
                for (int j = k + 1; j < n; j++)
                {
                    sumValue += m_matU[k][j] * matInverted[j][i];
                }
                matInverted[k][i] = -sumValue * tempValue;
                sumValue = 0.0;
            }

            //x(k,k) = 1/u(k,k) - X(k, k+1:n)L(k+1:n, k)
            for (int p = k + 1; p < n; p++)
            {
                sumValue += matInverted[k][p] * m_matL[p][k];
            }
            matInverted[k][k] = tempValue - sumValue;
        }

        MatrixType matP;
        matP.resize(n, n);
        matP.SetZero();

        int j = 0;
        int index = 0;
        for (int i = 0; i < n; i++)
        {
            j = m_pivot[index++];
            matP[i][j] = 1.0;
        }

        MatrixType matResult;
        MatrixMultiply(matResult, matInverted, matP);
        matInverted.Copy(matResult);

        return S_OK;
    }

    template<class M>
    typename Ludcmp<M>::ValueType Ludcmp<M>::GetDeterminant()
    {
        if (m_rows != m_cols) return 0;
        if (m_rows == 0 || m_cols == 0) return 0;
        if (m_singular) return 0;

        ValueType tempValue = 1.0;
        for (int i = 0; i < m_rows; i++) 
        {
            tempValue *= m_matLU[i][i];
        }

        return m_pivotSign * tempValue;
    }

    /// compute invert matrix
    template<class T, class A>
    void MatrixInvert(vtl::Matrix<T, A>& mat)
    {
        Ludcmp<vtl::Matrix<T, A> > lud;
        lud.Decompose(mat);
        lud.GetInverse(mat);
    }

    /// Compute the determinant of a matrix
    template<class T, class A>
    T MatrixDeterminant(const Matrix<T, A>& mat)
    {
        Ludcmp<vtl::Matrix<T, A> > lud;
        lud.Decompose(mat);
        return lud.GetDeterminant();
    }

} /// namespace vtl


#endif//__VTL_LUD_HPP__