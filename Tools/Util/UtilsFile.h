
#pragma once

#ifndef tstring
#include <string>
#if defined(_UNICODE) || defined(UNICODE)
#define tstring	std::wstring
#else
#define tstring	std::string
#endif
#endif

#include "wtypes.h"

namespace UtilFile{
    bool FileExists(const tstring& path);
    UINT GetFileSize(const tstring path);
    bool DirectoryExists(const tstring& path);
	bool CreateDirectory(const tstring& path);

    tstring ExtractFilePath(const tstring& path);
    tstring ExtractFileName(const tstring& path);
    tstring ExtractFileExt(const tstring& path);

    bool DeleteFile(const tstring& path);
	bool RenameFile(const tstring& path, const tstring& pathNew );
}