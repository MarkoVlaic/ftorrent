#ifndef FTORRENT_PEER_MESSAGE_H
#define FTORRENT_PEER_MESSAGE_H

#include <cstdint>
#include <string>
#include <vector>

#include "../serialization.h"
#include "../types.h"

namespace ftorrent {
namespace peer {
namespace messages {

    enum EMessageId {
        CHOKE = 0,
        UNCHOKE,
        INTERESTED,
        NOT_INTERESTED,
        HAVE,
        BITFIELD,
        REQUEST,
        PIECE,
        CANCEL,
        PORT, // not used
        HANDSHAKE, // internal
        KEEP_ALIVE // internal
    };

    typedef uint8_t MessageId;

    struct Message : public ftorrent::serialization::Serializable {
        Message(uint32_t l, MessageId i): len{l}, id{i} {}
        virtual ~Message() = default;

        virtual void serialize(ftorrent::serialization::Serializer&) override;
        uint32_t len;
        MessageId id;
    };

    struct KeepAlive : public Message {
        KeepAlive(): Message{0, EMessageId::KEEP_ALIVE} {}
    };

    struct Handshake : public Message {
        Handshake(): Message{0, EMessageId::HANDSHAKE} {};
        Handshake(const std::string& p, const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid);
        ~Handshake() = default;

        void serialize(ftorrent::serialization::Serializer&) override;

        std::string pstr;
        ftorrent::types::Hash info_hash;
        ftorrent::types::PeerId peer_id;
    };

    struct IdMessage : public Message {
        IdMessage(uint32_t len, MessageId i): Message{len, i} {};
        virtual ~IdMessage() = default;

        virtual void serialize(ftorrent::serialization::Serializer&) override;
    };

    struct Choke : public IdMessage {
        Choke(): IdMessage{1, EMessageId::CHOKE} {}
    };

    struct Unchoke : public IdMessage {
        Unchoke(): IdMessage{1, EMessageId::UNCHOKE} {}
    };

    struct Interested : public IdMessage {
        Interested(): IdMessage{1, EMessageId::INTERESTED} {}
    };

    struct NotInterested : public IdMessage {
        NotInterested(): IdMessage{1, EMessageId::NOT_INTERESTED} {}
    };

    struct Have : public IdMessage {
        Have(): Have{0} {} // Workaround
        Have(uint32_t i): IdMessage{5, EMessageId::HAVE}, index{i} {}
        void serialize(ftorrent::serialization::Serializer& ) override;

        uint32_t index;
    };

    struct BitField : public IdMessage {
        BitField(): BitField{std::vector<bool>{}} {} // Workaround
        BitField(const std::vector<bool>& bf):
            IdMessage{1 + static_cast<uint32_t>(bf.size() / 8 + (bf.size() % 8 > 0 ? 1 : 0)), EMessageId::BITFIELD},
            bitfield{bf}
        {}

        void serialize(ftorrent::serialization::Serializer& ) override;

        std::vector<bool> bitfield;
    };

    struct Request : public IdMessage {
        Request(): Request{0, 0, 0} {} // Workaround
        Request(uint32_t i, uint32_t b, uint32_t l):
            IdMessage{13, EMessageId::REQUEST},
            index{i}, begin{b}, length{l}
        {}

        void serialize(ftorrent::serialization::Serializer& ) override;

        uint32_t index;
        uint32_t begin;
        uint32_t length;
    };

    struct Piece : public IdMessage {
        Piece(): Piece{0, 0, std::vector<uint8_t>{}} {}
        Piece(uint32_t i, uint32_t b, const std::vector<uint8_t>& bl):
            IdMessage{9 + static_cast<uint32_t>(bl.size()), EMessageId::PIECE},
            index{i}, begin{b}, block(bl)
        {}

        void serialize(ftorrent::serialization::Serializer& ) override;

        uint32_t index;
        uint32_t begin;
        std::vector<uint8_t> block;
    };


    struct Cancel : public IdMessage {
        Cancel(): Cancel{0, 0, 0} {}
        Cancel(uint32_t i, uint32_t b, uint32_t l):
            IdMessage{13, EMessageId::CANCEL},
            index{i}, begin{b}, length{l}
        {}

        void serialize(ftorrent::serialization::Serializer& ) override;

        uint32_t index;
        uint32_t begin;
        uint32_t length;
    };

}; // messages
}; // peer
}; // ftorrent

#endif
