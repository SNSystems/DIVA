

all: dicheck

dicheck: dicheck.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) dicheck.cc -o dicheck
	ln -s dicheck trimtrailing
clean:
	-rm -f dicheck
	-rm -f trimtrailing

test:
	-./dicheck testcase
	-./dicheck dicheck.cc
