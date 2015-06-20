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

#define TRALL_MENU "We are ready!"
#define TELE_MENU_1 "1st Area"
#define TELE_MENU_2 "2nd Area"
#define TELE_MENU_3 "3rd Area"
#define TELE_MENU_4 "4th Area"

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
	HAS_20PROCENT_HEALTH_NEW_PHASE,

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
	
	EVENT_EMOTE_START,
	EVENT_EMOTE_END,
};

enum Spells
{
	//DeathWing
    SPELL_ASSAULT_ASPECTS     = 107018,
    SPELL_CATACLYSM           = 106523,
    SPELL_CORRUPTING_PARASITE = 108649,
    SPELL_ELEMENTIUM_BOLT     = 105651,

	// Mutated corruption
    SPELL_IMPALE              = 106400,
    SPELL_IMPALE_ASPECT       = 107029,
	
	// tentacle
    SPELL_BURNING_BLODD       = 105401,
    SPELL_AGONUZUNG_PAIN      = 106548,
    SPELL_CRUSH               = 106385,

	// Phase 2
    SPELL_CONGEALING_BLOOD    = 109102,
    SPELL_SHRAPNEL            = 106791,
    SPELL_TETANUS             = 106728,
    SPELL_CORRUPTED_BLOOD     = 106835,
};

enum Texts
{
	//trall
    SAY_TRALL_START           = 0,
    SAY_TRALL_START_1         = 1,
    SAY_TRALL_20PROCENT       = 2,
    SAY_TRALL_DEATH_DEATHWING = 3,

	//Deathwing
    SAY_AGGRO    = 0,
    SAY_KILL_ALL = 1,
    SAY_SPELL_1  = 2,
    SAY_SPELL_2  = 3,
    SAY_SPELL_3  = 4,
};

enum Phases
{
    PHASE_ONE                   = 1,
    PHASE_TWO                   = 2
};


class boss_deathwing : public CreatureScript
{
public:
    boss_deathwing() : CreatureScript("boss_deathwing") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return GetDragonSoulAI<boss_deathwingAI>(creature);
    }

    struct boss_deathwingAI: public BossAI
    {
        boss_deathwingAI(Creature* creature) : BossAI(creature, BOSS_DEATHWING)
        {
            instance = creature->GetInstanceScript();
        }

		InstanceScript* instance;
        EventMap events;
		uint8 _phase;
		uint32 phase;
		
		void Reset()
		{
			SetPhase(PHASE_ONE, true);
			events.Reset();
			me->RemoveAllAuras();
			instance->SetBossState(DATA_PORTALS_ON_OFF, DONE);
			me->SetCanFly(true);
			me->SetReactState(REACT_AGGRESSIVE);

			me->SetInCombatWithZone();

			Talk(SAY_AGGRO);
			_Reset();
		}

		 void SetPhaseEvents()
			{
				switch (_phase)
					{
						case PHASE_ONE:
							me->SetControlled(true, UNIT_STATE_STUNNED);//disable rotate
							SetCombatMovement(false);
							events.ScheduleEvent(EVENT_ASSAULT_ASPECTS, urand(60000,100000), _phase);
							//if(IsHeroic()) // no work
							events.ScheduleEvent(EVENT_CATACLYSM, 900000, _phase ); // no work
							events.ScheduleEvent(EVENT_CORRUPTING_PARASITE, urand(60000,120000), _phase);
							events.ScheduleEvent(EVENT_ELEMENTIUM_BOLT, 150); //no work
							break;
						case PHASE_TWO:
							me->SetReactState(REACT_DEFENSIVE);
							me->HandleEmoteCommand(EMOTE_ONESHOT_CUSTOM_SPELL_01);
							events.ScheduleEvent(EVENT_CONGEALING_BLOOD, urand(30000,60000), _phase);
							events.ScheduleEvent(EVENT_ELEMENTIUM_FRAGMENT, urand(60000,120000), _phase);
							events.ScheduleEvent(EVENT_ELEMENTIUM_TERROR, urand(40000,80000), _phase);
							events.ScheduleEvent(EVENT_CORRUPTED_BLOOD, urand(40000,80000), _phase);
							break;
						default:
							break;
					}
			}
		void StartPhaseTwo()
			{
				SetPhase(PHASE_TWO, true);
			}
		void IsSummonedBy(Unit* /*summoner*/)
		{
			me->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
			events.ScheduleEvent(EVENT_EMOTE_START, 30000);
			if(instance)
				instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add
		}

		void SetPhase(uint8 phase, bool setEvents = false)
			{
				events.Reset();

				events.SetPhase(phase);
				_phase = phase;
				if (setEvents)
					SetPhaseEvents();
			}
		void EnterCombat(Unit* /*who*/)
		{
			instance->SetBossState(DATA_PORTALS_ON_OFF, IN_PROGRESS);
			events.SetPhase(PHASE_ONE);
			if(instance)
				instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add
			events.ScheduleEvent(EVENT_SAY_TRALL_START, 5000);
			_EnterCombat();
		}

		void UpdateAI(uint32 const diff)
		{          
			if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
				 return;
			if (me->GetHealthPct() <= 21.0f && _phase == PHASE_ONE)
							StartPhaseTwo();
			events.Update(diff);

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_EMOTE_START:
					me->HandleEmoteCommand(EMOTE_ONESHOT_NONE);
					break;
				// SAY
				case EVENT_SAY_TRALL_START:
					if(Creature* trall = me->FindNearestCreature(NPC_MAELSTROM_TRALL, 300.0f, true))
					    trall->AI()->Talk(SAY_TRALL_START);
					events.ScheduleEvent(EVENT_SAY_TRALL_1, 3000);
					break;

				case EVENT_SAY_TRALL_1:
					if(Creature* trall = me->FindNearestCreature(NPC_MAELSTROM_TRALL, 300.0f, true))
					    trall->AI()->Talk(SAY_TRALL_START_1);
					break;

				case HAS_20PROCENT_HEALTH_NEW_PHASE:
					if(me->GetHealthPct() < 21)
					{
						events.CancelEvent(HAS_20PROCENT_HEALTH_NEW_PHASE);
						events.ScheduleEvent(PHASE_2, 150);
						
						if(Creature* trall = me->FindNearestCreature(NPC_MAELSTROM_TRALL, 300.0f, true))
						{
					        trall->AI()->Talk(SAY_TRALL_20PROCENT);
							if(Player* players = trall->FindNearestPlayer(500.0f))
							    trall->SendPlaySound(26600, players);
						}

					}
					events.ScheduleEvent(HAS_20PROCENT_HEALTH_NEW_PHASE, 5000);
					break;

				case EVENT_ASSAULT_ASPECTS:
					if(Unit* target = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_MAELSTROM_TRALL)))
						DoCast(target, SPELL_ASSAULT_ASPECTS);
					events.ScheduleEvent(EVENT_ASSAULT_ASPECTS, urand(40000,80000), PHASE_ONE);
					break;
					
				case EVENT_CORRUPTING_PARASITE:
					if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
					{
						DoCast(target, SPELL_CORRUPTING_PARASITE);
						me->SummonCreature(NPC_CORRUPTION_PARASITE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
					}
					events.ScheduleEvent(EVENT_CORRUPTING_PARASITE, urand(40000,80000), PHASE_ONE);
					break;
				case EVENT_EMOTE_END:
					me->HandleEmoteCommand(EMOTE_ONESHOT_CUSTOM_SPELL_02);
					break;
				case EVENT_CORRUPTED_BLOOD:
					if(me->GetHealthPct() < 15)
					{
						events.CancelEvent(EVENT_CORRUPTED_BLOOD);
						DoCast(SPELL_CORRUPTED_BLOOD);
					}
					events.ScheduleEvent(EVENT_CORRUPTED_BLOOD, 50000, PHASE_TWO);
					break;

				case EVENT_CONGEALING_BLOOD:
					if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
						me->SummonCreature(NPC_CONGEALING_BLOOD, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
					events.ScheduleEvent(EVENT_CORRUPTED_BLOOD, urand(30000,80000), PHASE_TWO);
					break;

				case EVENT_ELEMENTIUM_TERROR:
					events.ScheduleEvent(EVENT_ELEMENTIUM_FRAGMENT, urand(60000,90000), PHASE_TWO);
					if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
						me->SummonCreature(NPC_ELEMENTIUM_FRAGMENT, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
					if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
						me->SummonCreature(NPC_ELEMENTIUM_FRAGMENT, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
					if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
						me->SummonCreature(NPC_ELEMENTIUM_FRAGMENT, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
					break;

				case EVENT_ELEMENTIUM_FRAGMENT:
					events.ScheduleEvent(EVENT_ELEMENTIUM_FRAGMENT, urand(120000,200000), PHASE_TWO);
					if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
						me->SummonCreature(NPC_ELEMENTIUM_TERROR, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
					if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
						me->SummonCreature(NPC_ELEMENTIUM_TERROR, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
					if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
						me->SummonCreature(NPC_ELEMENTIUM_TERROR, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
					break;
				default:
					break;
				}
			}

			DoMeleeAttackIfReady();
		}

		void JustDied(Unit* /*kller*/)
		{
			instance->SetBossState(DATA_PORTALS_ON_OFF, DONE);
			if(instance)
				instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
			if(Unit* creature = me->FindNearestCreature(57694, 100.0f))
				me->Kill(creature);
			if(Unit* creature = me->FindNearestCreature(57686, 100.0f))
				me->Kill(creature);
			if(Unit* creature = me->FindNearestCreature(57696, 100.0f))
				me->Kill(creature);
			if(Unit* creature = me->FindNearestCreature(57695, 100.0f))
				me->Kill(creature);
			if(Unit* creature = me->FindNearestCreature(56844, 100.0f))
				me->Kill(creature);
			if(Unit* killer = me->FindNearestPlayer(1000.0f))
			    killer->SummonGameObject(RAID_MODE(GO_DEATHWING_LOOT_10N, GO_DEATHWING_LOOT_25N, GO_DEATHWING_LOOT_10H, GO_DEATHWING_LOOT_25H), -12075.2f,  12168.2f, -2.56926f, 3.57793f, 0.0f, 0.0f, -0.976295f, 0.216442f, 320000);
			if(Creature* trall = me->FindNearestCreature(NPC_MAELSTROM_TRALL, 300.0f, true))
				trall->AI()->Talk(SAY_TRALL_DEATH_DEATHWING);
			me->DespawnOrUnsummon(5000);
            _JustDied();
        }
    };
};

class npc_maelstrom_trall : public CreatureScript
{
    public:
        npc_maelstrom_trall() : CreatureScript("npc_maelstrom_trall") { }

		
		bool OnGossipHello(Player* player, Creature* creature)
		{
			 if (InstanceScript* instance = creature->GetInstanceScript())
			 {
			     player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, TRALL_MENU, GOSSIP_SENDER_MAIN, 10);
			 }

			 player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
			 return true;
		}

		bool OnGossipSelect(Player *player, Creature *creature, uint32 sender, uint32 uiAction)
		{
			if (sender == GOSSIP_SENDER_MAIN)
			{
				player->PlayerTalkClass->ClearMenus();
				switch(uiAction)
				{
				case 10:
					if (InstanceScript* instance = creature->GetInstanceScript())
					{
					    player->CLOSE_GOSSIP_MENU();
						instance->SetData(DATA_ATTACK_DEATHWING, IN_PROGRESS);
						creature->SummonCreature(NPC_DEATHWING_1, -11903.9f, 11989.1f, -113.204f, 2.16421f, TEMPSUMMON_TIMED_DESPAWN, 12000000);
						creature->SummonCreature(NPC_WING_TENTACLE_1, -11943.0f, 12238.7f, 1.3657f, 2.44553f, TEMPSUMMON_TIMED_DESPAWN, 12000000);
						creature->SummonCreature(NPC_WING_TENTACLE_1, -12097.8f, 12067.4f, 13.488f, 2.21657f, TEMPSUMMON_TIMED_DESPAWN, 12000000);
						creature->SummonCreature(NPC_ARM_TENTACLE_1, -12068.5f, 12131.7f, -2.8029f, 2.33847f, TEMPSUMMON_TIMED_DESPAWN, 12000000);
						creature->SummonCreature(NPC_ARM_TENTACLE_2, -12014.0f, 12194.3f, -6.1510f, 2.12930f, TEMPSUMMON_TIMED_DESPAWN, 12000000);
						creature->SummonCreature(57694, -11967.1f, 11958.8f, -49.9822f, 2.16421f, TEMPSUMMON_CORPSE_DESPAWN, 6000000);
						creature->SummonCreature(57686, -11852.1f, 12036.4f, -49.9821f, 2.16421f, TEMPSUMMON_CORPSE_DESPAWN, 6000000);
						creature->SummonCreature(57696, -11913.8f, 11926.5f, -60.3749f, 2.16421f, TEMPSUMMON_CORPSE_DESPAWN, 6000000);
						creature->SummonCreature(57695, -11842.2f, 11974.8f, -60.3748f, 2.16421f, TEMPSUMMON_CORPSE_DESPAWN, 6000000);
						creature->SummonCreature(56844, -11857.0f, 11795.6f, -73.9549f, 2.23402f, TEMPSUMMON_CORPSE_DESPAWN, 6000000);
						CAST_AI(npc_maelstrom_trall::npc_maelstrom_trallAI, creature->AI())->Ascpectaura(player);
						creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
					}
				    break;
				default:
					break;
				}
			}
			return true;
		}

		

    struct npc_maelstrom_trallAI : public ScriptedAI
    {
        npc_maelstrom_trallAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        void Ascpectaura(Player* pl)
        {
            Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers(); 
            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                {
                    Player *player = i->getSource();
                    if (player)
                    {
                        player->CastSpell(player, 105825, true);
                        player->CastSpell(player, 105823, true);
                        player->CastSpell(player, 106456, true);
                        player->CastSpell(player, 106026, true);
                    }
                }
            }
        }
		
	};
	    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_maelstrom_trallAI (creature);
    }

};

class Teleportation_to_the_platforms : public CreatureScript
{
    public:
        Teleportation_to_the_platforms() : CreatureScript("Teleportation_to_the_platforms") { }

		bool OnGossipHello(Player* player, Creature* creature)
		{
			 player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, TELE_MENU_1, GOSSIP_SENDER_MAIN, 20);
			 player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, TELE_MENU_2, GOSSIP_SENDER_MAIN, 21);
			 player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, TELE_MENU_3, GOSSIP_SENDER_MAIN, 22);
			 player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, TELE_MENU_4, GOSSIP_SENDER_MAIN, 23);

			 player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
			 return true;
		}

		bool OnGossipSelect(Player *player, Creature *creature, uint32 sender, uint32 uiAction)
		{
			if (sender == GOSSIP_SENDER_MAIN)
			{
				player->PlayerTalkClass->ClearMenus();
				switch(uiAction)
				{
				case 20:
					player->CLOSE_GOSSIP_MENU();
					player->TeleportTo(967, -11961.371f, 12294.102f, 1.284f, 5.094f);
				    break;
				case 21:
					player->CLOSE_GOSSIP_MENU();
					player->TeleportTo(967, -12058.748f, 12245.875f, -6.151f, 5.464f);
				    break;
				case 22:
					player->CLOSE_GOSSIP_MENU();
					player->TeleportTo(967, -12121.12f, 12174.846f, -2.735f, 5.604f);
				    break;
				case 23:
					player->CLOSE_GOSSIP_MENU();
					player->TeleportTo(967, -12157.026f, 12090.791f, 2.308f, 6.029f);
				    break;
				default:
					break;
				}
			}
			return true;
		}
};

class npc_arm_tentacle_one : public CreatureScript
{
    public:
        npc_arm_tentacle_one() : CreatureScript("npc_arm_tentacle_one") { }

		CreatureAI* GetAI(Creature* creature) const
		{
		    return GetDragonSoulAI<npc_arm_tentacle_oneAI>(creature);
		}

		struct npc_arm_tentacle_oneAI : public ScriptedAI
		{
		    npc_arm_tentacle_oneAI(Creature* creature) : ScriptedAI(creature)
		    {
			    instance = creature->GetInstanceScript();
							me->SetControlled(true, UNIT_STATE_STUNNED);//disable rotate
			SetCombatMovement(false);
		    }

			InstanceScript* instance;
			EventMap events;

			void Reset()
			{
				events.Reset();
			}
		void IsSummonedBy(Unit* /*summoner*/)
		{
			me->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
			events.ScheduleEvent(EVENT_EMOTE_START, 30000);
		}
			void EnterCombat(Unit* /*who*/)
			{
				DoCast(SPELL_BURNING_BLODD);
				events.ScheduleEvent(EVENT_SUMMON, 30000);
				instance->SetData(DATA_DAMAGE_DEATHWING, IN_PROGRESS);
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
				me->AddUnitState(UNIT_STATE_CANNOT_TURN);
			}

			void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
				{
                    switch (eventId)
                    {
					case EVENT_EMOTE_START:
					me->HandleEmoteCommand(EMOTE_ONESHOT_NONE);
						break;
					case EVENT_SUMMON:
						events.ScheduleEvent(EVENT_SUMMON, 5000);
						if(me->GetHealthPct() < 50)
						{
							events.CancelEvent(EVENT_SUMMON);
							me->SummonCreature(NPC_MYTATED_CORRUPTION, -12110.934f, 12184.085f, -2.731f, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
						}
						break;
					}
				}
			}

			void JustDied(Unit* /*killer*/)
            {
				instance->SetData(DATA_DAMAGE_DEATHWING, DONE);
			}

		};
};

class npc_arm_tentacle_two : public CreatureScript
{
    public:
        npc_arm_tentacle_two() : CreatureScript("npc_arm_tentacle_two") { }

		CreatureAI* GetAI(Creature* creature) const
		{
		    return GetDragonSoulAI<npc_arm_tentacle_twoAI>(creature);
		}

		struct npc_arm_tentacle_twoAI : public ScriptedAI
		{
		    npc_arm_tentacle_twoAI(Creature* creature) : ScriptedAI(creature)
		    {
									me->SetControlled(true, UNIT_STATE_STUNNED);//disable rotate
			SetCombatMovement(false);
			    instance = creature->GetInstanceScript();
		    }

			InstanceScript* instance;
			EventMap events;

			void Reset()
			{
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
				events.Reset();
			}

					void IsSummonedBy(Unit* /*summoner*/)
		{
			me->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
			events.ScheduleEvent(EVENT_EMOTE_START, 30000);
		}
			
			void EnterCombat(Unit* /*who*/)
			{
				DoCast(SPELL_BURNING_BLODD);
				events.ScheduleEvent(EVENT_SUMMON, 30000);
				instance->SetData(DATA_DAMAGE_DEATHWING, IN_PROGRESS);
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
			}

			void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
				{
                    switch (eventId)
                    {
					case EVENT_EMOTE_START:
					me->HandleEmoteCommand(EMOTE_ONESHOT_NONE);
						break;
					case EVENT_SUMMON:
						events.ScheduleEvent(EVENT_SUMMON, 5000);
						if(me->GetHealthPct() < 50)
						{
							events.CancelEvent(EVENT_SUMMON);
							me->SummonCreature(NPC_MYTATED_CORRUPTION, -12060.653f, 12235.851f, -6.255f, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
						}
						break;
					}
				}
			}

			void JustDied(Unit* /*killer*/)
            {
				instance->SetData(DATA_DAMAGE_DEATHWING, DONE);
			}

		};
};

class npc_wing_tentacle_one : public CreatureScript
{
    public:
        npc_wing_tentacle_one() : CreatureScript("npc_wing_tentacle_one") { }

		CreatureAI* GetAI(Creature* creature) const
		{
		    return GetDragonSoulAI<npc_wing_tentacle_oneAI>(creature);
		}

		struct npc_wing_tentacle_oneAI : public ScriptedAI
		{
		    npc_wing_tentacle_oneAI(Creature* creature) : ScriptedAI(creature)
		    {
			    instance = creature->GetInstanceScript();
							me->SetControlled(true, UNIT_STATE_STUNNED);//disable rotate
			SetCombatMovement(false);
		    }

			InstanceScript* instance;
			EventMap events;

			void Reset()
			{
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
				events.Reset();
			}

					void IsSummonedBy(Unit* /*summoner*/)
		{
			me->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
			events.ScheduleEvent(EVENT_EMOTE_START, 30000);
		}
			
			void EnterCombat(Unit* /*who*/)
			{
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
				DoCast(SPELL_BURNING_BLODD);
				events.ScheduleEvent(EVENT_SUMMON, 30000);
				instance->SetData(DATA_DAMAGE_DEATHWING, IN_PROGRESS);
			}

			void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
				{
                    switch (eventId)
                    {
					case EVENT_EMOTE_START:
					me->HandleEmoteCommand(EMOTE_ONESHOT_NONE);
						break;
					case EVENT_SUMMON:
						events.ScheduleEvent(EVENT_SUMMON, 5000);
						if(me->GetHealthPct() < 50)
						{
							events.CancelEvent(EVENT_SUMMON);
							if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
							me->SummonCreature(NPC_MYTATED_CORRUPTION, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 120000);
						}
						break;
					}
				}
			}

			void JustDied(Unit* killer)
            {
				instance->SetData(DATA_DAMAGE_DEATHWING, DONE);
			}

		};
};

class npc_maelstrom_ysera : public CreatureScript
{
    public:
        npc_maelstrom_ysera() : CreatureScript("npc_maelstrom_ysera") { }

		CreatureAI* GetAI(Creature* creature) const
		{
		    return GetDragonSoulAI<npc_maelstrom_yseraAI>(creature);
		}

		struct npc_maelstrom_yseraAI : public ScriptedAI
		{
		    npc_maelstrom_yseraAI(Creature* creature) : ScriptedAI(creature)
		    {
			    instance = creature->GetInstanceScript();
me->SetHover(true);

me->SetDisableGravity(true);

me->SetCanFly(true);
		    }

			InstanceScript* instance;
			EventMap events;

			void Reset()
			{
				me->SetCanFly(true);
			}

			void EnterCombat(Unit* /*who*/)
			{
			}

			void UpdateAI(uint32 const diff)
            {

			}

			void JustDied(Unit* killer)
            {
			}

		};
};

class npc_maelstrom_alexstrasza : public CreatureScript
{
    public:
        npc_maelstrom_alexstrasza() : CreatureScript("npc_maelstrom_alexstrasza") { }

		CreatureAI* GetAI(Creature* creature) const
		{
		    return GetDragonSoulAI<npc_maelstrom_alexstraszaAI>(creature);
		}

		struct npc_maelstrom_alexstraszaAI : public ScriptedAI
		{
		    npc_maelstrom_alexstraszaAI(Creature* creature) : ScriptedAI(creature)
		    {
			    instance = creature->GetInstanceScript();
me->SetHover(true);

me->SetDisableGravity(true);

me->SetCanFly(true);
		    }

			InstanceScript* instance;
			EventMap events;

			void Reset()
			{
				me->SetCanFly(true);
			}

			void EnterCombat(Unit* /*who*/)
			{
			}

			void UpdateAI(uint32 const diff)
            {

			}

			void JustDied(Unit* killer)
            {
			}

		};
};

class npc_maelstrom_kalecgos : public CreatureScript
{
    public:
        npc_maelstrom_kalecgos() : CreatureScript("npc_maelstrom_kalecgos") { }

		CreatureAI* GetAI(Creature* creature) const
		{
		    return GetDragonSoulAI<npc_maelstrom_kalecgosAI>(creature);
		}

		struct npc_maelstrom_kalecgosAI : public ScriptedAI
		{
		    npc_maelstrom_kalecgosAI(Creature* creature) : ScriptedAI(creature)
		    {
			    instance = creature->GetInstanceScript();
me->SetHover(true);

me->SetDisableGravity(true);

me->SetCanFly(true);
		    }

			InstanceScript* instance;
			EventMap events;

			void Reset()
			{
				me->SetCanFly(true);
			}

			void EnterCombat(Unit* /*who*/)
			{
			}

			void UpdateAI(uint32 const diff)
            {

			}

			void JustDied(Unit* killer)
            {
			}

		};
};

class npc_maelstrom_nozdormu : public CreatureScript
{
    public:
        npc_maelstrom_nozdormu() : CreatureScript("npc_maelstrom_nozdormu") { }

		CreatureAI* GetAI(Creature* creature) const
		{
		    return GetDragonSoulAI<npc_maelstrom_nozdormuAI>(creature);
		}

		struct npc_maelstrom_nozdormuAI : public ScriptedAI
		{
		    npc_maelstrom_nozdormuAI(Creature* creature) : ScriptedAI(creature)
		    {
			    instance = creature->GetInstanceScript();
me->SetHover(true);

me->SetDisableGravity(true);

me->SetCanFly(true);
		    }

			InstanceScript* instance;
			EventMap events;

			void Reset()
			{
				me->SetCanFly(true);
			}

			void EnterCombat(Unit* /*who*/)
			{
			}

			void UpdateAI(uint32 const diff)
            {

			}

			void JustDied(Unit* killer)
            {
			}

		};
};

void AddSC_boss_deathwing()
{
    new boss_deathwing();
    new npc_maelstrom_trall();
    new npc_arm_tentacle_one();
    new npc_arm_tentacle_two();
    new npc_wing_tentacle_one();
	new Teleportation_to_the_platforms();
    //new npc_mytated_corruption();
    new npc_maelstrom_alexstrasza();
    new npc_maelstrom_kalecgos();
    new npc_maelstrom_ysera();
    new npc_maelstrom_nozdormu();
}