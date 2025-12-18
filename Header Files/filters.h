#ifndef FILTERS_H
#define FILTERS_H

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <memory>

// External stb function declarations
extern "C" {
    unsigned char* stbi_load(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
    void stbi_image_free(void* retval_from_stbi_load);
    int stbi_write_png(char const* filename, int w, int h, int comp, const void* data, int stride_in_bytes);
    int stbi_write_jpg(char const* filename, int w, int h, int comp, const void* data, int quality);
    int stbi_write_bmp(char const* filename, int w, int h, int comp, const void* data);
}

namespace fs = std::filesystem;

// Structure to hold image data in RAM
struct ImageData {
    std::string original_path;
    std::string filename;
    int width;
    int height;
    int channels;
    std::unique_ptr<unsigned char[]> pixels;
    bool modified;

    ImageData() : width(0), height(0), channels(0), modified(false) {}
    
    // Move constructor
    ImageData(ImageData&& other) noexcept
        : original_path(std::move(other.original_path)),
          filename(std::move(other.filename)),
          width(other.width),
          height(other.height),
          channels(other.channels),
          pixels(std::move(other.pixels)),
          modified(other.modified) {
        other.width = 0;
        other.height = 0;
        other.channels = 0;
        other.modified = false;
    }
    
    // Move assignment
    ImageData& operator=(ImageData&& other) noexcept {
        if (this != &other) {
            original_path = std::move(other.original_path);
            filename = std::move(other.filename);
            width = other.width;
            height = other.height;
            channels = other.channels;
            pixels = std::move(other.pixels);
            modified = other.modified;
            
            other.width = 0;
            other.height = 0;
            other.channels = 0;
            other.modified = false;
        }
        return *this;
    }

    // Delete copy constructor and copy assignment
    ImageData(const ImageData&) = delete;
    ImageData& operator=(const ImageData&) = delete;
};

class Pipeline {
private:
    std::string base_folder;
    std::string input_folder;
    std::string output_folder;
    std::vector<ImageData> loaded_images;

    void initializeFolders() {
        // Create Morph folder if it doesn't exist
        if (!fs::exists(base_folder)) {
            fs::create_directory(base_folder);
            std::cout << "Created Morph folder: " << base_folder << std::endl;
        }

        // Create input subfolder
        if (!fs::exists(input_folder)) {
            fs::create_directory(input_folder);
            std::cout << "Created input folder: " << input_folder << std::endl;
        }

        // Create output subfolder
        if (!fs::exists(output_folder)) {
            fs::create_directory(output_folder);
            std::cout << "Created output folder: " << output_folder << std::endl;
        }
    }

    bool loadImageToRAM(const std::string& file_path) {
        ImageData img;
        
        // Load image data using stb_image
        unsigned char* data = stbi_load(file_path.c_str(), &img.width, &img.height, &img.channels, 0);
        
        if (!data) {
            std::cerr << "Failed to load: " << file_path << std::endl;
            return false;
        }

        // Store image data in RAM
        size_t data_size = img.width * img.height * img.channels;
        img.pixels = std::unique_ptr<unsigned char[]>(new unsigned char[data_size]);
        std::copy(data, data + data_size, img.pixels.get());
        
        // Store metadata
        img.original_path = file_path;
        img.filename = fs::path(file_path).filename().string();
        std::transform(img.filename.begin(), img.filename.end(), img.filename.begin(), ::tolower);
        img.modified = false;

        stbi_image_free(data);
        
        loaded_images.push_back(std::move(img));
        return true;
    }

public:
    Pipeline() : base_folder("Morph"), 
                 input_folder("Morph/input"), 
                 output_folder("Morph/output") {
        initializeFolders();
    }

    bool addInput(const std::string& path) {
        if (!fs::exists(path)) {
            std::cerr << "Path not found: " << path << std::endl;
            return false;
        }

        // Handle single file
        if (fs::is_regular_file(path)) {
            std::string ext = fs::path(path).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" ||
                ext == ".tga" || ext == ".gif" || ext == ".webp" || ext == ".tiff" || ext == ".tif") {
                
                std::string filename = fs::path(path).filename().string();
                std::cout << "Loading to RAM: " << filename << std::endl;
                
                if (loadImageToRAM(path)) {
                    std::cout << "Added to RAM: " << filename << std::endl;
                    return true;
                }
                return false;
            }
            else {
                std::cerr << "Not a valid image file" << std::endl;
                return false;
            }
        }

        // Handle directory
        if (!fs::is_directory(path)) {
            std::cerr << "Invalid path: " << path << std::endl;
            return false;
        }

        int count = 0;
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                
                if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" ||
                    ext == ".tga" || ext == ".gif" || ext == ".webp" || ext == ".tiff" || ext == ".tif") {
                    
                    std::string filename = entry.path().filename().string();
                    std::cout << "Loading to RAM: " << filename << std::endl;
                    
                    if (loadImageToRAM(entry.path().string())) {
                        count++;
                    }
                }
            }
        }

        std::cout << "Loaded " << count << " image(s) to RAM" << std::endl;
        return count > 0;
    }

    bool applyGrayscale(const std::string& target = "", double choice = 100) {
        if (loaded_images.empty()) {
            std::cerr << "No images loaded in RAM. Use: -i @\"path\"" << std::endl;
            return false;
        }

        choice = std::max(0.0, std::min(100.0, choice));
        double blend = choice / 100.0;

        const double rweight = 0.299;
        const double gweight = 0.587;
        const double bweight = 0.114;

        std::cout << "Applying grayscale (" << choice << "%) in RAM..." << std::endl;

        int processed = 0;
        for (auto& img : loaded_images) {
            // If target specified, only process matching image
            if (!target.empty()) {
                std::string target_lower = target;
                std::transform(target_lower.begin(), target_lower.end(), target_lower.begin(), ::tolower);
                if (img.filename != target_lower) {
                    continue;
                }
            }

            // Process pixels in RAM
            for (int i = 0; i < img.width * img.height; i++) {
                int idx = i * img.channels;
                unsigned char r = img.pixels[idx];
                unsigned char g = (img.channels > 1) ? img.pixels[idx + 1] : r;
                unsigned char b = (img.channels > 2) ? img.pixels[idx + 2] : r;

                unsigned char gray = static_cast<unsigned char>(
                    rweight * r + gweight * g + bweight * b
                );

                unsigned char new_r = static_cast<unsigned char>((1.0 - blend) * r + blend * gray);
                unsigned char new_g = static_cast<unsigned char>((1.0 - blend) * g + blend * gray);
                unsigned char new_b = static_cast<unsigned char>((1.0 - blend) * b + blend * gray);

                img.pixels[idx] = new_r;
                if (img.channels > 1) img.pixels[idx + 1] = new_g;
                if (img.channels > 2) img.pixels[idx + 2] = new_b;
            }

            img.modified = true;
            std::cout << "[OK] " << img.filename << " (processed in RAM)" << std::endl;
            processed++;

            if (!target.empty()) break; // Only process one if target specified
        }

        if (processed == 0 && !target.empty()) {
            std::cerr << "Image not found in RAM: " << target << std::endl;
            return false;
        }

        return true;
    }

    bool savePreview(const std::string& target = "") {
        if (loaded_images.empty()) {
            std::cerr << "No images loaded in RAM." << std::endl;
            return false;
        }

        std::cout << "Saving preview to: " << output_folder << std::endl;

        int saved = 0;
        for (const auto& img : loaded_images) {
            // If target specified, only save matching image
            if (!target.empty()) {
                std::string target_lower = target;
                std::transform(target_lower.begin(), target_lower.end(), target_lower.begin(), ::tolower);
                if (img.filename != target_lower) {
                    continue;
                }
            }

            fs::path output_path = fs::path(output_folder) / img.filename;
            fs::path original_path(img.original_path);
            std::string ext = original_path.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            bool success = false;
            if (ext == ".png") {
                success = stbi_write_png(output_path.string().c_str(), img.width, img.height, 
                                        img.channels, img.pixels.get(), img.width * img.channels);
            }
            else if (ext == ".jpg" || ext == ".jpeg") {
                success = stbi_write_jpg(output_path.string().c_str(), img.width, img.height, 
                                        img.channels, img.pixels.get(), 95);
            }
            else if (ext == ".bmp") {
                success = stbi_write_bmp(output_path.string().c_str(), img.width, img.height, 
                                        img.channels, img.pixels.get());
            }

            if (success) {
                std::cout << "[OK] " << img.filename << std::endl;
                saved++;
            }
            else {
                std::cerr << "[FAIL] " << img.filename << std::endl;
            }

            if (!target.empty()) break;
        }

        std::cout << "Saved " << saved << " preview(s) to disk" << std::endl;
        return saved > 0;
    }

    bool exportOutput(const std::string& output_path, bool clearRAM = true, const std::string& target = "") {
        if (loaded_images.empty()) {
            std::cerr << "No images loaded in RAM." << std::endl;
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

        std::cout << "Exporting from RAM to: " << output_path << std::endl;

        std::vector<size_t> to_remove;
        int exported = 0;

        for (size_t i = 0; i < loaded_images.size(); i++) {
            const auto& img = loaded_images[i];

            // If target specified, only export matching image
            if (!target.empty()) {
                std::string target_lower = target;
                std::transform(target_lower.begin(), target_lower.end(), target_lower.begin(), ::tolower);
                if (img.filename != target_lower) {
                    continue;
                }
            }

            fs::path output_file = out_dir / img.filename;
            fs::path original_path(img.original_path);
            std::string ext = original_path.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            bool success = false;
            if (ext == ".png") {
                success = stbi_write_png(output_file.string().c_str(), img.width, img.height, 
                                        img.channels, img.pixels.get(), img.width * img.channels);
            }
            else if (ext == ".jpg" || ext == ".jpeg") {
                success = stbi_write_jpg(output_file.string().c_str(), img.width, img.height, 
                                        img.channels, img.pixels.get(), 95);
            }
            else if (ext == ".bmp") {
                success = stbi_write_bmp(output_file.string().c_str(), img.width, img.height, 
                                        img.channels, img.pixels.get());
            }

            if (success) {
                std::cout << "[OK] " << img.filename << std::endl;
                exported++;
                if (clearRAM) {
                    to_remove.push_back(i);
                }
            }
            else {
                std::cerr << "[FAIL] " << img.filename << std::endl;
            }

            if (!target.empty()) break;
        }

        std::cout << "Export complete! (" << exported << " file(s))" << std::endl;

        if (clearRAM && !to_remove.empty()) {
            std::cout << "Clearing " << to_remove.size() << " image(s) from RAM..." << std::endl;
            // Remove in reverse order to maintain indices
            for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it) {
                loaded_images.erase(loaded_images.begin() + *it);
            }
            std::cout << "RAM cleared!" << std::endl;
        }

        return exported > 0;
    }

    void listInput() const {
        if (loaded_images.empty()) {
            std::cout << "No images loaded in RAM." << std::endl;
            return;
        }

        std::cout << "Images in RAM (" << loaded_images.size() << "):" << std::endl;
        for (const auto& img : loaded_images) {
            std::string status = img.modified ? " [MODIFIED]" : "";
            size_t ram_size = img.width * img.height * img.channels;
            double mb = ram_size / (1024.0 * 1024.0);
            std::cout << "  - " << img.filename << " (" << img.width << "x" << img.height 
                     << ", " << mb << " MB)" << status << std::endl;
        }
    }

    size_t getRAMUsage() const {
        size_t total = 0;
        for (const auto& img : loaded_images) {
            total += img.width * img.height * img.channels;
        }
        return total;
    }
};

#endif // FILTERS_H
