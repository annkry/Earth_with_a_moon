default: main

%: %.cpp
	g++ -I. $< -o $@ shader.cpp texture.cpp objloader.cpp -lGLEW -lGL -lglfw
clean:
	rm a.out *.o *~ main
