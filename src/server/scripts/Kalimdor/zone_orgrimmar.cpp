/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Orgrimmar
SD%Complete: 100
SDComment: Quest support: 2460, 5727, 6566
SDCategory: Orgrimmar
EndScriptData */

/* ContentData
npc_neeru_fireblade     npc_text + gossip options text missing
npc_shenthul
npc_thrall_warchief
EndContentData */

#include "ScriptPCH.h"

/*######
## npc_neeru_fireblade
######*/

#define QUEST_5727  5727

#define GOSSIP_HNF "You may speak frankly, Neeru..."
#define GOSSIP_SNF "[PH] ..."
class npc_neeru_fireblade : public CreatureScript
{
public:
    npc_neeru_fireblade() : CreatureScript("npc_neeru_fireblade") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
    {
        pPlayer->PlayerTalkClass->ClearMenus();
        switch (uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SNF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                pPlayer->SEND_GOSSIP_MENU(4513, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                pPlayer->CLOSE_GOSSIP_MENU();
                pPlayer->AreaExploredOrEventHappens(QUEST_5727);
                break;
        }
        return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        if (pCreature->IsQuestGiver())
            pPlayer->PrepareQuestMenu(pCreature->GetGUID());

        if (pPlayer->GetQuestStatus(QUEST_5727) == QUEST_STATUS_INCOMPLETE)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HNF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        pPlayer->SEND_GOSSIP_MENU(4513, pCreature->GetGUID());
        return true;
    }
};

/*######
## npc_shenthul
######*/

enum eShenthul
{
    QUEST_SHATTERED_SALUTE  = 2460
};

class npc_shenthul : public CreatureScript
{
public:
    npc_shenthul() : CreatureScript("npc_shenthul") { }

    bool OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_SHATTERED_SALUTE)
        {
            CAST_AI(npc_shenthul::npc_shenthulAI, pCreature->AI())->CanTalk = true;
            CAST_AI(npc_shenthul::npc_shenthulAI, pCreature->AI())->PlayerGUID = pPlayer->GetGUID();
        }
        return true;
    }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_shenthulAI (pCreature);
    }

    struct npc_shenthulAI : public ScriptedAI
    {
        npc_shenthulAI(Creature* c) : ScriptedAI(c) {}

        bool CanTalk;
        bool CanEmote;
        uint32 Salute_Timer;
        uint32 Reset_Timer;
        uint64 PlayerGUID;

        void Reset()
        {
            CanTalk = false;
            CanEmote = false;
            Salute_Timer = 6000;
            Reset_Timer = 0;
            PlayerGUID = 0;
        }

        void EnterCombat(Unit* /*who*/) {}

        void UpdateAI(const uint32 diff)
        {
            if (CanEmote)
            {
                if (Reset_Timer <= diff)
                {
                    if (Player* pPlayer = Unit::GetPlayer(*me, PlayerGUID))
                    {
                        if (pPlayer->GetTypeId() == TYPEID_PLAYER && pPlayer->GetQuestStatus(QUEST_SHATTERED_SALUTE) == QUEST_STATUS_INCOMPLETE)
                            pPlayer->FailQuest(QUEST_SHATTERED_SALUTE);
                    }
                    Reset();
                } else Reset_Timer -= diff;
            }

            if (CanTalk && !CanEmote)
            {
                if (Salute_Timer <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                    CanEmote = true;
                    Reset_Timer = 60000;
                } else Salute_Timer -= diff;
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void ReceiveEmote(Player* pPlayer, uint32 emote)
        {
            if (emote == TEXT_EMOTE_SALUTE && pPlayer->GetQuestStatus(QUEST_SHATTERED_SALUTE) == QUEST_STATUS_INCOMPLETE)
            {
                if (CanEmote)
                {
                    pPlayer->AreaExploredOrEventHappens(QUEST_SHATTERED_SALUTE);
                    Reset();
                }
            }
        }
    };
};

/*######
## npc_thrall_warchief
######*/

#define QUEST_6566              6566

#define SPELL_CHAIN_LIGHTNING   16033
#define SPELL_SHOCK             16034

#define GOSSIP_HTW "Please share your wisdom with me, Warchief."
#define GOSSIP_STW1 "What discoveries?"
#define GOSSIP_STW2 "Usurper?"
#define GOSSIP_STW3 "With all due respect, Warchief - why not allow them to be destroyed? Does this not strengthen our position?"
#define GOSSIP_STW4 "I... I did not think of it that way, Warchief."
#define GOSSIP_STW5 "I live only to serve, Warchief! My life is empty and meaningless without your guidance."
#define GOSSIP_STW6 "Of course, Warchief!"

//TODO: verify abilities/timers
class npc_thrall_warchief : public CreatureScript
{
public:
    npc_thrall_warchief() : CreatureScript("npc_thrall_warchief") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
    {
        pPlayer->PlayerTalkClass->ClearMenus();
        switch (uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                pPlayer->SEND_GOSSIP_MENU(5733, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
                pPlayer->SEND_GOSSIP_MENU(5734, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+3:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
                pPlayer->SEND_GOSSIP_MENU(5735, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+4:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
                pPlayer->SEND_GOSSIP_MENU(5736, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+5:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
                pPlayer->SEND_GOSSIP_MENU(5737, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+6:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_STW6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
                pPlayer->SEND_GOSSIP_MENU(5738, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+7:
                pPlayer->CLOSE_GOSSIP_MENU();
                pPlayer->AreaExploredOrEventHappens(QUEST_6566);
                break;
        }
        return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        if (pCreature->IsQuestGiver())
            pPlayer->PrepareQuestMenu(pCreature->GetGUID());

        if (pPlayer->GetQuestStatus(QUEST_6566) == QUEST_STATUS_INCOMPLETE)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HTW, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
        return true;
    }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_thrall_warchiefAI (pCreature);
    }

    struct npc_thrall_warchiefAI : public ScriptedAI
    {
        npc_thrall_warchiefAI(Creature* c) : ScriptedAI(c) {}

        uint32 ChainLightning_Timer;
        uint32 Shock_Timer;

        void Reset()
        {
            ChainLightning_Timer = 2000;
            Shock_Timer = 8000;
        }

        void EnterCombat(Unit * /*who*/) {}

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (ChainLightning_Timer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_CHAIN_LIGHTNING);
                ChainLightning_Timer = 9000;
            } else ChainLightning_Timer -= diff;

            if (Shock_Timer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_SHOCK);
                Shock_Timer = 15000;
            } else Shock_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_herezegor
######*/

#define HEREZEGOR_OPTION_1 "Yes, a map of the coastal approaches."

class npc_herezegor : public CreatureScript
{
	public:
	npc_herezegor() : CreatureScript("npc_herezegor") { }
	uint64 plrGUID;
	bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
	{
		player->PlayerTalkClass->ClearMenus();
		switch (action)
		{
			case GOSSIP_ACTION_INFO_DEF+1:
			{
				player->AddItem(58141, 1); // Add the quest item to the player (map)
				break;
			}
		}
		return true;
	}
	
	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (player->GetQuestStatus(26311) == QUEST_STATUS_INCOMPLETE)
		{
				player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, HEREZEGOR_OPTION_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
				player->PlayerTalkClass->SendGossipMenu(50431, creature->GetGUID());
			
		}
		return true;
	}

};

/*######
## npc_hellscream_demolisher
######*/
#define FIRST_ENGINEER_SAY_1 "Ready for inspection, sir!"
#define FIRST_ENGINEER_SAY_2 "See that? Ironclad!"
#define SECOND_ENGINEER_SAY_1 "This baby is all mine! I call her, \"The Throttler.\""
#define SECOND_ENGINEER_SAY_2 "That's right. Takes a lickin' but keeps on kickin' butt!"
#define THIRD_ENGINEER_SAY_1 "Ready for inspection! ...Be gentle."
#define THIRD_ENGINEER_SAY_2 "Uh oh. Um. Oops."
#define THIRD_ENGINEER_SAY_3 "Look, look, it's okay! It's just a tiny little glitch. See?"
#define SIEGEWORKERS_SAY_1 "We'll get right on this, sir!"
#define SIEGEWORKERS_SAY_2 "Yes sir! Right away, sir!"
#define SIEGEWORKERS_SAY_3 "We're on it!"
#define DEMOLISHER_FALL "The demolisher falls apart when inspected."
#define WORKERS_REPAIR "Punish the goblin, kicking him away! Workers goes to repair it right away."
// these defines need cleaned out to be set to enums & creature_text.
class npc_hellscream_demolisher : public CreatureScript
{
public:
npc_hellscream_demolisher() : CreatureScript("npc_hellscream_demolisher") {}
bool OnGossipHello(Player* player, Creature* creature)
{
if (player->GetQuestStatus(26294) == QUEST_STATUS_INCOMPLETE) // Weapons of mass dysfunction
{
if (CAST_AI(npc_hellscream_demolisher::npc_hellscream_demolisherAI, creature->AI())->demolisher_N == 1)
{
if (Creature* eng_chief1 = creature->FindNearestCreature(42671, 6.0f, true)) // Find the chief engineer
{
eng_chief1->MonsterSay(FIRST_ENGINEER_SAY_1, LANG_UNIVERSAL, NULL);
eng_chief1->MonsterSay(FIRST_ENGINEER_SAY_2, LANG_UNIVERSAL, NULL);
CAST_AI(npc_hellscream_demolisher::npc_hellscream_demolisherAI, creature->AI())->demolisher_N = 2;
}
}
else if (CAST_AI(npc_hellscream_demolisher::npc_hellscream_demolisherAI, creature->AI())->demolisher_N == 2)
{
if (Creature* eng_chief2 = creature->FindNearestCreature(42671, 6.0f, true)) // Find the chief engineer
{
eng_chief2->MonsterSay(SECOND_ENGINEER_SAY_1, LANG_UNIVERSAL, NULL);
eng_chief2->MonsterSay(SECOND_ENGINEER_SAY_2, LANG_UNIVERSAL, NULL);
CAST_AI(npc_hellscream_demolisher::npc_hellscream_demolisherAI, creature->AI())->demolisher_N = 3;
}
}
else if (CAST_AI(npc_hellscream_demolisher::npc_hellscream_demolisherAI, creature->AI())->demolisher_N == 3)
{
if (Creature* eng_chief3 = creature->FindNearestCreature(42671, 6.0f, true)) // Find the chief engineer
{
eng_chief3->MonsterSay(THIRD_ENGINEER_SAY_1, LANG_UNIVERSAL, NULL);
creature->MonsterTextEmote(DEMOLISHER_FALL, 0);
eng_chief3->MonsterSay(THIRD_ENGINEER_SAY_2, LANG_UNIVERSAL, NULL);
eng_chief3->MonsterSay(THIRD_ENGINEER_SAY_3, LANG_UNIVERSAL, NULL);
creature->MonsterTextEmote(WORKERS_REPAIR, 0);
player->CompleteQuest(26294);
CAST_AI(npc_hellscream_demolisher::npc_hellscream_demolisherAI, creature->AI())->demolisher_N = 1;
}
}
}
player->PlayerTalkClass->ClearMenus();
return true;
}
CreatureAI* GetAI(Creature* creature) const
{
return new npc_hellscream_demolisherAI(creature);
}
struct npc_hellscream_demolisherAI : public ScriptedAI
{
npc_hellscream_demolisherAI(Creature* creature) : ScriptedAI(creature) {}
uint32 demolisher_N;
void Reset()
{
demolisher_N = 1;
}
};
};



void AddSC_orgrimmar()
{
	new npc_hellscream_demolisher();
    new npc_neeru_fireblade();
    new npc_shenthul();
    new npc_thrall_warchief();
	new npc_herezegor();
}