// This file is part of Lattice Builder.
//
// Copyright (C) 2012-2016  Pierre L'Ecuyer and Universite de Montreal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "netbuilder/ProgressiveRowReducer.h"

#include <algorithm>

namespace NetBuilder{

        ProgressiveRowReducer::ProgressiveRowReducer():
        m_nCols(0),
        m_mat(0,0),
        // m_OriginalMatrix(0,0),
        m_rowOperations(0,0),
        m_pivotsColRowPositions(),
        m_pivotsRowColPositions(),
        m_columnsWithoutPivot(),
        m_rowsWithoutPivot()
    {};

    void ProgressiveRowReducer::reset(unsigned int nCols)  
    {
        m_nCols = nCols;
        m_nRows = 0;
        m_mat = GeneratingMatrix(0, m_nCols);
        // m_OriginalMatrix = GeneratingMatrix(0, m_nCols);
        m_columnsWithoutPivot.clear();
        for(unsigned int j = 0; j < nCols; ++j)
        {
            m_columnsWithoutPivot.insert(j); // TO DO
        }
        m_rowsWithoutPivot.clear();
        m_pivotsColRowPositions.clear();
        m_pivotsRowColPositions.clear();
        m_rowOperations.resize(0,m_nCols);
    }

    unsigned int ProgressiveRowReducer::computeRank() const
    {
        return (unsigned int) m_pivotsColRowPositions.size();
    }

    std::vector<unsigned int> ProgressiveRowReducer::computeRanks(unsigned int firstCol, unsigned int numCol) const
    {
        unsigned int rank = 0;
        std::vector<unsigned int> ranks(numCol, rank);
        unsigned int lastCol = firstCol;

        for(const auto& colRow : m_pivotsColRowPositions)
        {
            if(colRow.first >= firstCol+numCol)
            {
                break;
            }

            for(unsigned int col = lastCol; col < colRow.first ; ++col)
            {
                ranks[col-firstCol] = rank;
            }

            rank+=1;

            if (colRow.first >= firstCol)
            {
                lastCol = colRow.first;
            }
        }

        for(unsigned int col = lastCol; col < firstCol + numCol; ++col)
        {
            ranks[col-firstCol] = rank;
        }

        return ranks;
    }

    void ProgressiveRowReducer::pivotRowAndFindNewPivot(unsigned int row)
    {

        for( const auto& colRowPivot : m_pivotsColRowPositions)
        {
            
            if (m_mat(row,colRowPivot.first)) // if required, use the row to flip this bit
            {
                m_rowOperations[row] = m_rowOperations[row] ^ m_rowOperations[colRowPivot.second];
                m_mat[row] = m_mat[row] ^ m_mat[colRowPivot.second];
            }
        }

        unsigned int newPivotColPosition = m_nCols;
        for(std::set<unsigned int>::iterator it = m_columnsWithoutPivot.begin(); it != m_columnsWithoutPivot.end(); ++it)
        {
            if(m_mat(row, *it))
            {
                newPivotColPosition = *it;
                m_columnsWithoutPivot.erase(it); // this column will have a pivot
                break;
            }
        }

        if (newPivotColPosition < m_nCols) // if such a pivot exists
        {
            
            m_pivotsColRowPositions[newPivotColPosition] = row;
            m_pivotsRowColPositions[row] = newPivotColPosition;
            for(unsigned int i = 0; i < m_nRows; ++i) // for each row above the inserted row
            {
                if(i != row && m_mat(i, newPivotColPosition)) // if required, use the row to flip this bit
                {
                    m_mat[i] = m_mat[i] ^ m_mat[row];
                    m_rowOperations[i] = m_rowOperations[i] ^ m_rowOperations[row];
                }
            }
        }
        else // if not
        {
            m_rowsWithoutPivot.push_back(row);
        }
    }


    void ProgressiveRowReducer::addRow(GeneratingMatrix newRow)
    {
        unsigned int row = m_nRows;
        ++m_nRows;
        m_rowOperations.resize(m_nRows, m_nRows);
        m_rowOperations.flip(row,row);

        m_mat.vstack(std::move(newRow));
        // m_OriginalMatrix.vstack(newRow);
        // m_mat.vstack(newRow);

        pivotRowAndFindNewPivot(row);

    }

    void ProgressiveRowReducer::addColumn(GeneratingMatrix newCol)
    {
        newCol = m_rowOperations * newCol; // apply the row operations to the new column
        m_mat.stackRight(newCol); // stack right the new column
        // m_OriginalMatrix.stackRight(newCol);
        unsigned int col = m_nCols;
        ++m_nCols;

        unsigned int newPivotRowPosition = m_nRows;
        for(std::list<unsigned int>::iterator it = m_rowsWithoutPivot.begin(); it != m_rowsWithoutPivot.end(); ++it)
        {
            if(m_mat(*it,col))
            {
                newPivotRowPosition = *it;
                m_rowsWithoutPivot.erase(it); // this row will have a pivot
                break;
            }
        }

        if(newPivotRowPosition < m_nRows)
        {
            m_pivotsColRowPositions[col] = newPivotRowPosition;
            m_pivotsRowColPositions[newPivotRowPosition] = col;

            for(unsigned int i = 0; i < m_nRows; ++i)
            {
                if( i != newPivotRowPosition && m_mat(i,col))
                {
                    m_mat.flip(i,col);
                    m_rowOperations[i] = m_rowOperations[i] ^ m_rowOperations[newPivotRowPosition];
                }
            }
        }
        else
        {
            m_columnsWithoutPivot.insert(col);
        }
    }

    void ProgressiveRowReducer::exchangeRow(unsigned int rowIndex, GeneratingMatrix newRow, int verbose=0)
    {
        auto oldRowColPivotPosition = m_pivotsRowColPositions.find(rowIndex);
        if (oldRowColPivotPosition != m_pivotsRowColPositions.end())
        {
            int i_begin = 0;
            if (m_rowOperations(rowIndex, rowIndex) != 1){
                for(unsigned int i = 0; i < m_nRows; ++i)
                {
                    if(m_rowOperations(i,rowIndex)){
                        m_mat.swap_rows(i, rowIndex);
                        m_rowOperations.swap_rows(i, rowIndex);

                        auto oldiRowColPivotPosition = m_pivotsRowColPositions.find(i);
                        int oldiColPivotPosition;
                        if (oldRowColPivotPosition != m_pivotsRowColPositions.end()){
                            oldiColPivotPosition = (*oldiRowColPivotPosition).second;
                            m_pivotsColRowPositions.erase(oldiColPivotPosition);
                            m_pivotsRowColPositions.erase(rowIndex);
                            m_columnsWithoutPivot.insert(oldiColPivotPosition);
                        }
                        

                        m_pivotsColRowPositions[(*oldRowColPivotPosition).second] = i;
                        m_pivotsRowColPositions[i] = (*oldRowColPivotPosition).second;
                        
                        i_begin = i;
                        break;
                    }
                }
            }
            else{
                unsigned int colPositionPivot = (*oldRowColPivotPosition).second;
                m_pivotsRowColPositions.erase(oldRowColPivotPosition);
                m_pivotsColRowPositions.erase(colPositionPivot);
                m_columnsWithoutPivot.insert(colPositionPivot);
            }

            for(unsigned int i = i_begin; i < m_nRows; ++i)
            {
                if( i != rowIndex && m_rowOperations(i,rowIndex))
                {
                    m_mat[i] = m_mat[i] ^ m_mat[rowIndex];
                    m_rowOperations[i] = m_rowOperations[i] ^ m_rowOperations[rowIndex];
                }
            }
        }

        m_mat[rowIndex] = std::move(newRow[0]);
        // m_mat[rowIndex] = newRow[0];
        // m_OriginalMatrix[rowIndex] = newRow[0];

        for(unsigned int j = 0; j < m_nRows; ++j)
        {
            if (j != rowIndex)
            {
                m_rowOperations(rowIndex,j) = 0;
            }
        }
        m_rowOperations(rowIndex, rowIndex) = 1;

        pivotRowAndFindNewPivot(rowIndex);

    }

void first_pivot(GeneratingMatrix M, int verbose= 0){
    int k = M.nRows();
    int m = M.nCols();
    
    int i_pivot=0;
    int j=-1;
    int Pivots[k];
    for (int i=0; i<k; i++){
        Pivots[i] = -1;
    }
    
    while (i_pivot < k && j < m-1){
        j++;
        int i_temp = i_pivot;
        while (i_temp < k && M[i_temp][j] == 0){
            i_temp++;
        }
        if (i_temp >= k){  // pas d'element non nul sur la colonne
            continue;
        }
        M.swap_rows(i_temp, i_pivot);

        Pivots[i_pivot] = j;
        for (int i=i_pivot+1; i<k; i++){
            if (M[i][j] != 0){
                M[i] = M[i] ^ M[i_pivot];
            }
        }
        i_pivot++;
    }
    if (Pivots[k-1] == -1){
        assert(false);
    }
}

void ProgressiveRowReducer::check() const{
    first_pivot(m_rowOperations);

    std::vector<bool> check_row (m_nRows, 0);
    std::vector<bool> check_col (m_nCols, 0);


    // GeneratingMatrix prod = m_rowOperations * m_OriginalMatrix;
    // for (int i=0; i < m_nRows; i++){
    //     for (int j=0; j < m_nCols; j++){
    //         // std::cout << "checking prod" << std::endl;
    //         if (prod(i, j) != m_mat(i, j)){
    //             throw std::runtime_error("checking prod");
    //         }
    //     }
    // }
    for (const auto& colRow : m_pivotsColRowPositions){
        int col = colRow.first;
        int row = colRow.second;
        for (int i=0; i < m_nRows; i++){
            if (m_mat(i, col) != (i == row)){
                throw std::runtime_error("checking pivot");
            }
        }
        check_row[row] = 1;
        check_col[col] = 1;
    }

    for (const auto& rowCol : m_pivotsRowColPositions){
        int row = rowCol.first;
        int col = rowCol.second;
        if (check_row[row] != 1 || check_col[col] != 1){
            throw std::runtime_error("checking row col 1");
        }
    }

    for (const auto& col: m_columnsWithoutPivot){
        if (check_col[col] != 0){
            throw std::runtime_error("checking row col 2");
        }
        check_col[col] = 1;
    }
    for (const auto& row: m_rowsWithoutPivot){
        if (check_row[row] != 0){
            throw std::runtime_error("checking row col 3");
        }
        check_row[row] = 1;
    }

    for (const auto& r: check_row){
        if(r != 1){
            throw std::runtime_error("checking row col 4");
        }
    }
    for (const auto& c : check_col){
        if(c != 1){
            throw std::runtime_error("checking row col 5");
        }
    }

}

}
