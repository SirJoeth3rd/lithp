ODIR = ../obj

run: lithp
	./lithp_test

lithp: lithp.o parser.o tokenizer.o
	g++ -o lithp_test -I $(ODIR) $(ODIR)/*.o

lithp.o: lithp.cpp
	g++ -c -g lithp.cpp -o $(ODIR)/lithp.o

parser.o: parser.cpp
	g++ -c -g parser.cpp -o $(ODIR)/parser.o

tokenizer.o: tokenizer.cpp
	g++ -c -g tokenizer.cpp -o $(ODIR)/tokenizer.o
