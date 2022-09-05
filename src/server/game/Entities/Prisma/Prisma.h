
#pragma once

#include "Map.h"
#include "Creature.h"
#include "PrismaData.h"
#include "TemporarySummon.h"
#include "ObjectMgr.h"

class TC_GAME_API Prisma : public Creature
{
public:
    explicit Prisma(bool isWorldObject = false);

    void InitializePrisma();
    bool InitializePrismaFromGuid(uint32 guid);

    void SetOwner(Player* player) { owner = player; };
    Player* GetOwner() { return owner; };

    uint32 GetPrismaEntry() { return m_id; };
    uint16 CalculateStat(PrismaStats _stat);
    static uint32 CalculateDamage(Prisma* attacker, Prisma* target, uint32 move_id, PrismaWeathers weather, bool is_second_strike);
    void ApplyDamage(uint32 damage);
    void ApplyExperience(uint32 exp);
    void LevelUp();
    bool IsLast();

    void RestoreStamina();
    void RestoreMove();
    void RestoreStatus();

    bool IsPrismaDead() { return (m_current_stamina == 0); };
    static bool IsPrismaDead(Prisma* prisma) { return (prisma->GetCurrentStamina() == 0); };
    static int8 MoveSpeedPriority(int32 move_id);

    void SavePrismaToDB();
    void SetPrismaLevel(uint32 level, bool update = false);

    void AddNonVolatileStatus(PrismaNonVolatileStatus _status);
    void RemoveNonVolatileStatus(PrismaNonVolatileStatus _status);
    bool HasNonVolatileStatus(PrismaNonVolatileStatus _status);
    uint32 GetNonVolatileStatusFlags() { return m_status_non_volatile_flags; };
    void SetNonVolatileStatusFlags(uint32 flags) { m_status_non_volatile_flags = flags; };

    void AddVolatileStatus(PrismaVolatileStatus _status);
    void RemoveVolatileStatus(PrismaVolatileStatus _status);
    bool HasVolatileStatus(PrismaVolatileStatus _status);
    uint32 GetVolatileStatusFlags() { return m_status_volatile_flags; };
    void SetVolatileStatusFlags(uint32 flags) { m_status_volatile_flags = flags; };

    void AddVolatileCombatStatus(PrismaVolatileCombatStatus _status);
    void RemoveVolatileCombatStatus(PrismaVolatileCombatStatus _status);
    bool HasVolatileCombatStatus(PrismaVolatileCombatStatus _status);
    uint32 GetVolatileCombatStatusFlags() { return m_status_volatile_combat_flags; };
    void SetVolatileCombatStatusFlags(uint32 flags) { m_status_volatile_combat_flags = flags; };

    static bool HasPrisma(Player* player);
    static uint32 GenerateGUID();
    static Prisma* Invoke(Player* owner, uint8 num = 0);
    static uint32 GetTeamID(Player* player, uint8 num = 0);
    static uint32 GetTeamGUID(Player* player, uint8 num = 0);

    bool IndividualValueIsGenerated() { return _iv_generated; };
    PrismaIndividualValue const GetIV() { return _iv; };
    uint8 const GetStatIV(PrismaStats stat) { return _iv.GetStat(stat); };

    std::string const GetCharacteristic() { return _characteristic.GetCharacteristicName(); };

    bool NatureIsGenerated() { return _nature_generated; };
    PrismaNatures const GetNature() { return _nature.GetNature(); };
    uint8 const GetNatureID() { return _nature.nature; };
    std::string const GetNatureName() { return _nature.GetName(); };

    bool GenderIsGenerated() { return _gender_generated; };
    PrismaGenders const GetGender() { return _gender.GetGender(); };
    std::string const GetGenderName() { return _gender.GetGenderName(); };

    void InitializeTurnInformation();
    void SetMoveFailed() { m_turn_failed = true; };
    bool MoveFailed() { return m_turn_failed; };
    void SetMoveCritical() { m_turn_critical = true; };
    bool MoveCritical() { return m_turn_critical; };
    void SetMoveSpeed(uint32 _speed) { m_turn_speed = _speed; };
    uint32 MoveSpeed() { return m_turn_speed; };

    static float GetMoveCoefficient(PrismaTypes attack, PrismaTypes target_type);
    static float GetMoveCoefficient(PrismaTypes attack, PrismaTypes target_type1, PrismaTypes target_type2);

    static uint32 GetRequiredExperienceForNextLevel(int level, PrismaExperienceTypes Type);
    static int GetGainExperience(Prisma* self, Prisma* killed, bool against_trainer, bool use_multi_exp, int number_prisma_during_combat, float other_multiplicator);

    uint32 GetPrismaGUID() { return m_guid; };
    uint32 GetPrismaExperience() { return m_experience; };
    int32 GetPrismaItem() { return m_item; };
    uint32 GetPrismaLevel() { return m_level; };
    uint32 GetCurrentStamina() { return m_current_stamina; };
    PrismaStatistique GetCalculatedStat() { return _calculatedStat; };
    PrismaStatistique GetCurrentStat() { return _currentStat; };
    PrismaMoveSet GetPrismaMoveSet() { return _move; };

    void UseMove(uint32 id);

    static std::vector<std::string> SplitData(const std::string& data, const std::string& delimiter, bool add);

private:
    Player* owner;
    uint32 m_guid;
    uint32 m_id;
    uint32 m_experience;
    int32 m_item;
    uint32 m_level;
    uint32 m_current_stamina;

    /* personnal turn information */
    uint32 m_turn_speed;
    bool m_turn_failed;
    bool m_turn_critical;

    uint32 m_status_non_volatile_flags;
    int32 m_turn_non_volatile;

    uint32 m_status_volatile_flags;
    int32 m_turn_volatile;

    uint32 m_status_volatile_combat_flags;
    int32 m_turn_volatile_combat;

    PrismaIndividualValue _iv;
    PrismaEffortValue _ev;
    PrismaNature _nature;
    PrismaGender _gender;
    PrismaCharacteristic _characteristic;
    PrismaMoveSet _move;
    PrismaStatistique _calculatedStat;
    PrismaStatistique _currentStat;

    void GenerateIndividualValue();
    void GenerateCharacteristic();
    void GenerateNature();
    void GenerateGender();
    void GenerateCalculatedStat();
    void GenerateMoveSet();

    bool _iv_generated;
    bool _nature_generated;
    bool _gender_generated;
    bool _is_loaded_from_guid;

    static float GetCoefficientTypeNormal(PrismaTypes against);
    static float GetCoefficientTypeFire(PrismaTypes against);
    static float GetCoefficientTypeWater(PrismaTypes against);
    static float GetCoefficientTypeGrass(PrismaTypes against);
    static float GetCoefficientTypeElectric(PrismaTypes against);
    static float GetCoefficientTypeIce(PrismaTypes against);
    static float GetCoefficientTypeFighting(PrismaTypes against);
    static float GetCoefficientTypePoison(PrismaTypes against);
    static float GetCoefficientTypeGround(PrismaTypes against);
    static float GetCoefficientTypeFlying(PrismaTypes against);
    static float GetCoefficientTypePsychic(PrismaTypes against);
    static float GetCoefficientTypeBug(PrismaTypes against);
    static float GetCoefficientTypeRock(PrismaTypes against);
    static float GetCoefficientTypeGhost(PrismaTypes against);
    static float GetCoefficientTypeDark(PrismaTypes against);
    static float GetCoefficientTypeDragon(PrismaTypes against);
    static float GetCoefficientTypeSteel(PrismaTypes against);
    static float GetCoefficientTypeFairy(PrismaTypes against);


    static int GetSlowExperience(int level);
    static int GetMediumSlowExperience(int level);
    static int GetMediumFastExperience(int level);
    static int GetFastExperience(int level);
};

struct PrismaUnitTurnData
{
    Prisma* self;
    bool is_friend;
    bool played;
    uint32 move_id;
    uint32 speed;
    int32 speed_priority;

    uint32 selection;
    std::vector<Prisma*> friends;
    std::vector<Prisma*> enemies;

    void Initialize(Prisma* _self, bool _is_friend, uint32 _move_id, uint32 _selection,
        std::vector<Prisma*> _friends, std::vector<Prisma*> _enemies)
    {
        self = _self;
        is_friend = _is_friend;
        played = false;

        move_id = _move_id;
        speed = self->GetCalculatedStat().speed;
        speed_priority = Prisma::MoveSpeedPriority(move_id);

        selection = _selection;
        friends = _friends;
        enemies = _enemies;
    }

    void SetSelection(uint32 _selection, std::vector<Prisma*> _friends, std::vector<Prisma*> _enemies)
    {
        selection = _selection;
        friends = _friends;
        enemies = _enemies;
    }

    uint32 FinalSpeed()
    {
        return (speed + (speed_priority * 1000));
    }
};

struct PrismaTurnData
{
    std::vector<PrismaUnitTurnData> data;

    bool CanPlayTurn()
    {
        for (auto& turn_data : data)
            if (!turn_data.played)
                return false;

        return true;
    }

    void Reset() { data.clear(); }

    static bool SortBySpeed(const PrismaUnitTurnData& a, const PrismaUnitTurnData& b)
    {
        uint32 calculated_a = a.speed + (a.speed_priority * 1000);
        uint32 calculated_b = b.speed + (b.speed_priority * 1000);

        if (a.self->HasNonVolatileStatus(PrismaNonVolatileStatus::PARALYSIS))
            calculated_a /= 2;

        if (b.self->HasNonVolatileStatus(PrismaNonVolatileStatus::PARALYSIS))
            calculated_b /= 2;

        return calculated_a >= calculated_b;
    }
};

struct PrismaTurnInformation
{
    PrismaTurnInformations next;
    std::vector<Prisma*> targets;
};

struct PrismaMessageData
{
    std::string data;

    PrismaMessageData& operator <<(const std::string& arg)
    {
        data += arg;
        return *this;
    }

    PrismaMessageData& operator<<(int arg)
    {
        data += std::to_string(arg);
        return *this;
    }

    PrismaMessageData& operator<<(unsigned int arg)
    {
        data += std::to_string(arg);
        return *this;
    }
};
