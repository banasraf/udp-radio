SOURCES = audio-transmission.cc udp.cc threading.cc control-protocol.cc
SENDER_SOURCES = sender.cc sender-main.cc
RECEIVER_SOURCES = receiver-main.cc radio-menu.cc tcp-socket.cc system-stream.cc telnet-processor.cc menu.cc terminal.cc text-screen.cc menu-drawer.cc player.cc

all: sikradio-sender sikradio-receiver

sikradio-sender: ${SOURCES} ${SENDER_SOURCES}
	g++ -std=c++17 -pthread -Wall -Wextra -pedantic -O2 ${SOURCES} ${SENDER_SOURCES} -o sikradio-sender

sikradio-receiver: ${SOURCES} ${RECEIVER_SOURCES}
	g++ -std=c++17 -pthread -Wall -Wextra -pedantic -O2 ${SOURCES} ${RECEIVER_SOURCES} -o sikradio-receiver 

clean:
	rm -f *.o sikradio-sender sikradio-receiver
