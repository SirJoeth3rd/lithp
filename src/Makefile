ODIR = ../obj

run: lithp
	./lithp_test

lithp_terminal: lithp_terminal.o lithp.o parser.o tokenizer.o lithp_std_lib.o
	g++ -o lithp_terminal -I $(ODIR) $(ODIR)/*.o

lithp: lithp.o parser.o tokenizer.o
	g++ -o lithp_test -I $(ODIR) $(ODIR)/*.o

lithp_terminal.o: lithp_std_lib.cpp
	g++ -c -g lithp_terminal.cpp -o $(ODIR)/lithp_terminal.o

lithp_std_lib.o: lithp_std_lib.cpp
	g++ -c -g lithp_std_lib.cpp -o $(ODIR)/lithp_std_lib.o

lithp.o: lithp.cpp
	g++ -c -g lithp.cpp -o $(ODIR)/lithp.o

parser.o: parser.cpp
	g++ -c -g parser.cpp -o $(ODIR)/parser.o

tokenizer.o: tokenizer.cpp
	g++ -c -g tokenizer.cpp -o $(ODIR)/tokenizer.o
