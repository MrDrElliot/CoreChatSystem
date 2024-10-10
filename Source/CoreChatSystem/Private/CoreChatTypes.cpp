/*MIT License

Copyright (c) 2024 DrElliot

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#include "CoreChatTypes.h"

#include "Settings/CoreChatSettings.h"


bool FCoreChatChannelDescriptor::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint32 ChannelIndex = INDEX_NONE;
	UCoreChatSettings* Settings = UCoreChatSettings::Get();
		
	if(Ar.IsSaving())
	{
		ChannelIndex = Settings->DefaultChannels.Find(*this);
	}

	Ar.SerializeIntPacked(ChannelIndex);

	if(Ar.IsLoading())
	{
		const FCoreChatChannelDescriptor& FoundDescriptor = Settings->DefaultChannels[ChannelIndex];
		ChannelName = FoundDescriptor.ChannelName;
		Color = FoundDescriptor.Color;
		MessageCooldown = FoundDescriptor.MessageCooldown;
	}

	bOutSuccess = true;
	return true;
}

bool FCoreChatPayload::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	UCoreChatSettings* Settings = UCoreChatSettings::Get();
	int32 MaxLength = Settings->MaximumMessageLength; //@TODO impose max message content.

	uint64 Time = 0;

	if(Ar.IsSaving())
	{
		Time = Timestamp.ToUnixTimestamp();
	}

	Ar << Username;
	Ar << Content;
	
	Ar.SerializeIntPacked64(Time);

	if(Ar.IsLoading())
	{
		Timestamp = FDateTime::FromUnixTimestamp(Time);
	}

	bOutSuccess = true;
	return true;
	
}
