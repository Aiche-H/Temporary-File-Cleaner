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
        try
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
                    else if (ec.value() == ERROR_SHARING_VIOLATION)
                    {
                        std::cout << "Skipped: " << entry.path().string() << " - Access denied" << std::endl;
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            if (e.code().value() == ERROR_SHARING_VIOLATION)
            {
                std::cout << "Skipped: " << folder << " - Access denied" << std::endl;
            }
            else
            {
                std::cout << "Error deleting files in folder: " << folder << " - " << e.what() << std::endl;
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

    // Add a pause or wait for user input
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();

    return 0;
}