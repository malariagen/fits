CC=g++
MYSQL_CONFIG = /software/mysql-5.7.13/bin/mysql_config
#EMBLIBS = ${shell $(MYSQL_CONFIG) --libmysqld-libs}
LIBS=${shell $(MYSQL_CONFIG) --libs}
INC=${shell $(MYSQL_CONFIG) --cflags}
FLAGS=-std=c++11

OBJECTS=fits.o database.o condition_parser.o command_list.o database_abstraction_layer.o update_sanger.o tools.o

all: fits

fits: $(OBJECTS)
	$(CC) -o fits $(OBJECTS) $(FLAGS) $(LIBS) $(INC) $(CFLAGS)

%.o: src/%.cpp
	$(CC) -c $(FLAGS) $(INC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o fits
