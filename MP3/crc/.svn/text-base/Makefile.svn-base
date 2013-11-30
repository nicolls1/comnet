C := g++
LINKER := g++

C_FLAGS := -g -c -std=c++0x
L_FLAGS := -pthread

TARGETS := encoder

define link
$(LINKER) $^ $(L_FLAGS) -o $@;
endef

SUM_OBJS := crc.o

all: $(TARGETS)

encoder: $(SUM_OBJS)
	$(call link)

%.o: %.cpp
	$(CC) $(C_FLAGS) $^ -o $@

.PHONY: clean
clean:
	rm *.o
	rm $(TARGETS)
