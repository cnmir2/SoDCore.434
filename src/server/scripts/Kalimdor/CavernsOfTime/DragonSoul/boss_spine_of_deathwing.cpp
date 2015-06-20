/*
 * Copyright (C) 2013 WoW Source  <http://wowsource.info/>
 *
 * Copyright (C) 2013 WoWSource [WS] <http://wowsource.info/>
 *
 * Dont Share The SourceCode
 * and read our WoWSource Terms
 *
 */

/* ScriptData
SDName: boss_deathwing
SD%Complete: 50%
SDComment:
SDCategory: Boss Deathwing
EndScriptData
*/


#include "ScriptPCH.h"
#include "dragon_soul.h"
#include "Vehicle.h"
#include "InstanceScript.h"
#include "Map.h"

enum Events
{
	// Deathwing
	EVENT_ASSAULT_ASPECTS = 1,
	EVENT_CATACLYSM,
	EVENT_CORRUPTING_PARASITE,
	EVENT_ELEMENTIUM_BOLT,

	//tentacle
	EVENT_AGONUZUNG_PAIN,

	//other
	EVENT_SUMMON,
	EVENT_SUMMON_1,
	ATTACK_START,

	//Phase
	PHASE_1,
	PHASE_2,
	PHASE_3,

	//Phase 2
	EVENT_ELEMENTIUM_FRAGMENT,
	EVENT_ELEMENTIUM_TERROR,
	EVENT_CORRUPTED_BLOOD,
	EVENT_CONGEALING_BLOOD,

	//Phase 2 mob events
	EVENT_SHRAPNEL,
	EVENT_TETANUS,
	EVENT_CONGEALING_BLOOD_CAST,

	//trall
	EVENT_SAY_TRALL_START,
	EVENT_SAY_TRALL_1,
};

enum Spells
{
	SPELL_SEAL_ARMOR_BREACH = 105847,
	SPELL_ABSORB_BLOOD		= 105248,
	SPELL_NUCLEAR_BLAST		= 105845,
	
};

class npc_amalgamation : public CreatureScript
{
public:
    npc_amalgamation() : CreatureScript("npc_amalgamation") { }

    struct npc_amalgamationAI : public ScriptedAI
    {
        npc_amalgamationAI(Creature* creature) : ScriptedAI(creature)
        {
				
        }

        void EnterCombat(Unit* /*who*/)
        {
			DoCast(me, 109329);
			DoCast(me, 105845);
            events.ScheduleEvent(EVENT_CORRUPTING_PARASITE, 500);
                        GameObject* Plate = NULL;
                        Trinity::GameObjectInRangeCheck check(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 15);
                        Trinity::GameObjectLastSearcher<Trinity::GameObjectInRangeCheck> searcher(me, Plate, check);
                        me->VisitNearbyGridObject(30, searcher);
                        if (instance && Plate)
                            instance->SetData64(DATA_PLATE_DEATHWING_OPEN_TEST, Plate->GetGUID());
		}

	        void Reset()
        {
            events.Reset();
        }
		
        void UpdateAI(const uint32 diff)
        {
            if(!UpdateVictim())
                return;
				
            if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
            events.Update(diff);

        }
    private:
        EventMap events;
        bool notactive;
		InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_amalgamationAI (creature);
    }
};



void AddSC_boss_spine_of_deathwing()
{
	new npc_amalgamation();
}