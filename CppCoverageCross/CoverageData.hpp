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

#if defined(__CYGWIN__)
#include <string.h>
#else
#include <string>
#endif
#include <vector>
#include <memory>

namespace boost
{
	namespace filesystem
	{
		class path;
	}
}

namespace CppCoverage
{
	class ModuleCoverage;

	class CoverageData
	{
	public:
		typedef std::vector<std::unique_ptr<ModuleCoverage>> T_ModuleCoverageCollection;

	public:
		explicit CoverageData(const std::string& name, int exitCode);
		~CoverageData();

		CoverageData(CoverageData&&);
		CoverageData& operator=(CoverageData&&);
		ModuleCoverage& AddModule(const std::string& name);
		
		void SetName(const std::string&);
		void SetExitCode(int);

		const T_ModuleCoverageCollection& GetModules() const;
		const std::string& GetName() const;
		int GetExitCode() const;

	private:
		CoverageData(const CoverageData&) = delete;
		CoverageData& operator=(const CoverageData&) = delete;

	private:
		T_ModuleCoverageCollection modules_;
		std::string name_;
		int exitCode_;
	};
}


