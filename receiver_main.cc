#include <iostream>
#include <future>
#include <queue>
#include <unistd.h>
#include "threading.h"
#include "radio-menu.h"

using namespace std;

int main() {

    TcpListener listener(10001);
    MultiWriter output;
    bool running = true;
    std::list<std::future<void>> tasks_handles;
    std::shared_ptr<TcpStream> new_tcp_stream;

    MenuDrawer menu_drawer(15);
    menu_drawer.drawAt(text_screen().lock().get(), 3, 1, radio_menu().lock().get());

    tasks_handles.push_back(std::async([&]() { eventLoop(output); } ));
    while (running) {
        new_tcp_stream = listener.acceptClient();
        tasks_handles.push_back(std::async([&]() {
            UserConnection connection(output, new_tcp_stream);
            auto stream = connection.getStream();
            auto writer = connection.getWriter();
            output.addStream(writer);
            stream->writeBytes(text_screen().lock().get().initialBytes());
            stream->writeBytes(text_screen().lock().get().renderToBytes());
            stream->flushOutput();
            serveUser(*stream);
            output.removeStream(writer);
        }));

    }
    return 0;
}