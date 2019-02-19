#include "Backtrack.h"

/* Definitions */
#define TICK_INTERVAL (g_pGlobalVars->intervalPerTick)
#define TIME_TO_TICKS(dt) ((int)(0.5f + (float)(dt) / TICK_INTERVAL))
#define TICKS_TO_TIME( t ) ( TICK_INTERVAL *( t ) )
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* Pointers */
LegitBacktrack* g_Backtrack;
vars_bt variables;
backtrack_data bone_data[64][12];
backtrack_data entity_data[64][12];
std::list<sequence_data> sequences;

/* Utilities */
Vector getOrigin(C_BaseEntity* p_entity)
{
	Vector origin = p_entity->GetVecOrigin();

	return origin;
}

Vector get_hitbox_position(C_BaseEntity* p_entity, int hitbox_id) {

	matrix3x4_t bone_matrix[MAXSTUDIOBONES];

	if (p_entity->SetupBones(bone_matrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {

		auto studio_model = g_MdlInfo->GetStudiomodel2(p_entity->GetModel());

		if (studio_model) {

			auto hitbox = studio_model->pHitboxSet(0)->pHitbox(hitbox_id);

			if (hitbox) {

				auto min = Vector{}, max = Vector{};

				min = g_Math.VectorTransform(hitbox->bbmin, bone_matrix[hitbox->bone]);
				max = g_Math.VectorTransform(hitbox->bbmax, bone_matrix[hitbox->bone]);

				return Vector((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f);
			}
		}
	}
	return Vector{};
}

/* Fuctions */
void LegitBacktrack::run() {

	/* If our backtrack is disabled, don't do anything */
	if (!g_Settings.Legit.bBacktrack)
		return;

	/* If we enable the backtrack */
	if (g_Settings.Legit.bBacktrack) {

		/* Run the legit backtrack */
		LegitBacktrack::legit(g::pCmd);
	}
}

void LegitBacktrack::update() {

	/* Update our backtrack */
	LegitBacktrack::update_backtrack();
	LegitBacktrack::update_sequences();
}

void LegitBacktrack::clear() {

	auto net_channel = *reinterpret_cast<INetChannel**>(reinterpret_cast<std::uintptr_t>(g_pClientState) + 0x9C);

	if (net_channel) {

		net_channel->in_sequence_nr = 0.0f;
		sequences.clear();
	}
}

template<class T, class U>
T LegitBacktrack::clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	if (in >= high)
		return high;
	return in;
}

float LegitBacktrack::latency()
{
	const auto netch = g_pEngine->GetNetChannelInfo();

	if (netch)
		return netch->GetAvgLatency(0);

	return 0.0f;
}

bool LegitBacktrack::IsValidBackTrackTick(float simtime)
{
	float correct = clamp(latency() + lerpTime(), 0.f, 0.2f);
	float delta_time = correct - (g_pGlobalVars->curtime - simtime + lerpTime());
	return fabsf(delta_time) < 0.2f;
}

float LegitBacktrack::lerpTime()
{
	int ud_rate = g_pCvar->FindVar("cl_updaterate")->GetInt();
	float ratio = g_pCvar->FindVar("cl_interp_ratio")->GetFloat();
	return ratio / (float)ud_rate;
}

void LegitBacktrack::legit(CUserCmd* pCmd) {

	/* If our backtrack is disabled, don't do anything */
	if (!g_Settings.Legit.bBacktrack)
		return;

	/* If we enable the backtrack */
	if (g_Settings.Legit.bBacktrack) {

		int best_target = -1;
		float best_fov = FLT_MAX;

		PlayerInfo_t pInfo;

		/* Pointer to the local player */
		C_BaseEntity* p_local = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

		/* Don't do anything if the local player is dead */
		if (!p_local->IsAlive())
			return;

		/* Get the local player view direction */
		Vector view_direction = LegitBacktrack::angle_vector(pCmd->viewangles + (p_local->GetAimPunch() * 2.f));

		/* Loop through all connected clients */
		for (int i = 1; i < 65; i++) {

			/* Pointer to the entity (clients) */
			auto p_entity = g_pEntityList->GetClientEntity(i);

			/* Don't do anything if the local player is somehow invalid */
			if (!p_local)
				continue;

			/* If we don't have a entity to backtrack, don't do anything */
			if (!p_entity)
				continue;

			/* If the entity is the local player, don't do anything */
			if (p_entity == p_local)
				continue;

			/* If we can't get the player info, don't do anything */
			if (!g_pEngine->GetPlayerInfo(i, &pInfo))
				continue;

			/* If the entity is dormant, don't do anything */
			if (p_entity->IsDormant())
				continue;

			/* Don't backtrack players of the local player team */
			if (p_entity->GetTeam() == p_local->GetTeam())
				continue;

			if (!p_entity->SetupBones(bone_data[i][pCmd->command_number % 12].bone_matrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, g_pGlobalVars->curtime))
				continue;

			/* If the entity is alive, we can start searching for a candidate to backtrack */
			if (p_entity->IsAlive()) {

				/* Get the simulation time */
				float simtime = p_entity->GetSimulationTime();

				/* Get the hitbox position */
				Vector hitbox_position = get_hitbox_position(p_entity, 0);

				Vector Origin = p_entity->GetVecOrigin();

				Vector Angle = p_entity->GetEyeAngles();

				/* Store data about the entity (his position, etc) */
				entity_data[i][pCmd->command_number % 12] = backtrack_data{ simtime, hitbox_position, Origin, Angle };

				float fov_distance = LegitBacktrack::point_to_line(hitbox_position, p_local->GetEyePosition(), view_direction);

				if (best_fov > fov_distance) {

					best_fov = fov_distance;
					best_target = i;
				}
			}
		}

		/* Best simulation time (we will store in here) */
		float best_simtime;

		/* If we have a good target */
		if (best_target != -1) {

			/* Temporary float */
			float temporary_float = FLT_MAX;

			for (int t = 0; t < 12; ++t) {

				/* Temporary fov */
				float temp_fov_distance = LegitBacktrack::point_to_line(entity_data[best_target][t].hitbox_position, p_local->GetEyePosition(), view_direction);

				if (temporary_float > temp_fov_distance && entity_data[best_target][t].simtime > p_local->GetSimulationTime() - 1) {

					temporary_float = temp_fov_distance;
					best_simtime = entity_data[best_target][t].simtime;
				}
			}

			/* Do the magic :) */
			if (pCmd->buttons & IN_ATTACK)
				pCmd->tick_count = TIME_TO_TICKS(best_simtime);
		}
	}
}

void LegitBacktrack::update_backtrack() {

	if (!g_Settings.Legit.bBacktrack)
		return;

	variables.latest_tick = g_pGlobalVars->tickcount;
}

void LegitBacktrack::update_sequences() {

	auto net_channel = *reinterpret_cast<INetChannel**>(reinterpret_cast<std::uintptr_t>(g_pClientState) + 0x9C);

	if (net_channel) {

		if (net_channel->in_sequence_nr > variables.last_sequence) {

			variables.last_sequence = net_channel->in_sequence_nr;
			sequences.push_front(sequence_data(net_channel->in_reliable_state_count, net_channel->out_reliable_state_count, net_channel->in_sequence_nr, g_pGlobalVars->realtime));
		}

		if (sequences.size() > 2048)
			sequences.pop_back();
	}
}

inline Vector LegitBacktrack::angle_vector(Vector angle) {

	auto sy = sin(angle.y / 180.f * static_cast<float>(M_PI));
	auto cy = cos(angle.y / 180.f * static_cast<float>(M_PI));

	auto sp = sin(angle.x / 180.f * static_cast<float>(M_PI));
	auto cp = cos(angle.x / 180.f* static_cast<float>(M_PI));

	return Vector(cp*cy, cp*sy, -sp);
}

inline float LegitBacktrack::point_to_line(Vector point, Vector line_origin, Vector dir) {

	auto point_dir = point - line_origin;

	auto temp_offset = point_dir.Dot(dir) / (dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

	if (temp_offset < 0.000001f)
		return FLT_MAX;

	auto perpendicular_point = line_origin + (dir * temp_offset);

	return (point - perpendicular_point).Length();
}