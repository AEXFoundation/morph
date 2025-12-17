**Latest:** 0.0.3 (Alpha Release)

**Author:** [wzssmex@gmail.com](mailto:wzssmex@gmail.com).

Morph is designed as a **foundation for building powerful image processing workflows**. Whether you're automating batch operations, building graphics applications, or integrating image manipulation into your development pipeline, Morph provides the tools you need. This version introduces a new input and output system using -i@"path" and -o@"path", support for .tga .gif .webp .tif and .tiff files, a new "Grayscale" filter, bug fixes and new parameters.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Status](https://img.shields.io/badge/Status-In%20Development-orange)](https://github.com/yourusername/morph)

---

## Overview

### Purpose
Morph is designed as a **foundation for building powerful image processing workflows**. Whether you're automating batch operations, building graphics applications, or integrating image manipulation into your development pipeline, Morph provides the tools you need.

### Core Features

* **Multi-Language Support:** Native support for Python, C, and C++. Use it directly in scripts, embed it in applications, or integrate it into your UI frameworks.
* **Extensible Architecture:** Built to support a comprehensive library of composable image filters. Chain and combine filters to create sophisticated processing pipelines.
* **Command-Line Interface:** Direct command-line access makes it ideal for automation, scripting, and cross-platform workflows.

---

## Usage Guide

### 1. Input & Output System (`-i` / `-o`)

This system uses the `@` symbol to clearly designate file paths. All commands and filter names are **case-insensitive**, but file paths preserve their original casing.

#### A. Input (`-i @<path>`)

| Command | Purpose | Rules & Notes |
| :--- | :--- | :--- |
| **`-i @"C:\path\to\file.png"`** | Adds a single image file to the input pipeline. | Path must be enclosed in quotes if it contains spaces. |
| **`-i @"C:\path\to\folder"`** | Adds all supported images from the folder. | Supported file types: `.png`, `.jpg`, `.jpeg`, `.bmp`, `.tga`, `.gif`, `.webp`, `.tif`, `.tiff`. Filenames are stored in **lowercase**. |

#### B. Output (`-o @<path> [mode] [filename]`)

The path (`@`), mode (`keep`/`clear`), and optional filename can appear in any order after `-o`.

| Argument | Description | Default |
| :--- | :--- | :--- |
| **`@<path>`** | The required destination folder for the export. | (None - Must be supplied) |
| **`[mode]`** | Controls the action after export: **`clear`** (remove from input) or **`keep`** (retain in input). | `clear` |
| **`[filename]`** | Optional. Exports only this specific file. If omitted, all files are exported. | All files in input |

**Examples:**

* **Export All and Clear Input (Default):**
    `> -o @"C:\Output\Folder"`
* **Export All but Keep in Input:**
    `> -o keep @"C:\Output\Folder"`
* **Export Single File and Clear it from Input:**
    `> -o @"C:\Output" clear image.jpg`
* **Flexible Syntax:**
    `> -o keep image.jpg @"C:\Output"`

### 2. Input Management & Information (`@i`)

| Command | Purpose | Rules & Notes |
| :--- | :--- | :--- |
| **`@i`** | **Show Images:** Lists all current images stored in the input pipeline. | This replaces the old `list` command. |
| **`exit`** | Exits the program. | Case-insensitive (`EXIT`, `exit`, `Exit` all work). |

### 3. Filter Application (`@i <filter> <percent> [filename]`)

Filters are applied to the images currently in the input pipeline.

| Command | Purpose | Rules & Notes |
| :--- | :--- | :--- |
| **`@i grayscale 50%`** | Applies the Grayscale filter to **all** images in the input. | The filter name is case-insensitive. |
| **`@i grayscale 50% image.jpg`** | Applies the Grayscale filter to the single file `image.jpg`. | The filename must match the lowercase name in the input list. |

---

[© 2025 Aether, All Rights Reserved](https://www.instagram.com/aetherstudi0s/)
