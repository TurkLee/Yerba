#include <list>
#include "..\Settings\Settings.h"
#include "..\SDK\CClientState.h"
#include "..\SDK\IVModelInfoClient.h"
#include "..\SDK\Cvar.h"
#include "..\Utils\Mathematic.h"

struct sequence_data {

	sequence_data::sequence_data(int instate, int outstate, int seqnr, float realtime) {

		inreliablestate = instate;
		outreliablestate = outstate;
		sequencenr = seqnr;
		time = realtime;
	}

	int inreliablestate, outreliablestate, sequencenr;
	float time;

};

struct backtrack_data {

	float simtime;
	Vector hitbox_position;
	Vector Origin;
	Vector Angle;
	int tick_count;
	matrix3x4_t bone_matrix[128];
};

struct vars_bt {

	int latest_tick;
	int last_sequence;
	float interval_per_tick;
};

/* Externs */
extern backtrack_data bone_data[64][12];
extern backtrack_data entity_data[64][12];
extern std::list<sequence_data> sequences;

/* Functions */
class LegitBacktrack {
public:
	void OnCreateMove()
	{
		if (g_Settings.Legit.bBacktrack)
		{
			this->run();
			this->update();
		}
	};
	bool IsValidBackTrackTick(float simtime);

protected:
	void legit(CUserCmd* pCmd);
	void update_backtrack();
	void update_sequences();
private:
	void run();
	void update();
	void clear();
	Vector angle_vector(Vector angle);
	float point_to_line(Vector point, Vector line_origin, Vector dir);
	template<class T, class U>
	T clamp(T in, U low, U high);
	float latency();
	float lerpTime();
};

extern LegitBacktrack* g_Backtrack;