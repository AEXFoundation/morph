#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include "filters.h"

namespace fs = std::filesystem;

std::vector<std::string> splitCommand(const std::string& input) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string token;
    bool inQuotes = false;
    std::string current;

    for (char c : input) {
        if (c == '"') {
            inQuotes = !inQuotes;
        }
        else if (c == ' ' && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        }
        else {
            current += c;
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

int main() {
    Pipeline pipeline;
    bool running = true;
    while (running) {
        std::cout << "> ";

        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) continue;

        auto tokens = splitCommand(input);
        if (tokens.empty()) continue;

        // Only lowercase the command, not the paths
        std::string cmd = tokens[0];
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "exit" || cmd == "quit") {
            running = false;
        }
        else if (cmd == "-i" && tokens.size() >= 2) {
            std::string path = tokens[1];
            if (path[0] == '@') {
                // Import from path: -i @"path" (works for both files and folders)
                pipeline.addInputFolder(path.substr(1));
            }
            else {
                std::cerr << "Use -i @\"path\" to add images" << std::endl;
            }
        }
        else if (cmd == "@i") {
            if (tokens.size() == 1) {
                // Just @i shows list
                pipeline.listInput();
            }
            else if (tokens.size() >= 2) {
                std::string filter = tokens[1];
				std::string percent = (tokens.size() >= 3) ? tokens[2] : "";
                std::string target = (tokens.size() >= 4) ? tokens[3] : "";

                if (filter == "grayscale") {
                	size_t pos = percent.find('%');
                	if(pos != std::string::npos)
                		percent.erase(percent.begin() + pos);
				    double val = 100;
					try {
				        val = std::stod(percent);
				    } catch (const std::invalid_argument& e) {
				        std::cerr << "Invalid Value: "<< percent << std::endl;
				    }                    
					
					pipeline.applyGrayscale(target,val);
                }
                else {
                    std::cerr << "Unknown filter: " << filter << std::endl;
                }
            }
        }
        else if (cmd == "-o" && tokens.size() >= 2) {
            std::string path = tokens[1];
            if (path[0] == '@') {
                pipeline.exportOutput(path.substr(1));
            }
            else {
                std::cerr << "Use -o @\"path\" to export" << std::endl;
            }
        }
        else if (cmd == "list") {
            pipeline.listInput();
        }
        else {
            std::cerr << "Unknown command or missing arguments" << std::endl;
        }
    }

    std::cout << "\nGoodbye!" << std::endl;
    return 0;
}
