#ifndef VDE__UTIL__MESSAGEBUS_H
#define VDE__UTIL__MESSAGEBUS_H
#pragma once

#include <util/globalinstance.h>
#include <util/stringhash.h>

#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace vde::util
{
	class MessageBus;
	class MessageSubscription;

	using message_topic_t = string_hash_t;

	inline constexpr message_topic_t kDEFAULT_MESSAGE_TOPIC = 0;
	inline constexpr message_topic_t kANY_MESSAGE_TOPIC     = std::numeric_limits<message_topic_t>::max();

	struct MessageContext
	{
		MessageBus&      bus;
		message_topic_t  topic    = kDEFAULT_MESSAGE_TOPIC;
		bool             queued   = false;
		std::uint64_t    sequence = 0;
	};

	namespace detail
	{
		class MessageBusImpl
		{
		public:
			using subscription_id_t = std::uint64_t;
			using callback_t        = std::move_only_function<void(const void*, const MessageContext&)>;
			using queued_callback_t = std::move_only_function<void(MessageBus&)>;

			struct SubscriptionRecord
			{
				subscription_id_t id     = 0;
				message_topic_t   topic  = kANY_MESSAGE_TOPIC;
				bool              once   = false;
				bool              active = true;
				callback_t        callback;
			};

			struct QueuedMessage
			{
				queued_callback_t callback;
			};

			std::unordered_map<std::type_index, std::vector<SubscriptionRecord>> subscriptions;
			std::deque<QueuedMessage>                                            queuedMessages;

			subscription_id_t nextSubscriptionId = 1;
			std::uint64_t     nextSequence       = 1;
			std::size_t       dispatchDepth      = 0;
			bool              needsCleanup       = false;

			void CleanupInactive()
			{
				for (auto it = subscriptions.begin(); it != subscriptions.end(); )
				{
					auto& records = it->second;
					records.erase(std::remove_if(records.begin(), records.end(), [](const SubscriptionRecord& record) {
						return !record.active;
					}), records.end());

					if (records.empty())
						it = subscriptions.erase(it);
					else
						++it;
				}

				needsCleanup = false;
			}

			bool Unsubscribe(const std::type_index& type, subscription_id_t id)
			{
				auto it = subscriptions.find(type);
				if (it == subscriptions.end())
					return false;

				for (auto& record : it->second)
				{
					if (record.id != id || !record.active)
						continue;

					record.active = false;
					needsCleanup = true;

					if (dispatchDepth == 0)
						CleanupInactive();

					return true;
				}

				return false;
			}
		};
	}

	class MessageSubscription
	{
		friend class MessageBus;

		std::weak_ptr<detail::MessageBusImpl> m_impl;
		std::type_index                       m_messageType { typeid(void) };
		std::uint64_t                         m_subscriptionId = 0;

		void m_UnsubscribeInternal()
		{
			if (auto impl = m_impl.lock(); impl && m_subscriptionId != 0)
				impl->Unsubscribe(m_messageType, m_subscriptionId);

			m_impl.reset();
			m_messageType = std::type_index(typeid(void));
			m_subscriptionId = 0;
		}

	public:
		MessageSubscription() = default;
		~MessageSubscription() noexcept { Reset(); }

		MessageSubscription(const MessageSubscription&) = delete;
		MessageSubscription& operator=(const MessageSubscription&) = delete;

		MessageSubscription(MessageSubscription&& other) noexcept
			: m_impl(std::move(other.m_impl))
			, m_messageType(other.m_messageType)
			, m_subscriptionId(other.m_subscriptionId)
		{
			other.m_messageType = std::type_index(typeid(void));
			other.m_subscriptionId = 0;
		}

		MessageSubscription& operator=(MessageSubscription&& other) noexcept
		{
			if (this == &other)
				return *this;

			Reset();
			m_impl = std::move(other.m_impl);
			m_messageType = other.m_messageType;
			m_subscriptionId = other.m_subscriptionId;

			other.m_messageType = std::type_index(typeid(void));
			other.m_subscriptionId = 0;
			return *this;
		}

		bool IsValid() const
		{
			return m_subscriptionId != 0 && !m_impl.expired();
		}

		explicit operator bool() const
		{
			return IsValid();
		}

		void Reset()
		{
			m_UnsubscribeInternal();
		}
	};

	class MessageBus
		: public GlobalInstance<MessageBus>
	{
		std::shared_ptr<detail::MessageBusImpl> m_impl;

		template<typename Message>
		using message_t = std::remove_cvref_t<Message>;

		template<typename Message, typename Callback>
		static detail::MessageBusImpl::callback_t m_MakeCallback(Callback&& callback)
		{
			using callback_type = std::decay_t<Callback>;
			using message_type  = message_t<Message>;

			static_assert(
				std::is_invocable_v<callback_type&, const message_type&> ||
				std::is_invocable_v<callback_type&, const message_type&, const MessageContext&>,
				"MessageBus callbacks must be invocable with (const Message&) or (const Message&, const MessageContext&).");

			return [fn = callback_type(std::forward<Callback>(callback))](const void* payload, const MessageContext& context) mutable
			{
				const message_type& message = *reinterpret_cast<const message_type*>(payload);

				if constexpr (std::is_invocable_v<callback_type&, const message_type&, const MessageContext&>)
					std::invoke(fn, message, context);
				else
					std::invoke(fn, message);
			};
		}

		template<typename Message>
		void m_PublishInternal(const message_t<Message>& message, message_topic_t topic, bool queued, std::uint64_t sequence)
		{
			const auto type = std::type_index(typeid(message_t<Message>));
			auto it = m_impl->subscriptions.find(type);
			if (it == m_impl->subscriptions.end())
				return;

			MessageContext context {
				.bus      = *this,
				.topic    = topic,
				.queued   = queued,
				.sequence = sequence
			};

			auto& subscribers = it->second;
			++m_impl->dispatchDepth;

			const std::size_t initialCount = subscribers.size();
			for (std::size_t i = 0; i < initialCount; ++i)
			{
				auto& subscriber = subscribers[i];
				if (!subscriber.active)
					continue;

				if (subscriber.topic != kANY_MESSAGE_TOPIC && subscriber.topic != topic)
					continue;

				subscriber.callback(&message, context);

				if (subscriber.once)
				{
					subscriber.active = false;
					m_impl->needsCleanup = true;
				}
			}

			--m_impl->dispatchDepth;
			if (m_impl->dispatchDepth == 0 && m_impl->needsCleanup)
				m_impl->CleanupInactive();
		}

		template<typename Message>
		MessageSubscription m_AddSubscription(
			message_topic_t topic,
			bool once,
			detail::MessageBusImpl::callback_t&& callback)
		{
			const auto type = std::type_index(typeid(message_t<Message>));
			auto& subscribers = m_impl->subscriptions[type];

			detail::MessageBusImpl::SubscriptionRecord record;
			record.id = m_impl->nextSubscriptionId++;
			record.topic = topic;
			record.once = once;
			record.callback = std::move(callback);
			subscribers.push_back(std::move(record));

			MessageSubscription subscription;
			subscription.m_impl = m_impl;
			subscription.m_messageType = type;
			subscription.m_subscriptionId = subscribers.back().id;
			return subscription;
		}

	public:
		MessageBus()
			: m_impl(std::make_shared<detail::MessageBusImpl>())
		{
		}

		~MessageBus() noexcept = default;

		MessageBus(const MessageBus&) = delete;
		MessageBus& operator=(const MessageBus&) = delete;
		MessageBus(MessageBus&&) noexcept = default;
		MessageBus& operator=(MessageBus&&) noexcept = default;

		template<typename Message, typename Callback>
		MessageSubscription Subscribe(Callback&& callback, message_topic_t topic = kANY_MESSAGE_TOPIC)
		{
			return m_AddSubscription<Message>(
				topic,
				false,
				m_MakeCallback<Message>(std::forward<Callback>(callback)));
		}

		template<typename Message, typename Callback>
		MessageSubscription SubscribeOnce(Callback&& callback, message_topic_t topic = kANY_MESSAGE_TOPIC)
		{
			return m_AddSubscription<Message>(
				topic,
				true,
				m_MakeCallback<Message>(std::forward<Callback>(callback)));
		}

		template<typename Message, typename Callback>
		MessageSubscription SubscribeOn(message_topic_t topic, Callback&& callback)
		{
			return Subscribe<Message>(std::forward<Callback>(callback), topic);
		}

		template<typename Message, typename Callback>
		MessageSubscription SubscribeOnceOn(message_topic_t topic, Callback&& callback)
		{
			return SubscribeOnce<Message>(std::forward<Callback>(callback), topic);
		}

		template<typename Message, typename Object>
		MessageSubscription Subscribe(Object& object, void (Object::*method)(const message_t<Message>&), message_topic_t topic = kANY_MESSAGE_TOPIC)
		{
			return Subscribe<message_t<Message>>([&object, method](const message_t<Message>& message) {
				std::invoke(method, object, message);
			}, topic);
		}

		template<typename Message, typename Object>
		MessageSubscription Subscribe(Object& object, void (Object::*method)(const message_t<Message>&, const MessageContext&), message_topic_t topic = kANY_MESSAGE_TOPIC)
		{
			return Subscribe<message_t<Message>>([&object, method](const message_t<Message>& message, const MessageContext& context) {
				std::invoke(method, object, message, context);
			}, topic);
		}

		template<typename Message, typename Object>
		MessageSubscription Subscribe(Object& object, void (Object::*method)(const message_t<Message>&) const, message_topic_t topic = kANY_MESSAGE_TOPIC)
		{
			return Subscribe<message_t<Message>>([&object, method](const message_t<Message>& message) {
				std::invoke(method, object, message);
			}, topic);
		}

		template<typename Message, typename Object>
		MessageSubscription Subscribe(Object& object, void (Object::*method)(const message_t<Message>&, const MessageContext&) const, message_topic_t topic = kANY_MESSAGE_TOPIC)
		{
			return Subscribe<message_t<Message>>([&object, method](const message_t<Message>& message, const MessageContext& context) {
				std::invoke(method, object, message, context);
			}, topic);
		}

		template<typename Message>
		void Publish(Message&& message, message_topic_t topic = kDEFAULT_MESSAGE_TOPIC)
		{
			const std::uint64_t sequence = m_impl->nextSequence++;
			m_PublishInternal<message_t<Message>>(message, topic, false, sequence);
		}

		template<typename Message>
		void PublishOn(message_topic_t topic, Message&& message)
		{
			Publish(std::forward<Message>(message), topic);
		}

		template<typename Message, typename... Args>
		void Emplace(Args&&... args)
		{
			Publish(message_t<Message>{ std::forward<Args>(args)... });
		}

		template<typename Message, typename... Args>
		void EmplaceOn(message_topic_t topic, Args&&... args)
		{
			Publish(message_t<Message>{ std::forward<Args>(args)... }, topic);
		}

		template<typename Message>
		void Enqueue(Message&& message, message_topic_t topic = kDEFAULT_MESSAGE_TOPIC)
		{
			const std::uint64_t sequence = m_impl->nextSequence++;
			using stored_message_t = message_t<Message>;

			m_impl->queuedMessages.push_back({
				.callback = [payload = stored_message_t(std::forward<Message>(message)), topic, sequence](MessageBus& bus) mutable
				{
					bus.m_PublishInternal<stored_message_t>(payload, topic, true, sequence);
				}
			});
		}

		template<typename Message>
		void EnqueueOn(message_topic_t topic, Message&& message)
		{
			Enqueue(std::forward<Message>(message), topic);
		}

		template<typename Message, typename... Args>
		void EnqueueEmplace(Args&&... args)
		{
			Enqueue(message_t<Message>{ std::forward<Args>(args)... });
		}

		template<typename Message, typename... Args>
		void EnqueueEmplaceOn(message_topic_t topic, Args&&... args)
		{
			Enqueue(message_t<Message>{ std::forward<Args>(args)... }, topic);
		}

		std::size_t DispatchQueued(std::size_t maxMessages = std::numeric_limits<std::size_t>::max())
		{
			std::size_t dispatched = 0;

				while (dispatched < maxMessages && !m_impl->queuedMessages.empty())
				{
					auto queued = std::move(m_impl->queuedMessages.front());
					m_impl->queuedMessages.pop_front();
					queued.callback(*this);
					++dispatched;
				}

			return dispatched;
		}

		void ClearQueued()
		{
			m_impl->queuedMessages.clear();
		}

		std::size_t QueuedCount() const
		{
			return m_impl->queuedMessages.size();
		}

		void ClearAllSubscriptions()
		{
			for (auto& [_, subscribers] : m_impl->subscriptions)
			{
				for (auto& subscriber : subscribers)
					subscriber.active = false;
			}

			m_impl->needsCleanup = true;
			if (m_impl->dispatchDepth == 0)
				m_impl->CleanupInactive();
		}

		template<typename Message>
		void ClearSubscriptions(message_topic_t topic = kANY_MESSAGE_TOPIC)
		{
			const auto type = std::type_index(typeid(message_t<Message>));
			auto it = m_impl->subscriptions.find(type);
			if (it == m_impl->subscriptions.end())
				return;

			for (auto& subscriber : it->second)
			{
				if (topic == kANY_MESSAGE_TOPIC || subscriber.topic == topic)
					subscriber.active = false;
			}

			m_impl->needsCleanup = true;
			if (m_impl->dispatchDepth == 0)
				m_impl->CleanupInactive();
		}

		template<typename Message>
		std::size_t SubscriberCount(message_topic_t topic = kANY_MESSAGE_TOPIC) const
		{
			const auto type = std::type_index(typeid(message_t<Message>));
			auto it = m_impl->subscriptions.find(type);
			if (it == m_impl->subscriptions.end())
				return 0;

			std::size_t count = 0;
			for (const auto& subscriber : it->second)
			{
				if (!subscriber.active)
					continue;

				if (topic == kANY_MESSAGE_TOPIC || subscriber.topic == kANY_MESSAGE_TOPIC || subscriber.topic == topic)
					++count;
			}

			return count;
		}

		template<typename Message>
		bool HasSubscribers(message_topic_t topic = kANY_MESSAGE_TOPIC) const
		{
			return SubscriberCount<Message>(topic) > 0;
		}
	};
}

#endif /* VDE__UTIL__MESSAGEBUS_H */
