#include "RPCUtils.h"
#include <string>

#pragma comment(lib, "discord_game_sdk.dll")

discord::Core* core;
discord::Result result;

void callbackThread() {

	while (true) {
		core->RunCallbacks();
		Sleep(10);
	}
}


void RPCUtils::Initialize() {
		result = discord::Core::Create(1047247943880683570, DiscordCreateFlags_NoRequireDiscord, &core);
		if (result != discord::Result::Ok) {
			std::cout << "Failed to create Discord Core: " << std::to_string((int)result) << std::endl;
			system("pause");
			return;
		}
		
		CreateThread(0, 0, LPTHREAD_START_ROUTINE(callbackThread), NULL, 0, 0);
}

void RPCUtils::SetPresence(const char* state, const char* details, const char* largeImageKey,
	const char* largeImageText, discord::Timestamp startTime)
{
	static discord::Activity activity{};
	activity.SetState(state);
	activity.SetDetails(details);
	activity.GetAssets().SetLargeImage(largeImageKey);
	activity.GetAssets().SetLargeText(largeImageText);
	activity.GetTimestamps().SetStart(startTime);

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		if (result != discord::Result::Ok) {
			std::cout << "Failed to set activity: " << std::to_string((int)result) << std::endl;
		}
		});
}
