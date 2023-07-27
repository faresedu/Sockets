UTIL = ./src/util.cpp ./src/mkSocket.cpp ./src/server.cpp ./src/client.cpp ./src/application.cpp ./src/channel.cpp
MAIN1 = ./app/chat.cpp
MAIN2 = ./app/chatServer.cpp
BINARY1 = ./bin/client
BINARY2 = ./bin/server

all:
	@g++ -Wall -Wno-unused-result -Werror -g $(UTIL) $(MAIN1) -o $(BINARY1) -lm -pthread
	@g++ -Wall -Wno-unused-result -Werror -g $(UTIL) $(MAIN2) -o $(BINARY2) -lm -pthread

client:
	@$(BINARY1)

server:
	@$(BINARY2)

valgrind-client:
	@valgrind -s --tool=memcheck --leak-check=full  --track-origins=yes --show-leak-kinds=all --show-reachable=yes $(BINARY1)

valgrind-server:
	@valgrind -s --tool=memcheck --leak-check=full  --track-origins=yes --show-leak-kinds=all --show-reachable=yes $(BINARY2)
