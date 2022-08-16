/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

#include "zulgurub.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"

enum Says
{
    SAY_AGGRO                   = 0,
    SAY_FEAST_PROWLER           = 1,
    SAY_DEATH                   = 2
};

enum Spells
{
    SPELL_SHADOW_WORD_PAIN      = 24212, // Corrected
    SPELL_GOUGE                 = 12540, // Corrected
    SPELL_MARK_OF_ARLOKK        = 24210, // triggered spell 24211 Added to spell_dbc
    SPELL_RAVAGE                = 24213, // Corrected
    SPELL_CLEAVE                = 25174, // Searching for right spell
    SPELL_PANTHER_TRANSFORM     = 24190, // Transform to panther now used
    SPELL_SUMMON_PROWLER        = 24246, // Added to Spell_dbc
    SPELL_VANISH_VISUAL         = 24222, // Added
    SPELL_VANISH                = 24223, // Added
    SPELL_SUPER_INVIS           = 24235  // Added to Spell_dbc
};

enum Events
{
    EVENT_SHADOW_WORD_PAIN      = 1,
    EVENT_GOUGE                 = 2,
    EVENT_MARK_OF_ARLOKK        = 3,
    EVENT_RAVAGE                = 4,
    EVENT_TRANSFORM             = 5,
    EVENT_VANISH                = 6,
    EVENT_VANISH_2              = 7,
    EVENT_TRANSFORM_BACK        = 8,
    EVENT_VISIBLE               = 9,
    EVENT_SUMMON_PROWLERS       = 10
};

enum Phases
{
    PHASE_ALL                   = 0,
    PHASE_ONE                   = 1,
    PHASE_TWO                   = 2
};

enum Weapon
{
    WEAPON_DAGGER               = 10616
};

enum Misc
{
    MAX_PROWLERS_PER_SIDE       = 15
};

Position const PosMoveOnSpawn[1] =
{
    { -11561.9f, -1627.868f, 41.29941f, 0.0f }
};

// AWFUL HACK WARNING
// To whoever reads this: Zul'Gurub needs your love
// Need to do this calculation to increase/decrease Arlokk's damage by 35% (probably some aura missing)
// This is only to compile the scripts after the aura calculation revamp
float const DamageIncrease = 35.0f;
float const DamageDecrease = 100.f / (1.f + DamageIncrease / 100.f) - 100.f;

struct boss_arlokk : public BossAI
{
    boss_arlokk(Creature* creature) : BossAI(creature, DATA_ARLOKK)
    {
        Initialize();
    }

    void Initialize()
    {
        _summonCountA = 0;
        _summonCountB = 0;
    }

    void Reset() override
    {
        if (events.IsInPhase(PHASE_TWO))
            me->ApplyStatPctModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, DamageDecrease); // hack
        _Reset();
        Initialize();
        me->SetVirtualItem(0, uint32(WEAPON_DAGGER));
        me->SetVirtualItem(1, uint32(WEAPON_DAGGER));
        me->SetWalk(false);
        me->GetMotionMaster()->MovePoint(0, PosMoveOnSpawn[0]);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, 7s, 9s, 0, PHASE_ONE);
        events.ScheduleEvent(EVENT_GOUGE, 12s, 15s, 0, PHASE_ONE);
        events.ScheduleEvent(EVENT_SUMMON_PROWLERS, 6s, 0, PHASE_ALL);
        events.ScheduleEvent(EVENT_MARK_OF_ARLOKK, 9s, 11s, 0, PHASE_ALL);
        events.ScheduleEvent(EVENT_TRANSFORM, 15s, 20s, 0, PHASE_ONE);
        Talk(SAY_AGGRO);

        // Sets up list of Panther spawners to cast on
        std::list<Creature*> triggerList;
        GetCreatureListWithEntryInGrid(triggerList, me, NPC_PANTHER_TRIGGER, 100.0f);
        if (!triggerList.empty())
        {
            uint8 sideA = 0;
            uint8 sideB = 0;
            for (std::list<Creature*>::const_iterator itr = triggerList.begin(); itr != triggerList.end(); ++itr)
            {
                if (Creature* trigger = *itr)
                {
                    if (trigger->GetPositionY() < -1625.0f)
                    {
                        _triggersSideAGUID[sideA] = trigger->GetGUID();
                        ++sideA;
                    }
                    else
                    {
                        _triggersSideBGUID[sideB] = trigger->GetGUID();
                        ++sideB;
                    }
                }
            }
        }
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        BossAI::EnterEvadeMode(why);
        if (GameObject* object = instance->GetGameObject(DATA_GONG_BETHEKK))
            object->RemoveFlag(GO_FLAG_NOT_SELECTABLE);
        me->DespawnOrUnsummon(4s);
    }

    void SetData(uint32 id, uint32 /*value*/) override
    {
        if (id == 1)
            --_summonCountA;
        else if (id == 2)
            --_summonCountB;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SHADOW_WORD_PAIN:
                    DoCastVictim(SPELL_SHADOW_WORD_PAIN, true);
                    events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, 5s, 7s, 0, PHASE_ONE);
                    break;
                case EVENT_GOUGE:
                    DoCastVictim(SPELL_GOUGE, true);
                    break;
                case EVENT_SUMMON_PROWLERS:
                    if (_summonCountA < MAX_PROWLERS_PER_SIDE)
                    {
                        if (Unit* trigger = ObjectAccessor::GetUnit(*me, _triggersSideAGUID[urand(0, 4)]))
                        {
                            trigger->CastSpell(trigger, SPELL_SUMMON_PROWLER);
                            ++_summonCountA;
                        }
                    }
                    if (_summonCountB < MAX_PROWLERS_PER_SIDE)
                    {
                        if (Unit* trigger = ObjectAccessor::GetUnit(*me, _triggersSideBGUID[urand(0, 4)]))
                        {
                            trigger->CastSpell(trigger, SPELL_SUMMON_PROWLER);
                            ++_summonCountB;
                        }
                    }
                    events.ScheduleEvent(EVENT_SUMMON_PROWLERS, 6s, 0, PHASE_ALL);
                    break;
                case EVENT_MARK_OF_ARLOKK:
                {
                    Unit* target = SelectTarget(SelectTargetMethod::MaxThreat, urand(1, 3), 0.0f, false, true, -SPELL_MARK_OF_ARLOKK);
                    if (!target)
                        target = me->GetVictim();
                    if (target)
                    {
                        DoCast(target, SPELL_MARK_OF_ARLOKK, true);
                        Talk(SAY_FEAST_PROWLER, target);
                    }
                    events.ScheduleEvent(EVENT_MARK_OF_ARLOKK, 120s, 130s);
                    break;
                }
                case EVENT_TRANSFORM:
                {
                    DoCast(me, SPELL_PANTHER_TRANSFORM); // SPELL_AURA_TRANSFORM
                    me->SetVirtualItem(0, uint32(EQUIP_UNEQUIP));
                    me->SetVirtualItem(1, uint32(EQUIP_UNEQUIP));
                    /*
                    CreatureTemplate const* cinfo = me->GetCreatureTemplate();
                    me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg +((cinfo->mindmg/100) * 35)));
                    me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg +((cinfo->maxdmg/100) * 35)));
                    me->UpdateDamagePhysical(BASE_ATTACK);
                    */
                    me->AttackStop();
                    ResetThreatList();
                    me->SetReactState(REACT_PASSIVE);
                    me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_UNINTERACTIBLE);
                    DoCast(me, SPELL_VANISH_VISUAL);
                    DoCast(me, SPELL_VANISH);
                    events.ScheduleEvent(EVENT_VANISH, 1s, 0, PHASE_ONE);
                    break;
                }
                case EVENT_VANISH:
                    DoCast(me, SPELL_SUPER_INVIS);
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(0, frand(-11551.0f, -11508.0f), frand(-1638.0f, -1617.0f), me->GetPositionZ());
                    events.ScheduleEvent(EVENT_VANISH_2, 9s, 0, PHASE_ONE);
                    break;
                case EVENT_VANISH_2:
                    DoCast(me, SPELL_VANISH);
                    DoCast(me, SPELL_SUPER_INVIS);
                    events.ScheduleEvent(EVENT_VISIBLE, 7s, 10s, 0, PHASE_ONE);
                    break;
                case EVENT_VISIBLE:
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_UNINTERACTIBLE);
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                        AttackStart(target);
                    me->RemoveAura(SPELL_SUPER_INVIS);
                    me->RemoveAura(SPELL_VANISH);
                    events.ScheduleEvent(EVENT_RAVAGE, 10s, 14s, 0, PHASE_TWO);
                    events.ScheduleEvent(EVENT_TRANSFORM_BACK, 15s, 18s, 0, PHASE_TWO);
                    events.SetPhase(PHASE_TWO);
                    me->ApplyStatPctModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, DamageIncrease); // hack
                    break;
                case EVENT_RAVAGE:
                    DoCastVictim(SPELL_RAVAGE, true);
                    events.ScheduleEvent(EVENT_RAVAGE, 10s, 14s, 0, PHASE_TWO);
                    break;
                case EVENT_TRANSFORM_BACK:
                {
                    me->RemoveAura(SPELL_PANTHER_TRANSFORM); // SPELL_AURA_TRANSFORM
                    DoCast(me, SPELL_VANISH_VISUAL);
                    me->SetVirtualItem(0, uint32(WEAPON_DAGGER));
                    me->SetVirtualItem(1, uint32(WEAPON_DAGGER));
                    /*
                    CreatureTemplate const* cinfo = me->GetCreatureTemplate();
                    me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg));
                    me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg));
                    me->UpdateDamagePhysical(BASE_ATTACK);
                    */
                    me->ApplyStatPctModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, DamageDecrease); // hack
                    events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, 4s, 7s, 0, PHASE_ONE);
                    events.ScheduleEvent(EVENT_GOUGE, 12s, 15s, 0, PHASE_ONE);
                    events.ScheduleEvent(EVENT_TRANSFORM, 16s, 20s, 0, PHASE_ONE);
                    events.SetPhase(PHASE_ONE);
                    break;
                }
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        DoMeleeAttackIfReady();
    }

private:
    uint8 _summonCountA;
    uint8 _summonCountB;
    ObjectGuid _triggersSideAGUID[5];
    ObjectGuid _triggersSideBGUID[5];
};

/*######
## npc_zulian_prowler
######*/

enum ZulianProwlerSpells
{
    SPELL_SNEAK_RANK_1_1         = 22766,
    SPELL_SNEAK_RANK_1_2         = 7939,  // Added to Spell_dbc
    SPELL_MARK_OF_ARLOKK_TRIGGER = 24211  // Added to Spell_dbc
};

enum ZulianProwlerEvents
{
    EVENT_ATTACK                 = 1
};

Position const PosProwlerCenter[1] =
{
    { -11556.7f, -1631.344f, 41.2994f, 0.0f }
};

struct npc_zulian_prowler : public ScriptedAI
{
    npc_zulian_prowler(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript())
    {
        _sideData = 0;
    }

    void Reset() override
    {
        if (me->GetPositionY() < -1625.0f)
            _sideData = 1;
        else
            _sideData = 2;

        DoCast(me, SPELL_SNEAK_RANK_1_1);
        DoCast(me, SPELL_SNEAK_RANK_1_2);

        if (Creature* arlokk = _instance->GetCreature(DATA_ARLOKK))
            if (arlokk->IsAlive())
                me->GetMotionMaster()->MovePoint(0, arlokk->GetPosition());

        _events.ScheduleEvent(EVENT_ATTACK, 6s);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        me->GetMotionMaster()->Clear();
        me->RemoveAura(SPELL_SNEAK_RANK_1_1);
        me->RemoveAura(SPELL_SNEAK_RANK_1_2);
    }

    void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
    {
        Unit* unitCaster = caster->ToUnit();
        if (!unitCaster)
            return;

        if (spellInfo->Id == SPELL_MARK_OF_ARLOKK_TRIGGER) // Should only hit if line of sight
            me->Attack(unitCaster, true);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Creature* arlokk = _instance->GetCreature(DATA_ARLOKK))
        {
            if (arlokk->IsAlive())
                arlokk->GetAI()->SetData(_sideData, 0);
        }
        me->DespawnOrUnsummon(4s);
    }

    void UpdateAI(uint32 diff) override
    {
        if (UpdateVictim())
        {
            DoMeleeAttackIfReady();
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ATTACK:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0.0f, 100, false))
                        me->Attack(target, true);
                    break;
                default:
                    break;
            }
        }
    }

private:
    int32 _sideData;
    EventMap _events;
    InstanceScript* _instance;
};

/*######
## go_gong_of_bethekk
######*/

Position const PosSummonArlokk[1] =
{
    { -11507.22f, -1628.062f, 41.38264f, 3.159046f }
};

struct go_gong_of_bethekk : public GameObjectAI
{
    go_gong_of_bethekk(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* /*player*/) override
    {
        me->SetFlag(GO_FLAG_NOT_SELECTABLE);
        me->SendCustomAnim(0);
        me->SummonCreature(NPC_ARLOKK, PosSummonArlokk[0], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10min);
        return true;
    }
};

void AddSC_boss_arlokk()
{
    RegisterZulGurubCreatureAI(boss_arlokk);
    RegisterZulGurubCreatureAI(npc_zulian_prowler);
    RegisterZulGurubGameObjectAI(go_gong_of_bethekk);
}
