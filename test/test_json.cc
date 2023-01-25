#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <json/json.h>
using namespace std;

// testing of jsoncpp library to read and write to json
int main()
{
    Json::Value root;                // for modifying and storing new values
    Json::Reader reader;             // for reading the data
    Json::StyledWriter writer; // for writing in json files

    size_t naxes = 4;

    // opening file using fstream
    ifstream file;
    file.open("test/test_log.json");

    ofstream newFile;
    newFile.open("test/new_json_log.json");

    // check if there is any error is getting data from the json file
    if (!reader.parse(file, root, false))
    {
        cout << reader.getFormattedErrorMessages();
        exit(1);
    }

    for (Json::Value::ArrayIndex i = 0; i != root.size(); i++)
    {
        cout << "sourceID : " << root[i]["sourceID"].asInt() << endl;
        for (Json::Value::ArrayIndex j = 0; j != naxes; j++)
            cout << root[i]["slicerBegin"][j].asInt64() << " ";
        cout << endl;
    }

    // Updating the json data
    root[0]["sourceID"] = 0000;

    // adding new key
    root[0]["newKey"] = "newValue";

    // to add / update array
    Json::Value vec(Json::arrayValue);
    vec.append(Json::Value(100));
    vec.append(Json::Value(200));
    vec.append(Json::Value(300));
    vec.append(Json::Value(400));

    root[0]["slicerBegin"] = vec;

    // to create json file with multiple objects
    Json::Value writerArr(Json::arrayValue);
    writerArr.append(root[0]);
    writerArr.append(root[1]);

    // writer.write(newFile, writerArr);
    newFile << writer.write(writerArr);

    file.close();
    newFile.close();

    return 0;
}