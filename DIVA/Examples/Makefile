CPPFLAGS = -g -w

OBJECTS = helloworld.o example_01.o example_02.o example_03.o example_04.o example_05.o example_06.o example_07.o example_08.o example_09.o example_11.o example_12.o example_13.o example_14.o example_15.o
MULTIPLES = multiple.cpp multiple_a.cpp multiple_b.cpp multiple_c.cpp
EX16_SOURCES = example_16.cpp example_16_local.cpp example_16_global.cpp

$(OBJECTS): %.o : %.cpp
example_10.elf: example_10_lib.cpp example_10_main.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@
example_16.elf: $(EX16_SOURCES)
	$(CXX) $(CPPFLAGS) $^ -o $@
example_16_lto.elf: $(EX16_SOURCES)
	$(CXX) $(CPPFLAGS) $^ -o $@ -flto -O0
scopes_org.o: scopes.cpp
	$(CXX) $(CPPFLAGS) -c $^ -o $@ -O0 -D_A
scopes_mod.o: scopes.cpp
	$(CXX) $(CPPFLAGS) -c $^ -o $@ -O0 -U_A
good.o: $(MULTIPLES)
	$(CXX) $(CPPFLAGS) -c multiple.cpp -o $@ -O0 -D_A -D_B -D_C
broken.o: $(MULTIPLES)
	$(CXX) $(CPPFLAGS) -c multiple.cpp -o $@ -O0 -D_A      -D_C
fixed.o: $(MULTIPLES)
	$(CXX) $(CPPFLAGS) -c multiple.cpp -o $@ -O0 -D_A -D_B -D_C

all: $(OBJECTS) example_10.elf example_16.elf scopes_org.o scopes_mod.o good.o broken.o fixed.o
lto: example_16_lto.elf
clean:
	-rm -f *.o *.elf
