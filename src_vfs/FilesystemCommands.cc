#include "FilesystemCommands.h"
#include <sstream>
#include <format.h>
#include <CpputilsDebug.h>
#include <algorithm>

namespace SimpleFlashFs::Vfs
{

// ============================================================================
// FilesystemCommand
// ============================================================================    

std::string FilesystemCommand::get_absolute_path(const std::string_view& path) const
{
    std::string absolute_path = std::string(path);

    if( !absolute_path.starts_with('/') ) {
        absolute_path = Tools::format( "/%s/%s", m_vfs->get_current_drive(), absolute_path );
    }
    return absolute_path;
}

// ============================================================================
// ListCommand
// ============================================================================

CommandResult ListCommand::execute(const std::vector<std::string>& args)
{
    std::string output;

    if( m_vfs->get_current_drive().empty() ) {
        for( const auto & drive_name : m_vfs->get_drive_names() ) {
            output += '/' + std::string(drive_name) + "\n";
        }
        if( output.empty() ) {
            output = "(no drives registered)\n";
        }
        return {true, "OK", output};
    }

    auto list_file = [&output](const std::string_view& name, std::size_t size) {
        output += std::string(name) + " (" + std::to_string(size) + " bytes)\n";
        return true;
    };

   
    bool list_success = m_vfs->list_files(list_file, m_vfs->get_current_drive() );

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
        auto handle = m_vfs->open( get_absolute_path(filename), std::ios::in | std::ios::binary);
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
        auto source_handle = m_vfs->open( get_absolute_path(source), std::ios::in | std::ios::binary);
        if (!source_handle || !source_handle->valid()) {
            return {false, "cp: cannot open source file: " + source, ""};
        }

        auto dest_handle = m_vfs->open( get_absolute_path(dest), std::ios::out | std::ios::binary);
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
        auto source_handle = m_vfs->open( get_absolute_path(source), std::ios::in | std::ios::binary);
        if (!source_handle || !source_handle->valid()) {
            return {false, "mv: cannot open source file: " + source, ""};
        }

        // Try to use rename_file method
        if (source_handle->rename_file(get_absolute_path(dest))) {
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
        auto handle = m_vfs->open( get_absolute_path(filename), std::ios::in);
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

        auto handle = m_vfs->open( get_absolute_path(filename), std::ios::out | std::ios::app);
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
        if( drive.ends_with(':') ) {
            drive.pop_back();
        }

        auto drives = m_vfs->get_drive_names();

        if( std::find( drives.begin(), drives.end(), drive ) == drives.end() ) {
            return {false, "drive not found: " + drive, ""};
        }

        m_vfs->create(drive);
        
        return {true, "Ok", "Drive formatted: " + drive + "\n"};
    } catch (const std::exception& e) {
        return {false, "format: " + std::string(e.what()), ""};
    }
}

// ============================================================================
// ChangeDirectoryCommand
// ============================================================================

std::string ChangeDirectoryCommand::normalize_drive_name(const std::string_view& arg) const
{
    std::string normalized = std::string(arg);
    
    // Remove trailing colon (DOS style: a: -> a)
    if (!normalized.empty() && normalized.back() == ':') {
        normalized.pop_back();
    }
    
    // Remove leading slash (Unix style: /a -> a)
    if (!normalized.empty() && normalized.front() == '/') {
        normalized = normalized.substr(1);
    }
    
    return normalized;
}

CommandResult ChangeDirectoryCommand::execute(const std::vector<std::string>& args)
{
    // If no arguments, list all available drives
    if (args.size() < 2) {
        auto drive_names = m_vfs->get_drive_names();
        
        if (drive_names.empty()) {
            return {false, "No drives registered", ""};
        }
        
        std::string output = "Available drives:\n";
        for (const auto& drive : drive_names) {
            std::string current = (drive == m_vfs->get_current_drive()) ? " <- current" : "";
            output += "  " + std::string(drive) + current + "\n";
        }
        
        output += "\nUsage: cd <drive>  (e.g., cd a, cd /b, cd a:, cd b:)\n";
        return {true, "OK", output};
    }
    
    // Normalize the drive name
    std::string drive_name = normalize_drive_name(args[1]);
    
    // Check if drive exists and set it as current
    auto drive_names = m_vfs->get_drive_names();
    bool drive_found = false;
    for (const auto& drive : drive_names) {
        if (drive == drive_name) {
            drive_found = true;
            break;
        }
    }
    
    if (!drive_found) {
        return {false, "cd: drive not found: " + drive_name, ""};
    }
    
    if (m_vfs->set_current_drive(drive_name)) {
        return {true, "OK", "Changed to drive: " + drive_name + "\n"};
    } else {
        return {false, "cd: failed to change drive: " + drive_name, ""};
    }
}

// ============================================================================
// PrintWorkingDirectoryCommand
// ============================================================================

CommandResult PrintWorkingDirectoryCommand::execute(const std::vector<std::string>& args)
{
    auto current_drive = m_vfs->get_current_drive();
    
    if (current_drive.empty()) {
        return {true, "", "/\n"};
    }
    
    // Unix style output: /<drive>/
    std::string output = "/" + std::string(current_drive) + "/\n";
    return {true, "OK", output};
}

// ============================================================================
// DOSChangeDriveCommand
// ============================================================================

DOSChangeDriveCommand::DOSChangeDriveCommand(std::shared_ptr<SimpleFlashFs::Vfs::VfsServerInterface> vfs, const std::string & name ) 
: FilesystemCommand(vfs), 
    m_name(name)
{

}

SimpleFlashFs::Vfs::CommandResult DOSChangeDriveCommand::execute(const std::vector<std::string>& args)
{		
    if( !m_vfs->set_current_drive( m_name ) ) {
        return {false, "cd: failed to change drive: " + m_name, ""};
    }
    return {true, "OK", "Drive changed to " + m_name + "\n", false};
}

std::string DOSChangeDriveCommand::get_description() const { 
    return Tools::format( "Change current drive to %s", m_name ); 
}

std::string DOSChangeDriveCommand::get_usage() const { 
    return Tools::format( "%s: <drive>  - Change current drive (e.g., cd a, cd /b, cd a:, cd b:)", m_name ); 
}


} // namespace SimpleFlashFs::Vfs
