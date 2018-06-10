SOURCES = control-protocol.h control-protocol.cc errors.h audio-transmission.cc audio-transmission.h udp.cc udp.h threading.cc threading.h
SENDER_SOURCES = sender.h sender.cc sender_main.cc 
RECEIVER_SOURCES = receiver_main.cc radio-menu.cc radio-menu.h tcp-socket.cc tcp-socket.h byte-stream.h system-stream.cc system-stream.h telnet-processor.cc telnet-processor.h menu.cc menu.h terminal.cc terminal.h text-screen.cc text-screen.h menu-drawer.cc menu-drawer.h player.cc player.h

all: sikradio-sender sikradio-receiver

sikradio-sender: ${SOURCES} ${SENDER_SOURCES}
	g++ -std=c++17 -pthread -Wall -Wextra -pedantic -O2 ${SOURCES} ${SENDER_SOURCES} -o sikradio-sender

sikradio-receiver: ${SOURCES} ${RECEIVER_SOURCES}
	g++ -std=c++17 -pthread -Wall -Wextra -pedantic -O2 ${SOURCES} ${RECEIVER_SOURCES} -o sikradio-receiver 

clean:
	rm -f *.o sikradio-sender sikradio-receiver
