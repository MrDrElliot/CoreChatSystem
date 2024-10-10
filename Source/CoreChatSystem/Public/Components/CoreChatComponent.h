﻿/*MIT License

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
#include "CoreChatTypes.h"
#include "Components/ActorComponent.h"
#include "CoreChatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCoreChatMessageDelegate, const FCoreChatChannelDescriptor&, Channel, const FCoreChatPayload&, ChatMessage);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CORECHATSYSTEM_API UCoreChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCoreChatComponent();

protected:
	
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Core Chat")
	void SendMessage(FName Channel, FString Message);

	UFUNCTION(BlueprintCallable, Category = "Core Chat")
	void RegisterAsChannelListener(FName Channel);

	

	UFUNCTION(Server, Reliable, Category = "Core Chat")
	void Server_NotifyAsMessageListener(const FName& Channel);
	
	UFUNCTION(Server, Reliable, Category = "Core Chat")
	void Server_SendChatMessage(const FName& Channel, const FCoreChatPayload& ChatMessage);

	UFUNCTION(Client, Reliable, Category = "Core Chat")
	void Client_ReceiveChatPayload(const FCoreChatChannelDescriptor& Channel, const FCoreChatPayload& ChatMessage);

	UFUNCTION(Client, Reliable, Category = "Core Chat")
	void Client_ReceiveChatBufferHistory(const TArray<FCoreChatContainer>& BufferHistory);

	static void ParseAndFilterChatMessage(const FCoreChatPayload& InPayload, FCoreChatPayload& OutPayload);
	static bool CheckConditionals(const FCoreChatPayload& InPayload);

public:

	UPROPERTY(BlueprintAssignable)
	FCoreChatMessageDelegate OnMessagePayloadReceived;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Chat")
	TArray<FName> InitialChannels;

private:

	TArray<FCoreChatContainer> ChatContainer;
	TArray<FName> ListeningChannels;
};