#include <array>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <memory>
#include <vector>
#include <unistd.h>
#include <sys/types.h>

#include "service/manager.h"
#include "service/peer/peer_connection.h"
#include "service/peer/messages.h"
#include "service/peer/errors.h"
#include "service/metainfo.h"
#include "service/types.h"
#include "service/serialization.h"
#include "service/util.h"

int main(int argc, char* argv[]) {
    pid_t pid = getpid();
    std::cout << "started pid: " << pid << std::endl;
    ftorrent::Manager manager{argv[1], "demo-out.png", 16};
    manager.run();
}
