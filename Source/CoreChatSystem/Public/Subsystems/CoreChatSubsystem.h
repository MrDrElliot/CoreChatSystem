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
#include "Subsystems/WorldSubsystem.h"
#include "CoreChatSubsystem.generated.h"

struct FCoreChatChannelDescriptor;
class UCoreChatComponent;
struct FCoreChatContainer;
struct FCoreChatPayload;


/**
 * 
 */
UCLASS()
class CORECHATSYSTEM_API UCoreChatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

private:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FCoreChatMessageDelegate, FName&, FCoreChatPayload&)
public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;


	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Core Chat")
	void CreateChatChannel(FCoreChatChannelDescriptor Descriptor);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Core Chat")
	void RegisterListenerToChannel(UCoreChatComponent* InChatComponent, FName Channel);
	

	static UCoreChatSubsystem* Get(const UObject* WorldContextObject);

	FCoreChatMessageDelegate& GetOnMessageBroadcastDelegate() { return OnMessageBroadcast; }
	
	void UploadAndBroadcastChatMessage(UCoreChatComponent* Sender, const FName& ChannelName, const FCoreChatPayload& ChatMessage);
	
	void RegisterListener(UCoreChatComponent* InComponent, const FName& Channel);
	void UnregisterListener(UCoreChatComponent* InComponent, const FName& Channel);

	
private:

	void InitializeChannels();

private:

	FCoreChatMessageDelegate OnMessageBroadcast;
	TArray<FCoreChatContainer> ChatChannels;
	TMap<FName, TArray<TWeakObjectPtr<UCoreChatComponent>>> RegisteredListeners;	
};
