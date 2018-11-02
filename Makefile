CC=clang++
STD=-std=c++14
FRAMEWORKS=-framework ApplicationServices

OUT=screen_diff.out
IMAGE=Image2.jpg

ifndef DELAY
	DELAY=10
endif

all:
	$(CC) $(STD) -Wall main.cpp $(FRAMEWORKS) -o $(OUT)

run: all
	./$(OUT) -out $(IMAGE) -delay $(DELAY)
	open $(IMAGE)

clean:
	rm $(IMAGE)
	rm $(OUT)
