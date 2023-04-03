#include "digestpp.hpp"
#include <array>
#include <iostream>

#include "service/manager.h"

int main(int argc, char* argv[]) {
    ftorrent::Manager manager{"./test-file.torrent", "demo-out.png", 16};
    manager.run();
}
