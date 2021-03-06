// CppCoverageConsole.cpp : Defines the entry point for the console application.
//

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

#include "stdafx.h"

#include <iostream>
#include <boost/filesystem.hpp>

#ifdef _WIN32
#include <Tools/Tool.hpp>
#elif __linux__
#include <ToolsLinux/Tool.hpp>
#endif

#include "OpenCppCoverage.hpp"

//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
	Tools::CreateMiniDumpOnUnHandledException();
	
	try
	{
		OpenCppCoverage::OpenCppCoverage openCppCoverage;

		return openCppCoverage.Run(argc, argv, &std::wcerr);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 1;
}