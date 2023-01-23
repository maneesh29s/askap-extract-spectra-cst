#include <iostream>
#include <vector>
#include <fstream>

int main(int argc, char const *argv[])
{
    FILE *fp;
    fp = fopen( "data/test_array_data.dat" , "w" );

    int naxes = 4;
    size_t naxis[4] = {12427, 12424, 1, 1};

    size_t totpix = 1;
    for (int i = 0; i < naxes; i++)
    {
        totpix *= naxis[i];
    }

    // std::vector<float> arr(totpix);

    time_t seed = time(0);
    std::cerr << "Seed: " << seed << std::endl;
    srand(seed);

    std::cerr << "Array dim : " << totpix << std::endl;

    float offset = -5.0f;
    float range = 5.0f;
    float temp;
    for (size_t i = 0; i < totpix; i++)
    {
        temp = offset + range * (rand() / (float)RAND_MAX);

        fwrite(&temp , sizeof(float) , 1 , fp );
    }

    std::cerr << "Array creation done" << std::endl;

    fclose(fp);

    return 0;
}
