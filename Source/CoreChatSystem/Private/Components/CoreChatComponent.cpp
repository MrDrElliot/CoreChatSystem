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


#include "Components/CoreChatComponent.h"

#include "GameFramework/PlayerState.h"
#include "Settings/CoreChatSettings.h"
#include "Subsystems/CoreChatSubsystem.h"


UCoreChatComponent::UCoreChatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UCoreChatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(Cast<APlayerController>(GetOwner())->IsLocalPlayerController())
	{
		for (const auto Channel : InitialChannels)
		{
			RegisterAsChannelListener(Channel);
		}
	}
}

void UCoreChatComponent::SendMessage(const FName Channel, FString Message)
{
	UCoreChatSettings* Settings = UCoreChatSettings::Get();
	FCoreChatPayload ChatPayload;
	ChatPayload.Content = Message;
	ChatPayload.Timestamp = FDateTime::Now();
	ChatPayload.Username = GetOwner<APlayerController>()->GetPlayerState<APlayerState>()->GetPlayerName();

	// Client side check to verify message length.. still need to block serialization if it exceeds.
	if(Settings->MaximumMessageLength != 0)
	{
		if(Settings->MaximumMessageLength >= Message.Len())
		{
			Server_SendChatMessage(Channel, ChatPayload);
		}
	}
	else
	{
		Server_SendChatMessage(Channel, ChatPayload);
	}
}

void UCoreChatComponent::RegisterAsChannelListener(FName Channel)
{
	Server_NotifyAsMessageListener(Channel);
	ListeningChannels.AddUnique(Channel);
}

void UCoreChatComponent::ParseAndFilterChatMessage(const FCoreChatPayload& InPayload, FCoreChatPayload& OutPayload)
{
	if (InPayload.Content.IsEmpty())
	{
		OutPayload = InPayload;
		return;
	}

	FString Content = InPayload.Content;

	for (const FString& Filter : UCoreChatSettings::Get()->FilteredWords)
	{
		FString FilterReplacement;
		for (int i = 0; i < Filter.Len(); ++i)
		{
			FilterReplacement += "*";
		}
		Content = Content.Replace(*Filter, *FilterReplacement, ESearchCase::IgnoreCase);
	}

	OutPayload = InPayload;
	OutPayload.Content = Content;
}

bool UCoreChatComponent::CheckConditionals(const FCoreChatPayload& InPayload)
{
	UCoreChatSettings* Settings = UCoreChatSettings::Get();
	check(Settings);

	if(!Settings->bAllowEmptyMessages)
	{
		const FString& Content = InPayload.Content;

		if(Content.TrimStartAndEnd().IsEmpty())
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	return true;
}

void UCoreChatComponent::Server_NotifyAsMessageListener_Implementation(const FName& Channel)
{
	if(IsValid(this))
	{
		UCoreChatSubsystem::Get(this)->RegisterListener(this, Channel);
	}
}

void UCoreChatComponent::Client_ReceiveChatBufferHistory_Implementation(const TArray<FCoreChatContainer>& BufferHistory)
{
	//@TODO 
}

void UCoreChatComponent::Client_ReceiveChatPayload_Implementation(const FCoreChatChannelDescriptor& Channel, const FCoreChatPayload& ChatMessage)
{
	// Find if the channel already exists in the ChatContainer
	TOptional<FCoreChatContainer> FoundChannel;
	
	for (FCoreChatContainer& ChannelContainer : ChatContainer)
	{
		if (ChannelContainer.Channel == Channel)
		{
			FoundChannel = Channel;
			break;
		}
	}

	if (FoundChannel.IsSet())
	{
		// Channel already exists, add message to the existing channel
		FoundChannel->Messages.Add(ChatMessage);
		OnMessagePayloadReceived.Broadcast(Channel, ChatMessage);
	}
	else
	{
		// Channel does not exist, create a new channel and add the message
		FCoreChatContainer NewChannel;
		NewChannel.Channel = Channel;
		NewChannel.Messages.Add(ChatMessage);
		ChatContainer.Add(NewChannel);
		OnMessagePayloadReceived.Broadcast(Channel, ChatMessage);
	}
}

void UCoreChatComponent::Server_SendChatMessage_Implementation(const FName& Channel, const FCoreChatPayload& ChatMessage)
{
	const FCoreChatPayload& MutablePayload = ChatMessage;
	FCoreChatPayload FilteredPayload;

	if(!CheckConditionals(ChatMessage))
	{
		return;
	}
	
	ParseAndFilterChatMessage(MutablePayload, FilteredPayload);

	UCoreChatSubsystem::Get(this)->UploadAndBroadcastChatMessage(this, Channel, ChatMessage);
}

