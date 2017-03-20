// OpenCppCoverage is an open source code coverage for C++.
// Copyright (C) 2014 OpenCppCoverage
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include <memory>

namespace CppCoverage
{
    class FileCoverage;

    class ModuleCoverage
    {
    public:
        typedef std::vector<std::unique_ptr<FileCoverage>> T_FileCoverageCollection;

    public:
        explicit ModuleCoverage(const std::string &path);

        ~ModuleCoverage();

        FileCoverage &AddFile(const std::string &filename);

        const std::string &GetPath() const;

        const T_FileCoverageCollection &GetFiles() const;

    private:
        ModuleCoverage(const ModuleCoverage &) = delete;

        ModuleCoverage &operator=(const ModuleCoverage &) = delete;

    private:
        T_FileCoverageCollection files_;
        std::string path_;
    };
}


