#include <iostream>
#include <string>
#include <array>

using namespace std;

void getMusic(const string &url, string folder="songs")
{
    string command = "yt-dlp -x --audio-format mp3 -o " + folder + "'/%(title)s.mp3' " + url;

    
    std::array<char, 128> buffer;
    FILE *pipe = popen(command.c_str(), "r");

    if (!pipe) throw std::runtime_error("popen() failed");

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        std::cout << "LOG: " << buffer.data();

    pclose(pipe);
}