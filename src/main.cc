#include <iostream>
#include <vector>
#include <fstream>
#include <json/json.h>

int main(int argc, char const *argv[])
{
    Json::Reader jsonReader;             // for reading the data
    Json::Value root;                // for modifying and storing new values
    int naxes = 4;
    size_t naxis[4] = {12427, 12424, 1, 1};

    std::ifstream dataFile;
    dataFile.open("data/test_array_data.dat");
    std::ifstream jsonFile;
    jsonFile.open("data/selavy-process-24-processed.json");

    // check if there is any error is getting data from the json jsonFile
    if (!jsonReader.parse(jsonFile, root, false))
    {
        std::cout << jsonReader.getFormattedErrorMessages();
        exit(1);
    }

    size_t totpix = 1;
    for (int i = 0; i < naxes; i++)
    {
        totpix *= naxis[i];
    }

    std::cerr << "Array total elements : " << totpix << std::endl;

    float temp;
    for (size_t i = 0; i < totpix; i++)
    {
        dataFile.read((char *)&temp, sizeof(float));
    }

    std::cerr << "Array reader done" << std::endl;


    dataFile.close();
    jsonFile.close();

    return 0;
}
