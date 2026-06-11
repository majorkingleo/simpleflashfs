#pragma once

#include "FilesystemCommands.h"

namespace SimpleFlashFs::Vfs
{

class TestCount : public FilesystemCommand
{
public:
    using FilesystemCommand::FilesystemCommand;

    CommandResult execute(const std::vector<std::string>& args) override;

    std::string get_description() const override { return "write 1 to x into the file"; }
    std::string get_usage() const override { return "count [FILENAME MAX-VALUE] - write 1 to x into the file"; }
};

}

