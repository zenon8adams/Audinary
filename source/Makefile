INCLUDE_DIR  = include
IMODEL_DIR 	 = $(INCLUDE_DIR)/model
ISERVER_DIR  = $(INCLUDE_DIR)/server
IENCODER_DIR = $(INCLUDE_DIR)/encoder
MODEL_DIR 	 = source/model
SERVER_DIR 	 = source/server
ENCODER_DIR  = source/encoder
OBJS 		 = textual-time.o numbers-to-words.o http-server.o router.o huffman-coding.o json-builder.o

source/intermediate: $(OBJS)

textual-time.o: $(MODEL_DIR)/textual-time.cpp $(IMODEL_DIR)/textual-time.hpp $(IMODEL_DIR)/beeper.hpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -I$(INCLUDE_DIR) -c $(MODEL_DIR)/textual-time.cpp

numbers-to-words.o: $(MODEL_DIR)/numbers-to-words.cpp $(IMODEL_DIR)/numbers-to-words.hpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -I$(INCLUDE_DIR) -c $(MODEL_DIR)/numbers-to-words.cpp

http-server.o: $(SERVER_DIR)/http-server.cpp $(ISERVER_DIR)/http-server.hpp $(ISERVER_DIR)/router.hpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -I$(INCLUDE_DIR) -c $(SERVER_DIR)/http-server.cpp

router.o: $(SERVER_DIR)/router.cpp $(ISERVER_DIR)/router.hpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -I$(INCLUDE_DIR) -c $(SERVER_DIR)/router.cpp

huffman-coding.o: $(ENCODER_DIR)/huffman-coding.cpp $(IENCODER_DIR)/huffman-coding.hpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -I$(INCLUDE_DIR) -c $(ENCODER_DIR)/huffman-coding.cpp

json-builder.o: $(ENCODER_DIR)/json-builder.cpp $(IENCODER_DIR)/json-builder.hpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -I$(INCLUDE_DIR) -c $(ENCODER_DIR)/json-builder.cpp

clean:
	rm -f $(OBJS) $(PACKAGE)

.PHONY: all clean