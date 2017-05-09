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
#include "HandleInformation.hpp"

#include <Psapi.h>
#include <boost/algorithm/string.hpp>
#include <CppCoverageCross/CppCoverageException.hpp>

namespace CppCoverage
{
	namespace
	{
		//-------------------------------------------------------------------------
		std::vector<std::wstring> GetLogicalDrives()
		{
			std::vector<wchar_t> logicalDriveStrings(PathBufferSize);
			std::vector<std::wstring> logicalDrives;

			auto size = GetLogicalDriveStrings(
				static_cast<int>(logicalDriveStrings.size()), 
				&logicalDriveStrings[0]);

			if (!size)
				THROW(L"Cannot GetLogicalDriveStrings");

			size_t startIndex = 0;

			for (size_t i = 0; i < size; ++i)
			{
				if (!logicalDriveStrings[i])
				{
					std::wstring localDrive(&logicalDriveStrings[startIndex], &logicalDriveStrings[i]);
					logicalDrives.push_back(localDrive);
					startIndex = i + 1;
				}
			}

			return logicalDrives;
		}

		//-------------------------------------------------------------------------
		std::vector<std::pair<std::wstring, std::wstring>> GetQueryDosDevicesMapping()
		{
			std::vector<wchar_t> dosDevice(PathBufferSize);
			std::vector<std::pair<std::wstring, std::wstring>> queryDosDevicesMapping;

			for (const auto& logicalDrive : GetLogicalDrives())
			{
				auto pos = logicalDrive.find('\\');
				std::wstring drive = (pos != std::string::npos) ? logicalDrive.substr(0, pos) : logicalDrive;

				if (QueryDosDevice(drive.c_str(), &dosDevice[0], static_cast<int>(dosDevice.size())))
				{
					std::wstring dosDeviceName{ &dosDevice[0] };
					queryDosDevicesMapping.emplace_back(dosDeviceName, drive);
				}
			}

			// Handle network drive. We just remove prefix.
			queryDosDevicesMapping.emplace_back(L"\\Device\\Mup", L"");
			return queryDosDevicesMapping;
		}

		//-------------------------------------------------------------------------
		std::wstring GetFinalPathName(HANDLE hfile)
		{
			std::vector<wchar_t> buffer(PathBufferSize);

			if (!GetFinalPathNameByHandle(hfile, &buffer[0], static_cast<int>(buffer.size()) - 1, VOLUME_NAME_NT))
				THROW_LAST_ERROR(L"Cannot find path for the handle.", GetLastError());

			return &buffer[0];
		}
	}

	//-------------------------------------------------------------------------
	HandleInformation::HandleInformation() = default;

	//-------------------------------------------------------------------------
	std::wstring HandleInformation::ComputeFilename(HANDLE hfile) const
	{
		auto mappedFileName = GetFinalPathName(hfile);
		auto queryDosDevicesMapping = GetQueryDosDevicesMapping();

		for (const auto& queryDosDeviceMapping : queryDosDevicesMapping)
		{
			const auto& deviceName = queryDosDeviceMapping.first;
			const auto& logicalDrive = queryDosDeviceMapping.second;

			if (boost::algorithm::istarts_with(mappedFileName, deviceName))
			{
				boost::algorithm::ireplace_first(mappedFileName, deviceName, logicalDrive);
				return mappedFileName;
			}
		}

		THROW(L"Cannot find path for the handle: " << mappedFileName);
	}
}
