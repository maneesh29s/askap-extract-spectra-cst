// For testing C array vs CASA array indexing

#include <casacore/casa/Arrays.h>

#include "include/helper.hpp"

#include <assert.h>

static void customPrint(int *ptr, std::string name, int rowSize, int colSize);

int main()
{
    int rowSize = 3;
    int colSize = 5;
    std::vector<int> stdVec(rowSize * colSize);

    // C array, row major order
    // frequently changing dimension is last
    int array[rowSize][colSize];


    // Fortran / CASA array, column major order
    // frequently changing dimension is first
    casacore::IPosition pos(2, colSize, rowSize);
    casacore::Array<int> casaArray(pos);

    casacore::Matrix<int> casaMatrix(colSize, rowSize);

    casacore::Matrix<int> casaMatrix2(pos);
    assert(casaMatrix.shape().isEqual(casaMatrix2.shape()));

    std::cout << "Expected matrix :" << std::endl;

    // loop remains same for both C style and FORTRAN style
    for (size_t row = 0; row < rowSize; row++)
    {
        for (size_t col = 0; col < colSize; col++)
        {
            int pos = row * colSize + col;
            int val = pos + 100;
            std::cout << val << " ";

            stdVec[pos] = val;

            // C style accessing
            array[row][col] = val;

            casacore::IPosition currentPos = casacore::toIPositionInArray(pos, casaArray.shape());
            casacore::IPosition currentPos2(2, col , row);
            assert(currentPos.isEqual(currentPos2));

            casaArray(currentPos) = val;

            // FORTRAN style accessing
            casaMatrix(col,row) = val;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    int *ptr;
    std::string name;

    ptr = stdVec.data();
    name = "Standard Vector";
    customPrint(ptr, name, rowSize, colSize);

    ptr = (int *) array;
    name = "C Array";
    customPrint(ptr, name, rowSize, colSize);

    ptr = casaArray.data();
    name = "Casa Array";
    customPrint(ptr, name, rowSize, colSize);

    ptr = casaMatrix.data();
    name = "Casa Matrix";
    customPrint(ptr, name, rowSize, colSize);

    std::cerr << "CasaArray \n" << casaArray << std::endl;
    std::cerr << "CasaMatrix \n" << casaMatrix << std::endl;

}

static void customPrint(int *ptr, std::string name, int rowSize, int colSize)
{

    std::cout << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << name << " : " << std::endl;

    // iterating using row and col
    for (size_t row = 0; row < rowSize; row++)
    {
        for (size_t col = 0; col < colSize; col++)
        {
            std::cout << ptr[row * colSize + col] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Id    Value        Address        Diff" << std::endl;
    for (size_t i = 0; i < colSize * 2; i++)
    {
        std::string diff;
        if (i == 0)
            diff = "-";
        else
            diff = std::to_string(&ptr[i] - &ptr[i - 1]);
        std::cout << i << "      " << ptr[i] << "        " << &ptr[i] << "       " << diff << std::endl;
    }
    std::cout << std::endl;
};


/*
    // ------------------- Casa Array -------------------- //

    std::cout << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Casa Array : " << std::endl;
    for (size_t row = 0; row < rowSize; row++)
    {
        for (size_t col = 0; col < colSize; col++)
        {
            std::cout << casaArrayPtr[row * colSize + col] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Id    Value        Address        Diff" << std::endl;
    for (size_t i = 0; i < colSize * 2; i++)
    {
        std::string diff;
        if(i == 0) diff = "-";
        else diff = std::to_string(&casaArrayPtr[i] - &casaArrayPtr[i-1]);

        std::cout << i << "      " << casaArrayPtr[i] << "        " << &casaArrayPtr[i] << "       " << diff << std::endl;
    }
    std::cout << std::endl;
*/