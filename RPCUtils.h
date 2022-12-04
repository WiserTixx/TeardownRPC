#pragma once

#include <iostream>
#include "discord/discord.h"

class RPCUtils
{
public:
	static void Initialize();
	static void SetPresence(const char* state, const char* details, const char* largeImageKey, const char* largeImageText, discord::Timestamp startTime);
	static void RunCallbacks();
};