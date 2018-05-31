#pragma once
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
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

	void play_animation(int keyframenumber, int current_animation, int next_animation) {
		int curr_frames = animation[current_animation]->frames;
		int next_frames = animation[next_animation]->frames;

		quat q;
		vec3 tr;

		if (current_animation == next_animation) {
			q = animation[current_animation]->keyframes[keyframenumber].quaternion;
			tr = animation[current_animation]->keyframes[keyframenumber].translation;
		}
		else {

			float fframes = keyframenumber * ((float)curr_frames / (float)next_frames);
		
			// First get the mix'ins for the current animation
			quat curr_q1 = animation[current_animation]->keyframes[(int)fframes].quaternion;
			vec3 curr_tr1 = animation[current_animation]->keyframes[(int)fframes].translation;

			quat curr_q2 = animation[current_animation]->keyframes[(int)fframes + 1].quaternion;
			vec3 curr_tr2 = animation[current_animation]->keyframes[(int)fframes + 1].translation;

			float t = fframes - (int)fframes;
			quat curr_q = slerp(curr_q1, curr_q2, t);
			vec3 curr_tr = mix(curr_tr1, curr_tr2, t);


			// Now get the q and tr for the next animation
			quat next_q = animation[next_animation]->keyframes[keyframenumber].quaternion;
			vec3 next_tr = animation[next_animation]->keyframes[keyframenumber].translation;

			t = (float)keyframenumber / (float)next_frames;
			q = slerp(curr_q, next_q, t);
			tr = mix(curr_tr, next_tr, t);
		}
		
		if ((float)keyframenumber / (float)(next_frames - 1) >= 0.8f) {
			// We're in the last 10th of our animation - let's interpolate the rest to the beginning
			float t = (float)(keyframenumber - (int)((next_frames - 1) * 0.8f)) / (float)((next_frames - 1)- (int)((next_frames - 1) * 0.8f));

			quat q2 = animation[next_animation]->keyframes[0].quaternion;
			vec3 tr2 = animation[next_animation]->keyframes[0].translation;

			q = slerp(q, q2, t);
			tr = mix(tr, tr2, t);
		} 

		if (name == "Humanoid:Hips") {
			tr = vec3(0, 0, 0);
		}

		mat4 M = mat4(q);
		mat4 T = translate(mat4(1), tr);
		M = T * M;

		if (mat) {
			mat4 parent_mat = mat4(1);

			if (parent)
				parent_mat = *parent->mat;

			*mat = parent_mat * M;
		}

		for (int i = 0; i < kids.size(); i++)
			kids[i]->play_animation(keyframenumber, current_animation, next_animation);
	}

	void play_slowmo_animation1(int keyframenumber, int current_animation, int next_animation) {
		int curr_frames = animation[current_animation]->frames;
		int next_frames = animation[next_animation]->frames;

		int base_frame = 3 * (keyframenumber / 12);
		float slowmo = (float)(keyframenumber - ((keyframenumber / 12) * 12)) / 12.f;

		quat q;
		vec3 tr;

		if (current_animation == next_animation) {
			if (base_frame + 3 > curr_frames - 1) {
				quat q1 = animation[current_animation]->keyframes[base_frame].quaternion;
				vec3 tr1 = animation[current_animation]->keyframes[base_frame].translation;

				quat q2 = animation[current_animation]->keyframes[next_frames - 1].quaternion;
				vec3 tr2 = animation[current_animation]->keyframes[next_frames - 1].translation;

				q = slerp(q1, q2, slowmo * (3.f/((next_frames - 1) - base_frame)));
				tr = mix(tr1, tr2, slowmo * (3.f/((next_frames - 1) - base_frame)));
			}
			else {
				quat q1 = animation[current_animation]->keyframes[base_frame].quaternion;
				vec3 tr1 = animation[current_animation]->keyframes[base_frame].translation;

				quat q2 = animation[current_animation]->keyframes[base_frame + 1].quaternion;

				quat q3 = animation[current_animation]->keyframes[base_frame + 2].quaternion;

				quat q4 = animation[current_animation]->keyframes[base_frame + 3].quaternion;
				vec3 tr2 = animation[current_animation]->keyframes[base_frame + 3].translation;

				q = squad(q1, q4, q2, q3, slowmo);

				tr = mix(tr1, tr2, slowmo);
			}
		} 
		else {

			float fframes = keyframenumber * ((float)curr_frames / (float)next_frames);

			int curr_base_frame = 3 * ((int)fframes / 12);
			float curr_slowmo = ((float)((int)fframes - (((int)fframes / 12) * 12)) / 12.f) * ((float)curr_frames / (float)next_frames);
		
			// First get the mix'ins for the current animation
			quat curr_q;
			vec3 curr_tr;

			if (curr_base_frame + 3 > next_frames - 1) {
				quat curr_q1 = animation[current_animation]->keyframes[curr_base_frame].quaternion;
				vec3 curr_tr1 = animation[current_animation]->keyframes[curr_base_frame].translation;

				quat curr_q2 = animation[current_animation]->keyframes[next_frames - 1].quaternion;
				vec3 curr_tr2 = animation[current_animation]->keyframes[next_frames - 1].translation;

				curr_q = slerp(curr_q1, curr_q2, curr_slowmo * (3.f/((next_frames - 1) - curr_base_frame)));
				curr_tr = mix(curr_tr1, curr_tr2, curr_slowmo * (3.f/((next_frames - 1) - curr_base_frame)));
			}
			else {
				quat curr_q1 = animation[current_animation]->keyframes[curr_base_frame].quaternion;
				vec3 curr_tr1 = animation[current_animation]->keyframes[curr_base_frame].translation;

				quat curr_q2 = animation[current_animation]->keyframes[curr_base_frame + 1].quaternion;

				quat curr_q3 = animation[current_animation]->keyframes[curr_base_frame + 2].quaternion;

				quat curr_q4 = animation[current_animation]->keyframes[curr_base_frame + 3].quaternion;
				vec3 curr_tr2 = animation[current_animation]->keyframes[curr_base_frame + 3].translation;

				
				curr_q = squad(curr_q1, curr_q4, curr_q2, curr_q3, curr_slowmo);
				curr_tr = mix(curr_tr1, curr_tr2, curr_slowmo);
			}

			// Now get the q and tr for the next animation
			quat next_q;
			vec3 next_tr;

			if (base_frame + 3 > next_frames - 1) {
				quat next_q1 = animation[next_animation]->keyframes[base_frame].quaternion;
				vec3 next_tr1 = animation[next_animation]->keyframes[base_frame].translation;

				quat next_q2 = animation[next_animation]->keyframes[next_frames - 1].quaternion;
				vec3 next_tr2 = animation[next_animation]->keyframes[next_frames - 1].translation;

				next_q = slerp(next_q1, next_q2, slowmo * (3.f/((next_frames - 1) - base_frame)));
				next_tr = mix(next_tr1, next_tr2, slowmo * (3.f/((next_frames - 1) - base_frame)));
			}
			else {
				quat next_q1 = animation[next_animation]->keyframes[base_frame].quaternion;
				vec3 next_tr1 = animation[next_animation]->keyframes[base_frame].translation;

				quat next_q2 = animation[next_animation]->keyframes[base_frame + 1].quaternion;

				quat next_q3 = animation[next_animation]->keyframes[base_frame + 2].quaternion;

				quat next_q4 = animation[next_animation]->keyframes[base_frame + 3].quaternion;
				vec3 next_tr2 = animation[next_animation]->keyframes[base_frame + 3].translation;

				
				next_q = squad(next_q1, next_q4, next_q2, next_q3, slowmo);
				next_tr = mix(next_tr1, next_tr2, slowmo);
			}

			float t = (float)keyframenumber / ((float)next_frames * 4);
			q = slerp(curr_q, next_q, t);
			tr = mix(curr_tr, next_tr, t);
		}
		
		/*if ((float)keyframenumber / (float)(next_frames - 1) >= 0.8f) {
			// We're in the last 10th of our animation - let's interpolate the rest to the beginning
			float t = (float)(keyframenumber - (int)((next_frames - 1) * 0.8f)) / (float)((next_frames - 1)- (int)((next_frames - 1) * 0.8f));

			quat q2 = animation[next_animation]->keyframes[0].quaternion;
			vec3 tr2 = animation[next_animation]->keyframes[0].translation;

			q = slerp(q, q2, t);
			tr = mix(tr, tr2, t);
		}  */

		if (name == "Humanoid:Hips") {
			tr = vec3(0, 0, 0);
		}

		mat4 M = mat4(q);
		mat4 T = translate(mat4(1), tr);
		M = T * M;

		if (mat) {
			mat4 parent_mat = mat4(1);

			if (parent)
				parent_mat = *parent->mat;

			*mat = parent_mat * M;
		}

		for (int i = 0; i < kids.size(); i++)
			kids[i]->play_slowmo_animation(keyframenumber, current_animation, next_animation);

	}

	void play_slowmo_animation(int keyframenumber, int current_animation, int next_animation) {
		int curr_frames = animation[current_animation]->frames;
		int next_frames = animation[next_animation]->frames;

		int base_frame = keyframenumber / 10;
		float slowmo = float(keyframenumber - (base_frame * 10)) / 10.f;

		quat q;
		vec3 tr;

		if (current_animation == next_animation) {
			quat q1 = animation[current_animation]->keyframes[base_frame].quaternion;
			quat q2 = animation[current_animation]->keyframes[base_frame + 1].quaternion;

			vec3 tr1 = animation[current_animation]->keyframes[base_frame].translation;
			vec3 tr2 = animation[current_animation]->keyframes[base_frame + 1].translation;

			q = slerp(q1, q2, slowmo);
			tr = mix(tr1, tr2, slowmo);
		}
		else {
			float fframes = keyframenumber * ((float)(curr_frames - 1) / (float)next_frames);
			int curr_base_frame = fframes / 10;
			float curr_slowmo = float(fframes - (curr_base_frame * 10)) / 10.f;

			quat curr_q1 = animation[current_animation]->keyframes[curr_base_frame].quaternion;
			quat curr_q2 = animation[current_animation]->keyframes[curr_base_frame + 1].quaternion;

			vec3 curr_tr1 = animation[current_animation]->keyframes[curr_base_frame].translation;
			vec3 curr_tr2 = animation[current_animation]->keyframes[curr_base_frame + 1].translation;

			quat curr_q = slerp(curr_q1, curr_q2, curr_slowmo);
			vec3 curr_tr = mix(curr_tr1, curr_tr2, curr_slowmo);


			quat next_q1 = animation[next_animation]->keyframes[base_frame].quaternion;
			quat next_q2 = animation[next_animation]->keyframes[base_frame + 1].quaternion;

			vec3 next_tr1 = animation[next_animation]->keyframes[base_frame].translation;
			vec3 next_tr2 = animation[next_animation]->keyframes[base_frame + 1].translation;

			quat next_q = slerp(next_q1, next_q2, slowmo);
			vec3 next_tr = mix(next_tr1, next_tr2, slowmo);

			float t = (float)keyframenumber / (float)((next_frames - 1)  * 10);

			q = slerp(curr_q, next_q, t);
			tr = mix(curr_tr, next_tr, t);
		}

		if ((float)keyframenumber / (float)((next_frames - 1) * 10) >= 0.8f) {
			// We're in the last 10th of our animation - let's interpolate the rest to the beginning
			float t = (float)(keyframenumber - (int)((next_frames - 1) * 8.f)) / (float)((next_frames - 1) * 10.f - (int)((next_frames - 1) * 8.f));

			quat q2 = animation[next_animation]->keyframes[0].quaternion;
			vec3 tr2 = animation[next_animation]->keyframes[0].translation;

			q = slerp(q, q2, t);
			tr = mix(tr, tr2, t);
		} 

		

		if (name == "Humanoid:Hips") {
			tr = vec3(0, 0, 0);
		}

		mat4 M = mat4(q);
		mat4 T = translate(mat4(1), tr);
		M = T * M;

		if (mat) {
			mat4 parent_mat = mat4(1);

			if (parent)
				parent_mat = *parent->mat;

			*mat = parent_mat * M;
		}

		for (int i = 0; i < kids.size(); i++)
			kids[i]->play_slowmo_animation(keyframenumber, current_animation, next_animation);

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

	int getKeyFrameCount(int a) {
    	return animation[a]->frames;
	}

	long long getDuration(int a) {
    	return animation[a]->duration;
	}

};
int readtobone(std::string s, bone **root, all_animations *animations);