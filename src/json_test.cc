#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <json/value.h>
#include <json/json.h>
using namespace std;

int main()
{
    Json::Reader reader;             // for reading the data
    Json::Value root;                // for modifying and storing new values
    Json::StyledStreamWriter writer; // for writing in json files
    ofstream newFile;

    // opening file using fstream
    ifstream file("data/selavy-process-24-processed.json", std::ifstream::binary);

    // check if there is any error is getting data from the json file
    if (!reader.parse(file, root, false))
    {
        cout << reader.getFormattedErrorMessages();
        exit(1);
    }

    for (Json::Value::ArrayIndex i = 0; i != 5; i++)
    {
        cout << root[i]["sourceID"].asString() << endl;
        cout << root[i]["slicerBegin"][1] << endl;
    }

    // Updating the json data
    // root["Category"] = "NewCat";

    // // we can add new values as well
    // root["first"] = "Maneesh";
    // root["last"] = "Sutar";

    // // make the changes in a new file/original file
    // newFile.open("items2.json");
    // writer.write(newFile, root);
    // newFile.close();
    return 0;
}