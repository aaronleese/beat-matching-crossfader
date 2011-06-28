%: %.cpp
	g++ -O2 $< -o $@

%.noopt: %.cpp
	g++ $< -o $@

TARGETS=parallel parallel.noopt locality locality.noopt

all: $(TARGETS)

clean:
	rm $(TARGETS)