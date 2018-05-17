#pragma once
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;

class keyframe
{
public:
	quat quaternion;
	vec3 translation;
	long long timestamp_ms;
};

class bone_animation {
public:
	vector<keyframe> keyframes;
	string name, bone;
};

class animation {
public:
	vector<bone_animation> bone_animations;
	string name;
	long long duration;
	int keyframe_count;
};

class all_animations {
public:
	vector<animation> animations;
};

class bone
{
public:
	vector<bone_animation> animations;
	vector<keyframe> keyframes;
	mat4 *array_element = NULL;
	void set_matrix(int time) {
		if (array_element != NULL)
			*array_element = mat4(1.f);
	}
	string name;
	vec3 pos;
	quat q;
	bone *parent = NULL;
	vector<bone*> kids;
	int index;
	mat4 *mat = NULL;
	void write_to_VBOs(vec3 origin,vector<vec3> &vpos, vector<unsigned int> &imat)
	{
		vpos.push_back(origin);
		vec3 endp = origin + pos;
		vpos.push_back(endp);
		for (unsigned int i = 0; i < kids.size(); i++)
			kids[i]->write_to_VBOs(endp, vpos, imat);

	}

};
int readtobone(bone **root, all_animations *animations);