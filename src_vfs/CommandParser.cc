#include "CommandParser.h"
#include <sstream>
#include <algorithm>
#include <iostream>

namespace SimpleFlashFs::Vfs
{

CommandParser::CommandParser(VfsServerPtr vfs)
    : m_vfs(vfs)
{
    if (!m_vfs) {
        throw std::runtime_error("VFS server pointer is null");
    }
}

void CommandParser::register_command(const std::string& name, CommandPtr command, 
                                     const std::string& alias)
{
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    
    if (command) {
        m_commands[lower_name] = command;
        
        if (!alias.empty()) {
            std::string lower_alias = alias;
            std::transform(lower_alias.begin(), lower_alias.end(), lower_alias.begin(), ::tolower);
            m_commands[lower_alias] = command;
        }
    }
}

void CommandParser::unregister_command(const std::string& name)
{
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    m_commands.erase(lower_name);
}

bool CommandParser::has_command(const std::string& name) const
{
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    return m_commands.find(lower_name) != m_commands.end();
}

std::vector<std::string> CommandParser::get_available_commands() const
{
    std::vector<std::string> commands;
    for (const auto& [name, cmd] : m_commands) {
        // Only add primary command names (not aliases)
        // Simple heuristic: if it's not already in the list
        if (std::find(commands.begin(), commands.end(), name) == commands.end()) {
            commands.push_back(name);
        }
    }
    return commands;
}

CommandResult CommandParser::execute_command(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {  // Skip program name
        args.push_back(argv[i]);
    }
    return execute_from_args(args);
}

CommandResult CommandParser::execute_command_string(const std::string& command_line)
{
    std::vector<std::string> args = split_command_line(command_line);
    if (args.empty()) {
        return error("Empty command");
    }
    return execute_from_args(args);
}

CommandResult CommandParser::execute_from_args(const std::vector<std::string>& args)
{
    if (args.empty()) {
        return error("No command provided");
    }

    std::string command = args[0];
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    auto it = m_commands.find(command);
    if (it == m_commands.end()) {
        return error("Unknown command: " + command + ". Type 'help' for available commands.");
    }

    return it->second->execute(args);
}

std::vector<std::string> CommandParser::split_command_line(const std::string& command_line)
{
    std::vector<std::string> args;
    std::istringstream iss(command_line);
    std::string token;

    while (iss >> token) {
        // Remove quotes if present
        if (token.length() >= 2) {
            if ((token.front() == '"' && token.back() == '"') ||
                (token.front() == '\'' && token.back() == '\'')) {
                token = token.substr(1, token.length() - 2);
            }
        }
        args.push_back(token);
    }

    return args;
}

CommandResult CommandParser::error(const std::string& message) const
{
    return {false, message, ""};
}

CommandResult CommandParser::success(const std::string& message, const std::string& output) const
{
    return {true, message, output};
}

} // namespace SimpleFlashFs::Vfs
