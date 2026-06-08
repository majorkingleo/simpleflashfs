// Example Custom Commands for CommandParser
// This file demonstrates how to create custom commands that can be
// registered and used with the CommandParser system.

#pragma once

#include "CommandParser.h"
#include <SimpleFlashFsFileInterface.h>

namespace SimpleFlashFs::Vfs
{

// ============================================================================
// Example 1: File Info Command
// ============================================================================

/**
 * @brief Get detailed information about a file
 * Usage: fileinfo <file>
 */
class FileInfoCommand : public Command
{
private:
    std::shared_ptr<VfsServerInterface> m_vfs;

public:
    FileInfoCommand(std::shared_ptr<VfsServerInterface> vfs) : m_vfs(vfs) {}

    CommandResult execute(const std::vector<std::string>& args) override
    {
        if (args.size() < 2) {
            return {false, "fileinfo: missing filename argument", ""};
        }

        std::string filename = args[1];
        std::string output;

        try {
            auto handle = m_vfs->open(filename, std::ios::in | std::ios::binary);
            if (!handle || !handle->valid()) {
                return {false, "fileinfo: cannot open file: " + filename, ""};
            }

            // Gather file information
            size_t file_size = handle->file_size();
            size_t current_pos = handle->tellg();
            bool is_eof = handle->eof();

            output = "File: " + filename + "\n";
            output += "Size: " + std::to_string(file_size) + " bytes\n";
            output += "Position: " + std::to_string(current_pos) + "\n";
            output += "EOF: " + std::string(is_eof ? "yes" : "no") + "\n";

            return {true, "OK", output};
        } catch (const std::exception& e) {
            return {false, "fileinfo: " + std::string(e.what()), ""};
        }
    }

    std::string get_description() const override { return "Get file information"; }
    std::string get_usage() const override { return "fileinfo <file>  - Show file details"; }
};

// ============================================================================
// Example 2: Disk Usage Command
// ============================================================================

/**
 * @brief Show total disk usage statistics
 * Usage: du
 */
class DiskUsageCommand : public Command
{
private:
    std::shared_ptr<VfsServerInterface> m_vfs;

public:
    DiskUsageCommand(std::shared_ptr<VfsServerInterface> vfs) : m_vfs(vfs) {}

    CommandResult execute(const std::vector<std::string>& args) override
    {
        std::string output;
        size_t total_bytes = 0;
        size_t file_count = 0;

        // List all files and calculate totals
        bool success = m_vfs->list_files([&total_bytes, &file_count](
            const std::string_view& name, std::size_t size) {
            total_bytes += size;
            file_count++;
            return true;
        });

        if (!success) {
            return {false, "du: failed to list files", ""};
        }

        output = "Disk Usage Statistics\n";
        output += "====================\n";
        output += "Total files: " + std::to_string(file_count) + "\n";
        output += "Total size: " + std::to_string(total_bytes) + " bytes\n";
        output += "Average file size: " + 
                 (file_count > 0 ? std::to_string(total_bytes / file_count) : "0") + 
                 " bytes\n";

        return {true, "OK", output};
    }

    std::string get_description() const override { return "Show disk usage"; }
    std::string get_usage() const override { return "du  - Display disk usage statistics"; }
};

// ============================================================================
// Example 3: Hex Dump Command
// ============================================================================

/**
 * @brief Display file contents in hexadecimal format
 * Usage: hexdump <file> [lines]
 */
class HexDumpCommand : public Command
{
private:
    std::shared_ptr<VfsServerInterface> m_vfs;

    std::string byte_to_hex(std::byte b) const
    {
        static const char hex_chars[] = "0123456789ABCDEF";
        unsigned char val = static_cast<unsigned char>(b);
        return std::string() + hex_chars[val >> 4] + hex_chars[val & 0x0F];
    }

public:
    HexDumpCommand(std::shared_ptr<VfsServerInterface> vfs) : m_vfs(vfs) {}

    CommandResult execute(const std::vector<std::string>& args) override
    {
        if (args.size() < 2) {
            return {false, "hexdump: missing filename argument", ""};
        }

        std::string filename = args[1];
        int max_lines = 16;  // Default: show 256 bytes (16 * 16)
        if (args.size() >= 3) {
            try {
                max_lines = std::stoi(args[2]);
            } catch (...) {
                return {false, "hexdump: invalid line count", ""};
            }
        }

        try {
            auto handle = m_vfs->open(filename, std::ios::in | std::ios::binary);
            if (!handle || !handle->valid()) {
                return {false, "hexdump: cannot open file: " + filename, ""};
            }

            std::string output;
            std::byte buffer[16];
            size_t bytes_read;
            int lines = 0;

            while (lines < max_lines && (bytes_read = handle->read(buffer, 16)) > 0) {
                // Print offset
                output += std::string(4 - std::to_string(lines * 16).length(), '0') + 
                         std::to_string(lines * 16) + ": ";

                // Print hex bytes
                for (size_t i = 0; i < 16; ++i) {
                    if (i < bytes_read) {
                        output += byte_to_hex(buffer[i]) + " ";
                    } else {
                        output += "   ";
                    }
                    if (i == 7) output += " ";
                }

                // Print ASCII representation
                output += " | ";
                for (size_t i = 0; i < bytes_read; ++i) {
                    unsigned char c = static_cast<unsigned char>(buffer[i]);
                    output += (c >= 32 && c < 127) ? static_cast<char>(c) : '.';
                }
                output += "\n";

                lines++;
            }

            return {true, "OK", output};
        } catch (const std::exception& e) {
            return {false, "hexdump: " + std::string(e.what()), ""};
        }
    }

    std::string get_description() const override { return "Display file in hex"; }
    std::string get_usage() const override { return "hexdump <file> [lines]  - Show hex dump"; }
};

// ============================================================================
// Example 4: Search Command
// ============================================================================

/**
 * @brief Find files matching a pattern
 * Usage: find <pattern>
 */
class FindCommand : public Command
{
private:
    std::shared_ptr<VfsServerInterface> m_vfs;

    bool matches_pattern(const std::string& filename, const std::string& pattern) const
    {
        // Simple pattern matching: * matches anything
        if (pattern == "*") return true;
        return filename.find(pattern) != std::string::npos;
    }

public:
    FindCommand(std::shared_ptr<VfsServerInterface> vfs) : m_vfs(vfs) {}

    CommandResult execute(const std::vector<std::string>& args) override
    {
        if (args.size() < 2) {
            return {false, "find: missing search pattern", ""};
        }

        std::string pattern = args[1];
        std::string output;
        int match_count = 0;

        bool success = m_vfs->list_files([this, &pattern, &output, &match_count](
            const std::string_view& name, std::size_t size) {
            if (matches_pattern(std::string(name), pattern)) {
                output += std::string(name) + " (" + std::to_string(size) + " bytes)\n";
                match_count++;
            }
            return true;
        });

        if (!success) {
            return {false, "find: failed to list files", ""};
        }

        if (match_count == 0) {
            output = "(no files matching '" + pattern + "')\n";
        } else {
            output = "Found " + std::to_string(match_count) + " file(s):\n" + output;
        }

        return {true, "OK", output};
    }

    std::string get_description() const override { return "Find files by pattern"; }
    std::string get_usage() const override { return "find <pattern>  - Search for files"; }
};

// ============================================================================
// Example 5: Append Command
// ============================================================================

/**
 * @brief Append content from one file to another
 * Usage: append <source> <dest>
 */
class AppendCommand : public Command
{
private:
    std::shared_ptr<VfsServerInterface> m_vfs;

public:
    AppendCommand(std::shared_ptr<VfsServerInterface> vfs) : m_vfs(vfs) {}

    CommandResult execute(const std::vector<std::string>& args) override
    {
        if (args.size() < 3) {
            return {false, "append: missing source or destination argument", ""};
        }

        std::string source = args[1];
        std::string dest = args[2];

        try {
            auto source_handle = m_vfs->open(source, std::ios::in | std::ios::binary);
            if (!source_handle || !source_handle->valid()) {
                return {false, "append: cannot open source: " + source, ""};
            }

            auto dest_handle = m_vfs->open(dest, std::ios::out | std::ios::app);
            if (!dest_handle || !dest_handle->valid()) {
                return {false, "append: cannot open destination: " + dest, ""};
            }

            // Copy with append mode
            const size_t BUFFER_SIZE = 256;
            std::byte buffer[BUFFER_SIZE];
            size_t bytes_read;
            size_t total_bytes = 0;

            while ((bytes_read = source_handle->read(buffer, BUFFER_SIZE)) > 0) {
                if (dest_handle->write(buffer, bytes_read) != bytes_read) {
                    return {false, "append: write error", ""};
                }
                total_bytes += bytes_read;
            }

            if (!dest_handle->flush()) {
                return {false, "append: flush error", ""};
            }

            std::string msg = "Appended " + std::to_string(total_bytes) + 
                             " bytes from " + source + " to " + dest;
            return {true, msg, ""};
        } catch (const std::exception& e) {
            return {false, "append: " + std::string(e.what()), ""};
        }
    }

    std::string get_description() const override { return "Append file to another"; }
    std::string get_usage() const override { return "append <src> <dest>  - Append file"; }
};

} // namespace SimpleFlashFs::Vfs
