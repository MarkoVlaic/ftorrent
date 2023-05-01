#include <boost/endian/conversion.hpp>
#include <array>
#include <iostream> // TODO: remove

#include "service/peer/messages.h"
#include "service/serialization.h"

namespace ftorrent {
namespace peer {
namespace messages {
    using namespace ftorrent::serialization;

    void Message::serialize(ftorrent::serialization::Serializer& serializer) {
        serialization::serialize(boost::endian::native_to_big(len), serializer);
    }

    Handshake::Handshake(const std::string& p, const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid):
    Message{49 + static_cast<uint32_t>(p.size()), EMessageId::HANDSHAKE},
    pstr{p}, info_hash{ih}, peer_id{pid}
    {}

    void Handshake::serialize(ftorrent::serialization::Serializer& serializer) {
        uint8_t pstrlen = static_cast<uint8_t>(pstr.size());
        std::array<uint8_t, 8> reserved;
        reserved.fill(0);

        serialization::serialize(pstrlen, serializer);
        serialization::serialize(pstr, serializer);
        serialization::serialize(reserved, serializer);
        serialization::serialize(info_hash, serializer);
        serialization::serialize(peer_id, serializer);
    }

    void IdMessage::serialize(serialization::Serializer& serializer) {
        Message::serialize(serializer);
        serialization::serialize(id, serializer);
    }

    void Have::serialize(serialization::Serializer& serializer) {
        IdMessage::serialize(serializer);
        serialization::serialize(boost::endian::native_to_big(index), serializer);
    }

    void BitField::serialize(serialization::Serializer& serializer) {
        IdMessage::serialize(serializer);
        serialization::serialize(bitfield, serializer);
    }

    void Request::serialize(ftorrent::serialization::Serializer& serializer) {
        IdMessage::serialize(serializer);
        serialization::serialize(boost::endian::native_to_big(index), serializer);
        serialization::serialize(boost::endian::native_to_big(begin), serializer);
        serialization::serialize(boost::endian::native_to_big(length), serializer);
    }

    void Piece::serialize(ftorrent::serialization::Serializer& serializer) {
        IdMessage::serialize(serializer);
        serialization::serialize(boost::endian::native_to_big(index), serializer);
        serialization::serialize(boost::endian::native_to_big(begin), serializer);
        serialization::serialize(block, serializer);
    }

    void Cancel::serialize(ftorrent::serialization::Serializer& serializer) {
        IdMessage::serialize(serializer);
        serialization::serialize(boost::endian::native_to_big(index), serializer);
        serialization::serialize(boost::endian::native_to_big(begin), serializer);
        serialization::serialize(boost::endian::native_to_big(length), serializer);
    }

}; // messages

}; // peer

namespace serialization {
    template<>
    void deserialize(peer::messages::Message& msg, Deserializer& deserializer) {
        deserialize(msg.len, deserializer);
        msg.len = boost::endian::big_to_native(msg.len);
    }

    template<>
    void deserialize(peer::messages::Handshake& msg, Deserializer& deserializer) {
        uint8_t pstrlen;
        deserialize(pstrlen, deserializer);

        std::vector<uint8_t> pstr_bytes;
        pstr_bytes.resize(pstrlen);
        deserialize(pstr_bytes, deserializer);
        msg.pstr = std::string(pstr_bytes.begin(), pstr_bytes.end());

        std::array<uint8_t, 8> reserved;
        deserialize(reserved, deserializer);

        deserialize(msg.info_hash, deserializer);
        deserialize(msg.peer_id, deserializer);
    }

    template<>
    void deserialize(peer::messages::IdMessage& msg, Deserializer& deserializer) {
        deserialize(static_cast<peer::messages::Message&>(msg), deserializer);
        deserialize(msg.id, deserializer);
    }

    template<>
    void deserialize(peer::messages::Have& msg, Deserializer& deserializer) {
        std::cerr << "deser have\n";
        deserialize(static_cast<peer::messages::IdMessage&>(msg), deserializer);

        deserialize(msg.index, deserializer);
        msg.index = boost::endian::big_to_native(msg.index);
    }

    template<>
    void deserialize(peer::messages::BitField& msg, Deserializer& deserializer) {
        deserialize(static_cast<peer::messages::IdMessage&>(msg), deserializer);

        msg.bitfield.resize((msg.len - 1) * 8);
        deserialize(msg.bitfield, deserializer);
    }

    template<>
    void deserialize(peer::messages::Request& msg, Deserializer& deserializer) {
        deserialize(static_cast<peer::messages::IdMessage&>(msg), deserializer);

        deserialize(msg.index, deserializer);
        deserialize(msg.begin, deserializer);
        deserialize(msg.length, deserializer);

        msg.index = boost::endian::native_to_big(msg.index);
        msg.begin = boost::endian::native_to_big(msg.begin);
        msg.length = boost::endian::native_to_big(msg.length);
    }

    template<>
    void deserialize(peer::messages::Piece& msg, Deserializer& deserializer) {
        deserialize(static_cast<peer::messages::IdMessage&>(msg), deserializer);

        deserialize(msg.index, deserializer);
        deserialize(msg.begin, deserializer);

        msg.block.resize(msg.len - 9);
        deserialize(msg.block, deserializer);

        msg.index = boost::endian::native_to_big(msg.index);
        msg.begin = boost::endian::native_to_big(msg.begin);

    }

    template<>
    void deserialize(peer::messages::Cancel& msg, Deserializer& deserializer) {
        deserialize(static_cast<peer::messages::IdMessage&>(msg), deserializer);

        deserialize(msg.index, deserializer);
        deserialize(msg.begin, deserializer);
        deserialize(msg.length, deserializer);

        msg.index = boost::endian::native_to_big(msg.index);
        msg.begin = boost::endian::native_to_big(msg.begin);
        msg.length = boost::endian::native_to_big(msg.length);
    }

}; // serialization
}; // ftorrent
