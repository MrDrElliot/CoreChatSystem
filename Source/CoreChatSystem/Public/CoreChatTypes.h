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
#pragma once

#include "CoreMinimal.h"
#include "CoreChatTypes.generated.h"

class UCoreChatSettings;

USTRUCT(BlueprintType)
struct FCoreChatChannelDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FName ChannelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FLinearColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	float MessageCooldown;

public:

	FORCEINLINE bool operator == (const FCoreChatChannelDescriptor& Other) const
	{
		return ChannelName == Other.ChannelName;
	}
	
	bool operator == (const FName& OtherChannelName) const
	{
		return ChannelName == OtherChannelName;
	}

	
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	
};
template<>
struct TStructOpsTypeTraits<FCoreChatChannelDescriptor> : public TStructOpsTypeTraitsBase2<FCoreChatChannelDescriptor>
{
	enum
	{
		WithNetSerializer = true,
	};
};


USTRUCT(BlueprintType)
struct FCoreChatPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString Username;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FDateTime Timestamp;


public:

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	
};

template<>
struct TStructOpsTypeTraits<FCoreChatPayload> : public TStructOpsTypeTraitsBase2<FCoreChatPayload>
{
	enum
	{
		WithNetSerializer = true,
	};
};

USTRUCT(BlueprintType)
struct FCoreChatContainer
{
	GENERATED_BODY()

	FCoreChatContainer() = default;
	FCoreChatContainer(const FCoreChatChannelDescriptor& InChannel)
	{
		Channel = InChannel;
		Messages = {};
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FCoreChatChannelDescriptor Channel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	TArray<FCoreChatPayload> Messages;

	bool operator == (const FName& ChannelName) const
	{
		return Channel.ChannelName == ChannelName;
	}
	
};
