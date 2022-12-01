/**
 * @file ShitModule.cpp
 * @author yangzs
 * @brief
 * @version 0.1
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "ShitModule.hpp"

#ifndef UNICODE
typedef std::string String;
#else
typedef std::wstring String;
#endif

#ifdef _WIN32
#include <windows.h>
#include <strsafe.h>
#endif

namespace Shit
{
	static std::unordered_map<const char *, std::unique_ptr<Module>> sModuleTable;
#ifdef _WIN32
	void ErrorExit(LPTSTR lpszFunction)
	{
		// Retrieve the system error message for the last-error code

		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		// Display the error message and exit the process

		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
										  (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
		StringCchPrintf((LPTSTR)lpDisplayBuf,
						LocalSize(lpDisplayBuf) / sizeof(TCHAR),
						TEXT("%s failed with error %d: %s"),
						lpszFunction, dw, lpMsgBuf);
		MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
		ExitProcess(dw);
	}

	class Win32Module : public Module
	{

		HINSTANCE mHDLL = NULL;

	public:
		Win32Module(const char *moduleName)
		{
			std::string fileName = BuildModuleFileName(moduleName);
			mHDLL = LoadLibrary(fileName.data());
			if (mHDLL == NULL)
			{
				auto str = "LoadLibrary " + fileName + " ";
				ErrorExit(str.data());
				// ST_THROW("failed to load dll ", fileName.data(), "error code: ", GetLastError());
			}
		}
		~Win32Module()
		{
			FreeLibrary(mHDLL);
		}
		std::string BuildModuleFileName(const char* moduleName) override
		{
			std::string str = ExePath();
			str += "/";
			str += moduleName;
#ifndef NDEBUG
			str += 'd';
#endif
			str += ".dll";
			return str;
		}

		String ExePath()
		{
			TCHAR buffer[MAX_PATH] = {};
			GetModuleFileName(NULL, buffer, MAX_PATH);
			auto pos = String(buffer).find_last_of("\\/");
			return String(buffer).substr(0, pos);
		}

		void* LoadProc(const char* procName) override
		{
			return reinterpret_cast<void*>(GetProcAddress(mHDLL, procName));
		}
	};

#endif

	Module* ModuleManager::LoadModule(const char* moduleName)
	{
#ifdef _WIN32
		sModuleTable[moduleName] = std::unique_ptr<Module>(new Win32Module(moduleName));
		return sModuleTable[moduleName].get();
#endif //
	}

	Module* ModuleManager::GetModule(const char* moduleName)
	{
		if (sModuleTable.find(moduleName) == sModuleTable.end())
		{
			return LoadModule(moduleName);
		}
		return sModuleTable[moduleName].get();
	}

	void ModuleManager::UnLoadModule(const char* moduleName)
	{
		sModuleTable.erase(moduleName);
	}

}