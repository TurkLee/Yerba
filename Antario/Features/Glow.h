#pragma once
#include <Windows.h>
#include "../SDK/Vector.h"
#include "../SDK/CEntity.h"

class cglowmanager {
public:
	struct GlowObjectDefinition_t {
		GlowObjectDefinition_t() { memset(this, 0, sizeof(*this)); }

		class C_BaseEntity* m_pEntity;    //0x0000
		union {
			Vector m_vGlowColor;           //0x0004
			struct {
				float   m_flRed;           //0x0004
				float   m_flGreen;         //0x0008
				float   m_flBlue;          //0x000C
			};
		};
		float   m_flAlpha;                 //0x0010
		uint8_t pad_0014[4];               //0x0014
		float   m_flSomeFloat;             //0x0018
		uint8_t pad_001C[4];               //0x001C
		float   m_flAnotherFloat;          //0x0020
		bool    m_bRenderWhenOccluded;     //0x0024
		bool    m_bRenderWhenUnoccluded;   //0x0025
		bool    m_bFullBloomRender;        //0x0026
		uint8_t pad_0027[5];               //0x0027
		int32_t m_nGlowStyle;              //0x002C
		int32_t m_nSplitScreenSlot;        //0x0030
		int32_t m_nNextFreeSlot;           //0x0034

		bool IsUnused() const { return m_nNextFreeSlot != -2; }
	};

	GlowObjectDefinition_t* m_GlowObjectDefinitions;
	int		max_size;
	int		pad;
	int		size;
	GlowObjectDefinition_t* m_GlowObjectDefinitions2;
	int		currentObjects;
};