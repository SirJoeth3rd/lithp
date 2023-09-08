ODIR = ../obj

run: engine
	./engine_test

engine: engine.o parser.o tokenizer.o
	g++ -o engine_test -I $(ODIR) $(ODIR)/*.o

engine.o: engine.cpp
	g++ -c -g engine.cpp -o $(ODIR)/engine.o

parser.o: parser.cpp
	g++ -c -g parser.cpp -o $(ODIR)/parser.o

tokenizer.o: tokenizer.cpp
	g++ -c -g tokenizer.cpp -o $(ODIR)/tokenizer.o
