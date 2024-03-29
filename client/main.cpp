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

int main(int argc, char* argv[]) {
    if(argc < 3) {
        std::cout << "Usage: " << argv[0] << " <path-to-torrent-file> <path-to-output-file>";
        return 1;
    }

    pid_t pid = getpid();
    std::cout << "started pid: " << pid << std::endl;
    ftorrent::Manager manager{argv[1], argv[2], 16};
    manager.run();
}
