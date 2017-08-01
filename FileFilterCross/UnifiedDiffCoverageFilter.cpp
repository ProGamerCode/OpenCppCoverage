// OpenCppCoverage is an open source code coverage for C++.
// Copyright (C) 2016 OpenCppCoverage
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

#include <fstream>
#include <sstream>
#include <boost/optional/optional.hpp>
#include <ToolsCross/Log.hpp>
#include "UnifiedDiffCoverageFilter.hpp"
#include "UnifiedDiffParser.hpp"
#include "File.hpp"
#include "AmbiguousPathException.hpp"

namespace FileFilter
{
	namespace
	{
		//---------------------------------------------------------------------
		std::vector<File> ParseUnifiedDiff(const boost::filesystem::path& unifiedDiffPath)
		{
			std::wifstream ifs(unifiedDiffPath.string());

			if (!ifs)
				std::runtime_error("The file " + unifiedDiffPath.string() + " cannot be opened.");

			auto files = UnifiedDiffParser{}.Parse(ifs);
			LOG_DEBUG << L"Unified diff: " << unifiedDiffPath;
			for (const auto& file : files)
			{
				LOG_DEBUG << L"Selected lines for " << file.GetPath().wstring() << L": ";
				LOG_DEBUG << file.GetSelectedLines();
			}
			return files;
		}
	}

	//-------------------------------------------------------------------------
	UnifiedDiffCoverageFilter::UnifiedDiffCoverageFilter(
		const boost::filesystem::path& unifiedDiffPath,
		const boost::optional<boost::filesystem::path>& rootDiffFolder)
		: UnifiedDiffCoverageFilter{ ParseUnifiedDiff(unifiedDiffPath), rootDiffFolder }
	{
	}

	//-------------------------------------------------------------------------
	UnifiedDiffCoverageFilter::UnifiedDiffCoverageFilter(
		std::vector<File>&& files,
		const boost::optional<boost::filesystem::path>& rootDiffFolder)
		: pathMatcher_{ std::move(files), rootDiffFolder }
		, lastFile_ {nullptr}
	{
	}

	//-------------------------------------------------------------------------
	bool UnifiedDiffCoverageFilter::IsSourceFileSelected(const boost::filesystem::path& path)
	{
		return SearchFile(path) != nullptr;
	}

	//-------------------------------------------------------------------------
	bool UnifiedDiffCoverageFilter::IsLineSelected(const boost::filesystem::path& path, int lineNumber)
	{
		auto file = SearchFile(path);

		if (!file)
			return false;

		return file->IsLineSelected(lineNumber);
	}

	//-------------------------------------------------------------------------
	File* UnifiedDiffCoverageFilter::SearchFile(const boost::filesystem::path& path)
	{
		if (path == lastPath_)
			return lastFile_;

		try
		{
			auto file = pathMatcher_.Match(path);
			lastFile_ = file;
		}
		catch (const AmbiguousPathException& e)
		{
			std::ostringstream ostr;

			ostr << "A path is ambiguous in the unified diff file." << std::endl;
			ostr << e.GetPostFixPath() << " can be either " << e.GetFirstPossiblePath();
			ostr << " or " << e.GetSecondPossiblePath() << std::endl;
			ostr << "Please specify root folder. See help for more information.";

			throw std::runtime_error(ostr.str());
		}

		lastPath_ = path;
		return lastFile_;
	}

	//-------------------------------------------------------------------------
	std::vector<boost::filesystem::path> UnifiedDiffCoverageFilter::GetUnmatchedPaths() const
	{
		return pathMatcher_.GetUnmatchedPaths();
	}
}
