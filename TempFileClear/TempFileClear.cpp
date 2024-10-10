#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <Windows.h>
#include <TlHelp32.h>

std::vector<std::string> GetTempFolders()
{
    std::vector<std::string> tempFolders;
    std::string tempFolder = std::filesystem::temp_directory_path().string();
    tempFolders.push_back(tempFolder);
    return tempFolders;
}

bool IsElevated()
{
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize))
        {
            return Elevation.TokenIsElevated != 0;
        }
    }
    return false;
}

void DeleteFilesInFolders(const std::vector<std::string>& folders)
{
    for (const std::string& folder : folders)
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(folder))
        {
            if (entry.is_regular_file())
            {
                std::error_code ec;
                std::filesystem::remove(entry.path(), ec);
                if (!ec)
                {
                    std::cout << "Deleted: " << entry.path().string() << std::endl;
                }
                else
                {
                    std::cout << "Skipped: " << entry.path().string() << std::endl;
                }
            }
        }
    }
}

int main()
{
    std::vector<std::string> tempFolders = GetTempFolders();
    bool isElevated = IsElevated();
    std::cout << "Temp Folders: " << std::endl;
    for (const std::string& tempFolder : tempFolders)
    {
        std::wcout << std::wstring(tempFolder.begin(), tempFolder.end()) << std::endl; // Convert to wstring before outputting
    }
    std::cout << "Is Elevated: " << std::boolalpha << isElevated << std::endl;

    DeleteFilesInFolders(tempFolders);

    // Print all processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &processEntry))
        {
            do
            {
                std::wcout << "Skipped files were open due to: " << std::wstring(processEntry.szExeFile, processEntry.szExeFile + wcslen(processEntry.szExeFile)) << std::endl; // Convert to wstring before outputting
            } while (Process32Next(hSnapshot, &processEntry));
        }
        CloseHandle(hSnapshot);
    }

    // Add a pause or wait for user input
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();

    return 0;
}
