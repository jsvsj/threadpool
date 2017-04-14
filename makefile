CC=gcc
CFLAG=-Wall -g
BIN=main
OBJE=threadpool.o condition.o main.o
LIBS=-pthread -lrt
$(BIN):$(OBJE)
	$(CC) $(CFLAG) $^ -o $@ $(LIBS)

%.o:%.c
	$(CC) $(CFALGS) -c $< -o $@

.PHNOY:clean
clean:
	rm $(BIN) $(OBJE) -f
