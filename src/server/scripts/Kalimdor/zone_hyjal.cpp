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
 
#include "ScriptPCH.h"
#include "Vehicle.h"
#include "Unit.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CombatAI.h"
#include "PassiveAI.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "CreatureTextMgr.h"
#include "Vehicle.h"
#include "VehicleDefines.h"
#include "Spell.h"
#include "Player.h"
#include "Map.h"
#include "InstanceScript.h"

 
class emerald_flameweaver : public CreatureScript
{
	public:
		emerald_flameweaver() : CreatureScript("emerald_flameweaver") { }
		
		struct emerald_flameweaverAI : public ScriptedAI
		{
			emerald_flameweaverAI(Creature* creature) : ScriptedAI(creature)
			{}
				void JustSummoned(Creature* summon)
				{
						std::list<Player *> players = me->GetPlayersInRange(100, true);
						for (std::list<Player *>::const_iterator it = players.begin(); it != players.end(); it++)
						(*it)->KilledMonsterCredit(40856, 0);
						me->DespawnOrUnsummon();
				}
				
				void UpdateAI()
				{

						std::list<Player *> players = me->GetPlayersInRange(100, true);
						for (std::list<Player *>::const_iterator it = players.begin(); it != players.end(); it++)
						(*it)->KilledMonsterCredit(40856, 0);
						me->DespawnOrUnsummon();
				}
			
		};
 
		 CreatureAI* GetAI(Creature* creature) const
		{
		return new emerald_flameweaverAI(creature);
		}
};
 
 
void AddSC_zone_hyjal()
{
	new emerald_flameweaver();
}
