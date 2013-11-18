C := g++
LINKER := g++

C_FLAGS := -g -c -Wall -ansi -pedantic -std=c++0x 
L_FLAGS := -pthread

TARGETS := sender receiver test

define link
$(LINKER) $^ $(L_FLAGS) -o $@;
endef

SENDER_OBJS := sender.o udp_socket.o sender_state_machine.o
RECEIVER_OBJS := receiver.o udp_socket.o receiver_state_machine.o
TEST_OBJS := test.o

#.PHONY: all
all: $(TARGETS)

sender: $(SENDER_OBJS)
	$(call link)

receiver: $(RECEIVER_OBJS)
	$(call link)

test: $(TEST_OBJS)
	$(call link)

%.o: %.cpp
	$(CC) $(C_FLAGS) $^ -o $@

.PHONY: clean
clean:
	rm *.o
	rm $(TARGETS)
