#include <array>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <memory>
#include <vector>

#include "service/manager.h"
#include "service/peer/peer_connection.h"
#include "service/peer/messages.h"
#include "service/peer/errors.h"
#include "service/metainfo.h"
#include "service/types.h"
#include "service/serialization.h"
#include "service/util.h"

int main(int argc, char* argv[]) {
    ftorrent::Manager manager{"./test-file.torrent", "demo-out.png", 16};
    manager.run();
}
