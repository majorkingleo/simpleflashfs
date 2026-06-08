#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include "SimpleFlashFsVfs.h"

namespace SimpleFlashFs::Vfs
{

/**
 * @brief Command result structure
 */
struct CommandResult
{
    bool success;
    std::string message;
    std::string output;
};

/**
 * @brief Base class for command implementations
 */
class Command
{
public:
    virtual ~Command() = default;

    /**
     * @brief Execute the command with the given arguments
     * @param args vector of command arguments (args[0] is the command name)
     * @return CommandResult with success/failure and output
     */
    virtual CommandResult execute(const std::vector<std::string>& args) = 0;

    /**
     * @brief Get a brief description of the command
     */
    virtual std::string get_description() const = 0;

    /**
     * @brief Get detailed usage information for the command
     */
    virtual std::string get_usage() const = 0;
};

/**
 * @brief Generic extensible command parser for filesystem commands
 */
class CommandParser
{
public:
    using VfsServerPtr = std::shared_ptr<VfsServerInterface>;
    using CommandPtr = std::shared_ptr<Command>;

    CommandParser(VfsServerPtr vfs);

    /**
     * @brief Register a command with the parser
     * @param name the command name (case-insensitive)
     * @param command the command implementation
     * @param alias optional alias for the command
     */
    void register_command(const std::string& name, CommandPtr command, 
                         const std::string& alias = "");

    /**
     * @brief Unregister a command
     * @param name the command name
     */
    void unregister_command(const std::string& name);

    /**
     * @brief Check if a command is registered
     */
    bool has_command(const std::string& name) const;

    /**
     * @brief Parse and execute a command from command line arguments
     * @param argc argument count
     * @param argv arguments array
     * @return CommandResult with success/failure and output
     */
    CommandResult execute_command(int argc, char** argv);

    /**
     * @brief Parse and execute a single command string
     * @param command_line the command line as a single string
     * @return CommandResult with success/failure and output
     */
    CommandResult execute_command_string(const std::string& command_line);

    /**
     * @brief Parse and execute a command from pre-split arguments
     * @param args vector of command arguments
     * @return CommandResult with success/failure and output
     */
    CommandResult execute_from_args(const std::vector<std::string>& args);

    /**
     * @brief Get a list of all registered commands
     */
    std::vector<std::string> get_available_commands() const;

    /**
     * @brief Get the VFS server pointer
     */
    VfsServerPtr get_vfs() const { return m_vfs; }

protected:
    VfsServerPtr m_vfs;
    std::map<std::string, CommandPtr> m_commands;

    // Helper methods
    std::vector<std::string> split_command_line(const std::string& command_line);
    CommandResult error(const std::string& message) const;
    CommandResult success(const std::string& message = "", const std::string& output = "") const;

private:
    friend class Command;
};

} // namespace SimpleFlashFs::Vfs
