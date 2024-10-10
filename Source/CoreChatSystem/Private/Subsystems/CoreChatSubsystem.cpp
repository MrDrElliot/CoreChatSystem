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

#include "Subsystems/CoreChatSubsystem.h"

#include "Components/CoreChatComponent.h"
#include "Serialization/ArchiveLoadCompressedProxy.h"
#include "Serialization/ArchiveSaveCompressedProxy.h"
#include "Settings/CoreChatSettings.h"

void UCoreChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeChannels();
}

bool UCoreChatSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (Outer && Outer->IsA<UWorld>())
	{
		const UWorld* World = Cast<UWorld>(Outer);
		
		if (World && World->WorldType != EWorldType::Editor && World->GetNetMode() < NM_Client)
		{
			return Super::ShouldCreateSubsystem(Outer);
		}
	}
	return false;
}

void UCoreChatSubsystem::InitializeChannels()
{
	const UCoreChatSettings* ChatSettings = UCoreChatSettings::Get();
	if(ChatSettings->DefaultChannels.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Please specify a channels list in the project settings!"));
		return;
	}

	for (const auto Channel : ChatSettings->DefaultChannels)
	{
		ChatChannels.Add(FCoreChatContainer(Channel));
	}

}


void UCoreChatSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UCoreChatSubsystem::CreateChatChannel(FCoreChatChannelDescriptor Descriptor)
{
	for (auto Itr = ChatChannels.CreateIterator(); Itr; ++Itr)
	{
		FCoreChatContainer Container = *Itr;
		if(Container.Channel == Descriptor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Channel already exists!"));
			return;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Added New Channel!"));
	ChatChannels.Add(FCoreChatContainer(Descriptor));
}

void UCoreChatSubsystem::RegisterListenerToChannel(UCoreChatComponent* InChatComponent, FName Channel)
{
	RegisterListener(InChatComponent, Channel);
}

UCoreChatSubsystem* UCoreChatSubsystem::Get(const UObject* WorldContextObject)
{
	if(const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<UCoreChatSubsystem>();
	}
	return nullptr;
}

void UCoreChatSubsystem::UploadAndBroadcastChatMessage(UCoreChatComponent* Sender, const FName& ChannelName, const FCoreChatPayload& ChatMessage)
{
	// Find the channel by name
	for (auto& ChannelContainer : ChatChannels)
	{
		if (ChannelContainer.Channel.ChannelName != ChannelName)
		{
			continue;
		}

		// Add the message to the channel
		ChannelContainer.Messages.Add(ChatMessage);

		// Get the registered listeners for the given channel
		if (!RegisteredListeners.Contains(ChannelName))
		{
			return;
		}

		auto& Listeners = RegisteredListeners[ChannelName];

		// Iterate over the listeners for this channel
		for (int32 i = Listeners.Num() - 1; i >= 0; --i)
		{
			TWeakObjectPtr<UCoreChatComponent> ListenerComponent = Listeners[i];

			// Remove invalid components from the list
			if (!ListenerComponent.IsValid())
			{
				Listeners.RemoveAt(i);
				continue;
			}

			// Send the chat payload to valid listeners
			if (UCoreChatComponent* Component = ListenerComponent.Get())
			{
				Component->Client_ReceiveChatPayload(ChannelContainer.Channel, ChatMessage);
			}
		}
		break;
	}
}

void UCoreChatSubsystem::RegisterListener(UCoreChatComponent* InComponent, const FName& Channel)
{
	check(InComponent);

	// Find or create an entry for the channel in the RegisteredListeners map
	TArray<TWeakObjectPtr<UCoreChatComponent>>& Listeners = RegisteredListeners.FindOrAdd(Channel);

	// Add the listener to the array
	Listeners.AddUnique(InComponent);

}

void UCoreChatSubsystem::UnregisterListener(UCoreChatComponent* InComponent, const FName& Channel)
{
	check(InComponent);

	// Find the entry for the channel in the RegisteredListeners map
	if (TArray<TWeakObjectPtr<UCoreChatComponent>>* ListenersPtr = RegisteredListeners.Find(Channel))
	{
		// Remove the listener from the array
		TArray<TWeakObjectPtr<UCoreChatComponent>>& Listeners = *ListenersPtr;
		Listeners.Remove(InComponent);

		// If the array is now empty, remove the entry from the map
		if (Listeners.Num() == 0)
		{
			RegisteredListeners.Remove(Channel);
		}
	}
}


