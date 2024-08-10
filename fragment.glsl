#version 330
in vec2 tex_coord;
uniform sampler2D in_texture;
uniform float time;

struct sphere
{
	vec3 pos;
	float radius_sqr;
};

struct light
{
	vec3 pos;
	vec3 color;
	float intensity;
};

struct hit_info
{
	vec3 intersection_point;
	vec3 normal;
	bool hit_target;
	int target_id;
};

#define M_PI 3.1415926535897932384626433832795
#define SPHERE_COUNT 2
#define LIGHT_COUNT 3

sphere sphere_array[SPHERE_COUNT];
vec3 color_array[SPHERE_COUNT + 1];
light light_array[LIGHT_COUNT];
float floor_y = -3.0f;

hit_info cast_ray(vec3 dir, float step_size, float max_dist, int collision_count)
{
	hit_info res = hit_info(vec3(0.0f), vec3(0.0f), false, -1);
	vec3 step_vec = dir * step_size;
	vec3 curr_pos = dir;
	float total_dist = 0.0f;
	while(total_dist < max_dist)
	{
		if(curr_pos.y < floor_y)
		{
			res = hit_info(curr_pos - step_vec, vec3(0.0f, 1.0f, 0.0f), true, SPHERE_COUNT);
			if(--collision_count == 0) { return res; }
			step_vec = reflect(curr_pos, vec3(0.0f, 1.0f, 0.0f)) * step_size;
		}
		for(int i = 0; i < SPHERE_COUNT; i++)
		{
			vec3 diff = curr_pos - sphere_array[i].pos;
			if(dot(diff, diff) < sphere_array[i].radius_sqr)
			{
				vec3 normal = normalize(diff);
				res = hit_info(curr_pos - step_vec, normalize(diff), true, i);
				if(--collision_count == 0) { return res; }
				step_vec = reflect(curr_pos, -normal) * step_size;
			}
		}
		curr_pos += step_vec;
		total_dist += step_size;
	}
	return res;
}

bool cast_ray2light(vec3 start, float step_size, int light_index)
{
	vec3 step_vec = (light_array[light_index].pos - start) * step_size;
	vec3 curr_pos = start;
	float light_dist = length(start - light_array[light_index].pos);
	while(light_dist > 1e-1)
	{
		if(curr_pos.y < floor_y)
			return false;
		for(int i = 0; i < SPHERE_COUNT; i++)
		{
			vec3 diff = curr_pos - sphere_array[i].pos;
			if(dot(diff, diff) < sphere_array[i].radius_sqr)
			{
				return false;
			}
		}
		light_dist -= step_size;
		curr_pos += step_vec;
	}
	return true;
}

vec3 compute_lighting(vec3 intersection_point, vec3 normal, float step_size, vec3 color)
{
	for(int i = 0; i < LIGHT_COUNT; i++)
	{
		bool res = cast_ray2light(intersection_point, step_size, i);
		if(res == true)
		{
			color += light_array[i].intensity * light_array[i].color * pow(max(dot(normal,
							normalize(light_array[i].pos - intersection_point)), 0.0f), 1);
		}
	}
	return color;
}

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	vec3 white = vec3(1.0f);
	vec3 red = vec3(1.0f, 0.0f, 0.0f);
	vec3 green = vec3(0.0f, 1.0f, 0.0f);
	vec3 blue = vec3(0.0f, 0.0f, 1.0f);
	sphere_array[0] = sphere(vec3(0.0f, 0.0f, 4.0f), 2.0f);
	sphere_array[1] = sphere(vec3(-1.0f, 1.0f, 6.0f),	0.0f);
	color_array[0] = vec3(0.0f, 0.0f, 0.0f);
	color_array[1] = vec3(0.0f, 1.0f, 0.0f);
	color_array[2] = vec3(1.0f, 0.8f, 0.0f);
	light_array[0] = light(vec3(5.0f, 2.0f, 1.0f), red, 0.8f);
	light_array[1] = light(vec3(-5.0f, 2.0f, 1.0f), green, 0.8f);
	light_array[2] = light(vec3(0.0f, 2.0f, -1.0f), blue, 0.8f);
	vec3 ray_dir = normalize(vec3(2.0f * tex_coord.x - 1.0f, 2.0f * tex_coord.y - 1.0f, 1.0f));
	hit_info cope = cast_ray(ray_dir, 0.01f, 20.0f, 3);
	if(cope.hit_target == true)
	{
		vec3 color1 = color_array[cope.target_id];
		if(cope.target_id == SPHERE_COUNT)
			color1 = vec3(1.0f - 0.5f * (sign((sin(2.0f * M_PI * cope.intersection_point.x) +
								sin(2.0f * M_PI * cope.intersection_point.z))) + 1.0f));
		vec3 color2 = compute_lighting(cope.intersection_point, cope.normal, 0.01f, color1);
		gl_FragColor = vec4(color2, 1.0f);
	}
	else
	{
		gl_FragColor = vec4(vec3(0.0f), 1.0f);
	}
}
