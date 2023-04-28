#ifndef FTORRENT_EVENTS_H
#define FTORRENT_EVENTS_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace ftorrent {
namespace events {

    struct EventGroup {
    public:
        EventGroup(const std::string& n): name{n} {}
        std::string name;

        static EventGroup torrent() {
            static EventGroup t{"torrent"};
            return t;
        };

        static EventGroup tracker() {
            static EventGroup t{"tracker"};
            return t;
        }

        static EventGroup peer() {
            static EventGroup p{"peer"};
            return p;
        }
    };

    bool operator==(const EventGroup& lhs, const EventGroup& rhs);

    struct EventGroupHasher {
        std::size_t operator()(const EventGroup& eg) const {
            return std::hash<std::string>()(eg.name);
        }
    };

    struct Event{
        Event() = default;
        virtual ~Event() = default;

        EventGroup group{""};
        std::string name;
    };

    bool operator==(const Event& lhs, const Event& rhs);


    struct Subscriber {
        virtual void processEvent(std::shared_ptr<Event>) = 0;
        virtual ~Subscriber() = default;
    };

    using SubscriberPtr = std::shared_ptr<Subscriber>;

    class Dispatcher {
    public:
        void subscribe(SubscriberPtr, const EventGroup& eg);
        void publish(const std::shared_ptr<Event>&);

        static std::shared_ptr<Dispatcher> get() {
            static auto dispatcher = std::shared_ptr<Dispatcher>(new Dispatcher);
            return dispatcher;
        }
    private:
        Dispatcher(){}

        std::unordered_map<EventGroup, std::vector<SubscriberPtr>, EventGroupHasher> subscriber_groups;
    };

}; // events
}; // ftorrent

#endif
