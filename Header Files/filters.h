#ifndef FILTERS_H
#define FILTERS_H

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <algorithm>

// External stb function declarations
extern "C" {
    unsigned char* stbi_load(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
    void stbi_image_free(void* retval_from_stbi_load);
    int stbi_write_png(char const* filename, int w, int h, int comp, const void* data, int stride_in_bytes);
    int stbi_write_jpg(char const* filename, int w, int h, int comp, const void* data, int quality);
    int stbi_write_bmp(char const* filename, int w, int h, int comp, const void* data);
}

namespace fs = std::filesystem;

class Pipeline {
private:
    std::string input_folder;
    std::vector<std::string> input_images;

    void refreshInputList() {
        input_images.clear();
        if (!fs::exists(input_folder)) return;

        for (const auto& entry : fs::directory_iterator(input_folder)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" ||
                    ext == ".tga" || ext == ".gif" || ext == ".webp" || ext == ".tiff" || ext == ".tif") {
                    input_images.push_back(entry.path().string());
                }
            }
        }
    }

public:
    Pipeline() : input_folder("input") {
        if (!fs::exists(input_folder)) {
            fs::create_directory(input_folder);
        }
        refreshInputList();
    }

    bool addInputFolder(const std::string& folder_path) {
        if (!fs::exists(folder_path)) {
            std::cerr << "Path not found: " << folder_path << std::endl;
            return false;
        }

        if (fs::is_regular_file(folder_path)) {
            std::string ext = fs::path(folder_path).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" ||
                ext == ".tga" || ext == ".gif" || ext == ".webp" || ext == ".tiff" || ext == ".tif") {
                fs::path src(folder_path);
                std::string filename = src.filename().string();
                std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
                fs::path dest = fs::path(input_folder) / filename;
                try {
                    fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
                    std::cout << "Added: " << filename << std::endl;
                    refreshInputList();
                    return true;
                }
                catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to copy: " << e.what() << std::endl;
                    return false;
                }
            }
            else {
                std::cerr << "Not a valid image file" << std::endl;
                return false;
            }
        }

        if (!fs::is_directory(folder_path)) {
            std::cerr << "Invalid path: " << folder_path << std::endl;
            return false;
        }

        int count = 0;
        for (const auto& entry : fs::directory_iterator(folder_path)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" ||
                    ext == ".tga" || ext == ".gif" || ext == ".webp" || ext == ".tiff" || ext == ".tif") {
                    std::string filename = entry.path().filename().string();
                    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
                    fs::path dest = fs::path(input_folder) / filename;
                    try {
                        fs::copy_file(entry.path(), dest, fs::copy_options::overwrite_existing);
                        std::cout << "Added: " << filename << std::endl;
                        count++;
                    }
                    catch (const fs::filesystem_error& e) {
                        std::cerr << "Failed to copy " << filename << std::endl;
                    }
                }
            }
        }

        refreshInputList();
        std::cout << "Added " << count << " image(s) from folder" << std::endl;
        return count > 0;
    }

    bool applyGrayscale(const std::string& target = "") {
        std::vector<std::string> targets;

        if (target.empty()) {
            targets = input_images;
        }
        else {
            std::string target_lower = target;
            std::transform(target_lower.begin(), target_lower.end(), target_lower.begin(), ::tolower);

            auto it = std::find_if(input_images.begin(), input_images.end(),
                [&target_lower](const std::string& path) {
                    std::string filename = fs::path(path).filename().string();
                    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
                    return filename == target_lower;
                });

            if (it == input_images.end()) {
                std::cerr << "Image not found: " << target << std::endl;
                return false;
            }
            targets.push_back(*it);
        }

        if (targets.empty()) {
            std::cerr << "Input is empty. Use: -i @\"path\"" << std::endl;
            return false;
        }

        std::cout << "Applying grayscale..." << std::endl;

        for (const auto& img_path : targets) {
            int width, height, channels;
            unsigned char* data = stbi_load(img_path.c_str(), &width, &height, &channels, 0);

            if (!data) {
                std::cerr << "Failed to load: " << img_path << std::endl;
                continue;
            }

            for (int i = 0; i < width * height; i++) {
                int idx = i * channels;
                unsigned char r = data[idx];
                unsigned char g = (channels > 1) ? data[idx + 1] : r;
                unsigned char b = (channels > 2) ? data[idx + 2] : r;

                unsigned char gray = static_cast<unsigned char>(
                    0.299 * r + 0.587 * g + 0.114 * b
                    );

                data[idx] = gray;
                if (channels > 1) data[idx + 1] = gray;
                if (channels > 2) data[idx + 2] = gray;
            }

            bool success = false;
            fs::path path(img_path);
            std::string ext = path.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            if (ext == ".png") {
                success = stbi_write_png(img_path.c_str(), width, height, channels, data, width * channels);
            }
            else if (ext == ".jpg" || ext == ".jpeg") {
                success = stbi_write_jpg(img_path.c_str(), width, height, channels, data, 95);
            }
            else if (ext == ".bmp") {
                success = stbi_write_bmp(img_path.c_str(), width, height, channels, data);
            }

            if (success) {
                std::cout << "[OK] " << path.filename().string() << std::endl;
            }
            else {
                std::cerr << "[FAIL] " << path.filename().string() << std::endl;
            }

            stbi_image_free(data);
        }

        return true;
    }

    bool exportOutput(const std::string& output_path, bool clearInput = true, const std::string& target = "") {
        if (input_images.empty()) {
            std::cerr << "Input is empty. Use: -i @\"path\"" << std::endl;
            return false;
        }

        fs::path out_dir(output_path);
        if (!fs::exists(out_dir)) {
            try {
                fs::create_directories(out_dir);
            }
            catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to create output directory" << std::endl;
                return false;
            }
        }

        std::cout << "Exporting to: " << output_path << std::endl;

        std::vector<std::string> to_export;
        std::vector<std::string> to_remove;

        if (target.empty()) {
            to_export = input_images;
            if (clearInput) {
                to_remove = input_images;
            }
        }
        else {
            std::string target_lower = target;
            std::transform(target_lower.begin(), target_lower.end(), target_lower.begin(), ::tolower);

            auto it = std::find_if(input_images.begin(), input_images.end(),
                [&target_lower](const std::string& path) {
                    std::string filename = fs::path(path).filename().string();
                    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
                    return filename == target_lower;
                });

            if (it == input_images.end()) {
                std::cerr << "Image not found: " << target << std::endl;
                return false;
            }
            to_export.push_back(*it);
            if (clearInput) {
                to_remove.push_back(*it);
            }
        }

        for (const auto& img : to_export) {
            fs::path src(img);
            fs::path dest = out_dir / src.filename();

            try {
                fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
                std::cout << "[OK] " << src.filename().string() << std::endl;
            }
            catch (const fs::filesystem_error& e) {
                std::cerr << "[FAIL] " << src.filename().string() << std::endl;
            }
        }

        std::cout << "Export complete!" << std::endl;

        if (clearInput && !to_remove.empty()) {
            std::cout << "Clearing from input..." << std::endl;
            for (const auto& img : to_remove) {
                try {
                    fs::remove(img);
                    input_images.erase(std::remove(input_images.begin(), input_images.end(), img), input_images.end());
                }
                catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to remove: " << fs::path(img).filename().string() << std::endl;
                }
            }
            std::cout << "Input cleared!" << std::endl;
        }

        return true;
    }

    void listInput() const {
        if (input_images.empty()) {
            std::cout << "Input folder is empty." << std::endl;
            return;
        }

        std::cout << "Images in input (" << input_images.size() << "):" << std::endl;
        for (const auto& img : input_images) {
            std::cout << "  - " << fs::path(img).filename().string() << std::endl;
        }
    }
};

#endif // FILTERS_H
