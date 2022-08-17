#include "Prisma/prisma_helper.h"

using namespace PrismaHandler;

float PrismaStats::GetAttackCoefficient(Type attack, Type target_type)
{
    switch (attack)
    {
    default:
    case Type::NORMAL:
        return GetCoefficientTypeNormal(target_type);
        break;
    case Type::FIRE:
        return GetCoefficientTypeFire(target_type);
        break;
    case Type::WATER:
        return GetCoefficientTypeWater(target_type);
        break;
    case Type::LIGHTNING:
        return GetCoefficientTypeLightning(target_type);
        break;
    case Type::NATURE:
        return GetCoefficientTypeNature(target_type);
        break;
    case Type::ICE:
        return GetCoefficientTypeIce(target_type);
        break;
    case Type::FIGHTING:
        return GetCoefficientTypeFighting(target_type);
        break;
    case Type::POISON:
        return GetCoefficientTypePoison(target_type);
        break;
    case Type::EARTH:
        return GetCoefficientTypeEarth(target_type);
        break;
    case Type::AIR:
        return GetCoefficientTypeAir(target_type);
        break;
    case Type::PSIONIC:
        return GetCoefficientTypePsionic(target_type);
        break;
    case Type::PHANTOM:
        return GetCoefficientTypePhantom(target_type);
        break;
    case Type::DARK:
        return GetCoefficientTypeDark(target_type);
        break;
    case Type::METAL:
        return GetCoefficientTypeMetal(target_type);
        break;
    case Type::LIGHT:
        return GetCoefficientTypeLight(target_type);
        break;
    case Type::SOUND:
        return GetCoefficientTypeSound(target_type);
        break;
    }
}

float PrismaStats::GetAttackCoefficient(Type attack, Type target_type1, Type target_type2)
{
    return (GetAttackCoefficient(attack, target_type1) * GetAttackCoefficient(attack, target_type2));
}

float PrismaStats::GetCoefficientTypeNormal(Type against)
{
    switch (against)
    {
    case Type::PHANTOM:
        return 0.0f;
        break;
    case Type::METAL:
        return 0.5f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeFire(Type against)
{
    switch (against)
    {
    case Type::FIRE:
    case Type::WATER:
        return 0.5f;
        break;
    case Type::NATURE:
    case Type::ICE:
    case Type::METAL:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeWater(Type against)
{
    switch (against)
    {
    case Type::FIRE:
    case Type::EARTH:
        return 2.f;
        break;
    case Type::WATER:
    case Type::NATURE:
    case Type::SOUND:
        return 0.5f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeLightning(Type against)
{
    switch (against)
    {
    case Type::WATER:
    case Type::AIR:
    case Type::SOUND:
        return 2.f;
        break;
    case Type::LIGHTNING:
    case Type::NATURE:
        return 0.5f;
        break;
    case Type::EARTH:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeNature(Type against)
{
    switch (against)
    {
    case Type::FIRE:
    case Type::NATURE:
    case Type::POISON:
    case Type::AIR:
    case Type::METAL:
        return 0.5f;
        break;
    case Type::WATER:
    case Type::EARTH:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeIce(Type against)
{
    switch (against)
    {
    case Type::FIRE:
    case Type::WATER:
    case Type::ICE:
    case Type::METAL:
        return 0.5f;
        break;
    case Type::NATURE:
    case Type::EARTH:
    case Type::AIR:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeFighting(Type against)
{
    switch (against)
    {
    case Type::NORMAL:
    case Type::ICE:
    case Type::METAL:
        return 2.f;
        break;
    case Type::POISON:
    case Type::AIR:
    case Type::PSIONIC:
        return 0.5f;
        break;
    case Type::PHANTOM:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypePoison(Type against)
{
    switch (against)
    {
    case Type::NATURE:
    case Type::LIGHT:
        return 2.f;
        break;
    case Type::POISON:
    case Type::EARTH:
    case Type::PHANTOM:
    case Type::DARK:
        return 0.5f;
        break;
    case Type::METAL:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeEarth(Type against)
{
    switch (against)
    {
    case Type::FIRE:
    case Type::LIGHTNING:
    case Type::POISON:
    case Type::METAL:
        return 2.f;
        break;
    case Type::NATURE:
        return 0.5f;
        break;
    case Type::AIR:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeAir(Type against)
{
    switch (against)
    {
    case Type::LIGHTNING:
    case Type::METAL:
    case Type::SOUND:
        return 0.5f;
        break;
    case Type::NATURE:
    case Type::FIGHTING:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypePsionic(Type against)
{
    switch (against)
    {
    case Type::FIGHTING:
    case Type::POISON:
        return 2.f;
        break;
    case Type::PSIONIC:
    case Type::METAL:
        return 0.5f;
        break;
    case Type::DARK:
    case Type::LIGHT:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypePhantom(Type against)
{
    switch (against)
    {
    case Type::NORMAL:
        return 0.f;
        break;
    case Type::PSIONIC:
    case Type::PHANTOM:
        return 2.f;
        break;
    case Type::DARK:
    case Type::LIGHT:
        return 0.5f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeDark(Type against)
{
    switch (against)
    {
    case Type::FIGHTING:
    case Type::DARK:
    case Type::METAL:
        return 0.5f;
        break;
    case Type::PSIONIC:
    case Type::PHANTOM:
    case Type::LIGHT:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeMetal(Type against)
{
    switch (against)
    {
    case Type::FIRE:
    case Type::WATER:
    case Type::LIGHTNING:
    case Type::METAL:
        return 0.5f;
        break;
    case Type::ICE:
    case Type::DARK:
    case Type::LIGHT:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeLight(Type against)
{
    switch (against)
    {
    case Type::FIRE:
    case Type::POISON:
    case Type::METAL:
    case Type::LIGHT:
        return 0.5f;
        break;
    case Type::FIGHTING:
    case Type::DARK:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float PrismaStats::GetCoefficientTypeSound(Type against)
{
    switch (against)
    {
    case Type::WATER:
    case Type::AIR:
        return 2.f;
        break;
    case Type::LIGHTNING:
        return 0.5f;
        break;
    case Type::SOUND:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

int PrismaExperience::GetRequiredExperienceForNextLevel(int level, ExperienceType Type)
{
    int result = 0;

    switch (Type)
    {
    case ExperienceType::SLOW:
        result = (GetSlowExperience(level + 1) - GetSlowExperience(level));
        break;
    case ExperienceType::MEDIUM_SLOW:
        result = (GetMediumSlowExperience(level + 1) - GetMediumSlowExperience(level));
        break;
    default:
    case ExperienceType::MEDIUM_FAST:
        result = (GetMediumFastExperience(level + 1) - GetMediumFastExperience(level));
        break;
    case ExperienceType::FAST:
        result = (GetFastExperience(level + 1) - GetFastExperience(level));
        break;
    };

    if (result < 0)
        return 0;

    return result;
}

int PrismaExperience::GetGainExperience(int level, int target_level, int target_base_experience, bool against_trainer, bool use_multi_exp, int number_prisma_during_combat, float other_multiplicator)
{
    int   A = (target_level * 2) + 10;
    float B = (float(target_base_experience * target_level) / 5.f);
    int   C = (level + target_level + 10);

    against_trainer ? B *= 1.5f : NULL;
    use_multi_exp ? B /= 2.f : NULL;
    B /= ((number_prisma_during_combat <= 0) ? 1.f : float(number_prisma_during_combat));

    int gain = floor((floor(sqrt(double(A)) * (A * A) * B) / floor(sqrt(C) * (C * C)))) + 1;

    return floor(gain * other_multiplicator);
}

int PrismaExperience::GetSlowExperience(int level)
{
    return int((5.f * float(level * level * level)) / 4.f);
}

int PrismaExperience::GetMediumSlowExperience(int level)
{
    return int(((6.f / 5.f) * float(level * level * level)) - (15 * (level * level)) + (100 * level) - 140);
}

int PrismaExperience::GetMediumFastExperience(int level)
{
    return (level * level * level);
}

int PrismaExperience::GetFastExperience(int level)
{
    return int((4.f * float(level * level * level)) / 5.f);
}
