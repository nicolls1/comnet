C := g++
LINKER := g++

C_FLAGS := -g -c -std=c++0x -I'../'
L_FLAGS := -pthread

TARGETS := server client

define link
$(LINKER) $^ $(L_FLAGS) -o $@;
endef

#vpath %.cpp ../tools ./

SERVER_OBJS := server.o network.o 
CLIENT_OBJS := client.o network.o

#.PHONY: all
all: $(TARGETS)

server: $(SERVER_OBJS)
	$(call link)

client: $(CLIENT_OBJS)
	$(call link)

%.o: %.cpp
	$(CC) $(C_FLAGS) $^ -o $@

.PHONY: clean
clean:
	rm *.o
	rm $(TARGETS)
