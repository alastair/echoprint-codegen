//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//



#ifndef MATRIXUTILITY_H
#define MATRIXUTILITY_H

#include "Common.h"
#include "matrix.hpp"
#include <vector>

namespace ublas = boost::numeric::ublas;

typedef ublas::matrix<float> matrix_f;
typedef ublas::matrix<uint> matrix_u;

typedef ublas::matrix_row<matrix_f> matrix_row_f;
typedef ublas::matrix_row<const ublas::matrix<float> > const_matrix_row_f;
typedef ublas::matrix_column<matrix_f> matrix_column_f;
typedef ublas::matrix_range<matrix_f> matrix_range_f;

namespace MatrixUtility {
    inline uint rows(matrix_f A){ return A.size1();}
    inline uint cols(matrix_f A){ return A.size2();}
    bool FileOutput(const matrix_f& A, const char* filename);
    bool TextFileOutput(const matrix_f& A, const char* filename);
    bool TextFileOutputUint(const matrix_u& A, const char* filename);
    bool TextFileOutputVectorUint(const std::vector<uint>& A, int size1, int size2, const char* filename);
}
#endif
