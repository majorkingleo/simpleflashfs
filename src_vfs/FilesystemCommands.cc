#include "FilesystemCommands.h"
#include <sstream>

namespace SimpleFlashFs::Vfs
{

// ============================================================================
// ListCommand
// ============================================================================

CommandResult ListCommand::execute(const std::vector<std::string>& args)
{
    std::string output;
    bool list_success = m_vfs->list_files([&output](const std::string_view& name, std::size_t size) {
        output += std::string(name) + " (" + std::to_string(size) + " bytes)\n";
        return true;
    });

    if (!list_success) {
        return {false, "Failed to list files", ""};
    }

    if (output.empty()) {
        output = "(empty directory)\n";
    }

    return {true, "OK", output};
}

// ============================================================================
// CatCommand
// ============================================================================

CommandResult CatCommand::execute(const std::vector<std::string>& args)
{
    if (args.size() < 2) {
        return {false, "cat: missing filename argument", ""};
    }

    std::string filename = args[1];
    std::string content;

    try {
        auto handle = m_vfs->open(filename, std::ios::in | std::ios::binary);
        if (!handle || !handle->valid()) {
            return {false, "cat: cannot open file: " + filename, ""};
        }

        const size_t BUFFER_SIZE = 256;
        std::byte buffer[BUFFER_SIZE];
        size_t bytes_read = 0;
        
        while ((bytes_read = handle->read(buffer, BUFFER_SIZE)) > 0) {
            const char* byte_ptr = reinterpret_cast<const char*>(buffer);
            content.append(byte_ptr, bytes_read);
        }

        return {true, "OK", content};
    } catch (const std::exception& e) {
        return {false, "cat: " + std::string(e.what()), ""};
    }
}

// ============================================================================
// CopyCommand
// ============================================================================

CommandResult CopyCommand::execute(const std::vector<std::string>& args)
{
    if (args.size() < 3) {
        return {false, "cp: missing source or destination argument", ""};
    }

    std::string source = args[1];
    std::string dest = args[2];

    try {
        auto source_handle = m_vfs->open(source, std::ios::in | std::ios::binary);
        if (!source_handle || !source_handle->valid()) {
            return {false, "cp: cannot open source file: " + source, ""};
        }

        auto dest_handle = m_vfs->open(dest, std::ios::out | std::ios::binary);
        if (!dest_handle || !dest_handle->valid()) {
            return {false, "cp: cannot create destination file: " + dest, ""};
        }

        const size_t BUFFER_SIZE = 256;
        std::byte buffer[BUFFER_SIZE];
        size_t bytes_read = 0;

        while ((bytes_read = source_handle->read(buffer, BUFFER_SIZE)) > 0) {
            if (dest_handle->write(buffer, bytes_read) != bytes_read) {
                return {false, "cp: write error while copying", ""};
            }
        }

        if (!dest_handle->flush()) {
            return {false, "cp: flush error", ""};
        }

        return {true, "File copied: " + source + " -> " + dest, ""};
    } catch (const std::exception& e) {
        return {false, "cp: " + std::string(e.what()), ""};
    }
}

// ============================================================================
// MoveCommand
// ============================================================================

CommandResult MoveCommand::copy_and_remove(const std::string& source, const std::string& dest)
{
    // First copy the file
    CopyCommand copy_cmd(m_vfs);
    auto copy_result = copy_cmd.execute({"cp", source, dest});
    if (!copy_result.success) {
        return copy_result;
    }

    // Then remove the source
    RemoveCommand rm_cmd(m_vfs);
    auto rm_result = rm_cmd.execute({"rm", source});
    if (!rm_result.success) {
        return {false, "mv: copy succeeded but removal failed: " + source, ""};
    }

    return {true, "File moved: " + source + " -> " + dest, ""};
}

CommandResult MoveCommand::execute(const std::vector<std::string>& args)
{
    if (args.size() < 3) {
        return {false, "mv: missing source or destination argument", ""};
    }

    std::string source = args[1];
    std::string dest = args[2];

    try {
        auto source_handle = m_vfs->open(source, std::ios::in | std::ios::binary);
        if (!source_handle || !source_handle->valid()) {
            return {false, "mv: cannot open source file: " + source, ""};
        }

        // Try to use rename_file method
        if (source_handle->rename_file(dest)) {
            return {true, "File moved: " + source + " -> " + dest, ""};
        } else {
            // Fall back to copy + remove
            return copy_and_remove(source, dest);
        }
    } catch (const std::exception& e) {
        return {false, "mv: " + std::string(e.what()), ""};
    }
}

// ============================================================================
// RemoveCommand
// ============================================================================

CommandResult RemoveCommand::execute(const std::vector<std::string>& args)
{
    if (args.size() < 2) {
        return {false, "rm: missing filename argument", ""};
    }

    std::string filename = args[1];

    try {
        auto handle = m_vfs->open(filename, std::ios::in);
        if (!handle || !handle->valid()) {
            return {false, "rm: cannot open file: " + filename, ""};
        }

        if (handle->delete_file()) {
            return {true, "File deleted: " + filename, ""};
        } else {
            return {false, "rm: cannot delete file: " + filename, ""};
        }
    } catch (const std::exception& e) {
        return {false, "rm: " + std::string(e.what()), ""};
    }
}

// ============================================================================
// TouchCommand
// ============================================================================

CommandResult TouchCommand::execute(const std::vector<std::string>& args)
{
    if (args.size() < 2) {
        return {false, "touch: missing filename argument", ""};
    }

    std::string filename = args[1];

    try {
        auto handle = m_vfs->open(filename, std::ios::out | std::ios::app);
        if (!handle || !handle->valid()) {
            return {false, "touch: cannot create file: " + filename, ""};
        }

        if (!handle->flush()) {
            return {false, "touch: flush error", ""};
        }

        return {true, "File created/touched: " + filename, ""};
    } catch (const std::exception& e) {
        return {false, "touch: " + std::string(e.what()), ""};
    }
}

// ============================================================================
// HelpCommand
// ============================================================================

CommandResult HelpCommand::execute(const std::vector<std::string>& args)
{
    std::string help_text = R"(
SimpleFlashFS Command Parser Help
=================================

Available commands:

)";

    auto commands = m_parser->get_available_commands();
    for (const auto& cmd_name : commands) {
        if (m_parser->has_command(cmd_name)) {
            // We can't easily get the command object to call get_usage(),
            // so we'll provide a generic format
            help_text += "  " + cmd_name + "\n";
        }
    }

    help_text += R"(
Use 'help <command>' for more details on a specific command.
)";

    return {true, "OK", help_text};
}


// ============================================================================
// FormatCommand
// ============================================================================

CommandResult FormatCommand::execute(const std::vector<std::string>& args)
{
    if (args.size() < 2) {
        return {false, "format: missing drive argument", ""};
    }

    std::string drive = args[1];

    try {
        m_vfs->create(drive);
        
        return {true, "Drive formatted: " + drive, ""};
    } catch (const std::exception& e) {
        return {false, "format: " + std::string(e.what()), ""};
    }
}

} // namespace SimpleFlashFs::Vfs
