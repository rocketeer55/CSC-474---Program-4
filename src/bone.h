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

class animation_per_bone {
public:
	string name, bone;
	long long duration;
	int frames;
	vector<keyframe> keyframes;
};

class all_animations {
public:
	vector<animation_per_bone> animations;
};

class bone
{
public:
	vector<animation_per_bone*> animation;

	string name;
	vec3 pos;
	quat q;
	bone *parent = NULL;
	vector<bone*> kids;
	unsigned int index;
	mat4 *mat = NULL;

	void play_animation(int keyframenumber, string animationname) {
		for (unsigned int i = 0; i < animation.size(); i++) {
			if (animation[i]->name == animationname) {
				keyframenumber = keyframenumber % animation[i]->keyframes.size();
				quat q = animation[i]->keyframes[keyframenumber].quaternion;
				vec3 tr = animation[i]->keyframes[keyframenumber].translation;
				if (name == "Humanoid:Hips") {
					tr = vec3(0, 0, 0);
				}

				mat4 M = mat4(q);
				mat4 T = translate(mat4(1), tr);
				M = T * M;
				if (mat) {
					mat4 parentmat = mat4(1);

					if (parent)
						parentmat = *parent->mat;

					*mat = parentmat * M;
				}
			}
		}
		for (int i = 0; i < kids.size(); i++)
			kids[i]->play_animation(keyframenumber,animationname);
	}


	void write_to_VBOs(vec3 origin,vector<vec3> &vpos, vector<unsigned int> &imat)
	{
		if (parent) {
			imat.push_back(parent->index);
		}
		else {
			imat.push_back(index);
		}
		imat.push_back(index);

		vpos.push_back(origin);
		vec3 endp = origin + pos;
		vpos.push_back(endp);

		for (unsigned int i = 0; i < kids.size(); i++)
			kids[i]->write_to_VBOs(endp, vpos, imat);

	}

	void set_animations(all_animations *all_anim, mat4 *matrices, int &animsize) {
		for (unsigned int i = 0; i < all_anim->animations.size(); i++) {
			if (all_anim->animations[i].bone == name) {
				animation.push_back(&all_anim->animations[i]);
			}
		}

		mat = &matrices[index];
		animsize++;

		for (unsigned int i = 0; i < kids.size(); i++)
			kids[i]->set_animations(all_anim, matrices, animsize);
	}

};
int readtobone(std::string s, bone **root, all_animations *animations);