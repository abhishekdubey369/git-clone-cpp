#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <zlib.h>
#include <vector>
#include <openssl/sha.h>


using namespace std;

#define fs filesystem


string sha_file(string basicString);
void compressFile(string basicString, uLong *pInt, unsigned char data[20]);
void compressFile(const string data, uLong *bound, unsigned char *dest) ;
void hash_object(string file);


int main(int argc, char *argv[])
{
    // Flush after every cout / cerr
    cout << unitbuf;
    cerr << unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    cerr << "Logs from your program will appear here!\n";

    // Uncomment this block to pass the first stage
    //
    if (argc < 2) {
        cerr << "No command provided.\n";
        return EXIT_FAILURE;
    }
    
    string command = argv[1];
    // cout << "Command: " << command << '\n';
    
    if (command == "init") {
        try {
            fs::create_directory(".git");
            fs::create_directory(".git/objects");
            fs::create_directory(".git/refs");
    
            ofstream headFile(".git/HEAD");
            if (headFile.is_open()) {
                headFile << "ref: refs/heads/main\n";
                headFile.close();
            } else {
                cerr << "Failed to create .git/HEAD file.\n";
                return EXIT_FAILURE;
            }
    
            cout << "Initialized git directory\n";
        } catch (const fs::filesystem_error& e) {
            cerr << e.what() << '\n';
            return EXIT_FAILURE;
        }
    }else if (command == "cat--file" || command == "cat-file") {
        if (argc < 4 || (string(argv[2]) != "-p")) {
            cerr << "No object hash provided.\n";
            cerr << "required `-p <blob_sha>`\n";
            return EXIT_FAILURE;
        }

        const auto blobHash = string_view(argv[3],40);
        const auto blobDir = blobHash.substr(0, 2);
        const auto blobName = blobHash.substr(2);
        const auto blobPath = fs::path(".git") / "objects" / blobDir / blobName;

        auto input = ifstream(blobPath);

        if(!input.is_open()){
            cerr << "Failed to open" << blobPath << "file.\n";
            return EXIT_FAILURE;
        }

        const auto blobData = string(istreambuf_iterator<char>(input), istreambuf_iterator<char>());
        auto buf = string();
        buf.resize(blobData.size());
        while(true){
            auto len = buf.size();
            if(auto res = uncompress((uint8_t*)buf.data(), &len, (uint8_t*)blobData.data(), blobData.size()); res == Z_BUF_ERROR){
                buf.resize(len*2);
                break;
                }
            else if(res != Z_OK){
                cerr << "Failed to decompress blob data.\n";
                return EXIT_FAILURE;
            }
            else{
                buf.resize(len);
                break;
            }
        }
        cout << string_view(buf).substr(buf.find('\0')+1)<<flush;
    }else if (command == "hash--object" || command == "hash-object"){
        if(argc < 4 || (string(argv[2]) != "-w")){
            cerr << "No file provided.\n";
            cerr << "required `-w <file>`\n";
            return EXIT_FAILURE;
        }

        string hash = argv[3];
        hash_object(hash);

    }
     else {
        cerr << "Unknown command " << command << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}



void hash_object(string file) {
    ifstream t(file);
    stringstream data;
    data << t.rdbuf();
    string content = "blob " + to_string(data.str().length()) + '\0' + data.str();
    // Calculate SHA1 hash
    string buffer = sha_file(content);
    // Compress blob content
    uLong bound = compressBound(content.size());
    unsigned char compressedData[bound];
    compressFile(content, &bound, compressedData);
    // Write compressed data to .git/objects
    string dir = ".git/objects/" + buffer.substr(0,2);
    fs::create_directory(dir);
    string objectPath = dir + "/" + buffer.substr(2);
    ofstream objectFile(objectPath, ios::binary);
    objectFile.write((char *)compressedData, bound);
    objectFile.close();
}
void compressFile(const string data, uLong *bound, unsigned char *dest) {
    compress(dest, bound, (const Bytef *)data.c_str(), data.size());
}
string sha_file(string data) {
    unsigned char hash[20];
    SHA1((unsigned char *)data.c_str(), data.size(), hash);
    stringstream ss;
    ss << hex << setfill('0');
    for (const auto& byte : hash) {
        ss << setw(2) << static_cast<int>(byte);
    }
    cout << ss.str() << endl;
    return ss.str();
}
