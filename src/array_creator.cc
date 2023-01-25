#include <iostream>
#include <vector>
#include <fstream>

int main(int argc, char const *argv[])
{
    std::ofstream writer;
    writer.open("data/test_array_data.dat");

    if (argc < 2)
    {
        std::cerr << "Must provide dimensions as input. \n Usage: ./build/array_creator naxis1 naxis2 ...";
        exit(1);
    }

    size_t naxes = argc-1;
    std::cout << "naxes: " << naxes << std::endl;
    writer.write((char *)&naxes, sizeof(size_t));

    std::vector<size_t> naxis(naxes);
    for (size_t i = 0; i < naxes; i++)
    {
        naxis[i] = static_cast<size_t>(atoi(argv[i+1]));
        std::cout << "naxis" << i << ": " << naxis[i] << std::endl;
        writer.write((char *)&naxis[i], sizeof(size_t));
    }

    size_t totpix = 1;
    for (int i = 0; i < naxes; i++)
    {
        totpix *= naxis[i];
    }

    time_t seed = time(0);
    std::cout << "Seed: " << seed << std::endl;
    srand(seed);

    std::cout << "Array dim : " << totpix << std::endl;

    float offset = -5.0f;
    float range = 5.0f;
    float temp;
    for (size_t i = 0; i < totpix; i++)
    {
        temp = offset + range * (rand() / (float)RAND_MAX);

        writer.write((char *)&temp, sizeof(float));
    }

    std::cout << "Array creation done" << std::endl;

    writer.close();

    return 0;
}
