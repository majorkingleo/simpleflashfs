#pragma once

#include "CommandParser.h"
#include <SimpleFlashFsFileInterface.h>

namespace SimpleFlashFs::Vfs
{

/**
 * @brief Base class for filesystem commands with VFS access
 */
class FilesystemCommand : public Command
{
protected:
    std::shared_ptr<VfsServerInterface> m_vfs;

public:
    FilesystemCommand(std::shared_ptr<VfsServerInterface> vfs) : m_vfs(vfs) {}
    virtual ~FilesystemCommand() = default;
};

// ============================================================================
// Built-in filesystem commands
// ============================================================================

/**
 * @brief List files command (ls/dir)
 */
class ListCommand : public FilesystemCommand
{
public:
    using FilesystemCommand::FilesystemCommand;

    CommandResult execute(const std::vector<std::string>& args) override;
    std::string get_description() const override { return "List files in a directory"; }
    std::string get_usage() const override { return "ls [path]  - List files (default: a:)"; }
};

/**
 * @brief Display file contents (cat)
 */
class CatCommand : public FilesystemCommand
{
public:
    using FilesystemCommand::FilesystemCommand;

    CommandResult execute(const std::vector<std::string>& args) override;
    std::string get_description() const override { return "Display file contents"; }
    std::string get_usage() const override { return "cat <file>  - Display file contents"; }
};

/**
 * @brief Copy file (cp)
 */
class CopyCommand : public FilesystemCommand
{
public:
    using FilesystemCommand::FilesystemCommand;

    CommandResult execute(const std::vector<std::string>& args) override;
    std::string get_description() const override { return "Copy file"; }
    std::string get_usage() const override { return "cp <source> <dest>  - Copy file"; }
};

/**
 * @brief Move/rename file (mv)
 */
class MoveCommand : public FilesystemCommand
{
public:
    using FilesystemCommand::FilesystemCommand;

    CommandResult execute(const std::vector<std::string>& args) override;
    std::string get_description() const override { return "Move or rename file"; }
    std::string get_usage() const override { return "mv <source> <dest>  - Move/rename file"; }

private:
    CommandResult copy_and_remove(const std::string& source, const std::string& dest);
};

/**
 * @brief Delete file (rm)
 */
class RemoveCommand : public FilesystemCommand
{
public:
    using FilesystemCommand::FilesystemCommand;

    CommandResult execute(const std::vector<std::string>& args) override;
    std::string get_description() const override { return "Delete file"; }
    std::string get_usage() const override { return "rm <file>  - Delete file"; }
};

/**
 * @brief Create/touch file (touch)
 */
class TouchCommand : public FilesystemCommand
{
public:
    using FilesystemCommand::FilesystemCommand;

    CommandResult execute(const std::vector<std::string>& args) override;
    std::string get_description() const override { return "Create empty file"; }
    std::string get_usage() const override { return "touch <file>  - Create/touch file"; }
};

/**
 * @brief Help command
 */
class HelpCommand : public Command
{
private:
    std::shared_ptr<CommandParser> m_parser;

public:
    HelpCommand(std::shared_ptr<CommandParser> parser) : m_parser(parser) {}

    CommandResult execute(const std::vector<std::string>& args) override;
    std::string get_description() const override { return "Show help information"; }
    std::string get_usage() const override { return "help, ?  - Show this help message"; }
};

} // namespace SimpleFlashFs::Vfs
