#include <iostream>
#include <future>
#include <queue>
#include <unistd.h>
#include "threading.h"
#include "radio-menu.h"

using namespace std;

int main() {

    auto menu_server_handle = async([](){ menuServer(10001); });
    eventLoop();
    return 0;
}