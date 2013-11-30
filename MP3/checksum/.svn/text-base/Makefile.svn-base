C := g++
LINKER := g++

C_FLAGS := -g -c -std=c++0x -I'../'
L_FLAGS := -pthread

TARGETS := checksum

define link
$(LINKER) $^ $(L_FLAGS) -o $@;
endef

SUM_OBJS := checksum.o

all: $(TARGETS)

checksum: $(SUM_OBJS)
	$(call link)

%.o: %.cpp
	$(CC) $(C_FLAGS) $^ -o $@

.PHONY: clean
clean:
	rm *.o
	rm $(TARGETS)
