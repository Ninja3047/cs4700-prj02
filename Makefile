CC := gcc
INCDIRS := -I.
CFLAGS := -g -std=gnu99 -Wall -Wextra

all: sender receiver

sender: sender.o network.o
	$(CC) $(CFLAGS) $^ -o $@

receiver: receiver.o network.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCDIRS) -c $< -o $@

%.d: %.c
	$(CC) $(INCDIRS) -MM $< > $@

-include $(DEPFILES)

.PHONY: clean
clean:
	rm -f receiver.o sender.o network.o receiver sender
