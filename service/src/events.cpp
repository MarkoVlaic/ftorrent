#include <iostream>
#include "service/events/events.h"


namespace ftorrent {
namespace events {

    bool operator==(const EventGroup& lhs, const EventGroup& rhs) {
        return lhs.name == rhs.name;
    }

    bool operator==(const Event& lhs, const Event& rhs) {
        return lhs.group == rhs.group && lhs.name == rhs.name;
    }

    void Dispatcher::subscribe(SubscriberPtr s, const EventGroup& eg) {
        subscriber_groups[eg].push_back(s);
    }

    void Dispatcher::publish(const std::shared_ptr<Event>& e) {
        for(const SubscriberPtr& s : subscriber_groups[e->group]) {
            s->processEvent(e);
        }
    }

};
};
