#include "RPCUtils.h"
#include <Windows.h>
#include <tlhelp32.h>
#include "rapidxml/rapidxml.hpp"
#include <fstream>
#include "rapidxml/rapidxml_utils.hpp"
#include <exception>


#define ProgramExit(why) { \
MessageBoxA(NULL, why, "Teardown RPC", MB_OK | MB_ICONWARNING); \
return 0; \
}

int main() {
    SetConsoleTitleA("TeardownRPC - by Tixx");

    DWORD pid = 0;
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (processesSnapshot == INVALID_HANDLE_VALUE) {
        pid = 0;

        goto end;
    }

    Process32First(processesSnapshot, &processInfo);
    if (!std::wstring(L"teardown.exe").compare(processInfo.szExeFile))
    {
        CloseHandle(processesSnapshot);
        pid = processInfo.th32ProcessID;
        goto end;
    }

    while (Process32Next(processesSnapshot, &processInfo))
    {
        if (!std::wstring(L"teardown.exe").compare(processInfo.szExeFile))
        {
            pid = processInfo.th32ProcessID;
            break;
        }
    }

    end:
    CloseHandle(processesSnapshot);

    if (pid == 0) {
        ProgramExit("Teardown isn't open, please open your game in order to use the RPC")
    }

	auto teardownHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid); 

    FILETIME creationTime, exitTime, kernelTime, userTime;


    GetProcessTimes(teardownHandle, &creationTime, &exitTime, &kernelTime, &userTime);

	std::cout << "Starting the RPC" << std::endl;
	RPCUtils::Initialize();

    LARGE_INTEGER startTime, _timeConvert;

    startTime.HighPart = creationTime.dwHighDateTime;
    startTime.LowPart = creationTime.dwLowDateTime;

    _timeConvert.QuadPart = 11644473600000 * 10000;

    startTime.QuadPart -= _timeConvert.QuadPart;

    char* localappdata = NULL;
    size_t lenght;

    _dupenv_s(&localappdata, &lenght, "localappdata");

    std::string teardownFolder = localappdata + std::string("\\Teardown");


    std::string savegameFilepath = teardownFolder + "\\savegame.xml";
	
    while (true) {
        try {
            if (WaitForSingleObject(teardownHandle, 0) != WAIT_TIMEOUT) {
                ProgramExit("It seems like teardown got closed");
            }

            rapidxml::file<> xml(savegameFilepath.c_str());
            rapidxml::xml_document<> doc;
            doc.parse<0>(xml.data());
            
            auto base = doc.first_node("registry")->first_node("savegame");
            
            static std::string scriptName = "";

            if (scriptName == "") {
                if (base->first_node("mod")->first_node("local-teardownrpc") != nullptr) {
                    scriptName = "local-teardownrpc";
                }
                else if (base->first_node("mod")->first_node("steam-STEAMWORKSHOPMODIDHERETHATIDONTHAVEYET") != nullptr) {
                    scriptName = "steam-STEAMWORKSHOPMODIDHERETHATIDONTHAVEYET";
                }
                else {
					ProgramExit("Please install the TeardownRPC script")
                }
				
				rapidxml::file<> modsXml((teardownFolder + "\\mods.xml").c_str());
				rapidxml::xml_document<> modsDoc;
				modsDoc.parse<0>(modsXml.data());

				auto mods = modsDoc.first_node("mods");
				
                auto current_node = mods->first_node();
				
                while (true) {
                    if (current_node->first_attribute("id")->value() == scriptName) {
                        if (current_node->first_attribute("active")->value() == "false")
                            std::cout << "Please check if the TeardownRPC script is enabled" << std::endl;
                        break;
                    }
					
                    current_node = current_node->next_sibling();

                    if (current_node == nullptr)
                        break; //what
                }
				
				if (rapidxml::count_children( base->first_node("mod")->first_node(scriptName.c_str() )) == 0) {
					ProgramExit("Please open a map in Teardown to use the RPC")
				}
            }
			
            auto rpcScriptData = base->first_node("mod")->first_node(scriptName.c_str());
            auto gameMode = rpcScriptData->first_node("gamemode")->first_attribute()->value();
            auto levelName = rpcScriptData->first_node("levelname")->first_attribute()->value();
            auto isPaused = false;//std::stoi(rpcScriptData->first_node("ispaused")->first_attribute()->value()); maybe once....
            auto displayLevelName = std::stoi(rpcScriptData->first_node("displaylevelname")->first_attribute()->value());
            auto displayGameMode = std::stoi(rpcScriptData->first_node("displaygamemode")->first_attribute()->value());
            std::string largeImage = "teardown_" + std::string(rpcScriptData->first_node("logotype")->first_attribute()->value()); //normal, black, white, gold

            auto state = !displayGameMode ? "" : isPaused ? "In the menus" : gameMode;
            auto details = !displayLevelName ? "" : isPaused ? "" : levelName;
			
            RPCUtils::SetPresence(state, details, largeImage.c_str(), "Teardown", startTime.QuadPart / 10000000);
			

            Sleep(1000);
        }
        catch (std::exception& e) {
			std::cout << "An exception occurred: " << e.what() << std::endl;
			
            system("pause");
        }
    }

	system("pause");
	return 0;
}
