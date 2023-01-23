#include <iostream>
#include <vector>
#include <fstream>

int main(int argc, char const *argv[])
{
    FILE *fp;
    fp = fopen( "data/test_array_data.dat" , "r" );

    int naxes = 4;
    size_t naxis[4] = {12427, 12424, 1, 1};

    size_t totpix = 1;
    for (int i = 0; i < naxes; i++)
    {
        totpix *= naxis[i];
    }

    // std::vector<float> arr(totpix);

    std::cerr << "Array total elements : " << totpix << std::endl;

    float temp;
    for (size_t i = 0; i < totpix; i++)
    {
        fread(&temp , sizeof(float) , 1 , fp );
    }

    std::cerr << "Array reader done" << std::endl;

    fclose(fp);

    return 0;
}
