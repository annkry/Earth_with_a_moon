//the source file from: http://www.opengl-tutorial.org/
#ifndef OBJLOADER_H
#define OBJLOADER_H

bool loadOBJ(
	const char * path, 
	GLfloat out_vertices[], 
	GLfloat out_uvs[], 
	GLfloat out_normals[]
);



bool loadAssImp(
	const char * path, 
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals
);

#endif