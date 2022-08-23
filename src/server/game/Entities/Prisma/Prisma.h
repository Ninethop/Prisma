
#pragma once

#include "Map.h"
#include "Creature.h"
#include "PrismaData.h"
#include "ObjectMgr.h"

struct PrismaUnitTurnData
{
    bool played;
    uint32 speed;
    bool speed_priority;
    uint32 damage;
    Prisma* damage_target;
    uint32 heal;
    Prisma* heal_target;

    void Initialize()
    {
        played = false;
        speed = 0;
        speed_priority = false;
        damage = 0;
        damage_target = nullptr;
        heal = 0;
        heal_target = nullptr;
    }

    void SetDamage(Prisma* _target, uint32 _damage)
    {
        damage_target = _target;
        damage = _damage;
    }

    void SetHeal(Prisma* _target, uint32 _heal)
    {
        heal_target = _target;
        heal = _heal;
    }

    void SetSpeed(uint32 val, bool priority)
    {
        speed = val;
        speed_priority = priority;
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
        uint32 calculated_a = a.speed + (a.speed_priority ? 1000 : 0);
        uint32 calculated_b = b.speed + (b.speed_priority ? 1000 : 0);

        return calculated_a >= calculated_b;
    }
};

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

    bool IsPrismaDead() { return (m_current_stamina == 0); };
    static bool IsPrismaDead(Prisma* prisma) { return (prisma->GetCurrentStamina() == 0); };
    static bool MoveUseSpeedPriority(int32 move_id);

    void SavePrismaToDB();
    void SetPrismaLevel(uint32 level, bool update = false);

    void AddNonVolatileStatus(PrismaNonVolatileStatus _status);
    void RemoveNonVolatileStatus(PrismaNonVolatileStatus _status);
    bool HasNonVolatileStatus(PrismaNonVolatileStatus _status);

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

    static float GetMoveCoefficient(PrismaTypes attack, PrismaTypes target_type);
    static float GetMoveCoefficient(PrismaTypes attack, PrismaTypes target_type1, PrismaTypes target_type2);

    static int GetRequiredExperienceForNextLevel(int level, PrismaExperienceTypes Type);
    static int GetGainExperience(Prisma* self, Prisma* killed, bool against_trainer, bool use_multi_exp, int number_prisma_during_combat, float other_multiplicator);

    uint32 GetPrismaGUID() { return m_guid; };
    uint32 GetPrismaExperience() { return m_experience; };
    int32 GetPrismaItem() { return m_item; };
    uint32 GetPrismaLevel() { return m_level; };
    uint32 GetCurrentStamina() { return m_current_stamina; };
    PrismaStatistique GetCalculatedStat() { return _calculatedStat; };
    PrismaStatistique GetCurrentStat() { return _currentStat; };
    PrismaMoveSet* GetPrismaMoveSet() { return &_move; };

    static std::vector<std::string> SplitData(const std::string& data, const std::string& delimiter, bool add);

private:
    Player* owner;
    uint32 m_guid;
    uint32 m_id;
    uint32 m_experience;
    int32 m_item;
    uint32 m_level;
    uint32 m_current_stamina;

    uint32 m_status_non_volatile_flags;
    uint32 m_status_volatile_flags;
    uint32 m_status_volatile_combat_flags;

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

    bool _iv_generated;
    bool _nature_generated;
    bool _gender_generated;
    bool _is_loaded_from_guid;

    static float GetCoefficientTypeNormal(PrismaTypes against);
    static float GetCoefficientTypeFire(PrismaTypes against);
    static float GetCoefficientTypeWater(PrismaTypes against);
    static float GetCoefficientTypeLightning(PrismaTypes against);
    static float GetCoefficientTypeNature(PrismaTypes against);
    static float GetCoefficientTypeIce(PrismaTypes against);
    static float GetCoefficientTypeFighting(PrismaTypes against);
    static float GetCoefficientTypePoison(PrismaTypes against);
    static float GetCoefficientTypeEarth(PrismaTypes against);
    static float GetCoefficientTypeAir(PrismaTypes against);
    static float GetCoefficientTypePsionic(PrismaTypes against);
    static float GetCoefficientTypePhantom(PrismaTypes against);
    static float GetCoefficientTypeDark(PrismaTypes against);
    static float GetCoefficientTypeMetal(PrismaTypes against);
    static float GetCoefficientTypeLight(PrismaTypes against);
    static float GetCoefficientTypeSound(PrismaTypes against);

    static int GetSlowExperience(int level);
    static int GetMediumSlowExperience(int level);
    static int GetMediumFastExperience(int level);
    static int GetFastExperience(int level);
};
