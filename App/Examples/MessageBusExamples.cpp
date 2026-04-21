#include <util/messagebus.h>

#include <iostream>
#include <string>
#include <vector>

namespace us::examples
{
	struct PlayerDamagedMessage
	{
		int         amount = 0;
		std::string source;
	};

	struct UiToastMessage
	{
		std::string text;
	};

	struct CameraChangedMessage
	{
		std::string cameraName;
	};

	class CombatLog
	{
	public:
		void OnPlayerDamaged(const PlayerDamagedMessage& message, const vde::util::MessageContext& context)
		{
			entries.push_back(
				"[topic=" + std::to_string(context.topic) + "] damage=" +
				std::to_string(message.amount) + " source=" + message.source);
		}

		std::vector<std::string> entries;
	};

	void Example_BasicPublishSubscribe()
	{
		vde::util::MessageBus bus;

		[[maybe_unused]] auto subscription = bus.Subscribe<PlayerDamagedMessage>([](const PlayerDamagedMessage& message) {
			std::cout << "Basic: player damaged by " << message.source
			          << " for " << message.amount << " HP\n";
		});

		bus.Publish(PlayerDamagedMessage{ 12, "spike trap" });
	}

	void Example_TopicFiltering()
	{
		vde::util::MessageBus bus;

		[[maybe_unused]] auto uiSubscription = bus.SubscribeOn<UiToastMessage>("ui"_h, [](const UiToastMessage& message) {
			std::cout << "UI topic only: " << message.text << '\n';
		});

		bus.PublishOn("gameplay"_h, UiToastMessage{ "This one is ignored by the UI subscriber" });
		bus.PublishOn("ui"_h, UiToastMessage{ "Inventory saved" });
	}

	void Example_QueuedMessages()
	{
		vde::util::MessageBus bus;

		[[maybe_unused]] auto subscription = bus.Subscribe<CameraChangedMessage>(
			[](const CameraChangedMessage& message, const vde::util::MessageContext& context)
			{
				std::cout << "Queued=" << (context.queued ? "true" : "false")
				          << " camera=" << message.cameraName << '\n';
			});

		bus.Enqueue(CameraChangedMessage{ "DebugCamera" });
		bus.EnqueueOn("ui"_h, UiToastMessage{ "Camera switch requested" });
		bus.DispatchQueued();
	}

	void Example_MemberFunctionAndRaii()
	{
		vde::util::MessageBus bus;
		CombatLog log;

		{
			[[maybe_unused]] auto scopedSubscription = bus.Subscribe<PlayerDamagedMessage>(
				log,
				&CombatLog::OnPlayerDamaged,
				"combat"_h);

			bus.PublishOn("combat"_h, PlayerDamagedMessage{ 24, "boss slam" });
		}

		// The scoped subscription was destroyed, so this second message is ignored.
		bus.PublishOn("combat"_h, PlayerDamagedMessage{ 5, "late poison tick" });
	}

	void Example_GlobalBus()
	{
		[[maybe_unused]] auto subscription = vde::util::MessageBus::Global().SubscribeOnceOn<UiToastMessage>(
			"global/ui"_h,
			[](const UiToastMessage& message)
			{
				std::cout << "Global bus: " << message.text << '\n';
			});

		vde::util::MessageBus::Global().EmplaceOn<UiToastMessage>("global/ui"_h, "Profile saved");
	}

	void RunMessageBusExamples()
	{
		Example_BasicPublishSubscribe();
		Example_TopicFiltering();
		Example_QueuedMessages();
		Example_MemberFunctionAndRaii();
		Example_GlobalBus();
	}
}
